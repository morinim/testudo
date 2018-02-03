/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cassert>
#include <fstream>

#include "parameters.h"

namespace testudo
{

namespace
{

template<class T>
void clamp(T &v, const T &lo, const T &hi)
{
  assert(lo < hi);
  if (v < lo)
    v = lo;
  else if (v > hi)
    v = hi;
}

// Array used to calculate the piece/square values for BLACK pieces.
static const square flip[64] =
{
  A1,  B1,  C1,  D1,  E1,  F1,  G1,  H1,
  A2,  B2,  C2,  D2,  E2,  F2,  G2,  H2,
  A3,  B3,  C3,  D3,  E3,  F3,  G3,  H3,
  A4,  B4,  C4,  D4,  E4,  F4,  G4,  H4,
  A5,  B5,  C5,  D5,  E5,  F5,  G5,  H5,
  A6,  B6,  C6,  D6,  E6,  F6,  G6,  H6,
  A7,  B7,  C7,  D7,  E7,  F7,  G7,  H7,
  A8,  B8,  C8,  D8,  E8,  F8,  G8,  H8
};

}  // unnamed namespace

namespace detail
{

// Piece/square tables for opening / middle-game (`pcsq_m_`) and end-game
// (`pcsq_e_`).
//
// They're calculated starting from the `pcsq_*` data member in the parameters
// class. This approach allows to have a more narrow set of variables subject
// to optimization.
score pcsq_m_[piece::sup_id][64];
score pcsq_e_[piece::sup_id][64];

}  // namespace detail

score parameters::pcsq_pawn_file_mult_m_     =  5;
score parameters::pcsq_knight_centre_mult_e_ =  5;
score parameters::pcsq_knight_centre_mult_m_ =  5;
score parameters::pcsq_knight_rank_mult_m_   =  5;
score parameters::pcsq_bishop_centre_mult_e_ =  3;
score parameters::pcsq_bishop_centre_mult_m_ =  2;
score parameters::pcsq_rook_file_mult_m_     =  3;
score parameters::pcsq_queen_centre_mult_e_  =  4;
score parameters::pcsq_queen_centre_mult_m_  =  0;
score parameters::pcsq_king_centre_mult_e_   = 12;
score parameters::pcsq_king_file_mult_m_     = 10;
score parameters::pcsq_king_rank_mult_m_     = 10;

std::array<score, 4> parameters::pcsq_pawn_file_base_ = {-3, -1, 0, 1};
std::array<score, 4> parameters::pcsq_knight_centre_base_ = {-4, -2, 0, 1};
std::array<score, 8> parameters::pcsq_knight_rank_base_ =
  {-2, -1, 0, 1, 2, 3, 2, 1};
std::array<score, 4> parameters::pcsq_bishop_centre_base_ = {-3, -1, 0, 1};
std::array<score, 4> parameters::pcsq_rook_file_base_ = {-2, -1, 0, 1};
std::array<score, 4> parameters::pcsq_queen_centre_base_ = {-3, -1, 0, 1};
std::array<score, 4> parameters::pcsq_king_centre_base_ = {-3, -1, 0, 1};
std::array<score, 4> parameters::pcsq_king_file_base_ = {3, 4, 2, 0};
std::array<score, 8> parameters::pcsq_king_rank_base_ =
  {1, 0, -2, -3, -4, -5, -6, -7};

score parameters::pcsq_knight_backrank_base_m_ =   0;
score parameters::pcsq_knight_trapped_base_m_  = 100;
score parameters::pcsq_bishop_backrank_base_m_ =  10;
score parameters::pcsq_bishop_diagonal_base_m_ =   4;
score parameters::pcsq_queen_backrank_base_m_  =   5;

score parameters::pcsq_pawn_weight_  = 100;
score parameters::pcsq_piece_weight_ = 100;
score parameters::pcsq_king_weight_  = 100;

parameters db;

parameters::parameters()
{
  load();

  pcsq_init();
}

bool parameters::load()
{
  std::ifstream f("testudo.par");
  if (!f)
    return false;

  auto p0001(pcsq_pawn_file_base_);
  for (auto &e : p0001)  if (!(f >> e)) return false;

  auto p0002(pcsq_knight_centre_base_);
  for (auto &e : p0002)  if (!(f >> e)) return false;

  auto p0003(pcsq_knight_rank_base_);
  for (auto &e : p0003)  if (!(f >> e)) return false;

  auto p0004(pcsq_bishop_centre_base_);
  for (auto &e : p0004)  if (!(f >> e)) return false;

  auto p0005(pcsq_rook_file_base_);
  for (auto &e : p0005)  if (!(f >> e)) return false;

  auto p0006(pcsq_queen_centre_base_);
  for (auto &e : p0006)  if (!(f >> e)) return false;

  auto p0007(pcsq_king_centre_base_);
  for (auto &e : p0007)  if (!(f >> e)) return false;

  auto p0008(pcsq_king_file_base_);
  for (auto &e : p0008)  if (!(f >> e)) return false;

  auto p0009(pcsq_king_rank_base_);
  for (auto &e : p0009)  if (!(f >> e)) return false;

  auto p0010(pcsq_pawn_file_mult_m_);
  if (!f >> p0010)  return false;

  auto p0011(pcsq_knight_centre_mult_e_);
  if (!f >> p0011)  return false;

  auto p0012(pcsq_knight_centre_mult_m_);
  if (!f >> p0012)  return false;

  auto p0013(pcsq_knight_rank_mult_m_);
  if (!f >> p0013)  return false;

  auto p0014(pcsq_bishop_centre_mult_e_);
  if (!f >> p0014)  return false;

  auto p0015(pcsq_bishop_centre_mult_m_);
  if (!f >> p0015)  return false;

  auto p0016(pcsq_rook_file_mult_m_);
  if (!f >> p0016)  return false;

  auto p0017(pcsq_queen_centre_mult_e_);
  if (!f >> p0017)  return false;

  auto p0018(pcsq_queen_centre_mult_m_);
  if (!f >> p0018)  return false;

  auto p0019(pcsq_king_centre_mult_e_);
  if (!f >> p0019)  return false;

  auto p0020(pcsq_king_file_mult_m_);
  if (!f >> p0020)  return false;

  auto p0021(pcsq_king_rank_mult_m_);
  if (!f >> p0021)  return false;

  auto p0022(pcsq_knight_backrank_base_m_);
  if (!f >> p0022)  return false;

  auto p0023(pcsq_knight_trapped_base_m_);
  if (!f >> p0023)  return false;

  auto p0024(pcsq_bishop_backrank_base_m_);
  if (!f >> p0024)  return false;

  auto p0025(pcsq_bishop_diagonal_base_m_);
  if (!f >> p0025)  return false;

  auto p0026(pcsq_queen_backrank_base_m_);
  if (!f >> p0026)  return false;

  auto p0027(pcsq_pawn_weight_);
  if (!f >> p0027)  return false;

  auto p0028(pcsq_piece_weight_);
  if (!f >> p0028)  return false;

  auto p0029(pcsq_king_weight_);
  if (!f >> p0029)  return false;

  pcsq_pawn_file_base_     = p0001;
  pcsq_knight_centre_base_ = p0002;
  pcsq_knight_rank_base_   = p0003;
  pcsq_bishop_centre_base_ = p0004;
  pcsq_rook_file_base_     = p0005;
  pcsq_queen_centre_base_  = p0006;
  pcsq_king_centre_base_   = p0007;
  pcsq_king_file_base_     = p0008;
  pcsq_king_rank_base_     = p0009;

  pcsq_pawn_file_mult_m_     = p0010;
  pcsq_knight_centre_mult_e_ = p0011;
  pcsq_knight_centre_mult_m_ = p0012;
  pcsq_knight_rank_mult_m_   = p0013;
  pcsq_bishop_centre_mult_e_ = p0014;
  pcsq_bishop_centre_mult_m_ = p0015;
  pcsq_rook_file_mult_m_     = p0016;
  pcsq_queen_centre_mult_e_  = p0017;
  pcsq_queen_centre_mult_m_  = p0018;
  pcsq_king_centre_mult_e_   = p0019;
  pcsq_king_file_mult_m_     = p0020;
  pcsq_king_rank_mult_m_     = p0021;

  pcsq_knight_backrank_base_m_ = p0022;
  pcsq_knight_trapped_base_m_  = p0023;
  pcsq_bishop_backrank_base_m_ = p0024;
  pcsq_bishop_diagonal_base_m_ = p0025;
  pcsq_queen_backrank_base_m_  = p0026;
  pcsq_pawn_weight_            = p0027;
  pcsq_piece_weight_           = p0028;
  pcsq_king_weight_            = p0029;

  return true;
}

void parameters::pcsq_init()
{
  using namespace detail;

  for (auto &e :     pcsq_pawn_file_base_)  clamp(e, -20, 20);
  for (auto &e : pcsq_knight_centre_base_)  clamp(e, -20, 20);
  for (auto &e :   pcsq_knight_rank_base_)  clamp(e, -20, 20);
  for (auto &e : pcsq_bishop_centre_base_)  clamp(e, -20, 20);
  for (auto &e :     pcsq_rook_file_base_)  clamp(e, -20, 20);
  for (auto &e :  pcsq_queen_centre_base_)  clamp(e, -20, 20);
  for (auto &e :   pcsq_king_centre_base_)  clamp(e, -20, 20);
  for (auto &e :     pcsq_king_file_base_)  clamp(e, -20, 20);
  for (auto &e :     pcsq_king_rank_base_)  clamp(e, -20, 20);

  clamp(      pcsq_pawn_file_mult_m_, 1, 10);
  clamp(  pcsq_knight_centre_mult_e_, 1, 10);
  clamp(  pcsq_knight_centre_mult_m_, 1, 10);
  clamp(    pcsq_knight_rank_mult_m_, 1, 10);
  clamp(  pcsq_bishop_centre_mult_e_, 1, 10);
  clamp(  pcsq_bishop_centre_mult_m_, 1, 10);
  clamp(      pcsq_rook_file_mult_m_, 1, 10);
  clamp(   pcsq_queen_centre_mult_e_, 1, 10);
  clamp(   pcsq_queen_centre_mult_m_, 1, 10);
  clamp(    pcsq_king_centre_mult_e_, 1, 20);
  clamp(      pcsq_king_file_mult_m_, 1, 20);
  clamp(      pcsq_king_rank_mult_m_, 1, 20);

  clamp(pcsq_knight_backrank_base_m_, 0,  20);
  clamp( pcsq_knight_trapped_base_m_, 0, 120);
  clamp(pcsq_bishop_backrank_base_m_, 0,  20);
  clamp(pcsq_bishop_diagonal_base_m_, 0,  20);
  clamp(pcsq_queen_backrank_base_m_, 0,  20);

  clamp( pcsq_pawn_weight_, 0, 200);
  clamp(pcsq_piece_weight_, 0, 200);
  clamp( pcsq_king_weight_, 0, 200);

  // # PAWNS
  // ## File
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    pcsq_m_[WPAWN.id()][i] += pcsq_pawn_file_base_[f] * pcsq_pawn_file_mult_m_;
  }

