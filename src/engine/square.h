/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <random>

#if !defined(TESTUDO_SQUARE_H)
#define      TESTUDO_SQUARE_H

namespace testudo
{

using square = std::int8_t;

// Useful squares.
constexpr square A1 = 56;
constexpr square B1 = 57;
constexpr square C1 = 58;
constexpr square D1 = 59;
constexpr square E1 = 60;
constexpr square F1 = 61;
constexpr square G1 = 62;
constexpr square H1 = 63;
constexpr square A8 =  0;
constexpr square B8 =  1;
constexpr square C8 =  2;
constexpr square D8 =  3;
constexpr square E8 =  4;
constexpr square F8 =  5;
constexpr square G8 =  6;
constexpr square H8 =  7;

inline constexpr unsigned file(square s) { return s & 7; }
inline constexpr unsigned rank(square s) { return 7 - (s >> 3); }
inline constexpr bool valid(square s) { return s >= 0; }

}  // namespace testudo

#endif // include guard
