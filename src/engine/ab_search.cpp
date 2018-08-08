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

#include "ab_search.h"
#include "cache.h"
#include "eval.h"
#include "log.h"
#include "nonstd.h"
#include "util.h"

namespace testudo
{

namespace
{

// The following constants are used for move ordering:
// - captures / promotions are above the `SORT_CAPTURE` level;
// - killer moves have values near `SORT_KILLER`;
// - other quiet moves are (quite) below the `SORT_KILLER` value.
constexpr int SORT_CAPTURE = std::numeric_limits<int>::max() - 1000000;
constexpr int SORT_KILLER  = SORT_CAPTURE - 1000000;

/*****************************************************************************
// A convenient class to extract one move at time from the list of the legal
// ones.
// We don't sort the whole move list, but perform a selection sort each time a
// move is fetched.
// Root node is an exception requiring additional effort to score and sort
// moves.
*****************************************************************************/
class move_provider
{
public:
  enum class stage {hash = 0, move_gen, others};

  move_provider(const state &, const cache::slot *);

  move next(const driver &, unsigned);
  bool empty();

private:
  void move_gen();

  const state           &s_;
  stage              stage_;
  move          from_cache_;
  movelist           moves_;
  movelist::iterator start_;
};

// If there is a legal move from the hash table (`entry != nullptr`), move
// generation can be delayed: often the move is enough to cause a cutoff and
// save time.
move_provider::move_provider(const state &s, const cache::slot *entry)
  : s_(s), stage_(stage::hash), from_cache_(move::sentry()), moves_(), start_()
{
  if (entry && entry->best_move() && s_.is_legal(entry->best_move()))
    from_cache_ = entry->best_move();
  else
  {
    move_gen();
    stage_ = stage::others;
  }
}

void move_provider::move_gen()
{
  moves_ = s_.moves();
  start_ = moves_.begin();

  if (from_cache_)
  {
    auto where(std::find(start_, moves_.end(), from_cache_));
    assert(where != moves_.end());

    std::swap(*start_, *where);
    ++start_;
  }
}

bool move_provider::empty()
{
  if (from_cache_)
    return false;

  return moves_.empty();
}

move move_provider::next(const driver &d, unsigned ply)
{
  const auto move_score(
    [&](const move &m)
    {
      if (is_quiet(m))
      {
        if (m == d.killers[ply].first)
          return SORT_KILLER;
        if (m == d.killers[ply].second)
          return SORT_KILLER - 1;

        return d.history[s_[m.from].id()][m.to];

        //if (m.flags & move::castle)
        //  return 1;
      }

      score v(SORT_CAPTURE);

      // En passant gets a score lower than other PxP moves but are anyway
      // searched in the groups of the capture moves.
      if (is_capture(m))
        v += (s_[m.to].value() << 8) - s_[m.from].value();

      if (is_promotion(m))
        v += piece(WHITE, m.promote()).value();

      return v;
    });

  switch (stage_)
  {
  case stage::hash:
    stage_ = stage::move_gen;
    return from_cache_;

  case stage::move_gen:
    stage_ = stage::others;
    move_gen();
    // fall through

  default:
    if (start_ == moves_.end())
      return move::sentry();

    auto max(std::max_element(start_, moves_.end(),
                              [&](const auto &a, const auto &b)
                              {
                                return move_score(a) < move_score(b);
                              }));
    std::swap(*max, *start_);

    return *start_++;
  }
}

}  // unnamed namespace

/*****************************************************************************
 * Driver
 *****************************************************************************/
driver::driver(const std::vector<state> &ss)
  : path(ss), killers(MAX_DEPTH), history()
{
}

void driver::upd_move_heuristics(const move &m, piece p, unsigned ply,
                                 unsigned draft)
{
  assert(m);
  assert(is_quiet(m));
  assert(p.id() != EMPTY);
  assert(ply < killers.size());
  assert(depth >= ab_search::PLY);

  // ********* Killer heuristics *********
  // Makes sure killer moves will be different before saving secondary killer
  // move.
  if (killers[ply].first != m)
    killers[ply].second = killers[ply].first;

  killers[ply].first = m;

  // ********* History heuristics *********
  const int depth(draft / ab_search::PLY);

  auto &slot(history[p.id()][m.to]);
  slot += depth * depth;

  // Prevents table overflow.
  if (slot >= SORT_KILLER)
    for (unsigned i(0); i < piece::sup_id; ++i)
      for (unsigned sq(0); sq < 64; ++sq)
        history[i][sq] = (history[i][sq] + 1) / 2;
}

// Extraxt from the list of past known states (`ss`) a set of hash values used
// for repetition detection.
driver::path_info::path_info(const std::vector<state> &ss)
{
  assert(!ss.empty());

  std::transform(ss.begin(), ss.end(),  std::back_inserter(states),
                 [](const state &s) { return s.hash(); });

  assert(!states.empty());
  assert(states.back() == current.hash());
}

// Returns `true` if the current position (`states_.back()`) has been
// repeated (compares the current hash value to already seen values).
bool driver::path_info::repetitions() const
{
  assert(!states.empty());

  const auto current(states.size() - 1);
  for (std::size_t i(0); i < current; ++i)
    if (states[i] == states[current])
      return true;
  return false;
}

void driver::path_info::push(const state &current)
{
  states.push_back(current.hash());
}

void driver::path_info::pop()
{
  states.pop_back();
}

movelist ab_search::sorted_captures(const state &s)
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
score ab_search::quiesce(const state &s, score alpha, score beta)
{
  assert(alpha < beta);

  ++stats.qnodes;

  // The static evaluation is a "stand-pat" score (the term is taken from the
  // game of poker, where it denotes playing one's hand without drawing more
  // cards) and is used to establish a lower bound on the score.
  // Assuming we aren't in zugzwang, this is theoretically sound because we can
  // assume that there is at least one move that can either match or beat the
  // lower bound.
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

movelist ab_search::sorted_moves(const state &s)
{
  const auto entry(tt_->find(s.hash()));
  const move best_move(entry ? entry->best_move() : move::sentry());

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
      else if (is_capture(m))
        ms = (s[m.to].value() << 8) - s[m.from].value() + 1000000;
      if (is_promotion(m))
        ms += piece(WHITE, m.promote()).value() + 100000;

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

int ab_search::new_draft(int draft, bool in_check, const move &m) const
{
  int delta(-PLY);

  // The formula:
  // - do not allow entering quiescence search when in check;
  // - do not extend search when there is a lot of draft.
  if (in_check)
    delta += 2 * PLY * PLY / std::max(draft, 1);

  if (is_capture(m))
    delta += PLY/2;

  //if ((m.flags & move::pawn) && (rank(m.to) <= 1 || rank(m.to) >= 6))
  //  delta += PLY/2;

  return draft + std::min(0, delta);
}

// A slightly modified version of alphabeta (not stricly necessary but helps
// to avoid a lot of `if`):
// - the order of the moves is improved when a best move is found. This is
//   possible since the root moves are "permanent" (they're kept inside the
//   `stat` structure and are available even when the search is finished). THIS
//   IS AN IMPORTANT DIFFERENCE;
// - the function assumes that the position isn't a stalemate / immediate mate;
// - the function ignores draw by repetition / 50 moves rule (we want a move).
score ab_search::ab_root(score alpha, score beta, int draft)
{
  assert(alpha < beta);
  assert(draft >= PLY);

  ++stats.snodes;

  // Don't push the current state in the `path` vector: `root_state_` is
  // already present.
  assert(driver_.path.back() == root_state_.hash());

  auto &moves(stats.moves_at_root);
  if (moves.empty())
    moves = sorted_moves(root_state_);
  assert(!moves.empty());

  const bool in_check(root_state_.in_check());

  auto best_move(move::sentry());
  auto type(score_type::fail_low);

  for (std::size_t i(0); i < moves.size(); ++i)
  {
    const auto d(new_draft(draft, in_check, moves[i]));

    const auto s1(root_state_.after_move(moves[i]));
    score x;
    if (i == 0)
      x = -ab(s1, -beta, -alpha, 1, d);
    else
    {
      x = -ab(s1, -alpha - 1, -alpha, 1, d);
      if (alpha < x && x < beta)
        x = -ab(s1, -beta, -alpha, 1, d);
    }

    if (x > alpha)
    {
      best_move = moves[i];

      // Moves at the root node are very important and they're kept in the
      // best known order (given the search history).
      std::copy_backward(&moves[0], &moves[i], &moves[i + 1]);
      moves[0] = best_move;

      if (x >= beta)
      {
        type = score_type::fail_high;
        break;
      }

      alpha = x;
      type = score_type::exact;
    }
  }

  const auto val(type == score_type::fail_high ? beta : alpha);

  if (!search_stopped_)
    tt_->insert(root_state_.hash(), best_move, draft, type, val);

  return val;
}

// Recursively implements negamax alphabeta until draft is exhausted, at which
// time it calls `quiesce()`.
// The `ply` index measures the distance of the current node from the root
// node, while `draft` is the remaining depth to the horizon.
// There are various reasons to decouple the depth to horizon from the
// `ply`-index or depth from root. While the `ply`-index is incremented by one
// each time, the draft may be independently altered by various extension or
// reduction-schemes and may also consider fractional extensions (values less
// then `PLY`).
score ab_search::ab(const state &s, score alpha, score beta,
                    unsigned ply, int draft)
{
  assert(alpha < beta);
  assert(draft >= PLY);

  if (draft < PLY)
    return quiesce(s, alpha, beta);

  // Checks to see if we have searched enough nodes that it's time to peek at
  // how much time has been used / check for operator keyboard input.
  if (search_stopped_ || ++stats.snodes % nodes_between_checks == 0)
  {
    search_stopped_ =
      search_timer_.elapsed(constraint.max_time)
      || (constraint.max_nodes
          && stats.snodes + stats.qnodes > constraint.max_nodes)
      || input_available();

    if (search_stopped_)
      return 0;
  }

  driver_.path.push(s);
  auto guard = finally([&]{ driver_.path.pop(); });

  // Draws. Check for draw by repetition / 50 move draws also. This is the
  // quickest way to get out of further searching, with minimal effort.
  if (driver_.path.repetitions() || s.fifty() >= 100)
    return 0;

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
  if (entry && entry->draft() >= draft)
    switch (entry->type())
    {
    case score_type::fail_low:
      if (entry->value() <= alpha)
        return alpha;
      break;
    case score_type::fail_high:
      if (entry->value() >= beta)
        return beta;
      break;
    default:
      assert(entry->type() == score_type::exact);
      return entry->value();
    }

  move_provider moves(s, entry);
  const bool in_check(s.in_check());

  if (moves.empty())
    return in_check ? -INF + ply : 0;

  auto best_move(move::sentry());
  auto type(score_type::fail_low);
  bool first(true);

  for (move m; (m = moves.next(driver_, ply));)
  {
    const auto d(new_draft(draft, in_check, m));

    const auto s1(s.after_move(m));
    score x;
    if (first)
    {
      x = -ab(s1, -beta, -alpha, ply + 1, d);
      first = false;
    }
    else
    {
      x = -ab(s1, -alpha - 1, -alpha, ply + 1, d);
      if (alpha < x && x < beta)
        x = -ab(s1, -beta, -alpha, ply + 1, d);
    }

    if (x > alpha)
    {
      best_move = m;

      if (x >= beta)
      {
        type = score_type::fail_high;

        if (is_quiet(m))
          driver_.upd_move_heuristics(m, s[m.from], ply, draft);
        break;
      }

      type = score_type::exact;
      alpha = x;
    }
  }

  const auto val(type == score_type::fail_high ? beta : alpha);

  if (!search_stopped_)
    tt_->insert(s.hash(), best_move, draft, type, val);

  return val;
}

// Extract the PV from the transposition table.
// At least one move should always be availeble (even in case of immediate
// draw).
movelist ab_search::extract_pv() const
{
  std::vector<hash_t> history(driver_.path.states);

  auto s(root_state_);

  movelist pv;
  for (auto entry(tt_->find(s.hash()));
       entry && entry->best_move()
       && pv.size() <= 3 * stats.depth
       && (s.mate_or_draw(&history) == state::kind::standard || pv.empty())
       && s.make_move(entry->best_move());)
  {
    history.push_back(s.hash());

    pv.push_back(entry->best_move());
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
score ab_search::aspiration_search(score *alpha, score *beta, int draft)
{
  auto x(ab_root(*alpha, *beta, draft));

  if (search_stopped_)
    return 0;

  if (x <= *alpha || x >= *beta)
  {
    testudoOUTPUT << stats.depth << ' ' << (x <= *alpha ? "--" : "++") << ' '
                  << search_timer_.elapsed().count() / 10 << ' '
                  << stats.snodes << ' ' << stats.moves_at_root.front();

    x = ab_root(-INF, +INF, draft);
  }

  if (search_stopped_)
    return 0;

  stats.score_at_root = x;

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
move ab_search::run(bool verbose)
{
  switch (root_state_.mate_or_draw(&driver_.path.states))
  {
  case state::kind::mated:
  case state::kind::draw_stalemate:
    return move::sentry();

  default:
    search_timer_.restart();
    tt_->inc_age();
    stats.reset();
 }

  move best_move(move::sentry());

  score alpha(-INF), beta(+INF);
  stats.depth = 1;
  for (unsigned max(constraint.max_depth ? constraint.max_depth : 1000);
       stats.depth <= max;
       ++stats.depth)
  {
    const auto x(aspiration_search(&alpha, &beta, stats.depth * PLY));

    if (search_stopped_)
      break;

    best_move = stats.moves_at_root.front();
    const auto pv(extract_pv());
    assert(pv.front() == best_move);

    if (verbose)
    {
      testudoOUTPUT << stats.depth << ' ' << x << ' '
                    << search_timer_.elapsed().count() / 10 << ' '
                    << stats.snodes << ' ' << pv;
    }

    if (is_mate(x)
        || (root_state_.moves().size() == 1 && stats.depth == 5))
      break;

    // Custom early exit condition.
    if (constraint.condition && constraint.condition())
      break;
  }

  return best_move;
}

}  // namespace testudo
