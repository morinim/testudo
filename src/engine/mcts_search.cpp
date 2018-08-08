/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "mcts_search.h"
#include "eval.h"
#include "log.h"

namespace testudo
{

mcts_state::mcts_state(const state &s) : state_(s)
{
}

bool mcts_state::make_action(const action &a)
{
  return state_.make_move(a);
}

std::vector<mcts_state::action> mcts_state::actions() const
{
  return state_.moves();
}

std::vector<double> mcts_state::eval() const
{
  switch (state_.mate_or_draw())
  {
  case state::kind::mated:
    if (state_.side() == WHITE)
      return {1.0, 0.0};
    return {0.0, 1.0};

  case state::kind::draw_stalemate:
  case state::kind::draw_repetition:
  case state::kind::draw_fifty:
    return {0.0, 0.0};

  default:
    break;
  }

  double black_score(testudo::eval(state_));
  if (state_.side() == WHITE)
    black_score = -black_score;

  const auto scaled_score(black_score / 200.0);
  const auto sigmoid([](double v) { return v / (1 + std::abs(v)); });

  const auto v(0.5 + sigmoid(scaled_score) / 2.0);
  assert(0.0 <= v && v <= 1.0);

  return {v, 1.0 - v};
}

bool mcts_state::is_final() const
{
  return state_.mate_or_draw() != state::kind::standard;
}

unsigned mcts_state::agent_id() const
{
  return state_.side();
}

std::ostream &operator<<(std::ostream &o, const mcts_state &s)
{
  return o << s.state_;
}

move mcts_search::run(bool verbose)
{
  switch (root_state_.get_state().mate_or_draw())
  {
  case state::kind::mated:
  case state::kind::draw_stalemate:
    return move::sentry();

  default:
    stats.reset();
  }

  std::ostringstream ss;
  using mcts = light_mcts::uct<mcts_state>;
  mcts::params p;
  p.max_search_time = constraint.max_time;
  p.simulation_depth = 0;
  p.log_depth = 1;
  p.log = &ss;
  move best_move(move::sentry());

  const auto r(mcts(root_state_, p).run());

  if (verbose)
  {
    testudoOUTPUT << 10 << ' ' << r.second[root_state_.get_state().side()]
                  << ' ' << constraint.max_time.count() / 10 << ' '
                  << 1000 << ' ' << r.first;

    testudoOUTPUT << ss.str();
  }

  return r.first;
}

}  // namespace testudo
