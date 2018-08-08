/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_MCTS_H)
#define      TESTUDO_MCTS_H

#include "search.h"
#include "mcts/mcts.h"

namespace testudo
{

class mcts_state
{
public:
  using action = move;

  explicit mcts_state(const state &);

  bool make_action(const action &);
  std::vector<action> actions() const;
  std::vector<double> eval() const;
  bool is_final() const;
  unsigned agent_id() const;

  friend std::ostream &operator<<(std::ostream &, const mcts_state &);

  const state &get_state() const { return state_; }

private:
  state state_;
};

class mcts_search : public search
{
public:
  explicit mcts_search(const std::vector<state> &);

  move run(bool) final;

private:
  mcts_state root_state_;
};  // class mcts_search

// `states` is the sequence of states reached until now. It could be a partial
// list (e.g. for FEN positions) but `states.back()` must contain the current
// state.
inline mcts_search::mcts_search(const std::vector<state> &states)
  : search(), root_state_(states.back())
{
  assert(!states.empty());
}

}  // namespace testudo

#endif  // include guard
