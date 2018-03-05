/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_PIECE_H)
#define      TESTUDO_PIECE_H

#include <array>
#include <cassert>
#include <iostream>

#include "color.h"
#include "score.h"
#include "square.h"

namespace testudo
{

class piece
{
public:
  enum type {pawn = 0, king, knight, bishop, rook, queen, empty};

  using ID = std::uint8_t;
  static constexpr ID sup_id = 14;

  explicit constexpr piece(ID i = 0) noexcept : id_(i) {}
  constexpr piece(testudo::color c, unsigned t) noexcept : piece(c << 3 | t)
  {
    assert(t != empty);
  }

  constexpr ID id() const noexcept { return id_; }

  // Color of the piece (i.e. `BPAWN.color() == BLACK`. `EMPTY.color() differs
  // from `WHITE` and `BLACK`)
  constexpr std::uint8_t color() const noexcept;

  // Piece type (i.e. `BPAWN.type() == type::pawn`).
  constexpr enum type type() const noexcept;

  // When `!slide()` a piece can only move one square in any one direction.
  constexpr bool slide() const noexcept;

  // Directions the piece can move in.
  constexpr const auto &offsets() const noexcept;

  constexpr score value() const noexcept;

  constexpr char letter() const noexcept;

private:
  static constexpr std::initializer_list<int> offsets_[sup_id] =
  {
    {  9,  11},  // just the captures... pawns are special
    {-11, -10,  -9, -1, 1,  9, 10, 11},
    {-21, -19, -12, -8, 8, 12, 19, 21},
    {-11,  -9,   9, 11},
    {-10,  -1,   1, 10},
    {-11, -10,  -9, -1, 1,  9, 10, 11},

    {}, {},      // unused slots

    {-11,  -9},  // just the captures... pawns are special
    {-11, -10,  -9, -1, 1,  9, 10, 11},
    {-21, -19, -12, -8, 8, 12, 19, 21},
    {-11,  -9,   9, 11},
    {-10,  -1,   1, 10},
    {-11, -10,  -9, -1, 1,  9, 10, 11}
  };

  static constexpr score value_[empty+1] = {100, 2000, 325, 325, 500, 1000, 0};

  ID id_;
};

// Piece with *color+type* encondig. The general sequence is:
//     color << 3 | type
// `type` (3 bits) is assigned so that:
// - not sliding pieces have `type() > 2`;
// - pieces a Pawn can be promoted to have `type() > 1`.
constexpr piece   BPAWN(0b00000);
constexpr piece   BKING(0b00001);
constexpr piece BKNIGHT(0b00010);
constexpr piece BBISHOP(0b00011);
constexpr piece   BROOK(0b00100);
constexpr piece  BQUEEN(0b00101);
//        piece  UNUSED(0b00110);
//        piece  UNUSED(0b00111);
constexpr piece   WPAWN(0b01000);
constexpr piece   WKING(0b01001);
constexpr piece WKNIGHT(0b01010);
constexpr piece WBISHOP(0b01011);
constexpr piece   WROOK(0b01100);
constexpr piece  WQUEEN(0b01101);
//
constexpr piece   EMPTY(0b10110);

inline constexpr bool operator==(piece lhs, piece rhs)
{
  return lhs.id() == rhs.id();
}
inline constexpr bool operator!=(piece lhs, piece rhs)
{
  return !(lhs == rhs);
}

// The return value of this function is a `std::uint8_t` (and not a `color`).
// This is wanted and very useful since a third value (not equal to 'BLACK'
// or 'WHITE') is used to mark empty squares.
// To identify enemy pieces we can write:
//
//     if (board_[i].color() == !side()) ...
//
// instead of
//
//     if (board_[i] != EMPTY && board_[i].color() != side()) ...
//
// which is faster. Anyway consider that:
//
//    if (board_[i].color() == !side()) ...
//
// and
//
//    if (board_[i].color() != side()) ...
//
// are NOT equivalent. The first expression checks if the `i`th square contains
// an enemy piece, the second one if it contains an enemy piece or is empty.
inline constexpr std::uint8_t piece::color() const noexcept
{
  return id() >> 3;
}

inline constexpr enum piece::type piece::type() const noexcept
{
  return static_cast<enum type>(id() & 0b00111);
}

inline constexpr bool piece::slide() const noexcept
{
  assert(*this != EMPTY);
  return type() > 2;
}

inline constexpr const auto &piece::offsets() const noexcept
{
  assert(*this != EMPTY);
  return offsets_[id()];
}

inline constexpr score piece::value() const noexcept
{
  return value_[type()];
}

inline constexpr char piece::letter() const noexcept
{
  assert(*this != EMPTY);
  constexpr char letter_[sup_id] =
  {
    'p', 'k', 'n', 'b', 'r', 'q', '?', '?', 'P', 'K', 'N', 'B', 'R', 'Q'
  };
  return letter_[id()];
}

inline std::ostream &operator<<(std::ostream &o, piece p)
{
  return o << p.letter();
}

}  // namespace testudo

#endif  // include guard
