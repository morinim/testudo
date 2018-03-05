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

}  // unnamed namespace

parameters db;

parameters::parameters()
{
  load();

  pcsq_.init();
  pp_adj_.init();
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

  j["material"]["bishop_pair"] = bishop_pair_;
  j["material"]["knight_pair"] = knight_pair_;
  j["material"]["rook_pair"]   =   rook_pair_;

  j["pp_adj"]["knight_wo_pawns_base"] = pp_adj_.knight_wo_pawns_base;
  j["pp_adj"][  "rook_wo_pawns_base"] =   pp_adj_.rook_wo_pawns_base;

  j["pp_adj"]["knight_wo_pawns_d"] = pp_adj_.knight_wo_pawns_d;
  j["pp_adj"][  "rook_wo_pawns_d"] =   pp_adj_.rook_wo_pawns_d;

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

  bishop_pair_ = j["material"]["bishop_pair"];
  knight_pair_ = j["material"]["knight_pair"];
  rook_pair_   = j["material"][  "rook_pair"];

  clamp(bishop_pair_,   0, 50);
  clamp(knight_pair_, -20, 20);
  clamp(  rook_pair_, -30, 30);

  pp_adj_.knight_wo_pawns_base = j["pp_adj"]["knight_wo_pawns_base"];
  pp_adj_.rook_wo_pawns_base   = j["pp_adj"][  "rook_wo_pawns_base"];

  pp_adj_.knight_wo_pawns_d = j["pp_adj"]["knight_wo_pawns_d"];
  pp_adj_.rook_wo_pawns_d   = j["pp_adj"][  "rook_wo_pawns_d"];

  return true;
}

