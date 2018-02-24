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

#include <set>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "engine/testudo.h"
using namespace testudo;

struct fen_test_case
{
  ::state state;
  std::vector<std::uintmax_t> moves;
  std::vector<std::uintmax_t> captures;
};

const std::vector<fen_test_case> &test_set()
{
  // Directly accessing the `ts` array can be problematic (because of static
  // initialization order fiasco).
  static const std::vector<fen_test_case> ts(
  {
    {
      state("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"),
      {20, 400, 8902, 197281, /*4865609, 119060324, 3195901860, 84998978956*/},
      {0, 0, 34, 1576, /* 82719, 2812008 */}
    },
    {
      state(  // so called 'Kiwipete' by Peter McKenzie
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"),
      {48, 2039, 97862, /* 4085603, 193690690 */},
      {8, 351, 17102, /* 757163, 35043416 */}
    },
    {
      state("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"),
      {14, 191, 2812, 43238, 674624, /* 11030083, 178633661 */},
      {1, 14, 209, 3348, 52051, /* 940350, 14519036 */}
    },
    {
      state(  // by Steven Edwards
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -"),
      {6, 264, 9467, 422333, /* 15833292, 706045033 */},
      {0, 87, 1021, 131393, /* 2046173, 210369132 */}
    },
    {
      state("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"),
      {44, 1486, 62379, /* 2103487, 89941194 */},
      {}
    },
    {
      state(  // by Steven Edwards
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1"
        " w - - 0 10"),
      {46, 2079, 89890, /* 3894594, 164075551, 6923051137, 287188994746 */},
      {}
    },
    {
      state(  // Position with 218 legal moves (!) reported by Scott Gasch
        "3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/1Q4Rp/1K1BBNNk w - -"),
      {218},
      {}
    }
  });

  return ts;
}

enum class perft_type {all, capture};

template<perft_type T>
std::uintmax_t perft(const state &s, unsigned depth, unsigned print = 10000)
{
  movelist moves;

  if (depth == 1)
  {
    moves = (T == perft_type::all) ? s.moves() : s.captures();
    return moves.size();
  }
  else
    moves = s.moves();

  std::uintmax_t nodes(0);
  for (const auto &m : moves)
  {
    const state s1(s.after_move(m));
    const auto partial(perft<T>(s1, depth - 1));

    if (depth == print)
      std::cout << m << "  " << partial << '\n';

    nodes += partial;
  }

  return nodes;
}

bool hash_tree(const state &s, unsigned depth)
{
  if (!depth)
    return s.hash() == zobrist::hash(s);

  for (const auto &m : s.moves())
  {
    const state s1(s.after_move(m));

    if (!hash_tree(s1, depth - 1))
      return false;
  }

  return true;
}

template<class F>
void foreach_game(unsigned n, state pos, F f)
{
  for (unsigned i(0); i < n; ++i)
  {
    std::vector<hash_t> previous_states({pos.hash()});

    for (int mn(0);
         pos.mate_or_draw(&previous_states) == state::kind::standard;
         ++mn)
    {
      const auto moves(pos.moves());
      const auto rm(random::element(moves));

      f(pos, rm);

      CHECK(pos.make_move(rm));

      previous_states.push_back(pos.hash());
    }
  }
}


TEST_SUITE("BASE")
{

TEST_CASE("color")
{
  CHECK(BLACK == 0);
  CHECK(WHITE == 1);

  CHECK(!BLACK == WHITE);
  CHECK(!WHITE == BLACK);
}

TEST_CASE("square")
{
  CHECK(file(square(A8)) == 0);
  CHECK(file(square( 9)) == 1);
  CHECK(file(square(18)) == 2);
  CHECK(file(square(27)) == 3);
  CHECK(file(square(36)) == 4);
  CHECK(file(square(45)) == 5);
  CHECK(file(square(54)) == 6);
  CHECK(file(square(H1)) == 7);

  CHECK(rank(square(A8)) == 7);
  CHECK(rank(square( 9)) == 6);
  CHECK(rank(square(18)) == 5);
  CHECK(rank(square(27)) == 4);
  CHECK(rank(square(36)) == 3);
  CHECK(rank(square(45)) == 2);
  CHECK(rank(square(54)) == 1);
  CHECK(rank(square(H1)) == 0);

  for (square i(0); i < 64; ++i)
  {
    CHECK(file(i) < 8);
    CHECK(rank(i) < 8);
    valid(i);
  }

  CHECK(valid(A1));
  CHECK(valid(B1));
  CHECK(valid(C1));
  CHECK(valid(D1));
  CHECK(valid(E1));
  CHECK(valid(F1));
  CHECK(valid(G1));
  CHECK(valid(H1));
  CHECK(valid(A8));
  CHECK(valid(B8));
  CHECK(valid(C8));
  CHECK(valid(D8));
  CHECK(valid(E8));
  CHECK(valid(F8));
  CHECK(valid(G8));
  CHECK(valid(H8));
  CHECK(!valid(-1));

  CHECK(first_rank(BLACK) == rank(A8));
  CHECK(first_rank(WHITE) == rank(A1));
  CHECK(second_rank(BLACK) == rank(A7));
  CHECK(second_rank(WHITE) == rank(A2));
  CHECK(seventh_rank(BLACK) == rank(A2));
  CHECK(seventh_rank(WHITE) == rank(A7));
  CHECK(eighth_rank(BLACK) == rank(A1));
  CHECK(eighth_rank(WHITE) == rank(A8));

}

TEST_CASE("piece")
{
  CHECK(EMPTY.color() != BLACK);
  CHECK(EMPTY.color() != WHITE);

  CHECK(  !BPAWN.slide());
  CHECK(  !WPAWN.slide());
  CHECK(!BKNIGHT.slide());
  CHECK(!WKNIGHT.slide());
  CHECK( BBISHOP.slide());
  CHECK( WBISHOP.slide());
  CHECK(   BROOK.slide());
  CHECK(   WROOK.slide());
  CHECK(  BQUEEN.slide());
  CHECK(  WQUEEN.slide());
  CHECK(  !BKING.slide());
  CHECK(  !WKING.slide());

  CHECK(BPAWN.offsets().size()   == 0);
  CHECK(WPAWN.offsets().size()   == 0);
  CHECK(BKNIGHT.offsets().size() == 8);
  CHECK(WKNIGHT.offsets().size() == 8);
  CHECK(BBISHOP.offsets().size() == 4);
  CHECK(WBISHOP.offsets().size() == 4);
  CHECK(BROOK.offsets().size()   == 4);
  CHECK(WROOK.offsets().size()   == 4);
  CHECK(BQUEEN.offsets().size()  == 8);
  CHECK(WQUEEN.offsets().size()  == 8);
  CHECK(BKING.offsets().size()   == 8);
  CHECK(WKING.offsets().size()   == 8);

  CHECK(BPAWN.color()   == BLACK);
  CHECK(BKNIGHT.color() == BLACK);
  CHECK(BBISHOP.color() == BLACK);
  CHECK(BROOK.color()   == BLACK);
  CHECK(BQUEEN.color()  == BLACK);
  CHECK(BKING.color()   == BLACK);
  CHECK(WPAWN.color()   == WHITE);
  CHECK(WKNIGHT.color() == WHITE);
  CHECK(WBISHOP.color() == WHITE);
  CHECK(WROOK.color()   == WHITE);
  CHECK(WQUEEN.color()  == WHITE);
  CHECK(WKING.color()   == WHITE);

  CHECK(BPAWN.type()   == WPAWN.type());
  CHECK(BKNIGHT.type() == WKNIGHT.type());
  CHECK(BBISHOP.type() == WBISHOP.type());
  CHECK(BROOK.type()   == WROOK.type());
  CHECK(BQUEEN.type()  == WQUEEN.type());
  CHECK(BKING.type()   == WKING.type());

  CHECK(BPAWN.letter()   == 'p');
  CHECK(BKNIGHT.letter() == 'n');
  CHECK(BBISHOP.letter() == 'b');
  CHECK(BROOK.letter()   == 'r');
  CHECK(BQUEEN.letter()  == 'q');
  CHECK(BKING.letter()   == 'k');
  CHECK(WPAWN.letter()   == 'P');
  CHECK(WKNIGHT.letter() == 'N');
  CHECK(WBISHOP.letter() == 'B');
  CHECK(WROOK.letter()   == 'R');
  CHECK(WQUEEN.letter()  == 'Q');
  CHECK(WKING.letter()   == 'K');

  CHECK(BPAWN.value()   ==   WPAWN.value());
  CHECK(BKNIGHT.value() == WKNIGHT.value());
  CHECK(BBISHOP.value() == WBISHOP.value());
  CHECK(BROOK.value()   ==   WROOK.value());
  CHECK(BQUEEN.value()  ==  WQUEEN.value());
  CHECK(BKING.value()   ==   WKING.value());

  CHECK(EMPTY.value()   <    BPAWN.value());
  CHECK(BPAWN.value()   <  BKNIGHT.value());
  CHECK(BKNIGHT.value() <= BBISHOP.value());
  CHECK(BBISHOP.value() <    BROOK.value());
  CHECK(BROOK.value()   <   BQUEEN.value());
  CHECK(BQUEEN.value()  <    BKING.value());
}

TEST_CASE("state")
{
  const state start(state::setup::start);

  CHECK(start[A1] == WROOK);
  CHECK(start[B1] == WKNIGHT);
  CHECK(start[C1] == WBISHOP);
  CHECK(start[D1] == WQUEEN);
  CHECK(start[E1] == WKING);
  CHECK(start[F1] == WBISHOP);
  CHECK(start[G1] == WKNIGHT);
  CHECK(start[H1] == WROOK);

  CHECK(start[A8] == BROOK);
  CHECK(start[B8] == BKNIGHT);
  CHECK(start[C8] == BBISHOP);
  CHECK(start[D8] == BQUEEN);
  CHECK(start[E8] == BKING);
  CHECK(start[F8] == BBISHOP);
  CHECK(start[G8] == BKNIGHT);
  CHECK(start[H8] == BROOK);

  for (square i(0); i < 64; ++i)
    if (rank(i) == 1)
      CHECK(start[i] == WPAWN);
    else if (rank(i) == 6)
      CHECK(start[i] == BPAWN);
    else if (1 < rank(i) && rank(i) < 6)
      CHECK(start[i] == EMPTY);

  CHECK(start.side() == WHITE);
  CHECK(start.fifty() == 0);

  const auto moves(start.moves());
  CHECK(moves.size() == 20);

  state start1(
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
  CHECK(start == start1);

  start1.switch_side();
  CHECK(start1.side() == BLACK);
  start1.switch_side();
  CHECK(start == start1);
}

TEST_CASE("move")
{
  const state s(
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

  for (const auto &m : s.moves())
  {
    if (is_capture(m))
    {
      CHECK(!(m.flags & move::castle));
      CHECK(!(m.flags & move::two_squares));
    }

    if (m.flags & move::castle)
    {
      CHECK(!(m.flags & move::en_passant));
      CHECK(!(m.flags & move::two_squares));
      CHECK(!(m.flags & move::pawn));
      CHECK(!is_promotion(m));
    }

    if (m.flags & move::en_passant)
    {
      CHECK(is_capture(m));
      CHECK(!(m.flags & move::two_squares));
      CHECK((m.flags & move::pawn));
      CHECK(!is_promotion(m));
    }

    if (m.flags & move::two_squares)
    {
      CHECK(!is_promotion(m));
    }

    CHECK(!m.is_sentry());
  }
}

TEST_CASE("perft")
{
  for (const auto &test : test_set())
    for (unsigned i(0); i < test.moves.size(); ++i)
      CHECK(perft<perft_type::all>(test.state, i + 1) == test.moves[i]);
}

TEST_CASE("perft_captures")
{
  for (const auto &test : test_set())
    for (unsigned i(0); i < test.captures.size(); ++i)
      CHECK(perft<perft_type::capture>(test.state, i + 1) == test.captures[i]);
}

TEST_CASE("is_legal")
{
  foreach_game(100, state(state::setup::start),
               [](const state &pos, const move &m)
               {
                 CHECK(pos.is_legal(m));

                 // A legal move must have the correct flags.
                 for (unsigned i(0); i < 8; ++i)
                 {
                   move m1(m);
                   m1.flags ^= (1 << i);
                   CHECK(!pos.is_legal(m1));
                 }
               });
}

TEST_CASE("hash_values")
{
  std::set<hash_t> seen;

  for (unsigned p(BPAWN.id()); p <= WQUEEN.id(); ++p)
    for (square i(0); i < 64; ++i)
    {
      const auto h(zobrist::piece[p][i]);

      CHECK(h != 0);
      CHECK(seen.find(h) == seen.end());

      seen.insert(h);
    }

  CHECK(zobrist::side != 0);
  CHECK(seen.find(zobrist::side) == seen.end());
  seen.insert(zobrist::side);

  for (unsigned i(0); i < 8; ++i)
  {
    CHECK(zobrist::ep[i] != 0);
    CHECK(seen.find(zobrist::ep[i]) == seen.end());
    seen.insert(zobrist::ep[i]);
  }

  for (unsigned i(0); i < 16; ++i)
  {
    CHECK(zobrist::castle[i] != 0);
    CHECK(seen.find(zobrist::castle[i]) == seen.end());
    seen.insert(zobrist::castle[i]);
  }

  const state s(state::setup::start);
  CHECK(s.hash() == zobrist::hash(s));
}

TEST_CASE("hash_update")
{
  for (const auto &test : test_set())
    CHECK(hash_tree(test.state, test.moves.size()));
}

TEST_CASE("hash_store_n_probe")
{
  cache tt(20);

  foreach_game(100, state(state::setup::start),
               [&tt](const state &pos, const move &m)
               {
                 tt.insert(pos.hash(), m, pos.hash() & 0xFF,
                           score_type::exact, pos.hash() & 0xFFF);

                 const auto *slot(tt.find(pos.hash()));

                 // With an always-replace strategy, the last element inserted
                 // is always available.
                 CHECK(slot);
                 CHECK(slot->hash() == pos.hash());
                 CHECK(slot->best_move() == m);
                 CHECK(slot->draft() == (pos.hash() & 0xFF));
                 CHECK(slot->type() == score_type::exact);
                 CHECK(slot->value() == (pos.hash() & 0xFFF));
               });
}

// Verify that evaluation results are symmetrical between White and Black side
TEST_CASE("eval_flip")
{
  for (const auto &test : test_set())
    foreach_game(10, test.state,
                 [](const state &pos, const move &)
                 {
                   const auto v(eval(pos));

                   auto pos1(pos);
                   pos1.switch_side();
                   const auto v1(eval(pos1));

                   CHECK(v == -v1);
                 });
}

TEST_CASE("search_with_no_move_available")
{
  const state p("8/8/8/5K1k/8/8/8/7R b - -");

  cache tt;
  search s({p}, &tt);

  const move m(s.run(true));
  CHECK(!m);
}

}  // TEST_SUITE "BASE"

TEST_SUITE("ADVANCED" * doctest::skip())
{
/*
TEST_CASE("search")
{
  const state p1(
    "8/8/8/8/k7/8/5KP1/1Q6 w - - 1 63");

  cache tt;
  search s({p1}, &tt);

  s.max_time = std::chrono::milliseconds(3000000);

  const move m(s.run(true));
  std::cout << m << std::endl;
}
*/

TEST_CASE("transposition_table")
{
  // The Lasker-Reichhelm Position,composed by World Champion Emanuel Lasker
  // and Gustavus Charles Reichhelm in 1901, is most famous to solve with the
  // method of corresponding squares. It's also a test-position for the
  // efficiency of search tables, most notably the transposition table, where
  // most of today's programs find the only winning move Kb1 with an
  // appropriate (winning) score in less than one second. However, it's not
  // recommended to tune replacement schemes purely based on this position.
  const state fine70("8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -");
  //const state fine70("4k3/8/8/8/8/8/4P3/4K3 w - -");

  cache tt(21);
  search s({fine70}, &tt);

  s.max_time = std::chrono::milliseconds(10000);

  const move m(s.run(true));
  CHECK(m == move(A1, B1, 0));
}

}  // TEST_SUITE "ADVANCED"
