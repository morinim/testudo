/**
 *  \file
 *  \remark This file is part of Testudo.
 *
 *  \copyright Copyright (C) 2018 Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "engine/testudo.h"

// Runs a simple six-position benchmark to gauge performance. The test
// positions are hard-coded and the benchmark is calculated much like it would
// with an external "test" file. The test is a mix of opening, middlegame and
// endgame positions, with both tactical and positional aspects. This test is a
// speed measure only; the actual solutions to the positions are ignored.
std::chrono::seconds bench()
{
  using namespace testudo;
  using namespace std::chrono;

  struct test_elem
  {
    testudo::state state;
    unsigned depth;
  };

  const std::vector<test_elem> db(
  {
    {
      // Bratko-Kopec 2
      state("3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - -"),
      8
    },
    {
      // Bratko-Kopec 4
      state("rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq -"),
      8
    },
    {
      // Bratko-Kopec 8
      state("4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - -"),
      12
    },
    {
      // Bratko-Kopec 12
      state("r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - -"),
      8
    },
    {
      // Bratko-Kopec 22
      state("2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 b - -"),
      7
    },
    {
      // Bratko-Kopec 23
      state("r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq -"),
      7
    }
  });

  std::cout << "Running benchmark...\n\n";

  seconds total_time(0);
  std::uintmax_t snodes(0), qnodes(0);

  for (const auto &p : db)
  {
    std::cout << p.state;
    cache tt(20);

    timer t;
    search s({p.state}, &tt);

    s.max_time  = milliseconds(0);
    s.max_depth = p.depth;

    s.run(true);
    total_time += duration_cast<seconds>(t.elapsed());
    snodes += s.stats.snodes;
    qnodes += s.stats.qnodes;
    std::cout << '\n';
  }

  std::cout << "\nTotal time: " << total_time.count() << "s\n"
            << "Nodes: " << snodes + qnodes
            << " (search: " << snodes
            << ", quiescence: " << qnodes << ")\n"
            << "NPS: " << (snodes + qnodes) / total_time.count() << '\n';

  return total_time;
}

int main()
{
  bench().count();
}