  // ## Centre control
  pcsq_m_[WPAWN.id()][D3] += 10;
  pcsq_m_[WPAWN.id()][E3] += 10;

  pcsq_m_[WPAWN.id()][D4] += 20;
  pcsq_m_[WPAWN.id()][E4] += 20;

  pcsq_m_[WPAWN.id()][D5] += 10;
  pcsq_m_[WPAWN.id()][E5] += 10;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WPAWN.id()][i] *= pcsq_pawn_weight_;
    pcsq_m_[WPAWN.id()][i] /=               100;

    pcsq_e_[WPAWN.id()][i] *= pcsq_pawn_weight_;
    pcsq_e_[WPAWN.id()][i] /=               100;
  }

  // # KNIGHTS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto knight_centre(pcsq_knight_centre_base_[f]
                             + pcsq_knight_centre_base_[r]);

    pcsq_m_[WKNIGHT.id()][i] += knight_centre * pcsq_knight_centre_mult_m_;
    pcsq_e_[WKNIGHT.id()][i] += knight_centre * pcsq_knight_centre_mult_e_;
  }

  // ## Rank
  for (square i(0); i < 64; ++i)
    pcsq_m_[WKNIGHT.id()][i] += pcsq_knight_rank_base_[rank(i)]
                                * pcsq_knight_rank_mult_m_;

  // ## Back rank
  for (square i(A1); i <= H1; ++i)
    pcsq_m_[WKNIGHT.id()][i] -= pcsq_knight_backrank_base_m_;

  // ## "Trapped"
  pcsq_m_[WKNIGHT.id()][A8] -= pcsq_knight_trapped_base_m_;
  pcsq_m_[WKNIGHT.id()][H8] -= pcsq_knight_trapped_base_m_;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WKNIGHT.id()][i] *= pcsq_piece_weight_;
    pcsq_m_[WKNIGHT.id()][i] /=                100;

    pcsq_e_[WKNIGHT.id()][i] *= pcsq_piece_weight_;
    pcsq_e_[WKNIGHT.id()][i] /=                100;
  }

  // # BISHOPS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto bishop_centre(pcsq_bishop_centre_base_[f]
                             + pcsq_bishop_centre_base_[r]);

    pcsq_m_[WBISHOP.id()][i] += bishop_centre * pcsq_bishop_centre_mult_m_;

    pcsq_e_[WBISHOP.id()][i] += bishop_centre * pcsq_bishop_centre_mult_e_;
  }

  // ## Back rank
  for (square i(A1); i <= H1; ++i)
    pcsq_m_[WBISHOP.id()][i] -= pcsq_bishop_backrank_base_m_;

  // ## Main diagonals
  pcsq_m_[WBISHOP.id()][A1] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][B2] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][C3] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][D4] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][E5] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][F6] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][G7] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][H8] += pcsq_bishop_diagonal_base_m_;

  pcsq_m_[WBISHOP.id()][H1] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][G2] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][F3] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][E4] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][D5] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][C6] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][B7] += pcsq_bishop_diagonal_base_m_;
  pcsq_m_[WBISHOP.id()][A8] += pcsq_bishop_diagonal_base_m_;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WBISHOP.id()][i] *= pcsq_piece_weight_;
    pcsq_m_[WBISHOP.id()][i] /=                100;

    pcsq_e_[WBISHOP.id()][i] *= pcsq_piece_weight_;
    pcsq_e_[WBISHOP.id()][i] /=                100;
  }

  // # ROOKS
  // ## File
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    pcsq_m_[WROOK.id()][i] += pcsq_rook_file_base_[f] * pcsq_rook_file_mult_m_;
  }

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WROOK.id()][i] *= pcsq_piece_weight_;
    pcsq_m_[WROOK.id()][i] /=                100;

    pcsq_e_[WROOK.id()][i] *= pcsq_piece_weight_;
    pcsq_e_[WROOK.id()][i] /=                100;
  }

  // # QUEENS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto queen_centre(pcsq_queen_centre_base_[f]
                            + pcsq_queen_centre_base_[r]);

    pcsq_m_[WQUEEN.id()][i] += queen_centre * pcsq_queen_centre_mult_m_;
    pcsq_e_[WQUEEN.id()][i] += queen_centre * pcsq_queen_centre_mult_e_;
  }

  // ## Back rank
  for (square i(A1); i <= H1; ++i)
    pcsq_m_[WQUEEN.id()][i] -= pcsq_queen_backrank_base_m_;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WQUEEN.id()][i] *= pcsq_piece_weight_;
    pcsq_m_[WQUEEN.id()][i] /=                100;

    pcsq_e_[WQUEEN.id()][i] *= pcsq_piece_weight_;
    pcsq_e_[WQUEEN.id()][i] /=                100;
  }

  // # KINGS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto king_centre(pcsq_king_centre_base_[f]
                           + pcsq_king_centre_base_[r]);

    pcsq_e_[WKING.id()][i] += king_centre * pcsq_king_centre_mult_e_;
  }

  // ## File
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    pcsq_m_[WKING.id()][i] += pcsq_king_file_base_[f] * pcsq_king_file_mult_m_;
  }

  // ## Rank
  for (square i(0); i < 64; ++i)
    pcsq_m_[WKING.id()][i] += pcsq_king_rank_base_[rank(i)]
                              * pcsq_king_rank_mult_m_;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WKING.id()][i] *= pcsq_king_weight_;
    pcsq_m_[WKING.id()][i] /=               100;

    pcsq_e_[WKING.id()][i] *= pcsq_piece_weight_;
    pcsq_e_[WKING.id()][i] /=                100;
  }

  // Flipped copy for BLACK.
  for (unsigned t(piece::pawn); t <= piece::queen; ++t)
    for (square i(0); i < 64; ++i)
    {
      const auto pb(piece(BLACK, t).id());
      const auto pw(piece(WHITE, t).id());

      pcsq_e_[pb][flip[i]] = pcsq_e_[pw][i];
      pcsq_m_[pb][flip[i]] = pcsq_m_[pw][i];
    }
}

}  // namespace testudo
