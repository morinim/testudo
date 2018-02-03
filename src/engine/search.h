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

#include "state.h"
#include "timer.h"

namespace testudo
{

class cache;

/******************************************************************************
 * search class
 *****************************************************************************/
class search
{
public:
  explicit search(const state &, cache *);

  score alphabeta(const state &, score, score, unsigned, int);
  int quiesce(const state &, score, score);

  move operator()(bool);

  struct statistics
  {
    statistics() : snodes(0), qnodes(0) {}

    void reset() { *this = statistics(); }

    std::uintmax_t snodes;
    std::uintmax_t qnodes;
  } stats;

  std::chrono::milliseconds max_time;
  unsigned max_depth;

private:
  static constexpr int PLY = 16;

  int delta_draft(bool, unsigned, const move &);
  std::vector<move> extract_pv() const;
  std::vector<move> sorted_captures(const state &);
  std::vector<move> sorted_moves(const state &);

  state root_state_;

  cache *tt_;

  timer search_time_;
  bool search_stopped_;
};  // class search

inline search::search(const state &root, cache *tt)
  : stats(), max_time(0), max_depth(0), root_state_(root), tt_(tt),
    search_time_(), search_stopped_(false)
{
}

}  // namespace testudo

#endif  // include guard
