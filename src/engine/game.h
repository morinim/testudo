/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_GAME_H)
#define      TESTUDO_GAME_H

#include <cassert>
#include <chrono>

#include "state.h"
#include "cache.h"

namespace testudo
{

class game
{
public:
  game() : show_search_info(true), ics(false),
           tt_(), states_({state(state::setup::start)}), computer_side_(-1),
           max_depth_(0), time_info_()
  {}

  bool make_move(const move &);
  bool take_back(unsigned = 1);

  void max_depth(unsigned d) { max_depth_ = d; }

  void max_time(std::chrono::milliseconds);

  void level(unsigned m, std::chrono::milliseconds t)
  { time_info_.level(m, t); }

  void time(std::chrono::milliseconds t)
  { time_info_.time(t); }

  int computer_side() const { return computer_side_; }
  void computer_side(int);

  move think(bool, bool);

  const state &current_state() const
  { assert(!states_.empty());  return states_.back(); }

  void set_board(const std::string &s) { states_ = {state(s)}; }

  bool show_search_info;
  bool ics;

private:
  cache tt_;

  std::vector<state> states_;

  int computer_side_;                   // -1, BLACK, WHITE
  unsigned max_depth_;                  // Maximum search depth

  struct time_info
  {
    time_info() : max_time(0), moves_per_tc(0), tc(0), moves_left(0),
                  time_left(0)
    {}

    void level(unsigned, std::chrono::milliseconds);
    void time(std::chrono::milliseconds);
    std::chrono::milliseconds time_for_next_move();

    static constexpr double security_margin = 0.03;

    std::chrono::milliseconds max_time;  // maximum search time

    unsigned moves_per_tc;               // 0 is sudden-death time control
    std::chrono::milliseconds tc;

    unsigned moves_left;
    std::chrono::milliseconds time_left;
  } time_info_;
};  // class game

}  // namespace testudo

#endif  // include guard
