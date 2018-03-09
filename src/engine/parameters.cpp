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
#include "log.h"

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

const std::string parameters::pcsq::sec_name   =   "pcsq";
const std::string parameters::pp_adj::sec_name = "pp_adj";

parameters db;

parameters::parameters()
{
  if (!load())
  {
    testudoINFO << "Using default values for some/all parameters";
  }

  pcsq_.init();
  pp_adj_.init();
}

bool parameters::save() const
{
  nlohmann::json j;

  pcsq_.save(j);

  j["material"]["bishop_pair"] = bishop_pair_;
  j["material"]["knight_pair"] = knight_pair_;
  j["material"]["rook_pair"]   =   rook_pair_;

  pp_adj_.save(j);

  std::ofstream f("testudo.json");
  return !!f && f << j;
}

bool parameters::load()
{
  std::ifstream f("testudo.json");
  if (!f)
    return false;

  nlohmann::json j;
  if (!(f >> j))
    return false;

  if (!pcsq_.load(j))
  {
    testudoWARNING << "Partial initialization of the parameters "
                      "(missing 'pcsq' section)";
    return false;
  }

  bishop_pair_ = j["material"]["bishop_pair"];
  knight_pair_ = j["material"]["knight_pair"];
  rook_pair_   = j["material"][  "rook_pair"];

  clamp(bishop_pair_,   0, 50);
  clamp(knight_pair_, -20, 20);
  clamp(  rook_pair_, -30, 30);

  if (!pp_adj_.load(j))
  {
    testudoWARNING << "Partial initialization of the parameters "
                      "(missing 'pp_adj' section)";
    return false;
  }

  return true;
}

