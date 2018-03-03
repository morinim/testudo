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

void eval_m(const state &s, score_vector &e)
{
  for (square i(0); i < 64; ++i)
    if (s[i] != EMPTY)
      e.pcsq_m[s[i].color()] += db.pcsq_m(s[i], i);
}

void eval_e(const state &s, score_vector &e)
{
  for (square i(0); i < 64; ++i)
    if (s[i] != EMPTY)
      e.pcsq_e[s[i].color()] += db.pcsq_e(s[i], i);
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
  : phase(), material{0, 0}, pcsq_e{0, 0}, pcsq_m{0, 0}
{
  eval_e(s, *this);
  eval_m(s, *this);

  for (square i(0); i < 64; ++i)
    if (s[i] != EMPTY)
    {
      material[s[i].color()] += s[i].value();
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

  const auto ee(e.pcsq_e[s.side()] - e.pcsq_e[!s.side()]);
  const auto em(e.pcsq_m[s.side()] - e.pcsq_m[!s.side()]);

  return
    e.material[s.side()] - e.material[!s.side()]
    + (em * (256 - e.phase) + ee * e.phase) / 256;
}

}  // namespace testudo
