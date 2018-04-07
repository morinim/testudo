/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "test.h"
#include "cache.h"
#include "log.h"
#include "san.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace testudo
{

// Runs a suite of positions and produce a summary of how many it got right,
// wrong... It uses the time / depth / nodes constraints set in `c`.
// There is also an "early exit" counter: if the program finds and holds the
// solution move for `2` iterations, it will terminate the search. For
// absolutely correct results, this is not advisable as it could obviously
// change its mind later on but, for performance analysis, this saves a lot of
// time.
bool test(const std::string &epd, const search::constraints &c)
{
  std::ifstream f(epd);
  if (!f)
    return false;

  std::size_t positions(0), right(0);
  unsigned avg_depth(0);

  std::string line;
  while (std::getline(f, line))
  {
    std::istringstream ss(line);
    ++positions;

    std::string placement, stm, castling, ep;
    if (!(ss >> placement >> stm >> castling >> ep))
      return false;

    state pos(placement + " " + stm + " " + castling + " " + ep);
    std::vector<move> am;  // avoid move(s) vector
    std::vector<move> bm;  // best move(s) vector
    std::string id;

    std::string opcode;
    while (ss >> opcode)
    {
      std::vector<std::string> args;

      bool more_args(true);
      while (more_args)
      {
        std::string arg;
        if (!(ss >> arg))
          return false;

        if (arg.back() == ';')
        {
          arg = arg.substr(0, arg.length() - 1);
          more_args = false;
        }

        args.push_back(arg);
      }

      if (opcode == "am")
        for (const auto &arg : args)
          am.push_back(SAN::from(arg, pos));
      else if (opcode == "bm")
        for (const auto &arg : args)
          bm.push_back(SAN::from(arg, pos));
      else if (opcode == "id")
        id = args[0];
      else {}

      if (id.empty())
        id = std::to_string(positions);

      testudoDEBUG << epd << " testset - position " << id << " read";
    }

    const auto right_move(
      [&](const move &m)
      {
        return std::find(bm.begin(), bm.end(), m) != bm.end()
               && std::find(am.begin(), am.end(), m) == am.end();
      });

    cache tt(21);
    search s({pos}, &tt);
    unsigned correct_for(0);

    s.constraint = c;
    s.constraint.condition =  // early exit condition
      [&]()
      {
        if (right_move(s.stats.moves_at_root.front()))
          ++correct_for;
        else
          correct_for = 0;

        return correct_for >= 2 && s.stats.depth >= 5;
      };

    testudoOUTPUT << "Analyzing " << id;
    const auto m(s.run(true));

    if (right_move(m))
    {
      ++right;
      testudoOUTPUT << "! (" << right << '/' << positions << " = "
                    << static_cast<unsigned>(right * 100 / positions) << "%)";
    }

    avg_depth += s.stats.depth;
  }

  testudoOUTPUT << epd << " tested";
  testudoOUTPUT << "Results: " << right << '/' << positions;
  testudoOUTPUT << "Average depth: " << avg_depth / positions;

  return true;
}

}  // namespace testudo
