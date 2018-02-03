/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_SQUARE_H)
#define      TESTUDO_SQUARE_H

#include <cstdint>

#include "color.h"

namespace testudo
{

using square = std::int8_t;

// Useful squares.
constexpr square A8= 0, B8= 1, C8= 2, D8= 3, E8= 4, F8= 5, G8= 6, H8= 7,
                 A7= 8, B7= 9, C7=10, D7=11, E7=12, F7=13, G7=14, H7=15,
                 A6=16, B6=17, C6=18, D6=19, E6=20, F6=21, G6=22, H6=23,
                 A5=24, B5=25, C5=26, D5=27, E5=28, F5=29, G5=30, H5=31,
                 A4=32, B4=33, C4=34, D4=35, E4=36, F4=37, G4=38, H4=39,
                 A3=40, B3=41, C3=42, D3=43, E3=44, F3=45, G3=46, H3=47,
                 A2=48, B2=49, C2=50, D2=51, E2=52, F2=53, G2=54, H2=55,
                 A1=56, B1=57, C1=58, D1=59, E1=60, F1=61, G1=62, H1=63;

inline constexpr unsigned file(square s) { return s & 7; }
inline constexpr unsigned rank(square s) { return 7 - (s >> 3); }
inline constexpr bool valid(square s) { return s >= 0; }
inline constexpr unsigned pawn_base_rank(color c) { return c==BLACK ? 6 : 1; }

}  // namespace testudo

#endif // include guard
