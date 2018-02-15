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

#include "thirdparty/json.hpp"

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
// They're calculated starting from the `pcsq_` data member in the parameters
// class. This approach allows to have a more narrow set of variables subject
// to optimization.
score pcsq_m_[piece::sup_id][64];
score pcsq_e_[piece::sup_id][64];

}  // namespace detail

parameters db;

parameters::parameters()
{
  load();

  pcsq_init();
}

bool parameters::save() const
{
  using json = nlohmann::json;
  json j;

  j["pcsq"]["pawn_file_base"]         = pcsq_.pawn_file_base;
  j["pcsq"]["knight_centre_base"]     = pcsq_.knight_centre_base;
  j["pcsq"]["knight_rank_base"]       = pcsq_.knight_rank_base;
  j["pcsq"]["bishop_centre_base"]     = pcsq_.bishop_centre_base;
  j["pcsq"]["rook_file_base"]         = pcsq_.rook_file_base;
  j["pcsq"]["queen_centre_base"]      = pcsq_.queen_centre_base;
  j["pcsq"]["king_centre_base"]       = pcsq_.king_centre_base;
  j["pcsq"]["king_file_base"]         = pcsq_.king_file_base;
  j["pcsq"]["king_rank_base"]         = pcsq_.king_rank_base;

  j["pcsq"]["pawn_file_mult_m"]       = pcsq_.pawn_file_mult_m;
  j["pcsq"]["knight_centre_mult_e"]   = pcsq_.knight_centre_mult_e;
  j["pcsq"]["knight_centre_mult_m"]   = pcsq_.knight_centre_mult_m;
  j["pcsq"]["knight_rank_mult_m"]     = pcsq_.knight_rank_mult_m;
  j["pcsq"]["bishop_centre_mult_e"]   = pcsq_.bishop_centre_mult_e;
  j["pcsq"]["bishop_centre_mult_m"]   = pcsq_.bishop_centre_mult_m;
  j["pcsq"]["rook_file_mult_m"]       = pcsq_.rook_file_mult_m;
  j["pcsq"]["queen_centre_mult_e"]    = pcsq_.queen_centre_mult_e;
  j["pcsq"]["queen_centre_mult_m"]    = pcsq_.queen_centre_mult_m;
  j["pcsq"]["king_centre_mult_e"]     = pcsq_.king_centre_mult_e;
  j["pcsq"]["king_file_mult_m"]       = pcsq_.king_file_mult_m;
  j["pcsq"]["king_rank_mult_m"]       = pcsq_.king_rank_mult_m;

  j["pcsq"]["knight_backrank_base_m"] = pcsq_.knight_backrank_base_m;
  j["pcsq"]["knight_trapped_base_m"]  = pcsq_.knight_trapped_base_m;
  j["pcsq"]["bishop_backrank_base_m"] = pcsq_.bishop_backrank_base_m;
  j["pcsq"]["bishop_diagonal_base_m"] = pcsq_.bishop_diagonal_base_m;
  j["pcsq"]["queen_backrank_base_m"]  = pcsq_.queen_backrank_base_m;
  j["pcsq"]["pawn_weight"]            = pcsq_.pawn_weight;
  j["pcsq"]["piece_weight"]           = pcsq_.piece_weight;
  j["pcsq"]["king_weight"]            = pcsq_.king_weight;

  std::ofstream f("testudo.json");
  return !!f && f << j;
}

