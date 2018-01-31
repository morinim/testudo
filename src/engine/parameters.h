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

class parameters
{
public:
  parameters();

  score pcsq_e(piece p, square s) const { return pcsq_e_[p.id()][s]; }
  score pcsq_m(piece p, square s) const { return pcsq_m_[p.id()][s]; }

private:
  // Piece/square values for middle game.
  static score pcsq_m_[][64];
  // Piece/square values for end game.
  static score pcsq_e_[][64];
};  // class parameters

extern parameters db;

}  // namespace testudo

#endif  // include guard
