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
  CHECK(file(square(A8)) == FILE_A);
  CHECK(file(square(B7)) == FILE_B);
  CHECK(file(square(C6)) == FILE_C);
  CHECK(file(square(D5)) == FILE_D);
  CHECK(file(square(E4)) == FILE_E);
  CHECK(file(square(F3)) == FILE_F);
  CHECK(file(square(G2)) == FILE_G);
  CHECK(file(square(H1)) == FILE_H);

  CHECK(rank(square(A8)) == 7);
  CHECK(rank(square(B7)) == 6);
  CHECK(rank(square(C6)) == 5);
  CHECK(rank(square(D5)) == 4);
  CHECK(rank(square(E4)) == 3);
  CHECK(rank(square(F3)) == 2);
  CHECK(rank(square(G2)) == 1);
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
  CHECK(seventh_rank(BLACK) == second_rank(WHITE));
  CHECK(seventh_rank(WHITE) == second_rank(BLACK));
  CHECK(eighth_rank(BLACK) == first_rank(WHITE));
  CHECK(eighth_rank(WHITE) == first_rank(BLACK));

  CHECK(flip(G1) == G8);
  CHECK(flip(G8) == G1);
  CHECK(flip(B1) == B8);
  CHECK(flip(B8) == B1);

  CHECK(step_fwd(BLACK) == -step_fwd(WHITE));
  CHECK(E2 + 2 * step_fwd(WHITE) == E4);
  CHECK(E7 + 2 * step_fwd(BLACK) == E5);

  for (square i(0); i < 64; ++i)
    CHECK(rank(i) == rank(WHITE, i));

  for (square i(A8); i <= H8; ++i)
  {
    CHECK(rank(BLACK, i) == 0);
    CHECK(rank(WHITE, i) == 7);
  }
  for (square i(A7); i <= H7; ++i)
  {
    CHECK(rank(BLACK, i) == 1);
    CHECK(rank(WHITE, i) == 6);
  }
  for (square i(A2); i <= H2; ++i)
  {
    CHECK(rank(WHITE, i) == 1);
    CHECK(rank(BLACK, i) == 6);
  }
  for (square i(A1); i <= H1; ++i)
  {
    CHECK(rank(WHITE, i) == 0);
    CHECK(rank(BLACK, i) == 7);
  }

  CHECK(to_square(FILE_A, 1) == A2);
  CHECK(to_square(FILE_E, 3) == E4);
  CHECK(to_square(FILE_H, 9) == NO_SQ);
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

  CHECK(BPAWN.offsets().size()   == 2);
  CHECK(WPAWN.offsets().size()   == 2);
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

  foreach_game(100, s,
               [](const state &pos, const move &m)
               {
                 if (is_capture(m))
                 {
                   CHECK(!(m.flags & move::castle));
                   CHECK(!(m.flags & move::two_squares));
                 }

                 if (is_promotion(m))
                 {
                   CHECK((m.flags & move::pawn));
                   CHECK(rank(m.from) == seventh_rank(pos.side()));
                 }

                 if (is_quiet(m))
                 {
                   CHECK(!is_capture(m));
                   CHECK(!is_promotion(m));
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
               });
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

}  // TEST_SUITE "BASE"

