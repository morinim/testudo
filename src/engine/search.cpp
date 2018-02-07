/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <algorithm>
#include <memory>

#include "search.h"
#include "cache.h"
#include "eval.h"
#include "util.h"

namespace testudo
{

// Extraxt from the list of past known states (`ss`) a subset of hash values
// used for repetition detection.
search::search_path_info::search_path_info(const std::vector<state> &ss)
{
  assert(!states.empty());

  const state current(ss.back());

  const std::size_t start(ss.size() >= current.fifty()
                          ? ss.size() - current.fifty()
                          : 0);

  for (auto i(start); i < ss.size(); ++i)
    states.push_back(ss[i].hash());
}

// Returns `true` if the current position (`states_.back()`) has been
// repeated (compares the current hash value to already seen values).
bool search::search_path_info::repetitions() const
{
  assert(!states.empty());

  const std::size_t current(states.size() - 1);

  std::size_t i(current & 1);
  while (states[i] != states[current])  // current hash is used as a sentinel
    i += 2;

  return i != current;
}

void search::search_path_info::push(const state &current)
{
  states.push_back(current.hash());
}

void search::search_path_info::pop()
{
  states.pop_back();
}

std::vector<move> search::sorted_captures(const state &s)
{
  const auto move_score(
    [&](const move &m)
    {
      return 20 * s[m.to].value() - s[m.from].value() + 1000000;
    });

  auto captures(s.captures());
  std::sort(captures.begin(), captures.end(),
            [&](const auto &m1, const auto &m2)
            {
              return move_score(m1) > move_score(m2);
            });

  return captures;
}

// A recursive minimax search function with alpha-beta cutoffs (negamax). It
// searches capture sequences and allows the evaluation function to cut the
// search off (and set alpha). The idea is to find a position where there
// isn't a lot going on so the static evaluation function will work.
score search::quiesce(const state &s, score alpha, score beta)
{
  assert(alpha < beta);

  ++stats.qnodes;

  score x(eval(s));

  if (x >= beta)
    return beta;
  if (x > alpha)
    alpha = x;

  for (const auto &m : sorted_captures(s))
  {
    x = -quiesce(s.after_move(m), -beta, -alpha);

    if (x > alpha)
    {
      if (x >= beta)
        return beta;
      alpha = x;
    }
  }

  return alpha;
}

std::vector<move> search::sorted_moves(const state &s)
{
  const auto entry(tt_->find(s.hash()));
  const move best_move(entry ? entry->best_move : move::sentry());

  const auto move_score(
    [&](const move &m)
    {
      score ms(0);

      // The best move is from the hash table. Sometimes we don't get a best
      // move, like if everything failed low (returned a score <= alpha), but
      // other times there is a definite best move, like when something fails
      // high (returns a score >= beta).
      // If a best move is found, it will be searched first.
      if (m == best_move)
        ms = 2000000;
      else if (m.flags & move::capture)
        ms = 20 * s[m.to].value() - s[m.from].value() + 1000000;

      return ms;
    });

  auto moves(s.moves());
  std::sort(moves.begin(), moves.end(),
            [&](const auto &m1, const auto &m2)
            {
              return move_score(m1) > move_score(m2);
            });

  return moves;
}

int search::delta_draft(bool in_check, unsigned n_moves, const move &m) const
{
  int delta(-PLY);

  if (in_check)
    delta += 15*PLY/16;

  if (n_moves == 1)
    delta += 15*PLY/16;

  if (m.flags & move::capture)
    delta += PLY / 2;

  return std::min(0, delta);
}

// A slightly modified version of alphabeta (not stricly necessary but helps
// to avoid a lot of `if`):
// - the order of the moves is improved when a best move is found. This is
//   possible since the root moves are "permanent" (they're kept inside the
//   `stat` structure and are available even when the search is finished). THIS
//   IS AN IMPORTANT DIFFERENCE;
// - the function assumes that the position isn't a stalemate / immediate mate;
// - the function ignores draw by repetition / 50 moves rule (we want a move).
score search::alphabeta_root(const state &s, score alpha, score beta, int draft)
{
  assert(alpha < beta);

  ++stats.snodes;

  search_path_info_.push(s);
  auto guard = finally([&]{ search_path_info_.pop(); });

  auto &moves(stats.moves_at_root);
  if (moves.empty())
    moves = sorted_moves(root_state_);
  assert(!moves.empty());

  const bool in_check(s.in_check());

  auto best_move(move::sentry());
  for (std::size_t i(0); i < moves.size(); ++i)
  {
    const auto new_draft(draft + delta_draft(in_check, moves.size(), moves[i]));

    const auto x(draft > PLY ? -alphabeta(s.after_move(moves[i]),
                                          -beta, -alpha, 1, new_draft)
                             : -quiesce(s.after_move(moves[i]),
                                        -beta, -alpha));

    if (x > alpha)
    {
      best_move = moves[i];

      // Moves at the root node are very important and they're kept in the best
      // available order (given the search history).
      std::copy_backward(&moves[0], &moves[i], &moves[i+1]);
      moves[0] = best_move;

      if (x >= beta)
      {
        if (!search_stopped_)
          tt_->insert(s.hash(), best_move, draft, score_type::fail_high, beta);

        return beta;
      }

      alpha = x;
    }
  }

  if (!search_stopped_)
    tt_->insert(s.hash(), best_move, draft,
                !best_move ? score_type::fail_low : score_type::exact, alpha);

  return alpha;
}

// Recursively implements negamax alphabeta until draft is exhausted, at which
// time it calls quiesce().
// The `ply` index measures the distance of the current node from the root
// node, while `draft` is the remaining depth to the horizon.
// There are various reasons to decouple the depth to horizon from the
// `ply`-index or depth from root. While the `ply`-index is incremented by one
// each time, the draft may be independently altered by various extension or
// reduction-schemes and may also consider fractional extensions (values less
// then `PLY`).
score search::alphabeta(const state &s, score alpha, score beta,
                        unsigned ply, int draft)
{
  assert(alpha < beta);

  // Checks to see if we have searched enough nodes that it's time to peek at
  // how much time has been used / check for operator keyboard input.
  if (search_stopped_ || ++stats.snodes % 1000 == 0)
  {
    search_stopped_ = search_time_.elapsed(max_time);

    if (search_stopped_)
      return 0;
  }

  search_path_info_.push(s);
  auto guard = finally([&]{ search_path_info_.pop(); });

  // Check to see if this position has been searched before. If so, we may get
  // a real score, produce a cutoff or get nothing more than a good move to try
  // first.
  // There are four cases to handle:
  // - `score_type::exact`. Return the score, no further searching is needed
  //   from this position.
  // - `score_type::fail_low` which means that when this position was searched
  //   previously, every move was "refuted" by one of its descendents. As a
  //   result, when the search was completed, we returned `alpha` at that
  //   point. If the previous was lower than alpha we simply return `alpha`
  //   here as well.
  // - `score_type::fail_high` which means that when we encountered this
  //   position before, we searched one branch (probably) which promptly
  //   refuted the move at the previous ply.
  const auto entry(tt_->find(s.hash()));
  if (entry && entry->draft >= draft)
    switch (entry->type)
    {
    case score_type::exact:
      return entry->value;
    case score_type::fail_low:
      if (entry->value <= alpha)
        return alpha;
      break;
    case score_type::fail_high:
      if (entry->value >= beta)
        return beta;
      break;
    default:  // score_type::ignore
      ;
    }

  const auto moves(sorted_moves(s));

  const bool in_check(s.in_check());

  if (moves.empty())
    return in_check ? -MATE + ply : 0;

  if (search_path_info_.repetitions() || s.fifty() >= 100)
    return 0;

  auto best_move(move::sentry());
  for (const auto &m : moves)
  {
    const auto new_draft(draft + delta_draft(in_check, moves.size(), m));

    const auto x(draft > PLY ? -alphabeta(s.after_move(m), -beta, -alpha,
                                          ply + 1, new_draft)
                             : -quiesce(s.after_move(m), -beta, -alpha));

    if (x > alpha)
    {
      best_move = m;

      if (x >= beta)
      {
        if (!search_stopped_)
          tt_->insert(s.hash(), best_move, draft, score_type::fail_high, beta);

        return beta;
      }

      alpha = x;
    }
  }

  if (!search_stopped_)
    tt_->insert(s.hash(), best_move, draft,
                !best_move ? score_type::fail_low : score_type::exact, alpha);

  return alpha;
}

// Extract the PV from the transposition table.
// At least one should always be returned (even in case of immediate draw).
std::vector<move> search::extract_pv() const
{
  std::vector<move> pv;

  auto s(root_state_);
  for (auto entry(tt_->find(s.hash()));
       entry && !entry->best_move.is_sentry()
       && (pv.size() <= 2 * stats.depth || pv.empty())
       && s.make_move(entry->best_move);)
  {
    pv.push_back(entry->best_move);
    entry = tt_->find(s.hash());
  }

  return pv;
}

// Aspiration windows are a way to reduce the search space in an alpha-beta
// search.
// The technique is to use a guess of the expected value (usually from the last
// iteration in iterative deepening) and use a window around this as the
// alpha-beta bounds. Because the window is narrower, more beta cutoffs are
// achieved and the search takes a shorter time.
// The drawback is that if the true score is outside this window, then a costly
// re-search must be made.
score search::aspiration_search(score *alpha, score *beta, int draft)
{
  auto x(alphabeta_root(root_state_, *alpha, *beta, draft));

  if (search_stopped_)
    return 0;

  if (x <= *alpha || x >= *beta)
    x = alphabeta_root(root_state_, -INF, +INF, draft);

  if (search_stopped_)
    return 0;

  *alpha = x - 50;
  *beta  = x + 50;

  return x;
}

// Calls `aspiration_search` with increasing depth until allocated resources
// are exhausted.
// In case of an unfinished search (`search_stopped`), the program always has
// the option to fall back to the move selected in the last iteration of the
// search.
// This is called "iterative deepening". Iterative deepening, using a
// transposition table, embed the depth-first algorithms like alpha-beta into a
// framework with best-first characteristics.
move search::run(bool verbose)
{
  switch (root_state_.mate_or_draw(&search_path_info_.states))
  {
  case state::kind::mated:
  case state::kind::draw_stalemate:
    return move::sentry();

  default:
    search_time_.restart();
    tt_->inc_age();
    stats.reset();
  }

  move best_move(move::sentry());

  score alpha(-INF), beta(+INF);
  stats.depth = 1;
  for (unsigned max(max_depth ? max_depth : 1000);
       stats.depth <= max;
       ++stats.depth)
  {
    const auto x(aspiration_search(&alpha, &beta, stats.depth * PLY));

    if (search_stopped_)
      break;

    best_move = stats.moves_at_root.front();
    assert(extract_pv().front() == best_move);
    const auto pv(extract_pv());

    if (verbose)
    {
      std::cout << stats.depth << ' ' << x << ' '
                << search_time_.elapsed().count() / 10 << ' ' << stats.snodes;

      for (const auto &m : pv)
        std::cout << ' ' << m;

      std::cout << std::endl;
    }

    if (is_mate(x)
        || (root_state_.moves().size() == 1 && stats.depth == 5))
      break;
  }

  return best_move;
}

}  // namespace testudo
