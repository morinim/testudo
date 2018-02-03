/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_MOVE_H)
#define      TESTUDO_MOVE_H

#include "square.h"
#include "piece.h"

namespace testudo
{

struct move
{
  enum {capture = 1, castle = 2, en_passant = 4, two_squares = 8,
        pawn = 16, promotion_n = 32, promotion_b = 64, promotion_r = 128,
        promotion_q = 256,
        promotion = promotion_n|promotion_b|promotion_r|promotion_q};

  constexpr move(square from_sq, square to_sq, unsigned move_flags) noexcept
    : from(from_sq), to(to_sq), flags(move_flags)
  {
  }

  constexpr enum piece::type promote() const noexcept;

  // A sentinel value (empty move, end of iteration...).
  static constexpr move sentry() noexcept { return move(-1, -1, 0); }
  constexpr bool is_sentry() const noexcept { return !valid(from); }

  square    from;
  square      to;
  unsigned flags;  // capture, castle, en passant, pushing a pawn two squares,
                   // pawn move, promotion
};

inline constexpr bool operator==(const move &lhs, const move &rhs) noexcept
{
  return lhs.from == rhs.from && lhs.to == rhs.to
         && lhs.flags == rhs.flags;
}
inline constexpr bool operator!=(const move &lhs, const move &rhs) noexcept
{
  return !(lhs == rhs);
}
inline constexpr bool operator!(const move &m) noexcept
{
  return m.is_sentry();
}

inline constexpr enum piece::type move::promote() const noexcept
{
  return
    flags & promotion_q ?  piece::queen :
    flags & promotion_r ?   piece::rook :
    flags & promotion_b ? piece::bishop :
    flags & promotion_n ? piece::knight : piece::empty;
}

// Move in coordinate notation (g1f3, a7a8q).
std::ostream &operator<<(std::ostream &, const move &);

}  // namespace testudo

#endif  // include guard