// The general idea comes from Fruit.
void parameters::pcsq::init()
{
  for (auto &e :     pawn_file_base)  clamp(e, -20, 20);
  for (auto &e : knight_centre_base)  clamp(e, -20, 20);
  for (auto &e :   knight_rank_base)  clamp(e, -20, 20);
  for (auto &e : bishop_centre_base)  clamp(e, -20, 20);
  for (auto &e :     rook_file_base)  clamp(e, -20, 20);
  for (auto &e :  queen_centre_base)  clamp(e, -20, 20);
  for (auto &e :   king_centre_base)  clamp(e, -20, 20);
  for (auto &e :     king_file_base)  clamp(e, -20, 20);
  for (auto &e :     king_rank_base)  clamp(e, -20, 20);

  clamp(      pawn_file_mult_m, 1, 10);
  clamp(  knight_centre_mult_e, 1, 10);
  clamp(  knight_centre_mult_m, 1, 10);
  clamp(    knight_rank_mult_m, 1, 10);
  clamp(  bishop_centre_mult_e, 1, 10);
  clamp(  bishop_centre_mult_m, 1, 10);
  clamp(      rook_file_mult_m, 1, 10);
  clamp(   queen_centre_mult_e, 1, 10);
  clamp(   queen_centre_mult_m, 1, 10);
  clamp(    king_centre_mult_e, 1, 20);
  clamp(      king_file_mult_m, 1, 20);
  clamp(      king_rank_mult_m, 1, 20);

  clamp(knight_backrank_base_m, 0,  20);
  clamp( knight_trapped_base_m, 0, 120);
  clamp(bishop_backrank_base_m, 0,  20);
  clamp(bishop_diagonal_base_m, 0,  20);
  clamp( queen_backrank_base_m, 0,  20);

  clamp( pawn_weight, 0, 200);
  clamp(piece_weight, 0, 200);
  clamp( king_weight, 0, 200);

  // # PAWNS
  // ## File
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    mg[WPAWN.id()][i] += pawn_file_base[f] * pawn_file_mult_m;
  }

  // ## Centre control
  mg[WPAWN.id()][D3] += 10;
  mg[WPAWN.id()][E3] += 10;

  mg[WPAWN.id()][D4] += 20;
  mg[WPAWN.id()][E4] += 20;

  mg[WPAWN.id()][D5] += 10;
  mg[WPAWN.id()][E5] += 10;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    mg[WPAWN.id()][i] *= pawn_weight;
    mg[WPAWN.id()][i] /=         100;

    eg[WPAWN.id()][i] *= pawn_weight;
    eg[WPAWN.id()][i] /=         100;
  }

  // # KNIGHTS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto knight_centre(knight_centre_base[f] + knight_centre_base[r]);

    mg[WKNIGHT.id()][i] += knight_centre * knight_centre_mult_m;
    eg[WKNIGHT.id()][i] += knight_centre * knight_centre_mult_e;
  }

  // ## Rank
  for (square i(0); i < 64; ++i)
    mg[WKNIGHT.id()][i] += knight_rank_base[rank(i)] * knight_rank_mult_m;

  // ## Back rank
  for (square i(A1); i <= H1; ++i)
    mg[WKNIGHT.id()][i] -= knight_backrank_base_m;

  // ## "Trapped"
  mg[WKNIGHT.id()][A8] -= knight_trapped_base_m;
  mg[WKNIGHT.id()][H8] -= knight_trapped_base_m;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    mg[WKNIGHT.id()][i] *= piece_weight;
    mg[WKNIGHT.id()][i] /=          100;

    eg[WKNIGHT.id()][i] *= piece_weight;
    eg[WKNIGHT.id()][i] /=          100;
  }

  // # BISHOPS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto bishop_centre(bishop_centre_base[f] + bishop_centre_base[r]);

    mg[WBISHOP.id()][i] += bishop_centre * bishop_centre_mult_m;

    eg[WBISHOP.id()][i] += bishop_centre * bishop_centre_mult_e;
  }

  // ## Back rank
  for (square i(A1); i <= H1; ++i)
    mg[WBISHOP.id()][i] -= bishop_backrank_base_m;

  // ## Main diagonals
  mg[WBISHOP.id()][A1] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][B2] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][C3] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][D4] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][E5] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][F6] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][G7] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][H8] += bishop_diagonal_base_m;

  mg[WBISHOP.id()][H1] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][G2] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][F3] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][E4] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][D5] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][C6] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][B7] += bishop_diagonal_base_m;
  mg[WBISHOP.id()][A8] += bishop_diagonal_base_m;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    mg[WBISHOP.id()][i] *= piece_weight;
    mg[WBISHOP.id()][i] /=          100;

    eg[WBISHOP.id()][i] *= piece_weight;
    eg[WBISHOP.id()][i] /=          100;
  }

  // # ROOKS
  // ## File
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    mg[WROOK.id()][i] += rook_file_base[f] * rook_file_mult_m;
  }

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    mg[WROOK.id()][i] *= piece_weight;
    mg[WROOK.id()][i] /=          100;

    eg[WROOK.id()][i] *= piece_weight;
    eg[WROOK.id()][i] /=          100;
  }

  // # QUEENS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto queen_centre(queen_centre_base[f] + queen_centre_base[r]);

    mg[WQUEEN.id()][i] += queen_centre * queen_centre_mult_m;
    eg[WQUEEN.id()][i] += queen_centre * queen_centre_mult_e;
  }

  // ## Back rank
  for (square i(A1); i <= H1; ++i)
    mg[WQUEEN.id()][i] -= queen_backrank_base_m;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    mg[WQUEEN.id()][i] *= piece_weight;
    mg[WQUEEN.id()][i] /=          100;

    eg[WQUEEN.id()][i] *= piece_weight;
    eg[WQUEEN.id()][i] /=          100;
  }

  // # KINGS
  // ## Centre
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    const auto r(rank(i) < 4 ? rank(i) : 7 - rank(i));

    const auto king_centre(king_centre_base[f] + king_centre_base[r]);

    eg[WKING.id()][i] += king_centre * king_centre_mult_e;
  }

  // ## File
  for (square i(0); i < 64; ++i)
  {
    const auto f(file(i) < 4 ? file(i) : 7 - file(i));
    mg[WKING.id()][i] += king_file_base[f] * king_file_mult_m;
  }

  // ## Rank
  for (square i(0); i < 64; ++i)
    mg[WKING.id()][i] += king_rank_base[rank(i)] * king_rank_mult_m;

  // ## Weight
  for (square i(0); i < 64; ++i)
  {
    mg[WKING.id()][i] *= king_weight;
    mg[WKING.id()][i] /=         100;

    eg[WKING.id()][i] *= piece_weight;
    eg[WKING.id()][i] /=          100;
  }

  // Flipped copy for BLACK.
  for (unsigned t(piece::pawn); t <= piece::queen; ++t)
    for (square i(0); i < 64; ++i)
    {
      const auto pb(piece(BLACK, t).id());
      const auto pw(piece(WHITE, t).id());

      eg[pb][flip(i)] = eg[pw][i];
      mg[pb][flip(i)] = mg[pw][i];
    }
}

void parameters::pp_adj::init()
{
  clamp(knight_wo_pawns_base, -30,  0);
  clamp(  rook_wo_pawns_base,   0, 30);
  clamp(knight_wo_pawns_d,  0, 6);
  clamp(  rook_wo_pawns_d, -6, 0);

  n[0] = knight_wo_pawns_base;
  r[0] =   rook_wo_pawns_base;
  for (int i(1); i < 9; ++i)
  {
    n[i] = n[0] + knight_wo_pawns_d * i;
    r[i] = n[0] +   rook_wo_pawns_d * i;
  }
}

}  // namespace testudo
