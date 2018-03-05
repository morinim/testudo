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

#include <fstream>
#include <iomanip>

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
      9
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

  struct result
  {
    seconds time = seconds(0);
    std::uintmax_t snodes = 0;
    std::uintmax_t qnodes = 0;
    move best_move = move::sentry();
    score val = 0;
  };
  std::vector<result> results;

  std::cout << "Running benchmark...\n\n";

  for (const auto &p : db)
  {
    std::cout << p.state;
    cache tt(21);

    timer t;
    search s({p.state}, &tt);

    s.max_time  = milliseconds(0);
    s.max_depth = p.depth;

    const auto m(s.run(true));

    results.push_back({duration_cast<seconds>(t.elapsed()),
          s.stats.snodes, s.stats.qnodes, m, s.stats.score_at_root});

    std::cout << '\n';
  }

  std::ofstream out("bench.csv");

  const auto nps(
    [](auto nodes, seconds t)
    {
      return nodes / std::max<unsigned>(1, t.count());
    });

  const auto print(
    [&](const result &r)
    {
      std::cout << std::right << std::setw(6) << std::setfill(' ')
                << r.time.count() << "s "
                << std::right << std::setw(12) << std::setfill(' ') << r.snodes
                << ' '
                << std::right << std::setw(12) << std::setfill(' ') << r.qnodes
                << ' '
                << std::right << std::setw(8) << std::setfill(' ')
                << nps(r.snodes + r.qnodes, r.time) << ' '
                << std::left << std::setw(6) << std::setfill(' ')
                << r.best_move << ' '
                << std::right << std::setw(6) << std::setfill(' ') << r.val
                << '\n';

      if (!r.best_move.is_sentry())
        out << r.time.count() << ','
            << r.snodes << ',' << r.qnodes << ','
            << nps(r.snodes + r.qnodes, r.time) << ','
            << r.best_move << ',' << r.val << '\n';
    });

  int n(0);
  seconds total_time(0);
  std::uintmax_t snodes(0), qnodes(0);
  score val(0);
  for (const auto &r : results)
  {
    print(r);

    total_time += r.time;
    snodes += r.snodes;
    qnodes += r.qnodes;
    val += r.val;

    ++n;
  }

  val = val / n;

  std::cout << std::string(70, '-') << '\n';
  print(result{total_time, snodes, qnodes, move::sentry(), val});

  return total_time;
}

int main()
{
  bench().count();
}
