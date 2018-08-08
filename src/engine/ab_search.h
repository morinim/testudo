/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_AB_SEARCH_H)
#define      TESTUDO_AB_SEARCH_H

#include "search.h"
#include "timer.h"

namespace testudo
{

class cache;

struct driver
{
  static constexpr unsigned MAX_DEPTH = 1024;

  explicit driver(const std::vector<state> &);

  // Contains information about the path leading to the node being analyzed.
  struct path_info
  {
    explicit path_info(const std::vector<state> &);

    bool repetitions() const;

    void push(const state &);
    void pop();

    std::vector<hash_t> states;  // used for repetition detection
  } path;

  void upd_move_heuristics(const move &, piece p, unsigned, unsigned);

  std::vector<std::pair<move, move>> killers;
  int history[piece::sup_id][64];
};

class ab_search : public search
{
public:
  // The word 'ply' denotes a half-move, that is a move of one side only.
  // We extend/reduce in fractions of one ply (reason why `PLY != 1`).
  static constexpr int PLY = 4;

  ab_search(const std::vector<state> &, cache *);

  move run(bool) final;

private:
  static constexpr std::uintmax_t nodes_between_checks = 2048;

  score ab(const state &, score, score, unsigned, int);
  score ab_root(score, score, int);
  score aspiration_search(score *, score *, int);
  int new_draft(int, bool, const move &) const;
  movelist extract_pv() const;
  int quiesce(const state &, score, score);
  movelist sorted_captures(const state &);
  movelist sorted_moves(const state &);

  state root_state_;

  driver driver_;

  cache *tt_;

  timer  search_timer_;
};  // class ab_search

// - `states` is the sequence of states reached until now. It could be a
//   partial list (e.g. for FEN positions) but `states.back()` must contain the
//   current state.
// - `tt` is a pointer to an external hash table.
inline ab_search::ab_search(const std::vector<state> &states, cache *tt)
  : search(), root_state_(states.back()), driver_(states), tt_(tt),
    search_timer_()
{
  assert(!states.empty());
  assert(tt);

  assert(!driver_.path.states.empty());
}

}  // namespace testudo

#endif  // include guard