// The general idea comes from Fruit.
void parameters::pcsq::init()
{
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

bool parameters::pcsq::load(const nlohmann::json &j)
{
  pawn_file_base         = j[sec_name]["pawn_file_base"];
  knight_centre_base     = j[sec_name]["knight_centre_base"];
  knight_rank_base       = j[sec_name]["knight_rank_base"];
  bishop_centre_base     = j[sec_name]["bishop_centre_base"];
  rook_file_base         = j[sec_name]["rook_file_base"];
  queen_centre_base      = j[sec_name]["queen_centre_base"];
  king_centre_base       = j[sec_name]["king_centre_base"];
  king_file_base         = j[sec_name]["king_file_base"];
  king_rank_base         = j[sec_name]["king_rank_base"];

  pawn_file_mult_m       = j[sec_name]["pawn_file_mult_m"];
  knight_centre_mult_e   = j[sec_name]["knight_centre_mult_e"];
  knight_centre_mult_m   = j[sec_name]["knight_centre_mult_m"];
  knight_rank_mult_m     = j[sec_name]["knight_rank_mult_m"];
  bishop_centre_mult_e   = j[sec_name]["bishop_centre_mult_e"];
  bishop_centre_mult_m   = j[sec_name]["bishop_centre_mult_m"];
  rook_file_mult_m       = j[sec_name]["rook_file_mult_m"];
  queen_centre_mult_e    = j[sec_name]["queen_centre_mult_e"];
  queen_centre_mult_m    = j[sec_name]["queen_centre_mult_m"];
  king_centre_mult_e     = j[sec_name]["king_centre_mult_e"];
  king_file_mult_m       = j[sec_name]["king_file_mult_m"];
  king_rank_mult_m       = j[sec_name]["king_rank_mult_m"];

  knight_backrank_base_m = j[sec_name]["knight_backrank_base_m"];
  knight_trapped_base_m  = j[sec_name]["knight_trapped_base_m"];
  bishop_backrank_base_m = j[sec_name]["bishop_backrank_base_m"];
  bishop_diagonal_base_m = j[sec_name]["bishop_diagonal_base_m"];
  queen_backrank_base_m  = j[sec_name]["queen_backrank_base_m"];
  pawn_weight            = j[sec_name]["pawn_weight"];
  piece_weight           = j[sec_name]["piece_weight"];
  king_weight            = j[sec_name]["king_weight"];

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
  clamp(   queen_centre_mult_e, 0, 10);
  clamp(   queen_centre_mult_m, 0, 10);
  clamp(    king_centre_mult_e, 1, 20);
  clamp(      king_file_mult_m, 1, 20);
  clamp(      king_rank_mult_m, 1, 20);

  std::cout << "Value at load: " << queen_centre_mult_m << std::endl;

  clamp(knight_backrank_base_m, 0,  20);
  clamp( knight_trapped_base_m, 0, 120);
  clamp(bishop_backrank_base_m, 0,  20);
  clamp(bishop_diagonal_base_m, 0,  20);
  clamp( queen_backrank_base_m, 0,  20);

  return true;
}

void parameters::pcsq::save(nlohmann::json &j) const
{
  j[sec_name]["pawn_file_base"]         = pawn_file_base;
  j[sec_name]["knight_centre_base"]     = knight_centre_base;
  j[sec_name]["knight_rank_base"]       = knight_rank_base;
  j[sec_name]["bishop_centre_base"]     = bishop_centre_base;
  j[sec_name]["rook_file_base"]         = rook_file_base;
  j[sec_name]["queen_centre_base"]      = queen_centre_base;
  j[sec_name]["king_centre_base"]       = king_centre_base;
  j[sec_name]["king_file_base"]         = king_file_base;
  j[sec_name]["king_rank_base"]         = king_rank_base;

  j[sec_name]["pawn_file_mult_m"]       = pawn_file_mult_m;
  j[sec_name]["knight_centre_mult_e"]   = knight_centre_mult_e;
  j[sec_name]["knight_centre_mult_m"]   = knight_centre_mult_m;
  j[sec_name]["knight_rank_mult_m"]     = knight_rank_mult_m;
  j[sec_name]["bishop_centre_mult_e"]   = bishop_centre_mult_e;
  j[sec_name]["bishop_centre_mult_m"]   = bishop_centre_mult_m;
  j[sec_name]["rook_file_mult_m"]       = rook_file_mult_m;
  j[sec_name]["queen_centre_mult_e"]    = queen_centre_mult_e;
  j[sec_name]["queen_centre_mult_m"]    = queen_centre_mult_m;
  j[sec_name]["king_centre_mult_e"]     = king_centre_mult_e;
  j[sec_name]["king_file_mult_m"]       = king_file_mult_m;
  j[sec_name]["king_rank_mult_m"]       = king_rank_mult_m;

  j[sec_name]["knight_backrank_base_m"] = knight_backrank_base_m;
  j[sec_name]["knight_trapped_base_m"]  = knight_trapped_base_m;
  j[sec_name]["bishop_backrank_base_m"] = bishop_backrank_base_m;
  j[sec_name]["bishop_diagonal_base_m"] = bishop_diagonal_base_m;
  j[sec_name]["queen_backrank_base_m"]  = queen_backrank_base_m;
  j[sec_name]["pawn_weight"]            = pawn_weight;
  j[sec_name]["piece_weight"]           = piece_weight;
  j[sec_name]["king_weight"]            = king_weight;
}

void parameters::pp_adj::init()
{
  n[0] = knight_wo_pawns_base;
  r[0] =   rook_wo_pawns_base;
  for (int i(1); i < 9; ++i)
  {
    n[i] = n[0] + knight_wo_pawns_d * i;
    r[i] = n[0] +   rook_wo_pawns_d * i;
  }
}

bool parameters::pp_adj::load(const nlohmann::json &j)
{
  knight_wo_pawns_base = j[sec_name]["knight_wo_pawns_base"];
  rook_wo_pawns_base   = j[sec_name][  "rook_wo_pawns_base"];

  knight_wo_pawns_d = j[sec_name]["knight_wo_pawns_d"];
  rook_wo_pawns_d   = j[sec_name][  "rook_wo_pawns_d"];

  clamp(knight_wo_pawns_base, -30,  0);
  clamp(  rook_wo_pawns_base,   0, 30);
  clamp(knight_wo_pawns_d,  0, 6);
  clamp(  rook_wo_pawns_d, -6, 0);

  return true;
}

void parameters::pp_adj::save(nlohmann::json &j) const
{
  j[sec_name]["knight_wo_pawns_base"] = knight_wo_pawns_base;
  j[sec_name][  "rook_wo_pawns_base"] =   rook_wo_pawns_base;

  j[sec_name]["knight_wo_pawns_d"] = knight_wo_pawns_d;
  j[sec_name][  "rook_wo_pawns_d"] =   rook_wo_pawns_d;
}

}  // namespace testudo
