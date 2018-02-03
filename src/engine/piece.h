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
#include <iostream>

#include "color.h"
#include "random.h"
#include "score.h"
#include "square.h"
#include "zobrist.h"

namespace testudo
{

class piece
{
public:
  enum type {empty = 0, pawn, king, knight, bishop, rook, queen};

  using ID = std::uint8_t;

  explicit constexpr piece(ID i = 0) noexcept : id_(i) {}
  constexpr piece(testudo::color c, unsigned t) noexcept : piece(2 * t + c) {}

  constexpr ID id() const noexcept { return id_; }

  // Color of the piece (i.e. `BPAWN.color() == BLACK`).
  constexpr testudo::color color() const noexcept;

  // Piece type (i.e. `BPAWN.type() == type::pawn`).
  constexpr enum type type() const noexcept;

  // When `!slide()` a piece can only move one square in any one direction.
  constexpr bool slide() const noexcept;

  // Directions the piece can move in.
  constexpr const auto &offsets() const noexcept;

  constexpr score value() const noexcept;

  constexpr char letter() const noexcept;

  hash_t hash(square) const noexcept;

private:
  static constexpr ID sup_id = 14;

  static constexpr std::initializer_list<int> offsets_[sup_id] =
  {
    {}, {},
    {}, {},
    {-11, -10,  -9, -1, 1,  9, 10, 11},
    {-11, -10,  -9, -1, 1,  9, 10, 11},
    {-21, -19, -12, -8, 8, 12, 19, 21},
    {-21, -19, -12, -8, 8, 12, 19, 21},
    {-11,  -9,   9, 11},
    {-11,  -9,   9, 11},
    {-10,  -1,   1, 10},
    {-10,  -1,   1, 10},
    {-11, -10,  -9, -1, 1,  9, 10, 11},
    {-11, -10,  -9, -1, 1,  9, 10, 11},
  };

  static constexpr score value_[sup_id] = {0, 100, 10000, 345, 355, 525, 1000};

  ID id_;
};

// Piece with *type+color* encondig. The general sequence is:
//     (type << 1) | color
// `type` (3 bits) is assigned so that:
// - the most significant bit is set for sliding pieces;
// - pieces a Pawn can be promoted to have `type() > 2`.
constexpr piece   EMPTY(0b0000);
//        piece  UNUSED(0b0001);
constexpr piece   BPAWN(0b0010);
constexpr piece   WPAWN(0b0011);
constexpr piece   BKING(0b0100);
constexpr piece   WKING(0b0101);
constexpr piece BKNIGHT(0b0110);
constexpr piece WKNIGHT(0b0111);
constexpr piece BBISHOP(0b1000);
constexpr piece WBISHOP(0b1001);
constexpr piece   BROOK(0b1010);
constexpr piece   WROOK(0b1011);
constexpr piece  BQUEEN(0b1100);
constexpr piece  WQUEEN(0b1101);

inline constexpr testudo::color piece::color() const noexcept
{
  return id() & 1;
}

inline constexpr enum piece::type piece::type() const noexcept
{
  return static_cast<enum type>(id() >> 1);
}

inline constexpr bool piece::slide() const noexcept
{
  return id() & 0b1000;
}

inline constexpr const auto &piece::offsets() const noexcept
{
  return offsets_[id()];
}

inline constexpr score piece::value() const noexcept
{
  return value_[type()];
}

inline constexpr char piece::letter() const noexcept
{
  constexpr char letter_[sup_id] =
  {
    '1', '?',
    'p', 'P', 'k', 'K', 'n', 'N', 'b', 'B', 'r', 'R', 'q', 'Q'
  };
  return letter_[id()];
}

inline hash_t piece::hash(square i) const noexcept
{
  static const auto hash_ =
    random::fill2d<std::array<std::array<hash_t, 64>, piece::sup_id>>();

  return hash_[id()][i];
}

inline constexpr bool operator==(piece lhs, piece rhs)
{
  return lhs.id() == rhs.id();
}
inline constexpr bool operator!=(piece lhs, piece rhs)
{
  return !(lhs == rhs);
}

inline std::ostream &operator<<(std::ostream &o, piece p)
{
  return o << p.letter();
}

}  // namespace testudo

#endif  // include guard