bool parameters::load()
{
  std::ifstream f("testudo.json");
  if (!f)
    return false;

  using json = nlohmann::json;
  json j;
  if (!(f >> j))
    return false;

  pcsq_.pawn_file_base         = j["pcsq"]["pawn_file_base"];
  pcsq_.knight_centre_base     = j["pcsq"]["knight_centre_base"];
  pcsq_.knight_rank_base       = j["pcsq"]["knight_rank_base"];
  pcsq_.bishop_centre_base     = j["pcsq"]["bishop_centre_base"];
  pcsq_.rook_file_base         = j["pcsq"]["rook_file_base"];
  pcsq_.queen_centre_base      = j["pcsq"]["queen_centre_base"];
  pcsq_.king_centre_base       = j["pcsq"]["king_centre_base"];
  pcsq_.king_file_base         = j["pcsq"]["king_file_base"];
  pcsq_.king_rank_base         = j["pcsq"]["king_rank_base"];

  pcsq_.pawn_file_mult_m       = j["pcsq"]["pawn_file_mult_m"];
  pcsq_.knight_centre_mult_e   = j["pcsq"]["knight_centre_mult_e"];
  pcsq_.knight_centre_mult_m   = j["pcsq"]["knight_centre_mult_m"];
  pcsq_.knight_rank_mult_m     = j["pcsq"]["knight_rank_mult_m"];
  pcsq_.bishop_centre_mult_e   = j["pcsq"]["bishop_centre_mult_e"];
  pcsq_.bishop_centre_mult_m   = j["pcsq"]["bishop_centre_mult_m"];
  pcsq_.rook_file_mult_m       = j["pcsq"]["rook_file_mult_m"];
  pcsq_.queen_centre_mult_e    = j["pcsq"]["queen_centre_mult_e"];
  pcsq_.queen_centre_mult_m    = j["pcsq"]["queen_centre_mult_m"];
  pcsq_.king_centre_mult_e     = j["pcsq"]["king_centre_mult_e"];
  pcsq_.king_file_mult_m       = j["pcsq"]["king_file_mult_m"];
  pcsq_.king_rank_mult_m       = j["pcsq"]["king_rank_mult_m"];

  pcsq_.knight_backrank_base_m = j["pcsq"]["knight_backrank_base_m"];
  pcsq_.knight_trapped_base_m  = j["pcsq"]["knight_trapped_base_m"];
  pcsq_.bishop_backrank_base_m = j["pcsq"]["bishop_backrank_base_m"];
  pcsq_.bishop_diagonal_base_m = j["pcsq"]["bishop_diagonal_base_m"];
  pcsq_.queen_backrank_base_m  = j["pcsq"]["queen_backrank_base_m"];
  pcsq_.pawn_weight            = j["pcsq"]["pawn_weight"];
  pcsq_.piece_weight           = j["pcsq"]["piece_weight"];
  pcsq_.king_weight            = j["pcsq"]["king_weight"];

  return true;
}

