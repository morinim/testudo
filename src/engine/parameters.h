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

private:
  bool load();

  void pcsq_init();

  // Naming conventions:
  // 1. fixed prefix (`pcsq_` standing for PieCe SQuare)
  // 2. piece type (`pawn_`, `knight_`...)
  // 3. type (`centre_`, `rank_`, `file_`...)
  // 4. kind (`base_`, `mult_`, `weight_`...)
  // 5. phase (`e_` end-game, `m_` opening/middle-game). If missing the
  //    parameter isn't phase specific
  static score pcsq_pawn_file_mult_m_;                   // [  1;  10]
  static score pcsq_knight_centre_mult_e_;               // [  1;  10]
  static score pcsq_knight_centre_mult_m_;               // [  1;  10]
  static score pcsq_knight_rank_mult_m_;                 // [  1;  10]
  static score pcsq_bishop_centre_mult_e_;               // [  1;  10]
  static score pcsq_bishop_centre_mult_m_;               // [  1;  10]
  static score pcsq_rook_file_mult_m_;                   // [  1;  10]
  static score pcsq_queen_centre_mult_e_;                // [  1;  10]
  static score pcsq_queen_centre_mult_m_;                // [  1;  10]
  static score pcsq_king_centre_mult_e_;                 // [  1;  20]
  static score pcsq_king_file_mult_m_;                   // [  1;  20]
  static score pcsq_king_rank_mult_m_;                   // [  1;  20]

  static std::array<score, 4> pcsq_pawn_file_base_;      // [-20;  20]
  static std::array<score, 4> pcsq_knight_centre_base_;  // [-20;  20]
  static std::array<score, 8> pcsq_knight_rank_base_;    // [-20;  20]
  static std::array<score, 4> pcsq_bishop_centre_base_;  // [-20;  20]
  static std::array<score, 4> pcsq_rook_file_base_;      // [-20;  20]
  static std::array<score, 4> pcsq_queen_centre_base_;   // [-20;  20]
  static std::array<score, 4> pcsq_king_centre_base_;    // [-20;  20]
  static std::array<score, 4> pcsq_king_file_base_;      // [-20;  20]
  static std::array<score, 8> pcsq_king_rank_base_;      // [-20;  20]

  static score pcsq_knight_backrank_base_m_;             // [  0;  20]
  static score pcsq_knight_trapped_base_m_;              // [  0; 120]
  static score pcsq_bishop_backrank_base_m_;             // [  0;  20]
  static score pcsq_bishop_diagonal_base_m_;             // [  0;  20]
  static score pcsq_queen_backrank_base_m_;              // [  0;  20]

  static score pcsq_pawn_weight_;                        // [  0; 200]
  static score pcsq_piece_weight_;                       // [  0; 200]
  static score pcsq_king_weight_;                        // [  0; 200]
};  // class parameters

extern parameters db;

}  // namespace testudo

#endif  // include guard
