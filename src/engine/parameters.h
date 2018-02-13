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

namespace testudo
{

namespace detail
{
extern score pcsq_m_[piece::sup_id][64];
extern score pcsq_e_[piece::sup_id][64];
}

class parameters
{
public:
  parameters();

  score pcsq_e(piece p, square s) const { return detail::pcsq_e_[p.id()][s]; }
  score pcsq_m(piece p, square s) const { return detail::pcsq_m_[p.id()][s]; }

  bool save() const;

private:
  bool load();
  void pcsq_init();

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
    score queen_centre_mult_e  =  4;
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
  } pcsq_;
};  // class parameters

extern parameters db;

}  // namespace testudo

#endif  // include guard
