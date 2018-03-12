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

constexpr unsigned FILE_A = 0, FILE_B = 1, FILE_C = 2, FILE_D = 3,
                   FILE_E = 4, FILE_F = 5, FILE_G = 6, FILE_H = 7;

inline constexpr unsigned file(square s) noexcept { return s & 7; }

// The row of the square numbered 0-7 from White's side of the board.
inline constexpr unsigned rank(square s) noexcept { return 7 - (s >> 3); }

inline constexpr bool valid(square s) noexcept { return s >= 0; }

// Players customarily refer to ranks from their own perspectives. For example:
// White's king and other pieces start on his or her first (or "back") rank,
// whereas Black calls the same rank the eighth rank; White's seventh rank is
// Black's second and so on.
inline constexpr unsigned first_rank(color c) noexcept
{ return c == BLACK ? 7 : 0; }
inline constexpr unsigned second_rank(color c) noexcept
{ return c == BLACK ? 6 : 1; }
inline constexpr unsigned seventh_rank(color c) noexcept
{ return c == BLACK ? 1 : 6; }
inline constexpr unsigned eighth_rank(color c) noexcept
{ return c == BLACK ? 0 : 7; }

// An exclusive-or with `56` performs a vertical flip of the coordinates.
inline constexpr square flip(square sq) noexcept
{ return sq ^ 56; }

// Forward moving offset for a Pawn of a specific color.
inline constexpr int step_fwd(color c) noexcept
{ return c == BLACK ? 8 : -8; }

}  // namespace testudo

#endif // include guard
