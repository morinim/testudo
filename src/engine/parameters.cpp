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
void clamp(T &v, T lo, T hi)
{
  assert(lo <= hi);
  if (v < lo)
    v = lo;
  else if (v > hi)
    v = hi;
}

}  // unnamed namespace

const std::string parameters::pawn::sec_name   =   "pawn";
const std::string parameters::pcsq::sec_name   =   "pcsq";
const std::string parameters::pp_adj::sec_name = "pp_adj";

parameters db;

parameters::parameters()
{
  if (!load())
  {
    testudoINFO << "Using default values for some/all parameters";
  }

  pawn_.init();
  pcsq_.init();
  pp_adj_.init();
}

bool parameters::load()
{
  std::ifstream f("testudo.json");
  if (!f)
    return false;

  nlohmann::json j;
  if (!(f >> j))
    return false;

  bool ret(true);

  if (!pcsq_.load(j))
  {
    testudoWARNING << "Partial initialization of the parameters "
                      "(missing 'pcsq' section)";
    ret = false;
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
    ret = false;
  }

  if (!pawn_.load(j))
  {
    testudoWARNING << "Partial initialization of the parameters "
                      "(missing 'pawn' section)";
    ret =  false;
  }

  return ret;
}

bool parameters::save() const
{
  nlohmann::json j;

  pcsq_.save(j);

  j["material"]["bishop_pair"] = bishop_pair_;
  j["material"]["knight_pair"] = knight_pair_;
  j["material"]["rook_pair"]   =   rook_pair_;

  pp_adj_.save(j);

  pawn_.save(j);

  std::ofstream f("testudo.json");
  return !!f && f << j;
}

void parameters::pawn::init()
{
  const auto scale([](int y_min, int y_max, int x_min, int x_max, int x)
                   {
                     assert(y_min <= y_max);
                     assert(x_min < x_max);
                     assert(x_min <= x && x <= x_max);

                     const auto dy(y_max - y_min), dx(x_max - x_min);
                     return (dy * (x - x_min) + y_min * dx) / dx;
                   });

  passed_e[0] = passed_m[0] = 0;  // not used - just for security

  for (int r(1); r < 7; ++r)
  {
    passed_e[r] = scale(passed_min_e, passed_max_e, 1, 6, r);
    passed_m[r] = scale(passed_min_m, passed_max_m, 1, 6, r);

    protected_passed_e[r] = passed_e[r] * protected_passed_perc / 100;
  }
  assert(passed_e[6] == passed_max_e);
  assert(passed_e[1] == passed_min_e);
  assert(passed_m[6] == passed_max_m);
  assert(passed_m[1] == passed_min_m);
  assert(protected_passed_e[6] == passed_max_e * protected_passed_perc / 100);
  assert(protected_passed_e[1] == passed_min_e * protected_passed_perc / 100);

  for (unsigned f(0); f < 8; ++f)
  {
    const auto dist(f >= FILE_E ? FILE_H - f : f - FILE_A);

    weak_e[f] = scale(weak_min_e, weak_max_e, 0, 3, dist);
    weak_m[f] = scale(weak_min_m, weak_max_m, 0, 3, dist);

    weak_open_e[f] = weak_e[f] * weak_open_perc / 100;
    weak_open_m[f] = weak_m[f] * weak_open_perc / 100;
  }

  assert(weak_e[FILE_A] == weak_e[FILE_H]);
  assert(weak_m[FILE_A] == weak_m[FILE_H]);
  assert(weak_open_e[FILE_A] == weak_open_e[FILE_H]);
  assert(weak_open_m[FILE_A] == weak_open_m[FILE_H]);

  assert(weak_e[FILE_A] == weak_min_e);
  assert(weak_m[FILE_A] == weak_min_m);
  assert(weak_e[FILE_D] == weak_max_e);
  assert(weak_m[FILE_D] == weak_max_m);
  assert(weak_open_e[FILE_A] = weak_min_e * weak_open_perc / 100);
  assert(weak_open_m[FILE_A] = weak_min_m * weak_open_perc / 100);
  assert(weak_open_e[FILE_D] = weak_max_e * weak_open_perc / 100);
  assert(weak_open_m[FILE_D] = weak_max_m * weak_open_perc / 100);
}

bool parameters::pawn::load(const nlohmann::json &j)
{
  doubled_e =  j[sec_name]["doubled_e"];
  doubled_m =  j[sec_name]["doubled_m"];

  passed_min_e = j[sec_name]["passed_min_e"];
  passed_max_e = j[sec_name]["passed_max_e"];

  passed_min_m = j[sec_name]["passed_min_m"];
  passed_max_m = j[sec_name]["passed_max_m"];

  protected_passed_perc = j[sec_name]["protected_passed_perc"];

  shield1 = j[sec_name]["king_shield1"];
  shield2 = j[sec_name]["king_shield2"];

  weak_min_e = j[sec_name]["weak_min_e"];
  weak_max_e = j[sec_name]["weak_max_e"];

  weak_min_m = j[sec_name]["weak_min_m"];
  weak_max_m = j[sec_name]["weak_max_m"];

  weak_open_perc = j[sec_name]["weak_open_perc"];

  clamp(doubled_e,       -30, 0);
  clamp(doubled_m, doubled_e, 0);

  clamp(passed_min_m,            0, passed_min_e);
  clamp(passed_max_m, passed_min_m, passed_max_e);

  clamp(passed_min_e, passed_min_m, 100);
  clamp(passed_max_e, passed_min_e, 140);

  clamp(protected_passed_perc, 100, 200);

  clamp(shield1, 0,      20);
  clamp(shield2, 0, shield1);

  clamp(weak_min_e,          0, 20);
  clamp(weak_max_e, weak_min_e, 40);

  clamp(weak_min_m,          0, 20);
  clamp(weak_max_m, weak_min_m, 40);

  clamp(weak_open_perc, 100, 200);

  return true;
}

void parameters::pawn::save(nlohmann::json &j) const
{
  j[sec_name]["doubled_e"] = doubled_e;
  j[sec_name]["doubled_m"] = doubled_m;

  j[sec_name]["passed_min_e"] = passed_min_e;
  j[sec_name]["passed_max_e"] = passed_max_e;

  j[sec_name]["passed_min_m"] = passed_min_m;
  j[sec_name]["passed_max_m"] = passed_max_m;

  j[sec_name]["protected_passed_perc"] = protected_passed_perc;

  j[sec_name]["king_shield1"] = shield1;
  j[sec_name]["king_shield2"] = shield2;

  j[sec_name]["weak_min_e"] = weak_min_e;
  j[sec_name]["weak_max_e"] = weak_max_e;

  j[sec_name]["weak_min_m"] = weak_min_m;
  j[sec_name]["weak_max_m"] = weak_max_m;

  j[sec_name]["weak_open_perc"] = weak_open_perc;
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
