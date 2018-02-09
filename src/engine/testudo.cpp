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

std::chrono::seconds xboard_time(const std::string &s)
{
  const auto split([](const std::string &t, char delim = ':')
                   {
                     std::vector<std::string> ret;
                     std::istringstream ss(t);
                     std::string item;
                     while (std::getline(ss, item, delim))
                       ret.push_back(item);
                     return ret;
                   });

  const auto parts(split(s));

  int seconds;
  switch (parts.size())
  {
  case 2:  seconds = stoi(parts[0]) * 60 + stoi(parts[1]);  break;
  case 1:  seconds = stoi(parts[0]) * 60;                   break;
  default:                                                  break;
  }

  return std::chrono::seconds(seconds);
}

void loop()
{
  using namespace std::chrono_literals;

  game g;

  for (;;)
  {
    std::cout << std::flush;

    if (g.current_state().side() == g.computer_side())
    {
      const auto m(g.think(g.show_search_info));
      if (!m)
        g.computer_side(-1);
      else
      {
        g.make_move(m);
        print_move_or_result(g.current_state(), m);
      }
      continue;
    }

    std::string line;
    while (!std::getline(std::cin, line))
      std::this_thread::sleep_for(500ms);

    std::istringstream is(line);
    std::string cmd;
    is >> std::skipws >> cmd;

    if (cmd == "accepted" || cmd == "easy" || cmd == "hard" || cmd == "otim"
        || cmd == "random" || cmd == "xboard")
      continue;
    if (cmd == "force")
    {
      g.computer_side(-1);
      continue;
    }
    if (cmd == "go")
    {
      g.computer_side(g.current_state().side());
      continue;
    }
    if (cmd == "hint")
    {
      const auto m(g.think(false));

      if (!m.is_sentry())
        std::cout << "Hint: " << m << std::endl;
      continue;
    }
    if (cmd == "ics")
    {
      std::string server;  is >> server;
      g.ics = (server != "-");
      std::cout << "# Setting ICS server to: " << g.ics << std::endl;
      continue;
    }
    if (cmd == "level")
    {
      unsigned    moves;  is >> moves;
      std::string stime;  is >> stime;
      auto time(xboard_time(stime));

      // Covers a special rule on some ICS implementations: if you ask for a
      // game with `base=0`, the clocks really start at 10 seconds instead of
      // 0. xboard itself doesn't know about this rule, so it passes the 0 on
      // to the engine instead of changing it to 0:10.
      if (g.ics && time == 0s)
      {
        time = 10s;
        std::cout << "Adjusting time to 10s" << std::endl;
      }

      g.level(moves, time);
      continue;
    }
    if (cmd == "new")
    {
      g = decltype(g)();
      g.computer_side(BLACK);
      g.max_depth(0);
      continue;
    }
    if (cmd == "nopost")
    {
      g.show_search_info = false;
      continue;
    }
    if (cmd == "protover")
    {
      int version;  is >> version;  // skips version
      std::cout << "feature myname=\"TESTUDO 0.9\" playother=1 sigint=0 "
                   "colors=0 setboard=1 ics=1 debug=1 done=1" << std::endl;
      continue;
    }
    if (cmd == "playother")
    {
      g.computer_side(!g.current_state().side());
      continue;
    }
    if (cmd == "post")
    {
      g.show_search_info = true;
      continue;
    }
    if (cmd == "quit")
      return;
    if (cmd == "remove")
    {
      g.take_back(2);
      continue;
    }
    if (cmd == "result")
    {
      g.computer_side(-1);
      continue;
    }
    if (cmd == "setboard")
    {
      std::string fen;  is >> fen;
      g.set_board(fen);
      continue;
    }
    if (cmd == "sd")
    {
      unsigned d;  is >> d;
      g.max_depth(d);
      continue;
    }
    if (cmd == "st")
    {
      int t;  is >> t;
      g.max_time(std::chrono::seconds(t));
      continue;
    }
    if (cmd == "time")
    {
      int t;  is >> t;
      g.time(std::chrono::milliseconds(t * 10));
      continue;
    }
    if (cmd == "undo")
    {
      g.take_back();
      continue;
    }

    const move m(g.current_state().parse_move(cmd));
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
  testudo::log::setup_stream("testudo");

  testudo::CECP::loop();
}
#endif
