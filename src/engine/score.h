/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_SCORE_H)
#define      TESTUDO_SCORE_H

#include <cmath>

namespace testudo
{

// The score unit is the Centipawn which corresponds to one hundredth of a Pawn
// unit. Fixed point representation with centipawn fractions allows a smooth
// relation of all piece values with a reasonable granularity to distinguish
// positional scores.
using score = int;

constexpr score  INF(32000);
constexpr score MATE(31000);

inline bool is_mate(score s) { return std::abs(s) >= MATE; }

}  // namespace testudo

#endif  // include guard
