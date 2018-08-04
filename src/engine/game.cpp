/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "game.h"
#include "log.h"
#include "search.h"

namespace testudo
{

bool game::make_move(const move &m)
{
  auto current(current_state());

  if (current.make_move(m))
  {
    states_.push_back(current);
    return true;
  }

  return false;
}

bool game::take_back(unsigned n)
{
  if (states_.size() <= n)  // at least a one-state history
    return false;

  while (n--)
    states_.pop_back();

  return true;
}

void game::max_time(std::chrono::milliseconds t)
{
  time_info_.max_time = t;
  max_depth(0);
}

// Runs the search algoritm on the current position (given the active search
// parameters).
// If the `analyze_mode` is active the search continues until the interface
// sends a command.
// Returns the best move found (if available).
move game::think(bool verbose, bool analyze_mode)
{
  search s(states_, &tt_);

  if (analyze_mode)
  {
    s.constraint.max_depth = 0;
    s.constraint.max_time  = std::chrono::milliseconds(0);
  }
  else
  {
    s.constraint.max_depth = max_depth_;
    s.constraint.max_time  = time_info_.time_for_next_move();
  }

  return s.run(verbose);
}

void game::time_info::level(unsigned moves, std::chrono::milliseconds time)
{
  using namespace std::chrono_literals;

  testudoINFO << "Setting time control to " << moves << ' '
              << std::chrono::duration_cast<std::chrono::seconds>(time).count()
              << 's';

  moves_per_tc = moves;
  tc           =  time;

  moves_left = moves;
  time_left  =  time;
}

void game::time_info::time(std::chrono::milliseconds t)
{
  testudoINFO << "Updating time to next time control from "
              << time_left.count() << "ms to " << t.count() << "ms";

  time_left = t;
}

std::chrono::milliseconds game::time_info::time_for_next_move()
{
  using namespace std::chrono_literals;

  std::chrono::milliseconds t;

  if (time_left == 0ms)
    time_left = 100ms;

  // Simplest situation: fixed time per move (if `max_time == 0` there isn't a
  // time limit).
  if (!moves_per_tc && tc == 0ms)
    return max_time;
  // SUDDEN-DEATH TIME CONTROL (play the whole game in a fixed period).
  // > It can be handled by always considering that X moves always remain until
  // > the time control.  Let's say that you pick the number 30 as X. You'll
  // > use about 1/30 of your remaining time on the first move. When it's time
  // > to move again, you'll use 1/30 of the remaining time again. This works
  // > rather well in practice. You move faster and faster as the game goes on,
  // > which is probably what you should do anyway.
  // > So this is a lot easier to implement than the tournament time control
  // > logic.
  // (Bruce Moreland)
  else if (!moves_per_tc)
  {
    t = time_left / 30;
  }
  else  // TOURNAMENT TIME CONTROL (X moves in Y minutes)
  {
    if (moves_left == 0)
      moves_left = moves_per_tc;

    t = time_left /
        (moves_left + std::min<unsigned>(1, moves_per_tc * security_margin));

    --moves_left;
  }

  if (max_time != 0ms)
    t = std::min(max_time, t);

  testudoINFO << "Time for next move: " << t.count() << "ms";
  return t;
}

void game::computer_side(int s)
{
  switch (s)
  {
  case BLACK:
  case WHITE:
    computer_side_ = s;
    break;
  default:
    computer_side_ = -1;
  }
}

}  // namespace testudo
