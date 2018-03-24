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

#include <fstream>
#include <sstream>

namespace testudo
{

bool test(const std::string &epd, const search::constraints &)
{
  std::ifstream f(epd);
  if (!f)
    return false;

  std::size_t count(0);
  std::string line;
  while (std::getline(f, line))
  {
    std::istringstream ss(line);
    ++count;

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
        id = std::to_string(count);

      testudoDEBUG << epd << " testset - position " << id << " read";
    }

    cache tt(21);
    search s({pos}, &tt);

    s.constraint.max_time  = std::chrono::milliseconds(0);
    s.constraint.max_depth = 6;

    testudoOUTPUT << "Analyzing " << id;
    const auto m(s.run(true));
  }

  return true;
}

}  // namespace testudo