// The general idea comes from Fruit.
void parameters::pcsq_init()
{
  using namespace detail;

  for (auto &e :     pcsq_.pawn_file_base)  clamp(e, -20, 20);
  for (auto &e : pcsq_.knight_centre_base)  clamp(e, -20, 20);
  for (auto &e :   pcsq_.knight_rank_base)  clamp(e, -20, 20);
  for (auto &e : pcsq_.bishop_centre_base)  clamp(e, -20, 20);
  for (auto &e :     pcsq_.rook_file_base)  clamp(e, -20, 20);
  for (auto &e :  pcsq_.queen_centre_base)  clamp(e, -20, 20);
  for (auto &e :   pcsq_.king_centre_base)  clamp(e, -20, 20);
  for (auto &e :     pcsq_.king_file_base)  clamp(e, -20, 20);
  for (auto &e :     pcsq_.king_rank_base)  clamp(e, -20, 20);

  clamp(      pcsq_.pawn_file_mult_m, 1, 10);
  clamp(  pcsq_.knight_centre_mult_e, 1, 10);
  clamp(  pcsq_.knight_centre_mult_m, 1, 10);
  clamp(    pcsq_.knight_rank_mult_m, 1, 10);
  clamp(  pcsq_.bishop_centre_mult_e, 1, 10);
  clamp(  pcsq_.bishop_centre_mult_m, 1, 10);
  clamp(      pcsq_.rook_file_mult_m, 1, 10);
  clamp(   pcsq_.queen_centre_mult_e, 1, 10);
  clamp(   pcsq_.queen_centre_mult_m, 1, 10);
  clamp(    pcsq_.king_centre_mult_e, 1, 20);
  clamp(      pcsq_.king_file_mult_m, 1, 20);
  clamp(      pcsq_.king_rank_mult_m, 1, 20);

  clamp(pcsq_.knight_backrank_base_m, 0,  20);
  clamp( pcsq_.knight_trapped_base_m, 0, 120);
  clamp(pcsq_.bishop_backrank_base_m, 0,  20);
  clamp(pcsq_.bishop_diagonal_base_m, 0,  20);
  clamp( pcsq_.queen_backrank_base_m, 0,  20);

  clamp( pcsq_.pawn_weight, 0, 200);
  clamp(pcsq_.piece_weight, 0, 200);
  clamp( pcsq_.king_weight, 0, 200);

  // # PAWNS
  // ## File
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    pcsq_m_[WPAWN.id()][i] += pcsq_.pawn_file_base[f] * pcsq_.pawn_file_mult_m;
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
    pcsq_m_[WPAWN.id()][i] *= pcsq_.pawn_weight;
    pcsq_m_[WPAWN.id()][i] /=               100;

    pcsq_e_[WPAWN.id()][i] *= pcsq_.pawn_weight;
    pcsq_e_[WPAWN.id()][i] /=               100;
  }

  // # KNIGHTS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto knight_centre(pcsq_.knight_centre_base[f]
                             + pcsq_.knight_centre_base[r]);

    pcsq_m_[WKNIGHT.id()][i] += knight_centre * pcsq_.knight_centre_mult_m;
    pcsq_e_[WKNIGHT.id()][i] += knight_centre * pcsq_.knight_centre_mult_e;
  }

  // ## Rank
  for (square i(0); i < 64; ++i)
    pcsq_m_[WKNIGHT.id()][i] += pcsq_.knight_rank_base[rank(i)]
                                * pcsq_.knight_rank_mult_m;

  // ## Back rank
  for (square i(A1); i <= H1; ++i)
    pcsq_m_[WKNIGHT.id()][i] -= pcsq_.knight_backrank_base_m;

  // ## "Trapped"
  pcsq_m_[WKNIGHT.id()][A8] -= pcsq_.knight_trapped_base_m;
  pcsq_m_[WKNIGHT.id()][H8] -= pcsq_.knight_trapped_base_m;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WKNIGHT.id()][i] *= pcsq_.piece_weight;
    pcsq_m_[WKNIGHT.id()][i] /=                100;

    pcsq_e_[WKNIGHT.id()][i] *= pcsq_.piece_weight;
    pcsq_e_[WKNIGHT.id()][i] /=                100;
  }

  // # BISHOPS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto bishop_centre(pcsq_.bishop_centre_base[f]
                             + pcsq_.bishop_centre_base[r]);

    pcsq_m_[WBISHOP.id()][i] += bishop_centre * pcsq_.bishop_centre_mult_m;

    pcsq_e_[WBISHOP.id()][i] += bishop_centre * pcsq_.bishop_centre_mult_e;
  }

  // ## Back rank
  for (square i(A1); i <= H1; ++i)
    pcsq_m_[WBISHOP.id()][i] -= pcsq_.bishop_backrank_base_m;

  // ## Main diagonals
  pcsq_m_[WBISHOP.id()][A1] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][B2] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][C3] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][D4] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][E5] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][F6] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][G7] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][H8] += pcsq_.bishop_diagonal_base_m;

  pcsq_m_[WBISHOP.id()][H1] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][G2] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][F3] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][E4] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][D5] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][C6] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][B7] += pcsq_.bishop_diagonal_base_m;
  pcsq_m_[WBISHOP.id()][A8] += pcsq_.bishop_diagonal_base_m;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WBISHOP.id()][i] *= pcsq_.piece_weight;
    pcsq_m_[WBISHOP.id()][i] /=                100;

    pcsq_e_[WBISHOP.id()][i] *= pcsq_.piece_weight;
    pcsq_e_[WBISHOP.id()][i] /=                100;
  }

  // # ROOKS
  // ## File
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    pcsq_m_[WROOK.id()][i] += pcsq_.rook_file_base[f] * pcsq_.rook_file_mult_m;
  }

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WROOK.id()][i] *= pcsq_.piece_weight;
    pcsq_m_[WROOK.id()][i] /=                100;

    pcsq_e_[WROOK.id()][i] *= pcsq_.piece_weight;
    pcsq_e_[WROOK.id()][i] /=                100;
  }

  // # QUEENS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto queen_centre(pcsq_.queen_centre_base[f]
                            + pcsq_.queen_centre_base[r]);

    pcsq_m_[WQUEEN.id()][i] += queen_centre * pcsq_.queen_centre_mult_m;
    pcsq_e_[WQUEEN.id()][i] += queen_centre * pcsq_.queen_centre_mult_e;
  }

  // ## Back rank
  for (square i(A1); i <= H1; ++i)
    pcsq_m_[WQUEEN.id()][i] -= pcsq_.queen_backrank_base_m;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WQUEEN.id()][i] *= pcsq_.piece_weight;
    pcsq_m_[WQUEEN.id()][i] /=                100;

    pcsq_e_[WQUEEN.id()][i] *= pcsq_.piece_weight;
    pcsq_e_[WQUEEN.id()][i] /=                100;
  }

  // # KINGS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto king_centre(pcsq_.king_centre_base[f]
                           + pcsq_.king_centre_base[r]);

    pcsq_e_[WKING.id()][i] += king_centre * pcsq_.king_centre_mult_e;
  }

  // ## File
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    pcsq_m_[WKING.id()][i] += pcsq_.king_file_base[f] * pcsq_.king_file_mult_m;
  }

  // ## Rank
  for (square i(0); i < 64; ++i)
    pcsq_m_[WKING.id()][i] += pcsq_.king_rank_base[rank(i)]
                              * pcsq_.king_rank_mult_m;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    pcsq_m_[WKING.id()][i] *= pcsq_.king_weight;
    pcsq_m_[WKING.id()][i] /=               100;

    pcsq_e_[WKING.id()][i] *= pcsq_.piece_weight;
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
