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
#include <cassert>

#include "search.h"

namespace testudo
{

const cache::info *cache::find(hash_t h) const noexcept
{
  const auto &p(tt_[get_index(h)]);

  if (p.first.hash == h)
    return &p.first;
  else if (p.second.hash == h)
    return &p.second;

  return nullptr;
}

// When you get a result from a search, and you want to store an element in the
// table, it's important to record how deep the search went. If you searched
// this position with a 3-ply search, and you come along later planning to do a
// 10-ply search, you can't assume that the information in this hash element is
// accurate. So the search draft for the sub-tree is also recorded.
// In an alpha-beta search, rarely do you get an exact value when you search a
// node.  "Alpha" and "beta" exist to help you prune out useless sub-trees, but
// the minor disadvantage to using alpha-beta is that you don't often know
// exactly how bad or good a node is, you just know that it is bad enough or
// good enough that you don't need to waste any more time on it.
//
// Of course, this raises the question as to what value you store in the hash
// element, and what you can do with it when you retrieve it. The answer is to
// store a value, and a flag that indicates what the value means.
// If you store, let's say, a 16 in the `value` field, and `exact` in the
// `type` field, this means that the value of the node was exactly 16. If you
// store `fail_low` in the `type` field, the value of the node was at most 16.  // If you store `fail_high`, the value is at least 16.
void cache::insert(hash_t h, const move &m, int draft, node_type t,
                   score v) noexcept
{
  // Adjust mate scores. Mate scores are weird because they change depending
  // upon where in the tree they are found.  When stored in the hash table,
  // additional weirdness can result. This problem can be solved by converting
  // any mate scores to bounds, then storing the bounds.
  // A few weird cases were mates that are failing low, and -MATES that are
  // failing high. These are stored without any bound information, so it's not
  // possible to cut off on these later.
  // The idea is due to Bruce Moreland.
  if (v >= MATE - 500)
  {
    if (t == node_type::fail_low)  // failing low on MATE
      t = node_type::ignore;       // don't allow a cutoff later
    else
    {
      t = node_type::fail_high;    // pv or fail high, turned into a fail high
      v = MATE - 500;
    }
  }
  else if (v <= -MATE + 500)
  {
    if (t == node_type::fail_high)  // fail high on -MATE
      t = node_type::ignore;        // dont't allow cutoff later
    else
    {
      t = node_type::fail_low;      // pv or fail low, turned into a fail low
      v = -MATE + 500;
    }
  }

  info i(h, m, draft, t, v, age_);

  auto &entry(tt_[get_index(h)]);
  if (entry.first.draft <= i.draft || entry.first.age != age_)
    entry.first = i;
  else
    entry.second = i;
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

int search::delta_draft(bool in_check, unsigned n_moves, const move &m)
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

  ++stats.snodes;

  // Checks to see if we have searched enough nodes that it's time to peek at
  // how much time has been used / check for operator keyboard input.
  if (stats.snodes % 1000 == 0 || search_stopped_)
  {
    search_stopped_ = search_time_.elapsed(max_time);

    if (search_stopped_)
      return 0;
  }

  // Check to see if this position has been searched before. If so, we may get
  // a real score, produce a cutoff or get nothing more than a good move to try
  // first.
  // There are four cases to handle:
  // - `node_type::exact`. Return the score, no further searching is needed
  //   from this position.
  // - `node_type::fail_low` which means that when this position was searched
  //   previously, every move was "refuted" by one of its descendents. As a
  //   result, when the search was completed, we returned `alpha` at that
  //   point. If the previous was lower than alpha we simply return `alpha`
  //   here as well.
  // - `node_type::fail_high` which means that when we encountered this
  //   position before, we searched one branch (probably) which promptly
  //   refuted the move at the previous ply.
  const auto hash(s.hash());
  if (ply)
  {
    const auto entry(tt_->find(hash));
    if (entry && entry->draft >= draft)
      switch (entry->type)
      {
      case node_type::exact:
        return entry->value;
      case node_type::fail_low:
        if (entry->value <= alpha)
          return alpha;
        break;
      case node_type::fail_high:
        if (entry->value >= beta)
          return beta;
        break;
      default:  // node_type::ignore
        ;
      }
  }

  const auto moves(sorted_moves(s));

  const bool in_check(s.in_check());

  if (moves.empty())
    return in_check ? -MATE + ply : 0;

  if (ply && (s.repetitions() || s.fifty() >= 100))
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
      if (x >= beta)
      {
        if (!search_stopped_)
          tt_->insert(hash, m, draft, node_type::fail_high, beta);

        return beta;
      }

      best_move = m;
      alpha = x;
    }
  }

  if (!search_stopped_)
    tt_->insert(hash, best_move, draft,
                !best_move ? node_type::fail_low : node_type::exact, alpha);

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
       && (s.repetitions() <= 2 || pv.empty())
       && s.make_move(entry->best_move);)
  {
    pv.push_back(entry->best_move);
    entry = tt_->find(s.hash());
  }

  return pv;
}

move search::operator()(bool verbose)
{
  search_time_.restart();
  tt_->inc_age();

  stats.reset();

  score alpha(-INF), beta(+INF);

  move best_move(move::sentry());
  for (int i(1), max(max_depth ? max_depth : 1000); i <= max;)
  {
    auto x(alphabeta(root_state_, alpha, beta, 0, i * PLY));

    if (search_stopped_)
      break;

    if (x <= alpha || x >= beta)
    {
      alpha = -INF;
      beta  = +INF;
      continue;
    }

    alpha = x - 50;
    beta  = x + 50;

    const auto pv(extract_pv());
    if (!pv.empty())
      best_move = pv[0];

    if (verbose)
    {
      std::cout << i << ' ' << x << ' ' << search_time_.elapsed().count() / 10
                << ' ' << stats.snodes;

      for (const auto &m : pv)
        std::cout << ' ' << m;

      std::cout << std::endl;
    }

    if (is_mate(x))
      break;

    ++i;
  }

  return best_move;
}

}  // namespace testudo
