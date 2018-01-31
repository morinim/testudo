/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <sstream>
#include <thread>
#include <vector>

#include "testudo.h"

namespace testudo
{

/******************************************************************************
 * game class
 *****************************************************************************/
class game
{
public:
  game() : current_state(), show_search_info(true), tt_(), previous_states_(),
           computer_side_(-1), max_depth_(0), max_time_(1000)
  {}

  bool make_move(const move &m)
  {
    previous_states_.push_back(current_state);

    if (current_state.make_move(m))
      return true;

    take_back();
    return false;
  }

  bool take_back(unsigned n = 1)
  {
    if (previous_states_.size() < n)
      return false;

    while (n-- > 1)
      previous_states_.pop_back();

    current_state = previous_states_.back();
    previous_states_.pop_back();
    return true;
  }

  void max_depth(unsigned d)
  {
    max_depth_ = d;
  }

  void max_time(std::chrono::milliseconds t)
  {
    max_time_ = t;
    max_depth(0);
  }

  int computer_side() const { return computer_side_; }
  void computer_side(int);

  move think(bool);

  state current_state;
  bool show_search_info;

private:
  cache tt_;

  std::vector<state> previous_states_;

  int computer_side_;  // -1, BLACK, WHITE
  unsigned max_depth_;
  std::chrono::milliseconds max_time_;
};  // class game

// Runs the search algoritm on the current position (given the active search
// parameters).
// Returns the best move found (if available).
move game::think(bool verbose)
{
  search s(current_state, &tt_);

  s.max_depth = max_depth_;
  s.max_time = max_time_;

  return s(verbose);
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

/******************************************************************************
 * CECP interface (aka Xboard)
 *****************************************************************************/
namespace CECP
{

void print_move_or_result(const state &s, const move &m)
{
  std::cout << "move " << m << std::endl;

  switch (s.mate_or_draw())
  {
  case state::kind::mated:
    if (s.side() == WHITE)
      std::cout << "0-1 {Black mates}\n";
    else
      std::cout << "1-0 {White mates}\n";
    break;

  case state::kind::draw_stalemate:
    std::cout << "1/2-1/2 {Stalemate}\n";
    break;

  case state::kind::draw_fifty:
    std::cout << "1/2-1/2 {Draw by fifty move rule}\n";
    break;

  case state::kind::draw_repetition:
    std::cout << "1/2-1/2 {Draw by repetition}\n";
    break;

  default:
    ;
  }
}

void loop()
{
  game g;

  for (;;)
  {
    std::cout << std::flush;

    if (g.current_state.side() == g.computer_side()
        /*&& g.current_state.mate_or_draw() == state::kind::standard*/)
    {
      const auto m(g.think(g.show_search_info));
      if (!m)
        g.computer_side(-1);
      else
      {
        g.make_move(m);
        print_move_or_result(g.current_state, m);
      }
      continue;
    }

    std::string line;
    while (!std::getline(std::cin, line))
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::istringstream is(line);
    std::string cmd;
    is >> std::skipws >> cmd;

    if (cmd == "accepted" || cmd == "easy" || cmd == "hard" || cmd == "level"
        || cmd == "otim" || cmd == "random" || cmd == "xboard")
      continue;
    if (cmd == "protover")
    {
      int version;  is >> version;  // skips version
      std::cout << "feature myname=\"TESTUDO 0.9\" playother=1 sigint=0 "
                   "colors=0 setboard=1 debug=1 done=1" << std::endl;
      continue;
    }
    if (cmd == "new")
    {
      g = decltype(g)();
      g.computer_side(BLACK);
      g.max_depth(0);
      continue;
    }
    else if (cmd == "result")
    {
      g.computer_side(-1);
      continue;
    }
    if (cmd == "quit")
      return;
    if (cmd == "force")
    {
      g.computer_side(-1);
      continue;
    }
    if (cmd == "playother")
    {
      g.computer_side(!g.current_state.side());
      continue;
    }
    if (cmd == "st")
    {
      int t;  is >> t;
      g.max_time(std::chrono::milliseconds(t * 1000));
      continue;
    }
    if (cmd == "sd")
    {
      unsigned d;  is >> d;
      g.max_depth(d);
      continue;
    }
    if (cmd == "time")
    {
      int t;  is >> t;
      g.max_time(std::chrono::milliseconds(t * 10 / 30));
      continue;
    }
    else if (cmd == "go")
    {
      g.computer_side(g.current_state.side());
      continue;
    }
    if (cmd == "hint")
    {
      const auto m(g.think(false));

      if (!m.is_sentry())
        std::cout << "Hint: " << m << std::endl;
      continue;
    }
    if (cmd == "undo")
    {
      g.take_back();
      continue;
    }
    if (cmd == "remove")
    {
      g.take_back(2);
      continue;
    }
    if (cmd == "post")
    {
      g.show_search_info = true;
      continue;
    }
    if (cmd == "nopost")
    {
      g.show_search_info = false;
      continue;
    }
    if (cmd == "setboard")
    {
      std::string fen;  is >> fen;
      g.current_state = state(fen);
      continue;
    }

    const move m(g.current_state.parse_move(cmd));
    if (!m)
      std::cout << "Error (unknown command): " << cmd << std::endl;
    else
      g.make_move(m);
  }
}

}  // namespace CECP

}  // namespace testudo

/******************************************************************************
 * main
 *****************************************************************************/
#if !defined(TESTUDO_CONFIG_TEST)
int main()
{
  testudo::CECP::loop();
}
#endif