TEST_SUITE("EVAL")
{
// Verify phase range.
TEST_CASE("eval_phase")
{
  for (const auto &test : test_set())
    foreach_game(100, test.state,
                 [](const state &pos, const move &)
                 {
                   score_vector e(pos);

                   CHECK(0 <= e.phase);
                   CHECK(e.phase <= 256);
                 });
}

// Verify that evaluation results are symmetrical between White and Black side.
TEST_CASE("eval_flip")
{
  for (const auto &test : test_set())
    foreach_game(100, test.state,
                 [](const state &pos, const move &)
                 {
                   const auto v(eval(pos));

                   auto pos1(pos);
                   pos1.switch_side();
                   const auto v1(eval(pos1));
                   CHECK(v == -v1);

                   // A side to move relative, bug free static evaluation
                   // should have the same score for both positions
                   const auto pos2(pos.color_flip());
                   const auto v2(eval(pos2));
                   CHECK(v == v2);
                 });
}

TEST_CASE("king_shield")
{
  // Cannot castle anymore: just consider the current pawn shield (full
  // shield).
  const state s1("4k3/3ppp2/8/8/8/8/5PPP/6K1 w - -");
  const score_vector sv1(s1);
  CHECK(sv1.king_shield[WHITE] == 3 * db.pawn_shield1());
  CHECK(sv1.king_shield[BLACK] == 3 * db.pawn_shield1());

  // Cannot castle anymore: just consider the current pawn shield (almost full
  // shield).
  const state s2("4k3/3p1p2/4p3/8/8/6P1/5P1P/6K1 w - -");
  const score_vector sv2(s2);
  CHECK(sv2.king_shield[WHITE] == 2 * db.pawn_shield1() + db.pawn_shield2());
  CHECK(sv2.king_shield[BLACK] == 2 * db.pawn_shield1() + db.pawn_shield2());

  // Player can castle:
  // - if castling is favourable take the average of the current position and
  //   the after castling position;
  // - if castling isn't favourable just consider the current situation.
  const state s3("4k3/3ppp2/8/8/8/8/PPP5/4K3 w kQ -");
  const score_vector sv3(s3);
  CHECK(sv3.king_shield[WHITE] == 3 * db.pawn_shield1() / 2);
  CHECK(sv3.king_shield[BLACK] == 3 * db.pawn_shield1());

  const state s4("4k3/5p2/4p3/3p4/8/8/PPP5/4K3 w Q -");
  const score_vector sv4(s4);
  CHECK(sv4.king_shield[WHITE] == 3 * db.pawn_shield1() / 2);
  CHECK(sv4.king_shield[BLACK] == db.pawn_shield1() + db.pawn_shield2());
}

TEST_CASE("pawn_structure")
{
  const state s1("8/8/8/8/8/8/P7/K6k w - -");
  const score_vector sv1(s1);
  CHECK(sv1.pawns_e[WHITE] == db.pawn_passed_e(1) + db.pawn_weak_e(FILE_A));
  CHECK(sv1.pawns_m[WHITE]
        == db.pawn_passed_m(1) + db.pawn_weak_open_m(FILE_A));

  const state s2("8/P7/8/8/8/8/8/K6k w - -");
  const score_vector sv2(s2);
  CHECK(sv2.pawns_e[WHITE] == db.pawn_passed_e(6) + db.pawn_weak_e(FILE_A));
  CHECK(sv2.pawns_m[WHITE]
        == db.pawn_passed_m(6) + db.pawn_weak_open_m(FILE_A));

  const state s3("8/8/8/8/8/Pp6/1P6/K6k w - -");
  const score_vector sv3(s3);
  CHECK(sv3.pawns_e[WHITE]
        == db.pawn_protected_passed_e(2) + db.pawn_weak_e(FILE_B));
  CHECK(sv3.pawns_m[WHITE]
        == db.pawn_passed_m(2) + db.pawn_weak_m(FILE_B));

  const state s4("8/Pp6/1P/8/8/8/8/K6k w - -");
  const score_vector sv4(s4);
  CHECK(sv4.pawns_e[WHITE]
        == db.pawn_protected_passed_e(6) + db.pawn_weak_e(FILE_B));
  CHECK(sv4.pawns_m[WHITE]
        == db.pawn_passed_m(6) + db.pawn_weak_m(FILE_B));

  const state s5("8/8/Pp6/8/8/8/1P/K6k w - -");
  const score_vector sv5(s5);
  CHECK(sv5.pawns_e[WHITE]
        == db.pawn_passed_e(5) + db.pawn_weak_e(FILE_B));
  CHECK(sv5.pawns_m[WHITE]
        == db.pawn_passed_m(5) + db.pawn_weak_m(FILE_B));

  const state s6("8/8/8/PP/8/8/8/K6k w - -");
  const score_vector sv6(s6);
  CHECK(sv6.pawns_e[WHITE] == 2 * db.pawn_protected_passed_e(4));
  CHECK(sv6.pawns_m[WHITE] == 2 * db.pawn_passed_m(4));

  const state s7("8/8/3p4/3P4/3P4/8/8/K6k w - -");
  const score_vector sv7(s7);
  CHECK(sv7.pawns_e[WHITE]
        == 2 * db.pawn_weak_e(FILE_D) + db.pawn_doubled_e());
  CHECK(sv7.pawns_m[WHITE]
        == 2 * db.pawn_weak_m(FILE_D) + db.pawn_doubled_m());

  const state s8("8/8/8/3P4/3P4/8/8/K6k w - -");
  const score_vector sv8(s8);
  CHECK(sv8.pawns_e[WHITE]
        == 2 * db.pawn_weak_e(FILE_D) + db.pawn_passed_e(4)
           + db.pawn_doubled_e());
  CHECK(sv8.pawns_m[WHITE]
        == 2 * db.pawn_weak_open_m(FILE_D) + db.pawn_passed_m(4)
           + db.pawn_doubled_m());

  const state s8b("7r/8/8/3P4/3P4/8/8/K6k w - -");
  const score_vector sv8b(s8b);
  CHECK(sv8b.pawns_e[WHITE]
        == 2 * db.pawn_weak_open_e(FILE_D) + db.pawn_passed_e(4)
           + db.pawn_doubled_e());
  CHECK(sv8b.pawns_m[WHITE]
        == 2 * db.pawn_weak_open_m(FILE_D) + db.pawn_passed_m(4)
           + db.pawn_doubled_m());

  const state s9("8/1p6/8/3P4/3P4/2P5/8/K6k w - -");
  const score_vector sv9(s9);
  CHECK(sv9.pawns_e[WHITE]
        == db.pawn_passed_e(4) + db.pawn_doubled_e() + db.pawn_weak_e(FILE_C));
  CHECK(sv9.pawns_m[WHITE]
        == db.pawn_passed_m(4) + db.pawn_doubled_m()
           + db.pawn_weak_open_m(FILE_C));

  const state s10("8/8/8/8/8/1PP5/8/K6k w - -");
  const score_vector sv10(s10);
  CHECK(sv10.pawns_e[WHITE] == 2 * db.pawn_protected_passed_e(2));
  CHECK(sv10.pawns_m[WHITE] == 2 * db.pawn_passed_m(2));
}

}  // TEST_SUITE "EVAL"

