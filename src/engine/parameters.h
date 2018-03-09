/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_PARAMETERS_H)
#define      TESTUDO_PARAMETERS_H

#include "piece.h"
#include "score.h"
#include "thirdparty/json.hpp"

namespace testudo
{

class parameters
{
public:
  parameters();

  score pcsq_e(piece p, square s) const { return pcsq_.eg[p.id()][s]; }
  score pcsq_m(piece p, square s) const { return pcsq_.mg[p.id()][s]; }

  score bishop_pair() const noexcept { return bishop_pair_; }
  score knight_pair() const noexcept { return knight_pair_; }
  score rook_pair() const noexcept { return rook_pair_; }

  score n_adj(unsigned pawns) const
  { assert(pawns < 9);  return pp_adj_.n[pawns]; }
  score r_adj(unsigned pawns) const
  { assert(pawns < 9);  return pp_adj_.r[pawns]; }

  bool save() const;

private:
  bool load();

  // Naming conventions:
  // 1. piece type (`pawn_`, `knight_`...)
  // 2. type (`centre_`, `rank_`, `file_`...)
  // 3. kind (`base_`, `mult_`, `weight_`...)
  // 4. phase (`e_` end-game, `m_` opening/middle-game). If missing the
  //    parameter isn't phase specific
  struct pcsq
  {
    score pawn_file_mult_m     =  5;  // [1; 10]
    score knight_centre_mult_e =  5;
    score knight_centre_mult_m =  5;
    score knight_rank_mult_m   =  5;
    score bishop_centre_mult_e =  3;
    score bishop_centre_mult_m =  2;
    score rook_file_mult_m     =  3;
    score queen_centre_mult_e  =  4;  // [0; 10]
    score queen_centre_mult_m  =  0;
    score king_centre_mult_e   = 12;  // [1; 20]
    score king_file_mult_m     = 10;
    score king_rank_mult_m     = 10;

    std::array<score, 4> pawn_file_base    { {-3,-1, 0, 1} };  // [-20; 20]
    std::array<score, 4> knight_centre_base{ {-4,-2, 0, 1} };
    std::array<score, 8> knight_rank_base  { {-2,-1, 0, 1, 2, 3, 2, 1} };
    std::array<score, 4> bishop_centre_base{ {-3,-1, 0, 1} };
    std::array<score, 4> rook_file_base    { {-2,-1, 0, 1} };
    std::array<score, 4> queen_centre_base { {-3,-1, 0, 1} };
    std::array<score, 4> king_centre_base  { {-3,-1, 0, 1} };
    std::array<score, 4> king_file_base    { { 3, 4, 2, 0} };
    std::array<score, 8> king_rank_base    { { 1, 0,-2,-3,-4,-5,-6,-7} };

    score knight_backrank_base_m =   0;  // [ 0;  20]
    score knight_trapped_base_m  = 100;  // [ 0; 120]
    score bishop_backrank_base_m =  10;  // [ 0;  20]
    score bishop_diagonal_base_m =   4;
    score queen_backrank_base_m  =   5;

    score pawn_weight  = 100;  // [  0; 200]
    score piece_weight = 100;
    score king_weight  = 100;

    void init();
    bool load(const nlohmann::json &);
    void save(nlohmann::json &) const;

    // Piece/square tables for opening/middle-game (`mg`) and end-game (`eg`).
    //
    // They're calculated starting from the other `pcsq_` data member. This
    // approach allows to have a more narrow set of variables subject to
    // optimization.
    score mg[piece::sup_id][64];
    score eg[piece::sup_id][64];

  private:
    static const std::string sec_name;
  } pcsq_;

  score bishop_pair_ =  30;  // [  0; 50]
  score knight_pair_ =  -5;  // [-20; 20]
  score rook_pair_   = -16;  // [-30; 30]

  // Adjustements of piece value based on the number of pawns.
  struct pp_adj
  {
    score knight_wo_pawns_base = -20;  // [-30;  0]
    score rook_wo_pawns_base   =  15;  // [  0; 30]

    score knight_wo_pawns_d  =  4;  // [ 0; 6]
    score rook_wo_pawns_d    = -3;  // [-6; 0]

    void init();
    bool load(const nlohmann::json &);
    void save(nlohmann::json &) const;

    score n[9];
    score r[9];

  private:
    static const std::string sec_name;
  } pp_adj_;
};  // class parameters

extern parameters db;

}  // namespace testudo

#endif  // include guard
