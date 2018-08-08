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
#include <chrono>
#include <functional>

#include "state.h"

namespace testudo
{

class search
{
public:
  search();
  virtual ~search() {}

  virtual move run(bool) = 0;

  struct statistics
  {
    statistics() : moves_at_root(), snodes(0), qnodes(0), depth(0),
                   score_at_root(0) {}
    void reset() { *this = statistics(); }

    movelist       moves_at_root;
    std::uintmax_t        snodes;  // search nodes
    std::uintmax_t        qnodes;  // quiescence search nodes
    unsigned               depth;  // depth reached
    score          score_at_root;
  } stats;

  struct constraints
  {
    constraints() : max_time(0), max_depth(0), max_nodes(0), condition() {}

    std::chrono::milliseconds max_time;
    unsigned                 max_depth;
    std::uintmax_t           max_nodes;

    std::function<bool()> condition;  // custom early exit condition
  } constraint;

protected:
  bool search_stopped_;
};  // class search

// `states` is the sequence of states reached until now. It could be a partial
// list (e.g. for FEN positions) but `states.back()` must contain the current
// state.
inline search::search() : stats(), constraint(), search_stopped_(false)
{
  assert(!states.empty());
}

}  // namespace testudo

#endif  // include guard
