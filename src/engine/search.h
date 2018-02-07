/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_SEARCH_H)
#define      TESTUDO_SEARCH_H

#include <cassert>

#include "state.h"
#include "timer.h"

namespace testudo
{

class cache;

class search
{
public:
  search(const std::vector<state> &, cache *);

  move run(bool);

  struct statistics
  {
    statistics() : moves_at_root(), snodes(0), qnodes(0), depth(0) {}
    void reset() { *this = statistics(); }

    movelist       moves_at_root;
    std::uintmax_t        snodes;  // search nodes
    std::uintmax_t        qnodes;  // quiescence search nodes
    unsigned               depth;  // depth reached
  } stats;

  std::chrono::milliseconds max_time;
  unsigned                 max_depth;

private:
  static constexpr int PLY = 16;
  static constexpr std::uintmax_t nodes_between_checks = 2048;

  score alphabeta(const state &, score, score, unsigned, int);
  score alphabeta_root(const state &, score, score, int);
  score aspiration_search(score *, score *, int);
  int delta_draft(bool, unsigned, const move &) const;
  movelist extract_pv() const;
  int quiesce(const state &, score, score);
  movelist sorted_captures(const state &);
  movelist sorted_moves(const state &);

  state root_state_;

  struct search_path_info
  {
    explicit search_path_info(const std::vector<state> &);

    bool repetitions() const;

    void push(const state &);
    void pop();

    std::vector<hash_t> states;  // used for repetition detection
  } search_path_info_;

  cache *tt_;

  timer search_time_;
  bool search_stopped_;
};  // class search

// `states` is the sequence of states reached until now. It could be a partial
// list (e.g. for FEN positions) but `states.back()` must contain the current
// state.
inline search::search(const std::vector<state> &states, cache *tt)
  : stats(), max_time(0), max_depth(0), root_state_(states.back()),
    search_path_info_(states), tt_(tt), search_time_(), search_stopped_(false)
{
  assert(!states.empty());
  assert(tt);
}

}  // namespace testudo

#endif  // include guard
