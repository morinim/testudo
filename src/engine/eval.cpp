/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "eval.h"
#include "parameters.h"

namespace testudo
{

void eval_king_shield(const state &s, score_vector &e)
{
  const auto shelter_file(
    [&s](color c, square sq)
    {
      assert(valid(sq));

      const piece pawn(piece(c, piece::pawn));

      assert(valid(sq + step_fwd(c)));
      if (s[sq + step_fwd(c)] == pawn)
        return db.pawn_shield1();

      assert(valid(sq + 2 * step_fwd(c)));
      if (s[sq + 2 * step_fwd(c)] == pawn)
        return db.pawn_shield2();

      return 0;
    });

  const auto shelter_square(
    [&shelter_file](color c, square sq)
    {
      assert(valid(sq));

      const auto r(rank(sq));
      if (r == first_rank(c) || r == second_rank(c))
      {
        score ret(shelter_file(c, sq));
        if (file(sq) > FILE_A)
          ret += shelter_file(c, sq - 1);
        if (file(sq) < FILE_H)
          ret += shelter_file(c, sq + 1);

        return ret;
      }

      return 0;
    });

  for (unsigned c(0); c < 2; ++c)
  {
    const score bonus_here(shelter_square(c, s.king_square(c)));
    score bonus_castle(bonus_here);

    // If castling is:
    // - favourable, take the average of the current position and the after
    //   castling position;
    // - not favourable, just consider the current situation.

    if (s.kingside_castle(c))
    {
      const score tmp(shelter_square(c, c == WHITE ? G1 : G8));
      if (tmp > bonus_castle)
        bonus_castle = tmp;
    }
    if (s.queenside_castle(c))
    {
      const score tmp(shelter_square(c, c == WHITE ? B1 : B8));
      if (tmp > bonus_castle)
        bonus_castle = tmp;
    }

    e.king_shield[c] = (bonus_here + bonus_castle) / 2;
  }
}

void eval_pawn_e(const state &s, square i, score_vector &e)
{
  assert(s[i].type() == piece::pawn);
  const color c(s[i].color());
  const piece pawn(s[i]), xpawn(!c, piece::pawn);

  e.pcsq_e[c] += db.pcsq_e(pawn, i);

  bool is_passed(  true);  // we will be trying to disprove that
  bool is_opposed(false);

  // The loop below detects doubled pawns, passed pawns and sets a flag on
  // finding that our pawn is opposed by enemy pawn.
  for (square sq(i + step_fwd(c)); rank(c, sq) < 7; sq += step_fwd(c))
  {
    if (s[sq].type() == piece::pawn)
    {
      is_passed = false;

      if (s[sq] == pawn)
        e.pawns_e[c] += db.pawn_doubled_e();
      else
      {
        assert(s[sq] == xpawn);
        is_opposed = true;
        break;
      }
    }

    if (file(sq) > FILE_A && s[sq - 1] == xpawn)
      is_passed = false;
    else if (file(sq) < FILE_H && s[sq + 1] == xpawn)
      is_passed = false;
  }

  // Isolated and/or backward pawn.
  bool is_weak(true);  // we will be trying to disprove that

  // Going backwards and checking whether pawn has support.
  unsigned steps(0);
  for (square sq(i); rank(c, sq); sq -= step_fwd(c), ++steps)
  {
    if (file(sq) > FILE_A && s[sq - 1] == pawn)
    {
      is_weak = false;
      break;
    }
    if (file(sq) < FILE_H && s[sq + 1] == pawn)
    {
      is_weak = false;
      break;
    }
  }

  if (is_passed)
  {
    const bool is_directly_supported(!is_weak && steps <= 1);
    const auto r(rank(c, i));

    // In the endgame we score passed pawns higher if they are protected or if
    // their advance is supported by friendly pawns.
    if (is_directly_supported)
      e.pawns_e[c] += db.pawn_protected_passed_e(r);
    else
      e.pawns_e[c] += db.pawn_passed_e(r);
  }

  if (is_weak)
  {
    const auto f(file(i));
    if (!is_opposed
        && (s.piece_count(!c, piece::rook) || s.piece_count(!c, piece::queen)))
      e.pawns_e[c] += db.pawn_weak_open_e(f);
    else
      e.pawns_e[c] += db.pawn_weak_e(f);
  }
}

void eval_pawn_m(const state &s, square i, score_vector &e)
{
  assert(s[i].type() == piece::pawn);
  const color c(s[i].color());
  const piece pawn(s[i]), xpawn(!c, piece::pawn);

  e.pcsq_m[c] += db.pcsq_m(pawn, i);

  bool is_passed(  true);  // we will be trying to disprove that
  bool is_opposed(false);

  // The loop below detects doubled pawns, passed pawns and sets a flag on
  // finding that our pawn is opposed by enemy pawn.
  for (square sq(i + step_fwd(c)); rank(c, sq) < 7; sq += step_fwd(c))
  {
    if (s[sq].type() == piece::pawn)
    {
      is_passed = false;

      if (s[sq] == pawn)
        e.pawns_m[c] += db.pawn_doubled_m();
      else
      {
        assert(s[sq] == xpawn);
        is_opposed = true;
        break;
      }
    }

    if (file(sq) > FILE_A && s[sq - 1] == xpawn)
      is_passed = false;
    else if (file(sq) < FILE_H && s[sq + 1] == xpawn)
      is_passed = false;
  }

  // Isolated and/or backward pawn.
  bool is_weak(true);  // we will be trying to disprove that

  // Going backwards and checking whether pawn has support.
  unsigned steps(0);
  for (square sq(i); rank(c, sq); sq -= step_fwd(c), ++steps)
  {
    if (file(sq) > FILE_A && s[sq - 1] == pawn)
    {
      is_weak = false;
      break;
    }
    if (file(sq) < FILE_H && s[sq + 1] == pawn)
    {
      is_weak = false;
      break;
    }
  }

  if (is_passed)
    e.pawns_m[c] += db.pawn_passed_m(rank(c, i));

  if (is_weak)
  {
    const auto f(file(i));
    e.pawns_m[c] += is_opposed ? db.pawn_weak_m(f) : db.pawn_weak_open_m(f);
  }
}

void eval_e(const state &s, score_vector &e)
{
  for (square i(0); i < 64; ++i)
    if (s[i] != EMPTY)
      switch (s[i].id())
      {
      case BPAWN.id():
      case WPAWN.id():
        eval_pawn_e(s, i, e);
        break;

      default:
        e.pcsq_e[s[i].color()] += db.pcsq_e(s[i], i);
      }

  e.eg = e.pcsq_e[s.side()] - e.pcsq_e[!s.side()]
         + e.pawns_e[s.side()] - e.pawns_e[!s.side()];
}

void eval_m(const state &s, score_vector &e)
{
  for (square i(0); i < 64; ++i)
    if (s[i] != EMPTY)
      switch (s[i].id())
      {
      case BPAWN.id():
      case WPAWN.id():
        eval_pawn_m(s, i, e);
        break;

      default:
        e.pcsq_m[s[i].color()] += db.pcsq_m(s[i], i);
      }

  eval_king_shield(s, e);

  e.mg = e.pcsq_m[s.side()] - e.pcsq_m[!s.side()]
         + e.king_shield[s.side()] - e.king_shield[!s.side()]
         + e.pawns_m[s.side()] - e.pawns_m[!s.side()];
}

// Phase index: 0 is opening, 256 endgame.
int phase256(const state &s)
{
  constexpr int knight_phase = 1;
  constexpr int bishop_phase = 1;
  constexpr int rook_phase   = 2;
  constexpr int queen_phase  = 4;

  constexpr int total_phase =
    knight_phase * 4 + bishop_phase * 4 + rook_phase * 4 + queen_phase * 2;

  int p(total_phase);

  p -= s.piece_count(BLACK, piece::knight);
  p -= s.piece_count(BLACK, piece::bishop);
  p -= s.piece_count(BLACK,   piece::rook);
  p -= s.piece_count(BLACK,  piece::queen);

  p -= s.piece_count(WHITE, piece::knight);
  p -= s.piece_count(WHITE, piece::bishop);
  p -= s.piece_count(WHITE,   piece::rook);
  p -= s.piece_count(WHITE,  piece::queen);

  p = std::max(0, p);

  assert(0 <= p && p <= total_phase);
  p = (p * 256 + total_phase / 2) / total_phase;
  assert(0 <= p && p <= 256);

  return p;
}

score_vector::score_vector(const state &s)
  : phase(), material{0, 0}, adjust_material{0, 0}, king_shield{0, 0},
    pawns_e{0, 0}, pawns_m{0, 0}, pcsq_e{0, 0}, pcsq_m{0, 0}, eg(), mg()
{
  eval_e(s, *this);
  eval_m(s, *this);

  for (square i(0); i < 64; ++i)
    if (s[i] != EMPTY)
      material[s[i].color()] += s[i].value();

  // Adjusting material value for the various combinations of pieces.
  for (unsigned c(BLACK); c <= WHITE; ++c)
  {
    const auto pawns(  s.piece_count(c,   piece::pawn));
    const auto knights(s.piece_count(c, piece::knight));
    const auto rooks(  s.piece_count(c,   piece::rook));

    if (s.piece_count(c, piece::bishop) > 1)
      adjust_material[c] += db.bishop_pair();
    if (knights > 1)
      adjust_material[c] += db.knight_pair();
    if (rooks > 1)
      adjust_material[c] += db.rook_pair();

    adjust_material[c] += db.n_adj(pawns) * knights;
    adjust_material[c] += db.r_adj(pawns) *   rooks;
  }

  // The, so called, tapered eval: a technique used in evaluation to make a
  // smooth transition between the phases of the game using a fine grained
  // numerical game phase value considering type of captured pieces so far.
  // The technique aggregates two distinct scores for the position, with
  // weights corresponding to the opening and endgame. The current game phase
  // is then used to interpolate between these values. The idea is to remove
  // evaluation discontinuity.
  phase = phase256(s);
}

score eval(const state &s)
{
  score_vector e(s);

  return
    e.material[s.side()] - e.material[!s.side()]
    + e.adjust_material[s.side()] - e.adjust_material[!s.side()]
    + (e.mg * (256 - e.phase) + e.eg * e.phase) / 256;
}

}  // namespace testudo