TEST_SUITE("SEARCH")
{

TEST_CASE("search_with_no_move_available")
{
  const state p("8/8/8/5K1k/8/8/8/7R b - -");

  cache tt;
  search s({p}, &tt);

  const move m(s.run(true));
  CHECK(!m);
}

TEST_CASE("draw_position")
{
  // From a game with TSCP.
  const state p("8/6pk/1p3pQp/q4P2/2PP4/r1PKP2P/p7/R7 b - - 14 55");

  cache tt;
  search s({p}, &tt);
  s.max_depth = 9;

  s.run(true);
  CHECK(s.stats.score_at_root == 0);
}

TEST_CASE("draw_position2")
{
  // From a game with TSCP.
  const state p("q7/6k1/1p4p1/3p4/2pP1Q1P/p1P1PK2/2P4P/8 w - - 8 61");

  cache tt;
  search s({p}, &tt);
  s.max_depth = 10;

  s.run(true);
  CHECK(s.stats.score_at_root == 0);
}

TEST_CASE("draw_position3")
{
  // From <http://www.talkchess.com/forum/viewtopic.php?t=64800> (Uri Blass)
  const state p("k1b5/1p1p1p1p/pPpPpPpP/P1P1P1P1/8/8/8/K1B5 w - - 0 1 ");

  cache tt;
  search s({p}, &tt);
  s.max_depth = 102;

  s.run(true);
  CHECK(s.stats.score_at_root == 0);
}

}  // TEST_SUITE "SEARCH"

TEST_CASE("SAN")
{
  const state s(state::setup::start);
  const auto moves(s.moves());

  CHECK(std::find(moves.begin(), moves.end(), SAN::from("d4", s))
        != moves.end());
  CHECK(std::find(moves.begin(), moves.end(), SAN::from("e4", s))
        != moves.end());
  CHECK(std::find(moves.begin(), moves.end(), SAN::from("Nc3", s))
        != moves.end());
  CHECK(std::find(moves.begin(), moves.end(), SAN::from("Nf3", s))
        != moves.end());
  CHECK(!SAN::from("Bc4", s));
}

TEST_SUITE("ADVANCED" * doctest::skip())
{

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

TEST_CASE("quiescence_search_explosion")
{
  // See <http://www.talkchess.com/forum/viewtopic.php?t=63590>
  const state p("1QqQqQq1/r6Q/Q6q/q6Q/B2q4/q6Q/k6K/1qQ1QqRb w - -");

  cache tt;
  search s({p}, &tt);
  s.max_depth = 1;

  const move m(s.run(true));
  CHECK(m);
}

}  // TEST_SUITE "ADVANCED"
