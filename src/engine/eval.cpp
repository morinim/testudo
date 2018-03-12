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

void eval_m(const state &s, score_vector &e)
{
  for (square i(0); i < 64; ++i)
    if (s[i] != EMPTY)
      e.pcsq_m[s[i].color()] += db.pcsq_m(s[i], i);

  eval_king_shield(s, e);

  e.mg = e.pcsq_m[s.side()] - e.pcsq_m[!s.side()]
         + e.king_shield[s.side()] - e.king_shield[!s.side()];
}

void eval_e(const state &s, score_vector &e)
{
  for (square i(0); i < 64; ++i)
    if (s[i] != EMPTY)
      e.pcsq_e[s[i].color()] += db.pcsq_e(s[i], i);

  e.eg = e.pcsq_e[s.side()] - e.pcsq_e[!s.side()];
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
    pcsq_e{0, 0}, pcsq_m{0, 0}, eg(), mg()
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
