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
  using flags_t = std::uint16_t;
  enum
  {
    capture = 1, castle = 2, en_passant = 4, two_squares = 8, pawn = 16,
    promotion_n = 32, promotion_b = 64, promotion_r = 128, promotion_q = 256
  };

  move() = default;
  constexpr move(square from_sq, square to_sq, flags_t move_flags) noexcept
    : from(from_sq), to(to_sq), flags(move_flags)
  {
  }

  constexpr enum piece::type promote() const noexcept;

  // A sentinel value (empty move, end of iteration...).
  static constexpr move sentry() noexcept { return move(0, 0, 0); }
  constexpr bool is_sentry() const noexcept { return from == to; }

  constexpr explicit operator bool() const noexcept { return !is_sentry(); }

  square    from;
  square      to;
  flags_t  flags;  // capture, castle, en passant, pushing a pawn two squares,
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

inline constexpr enum piece::type move::promote() const noexcept
{
  return
    flags & promotion_q ?  piece::queen :
    flags & promotion_r ?   piece::rook :
    flags & promotion_b ? piece::bishop :
    flags & promotion_n ? piece::knight : piece::empty;
}

inline constexpr bool is_capture(const move &m) noexcept
{
  return m.flags & move::capture;
}

inline constexpr bool is_promotion(const move &m) noexcept
{
  return m.flags
         & (move::promotion_n|move::promotion_b
            |move::promotion_r|move::promotion_q);
}

inline constexpr bool is_quiet(const move &m)
{
  return !(m.flags & (move::capture
                      |move::promotion_n|move::promotion_b
                      |move::promotion_r|move::promotion_q));
}

// Move in coordinate notation (g1f3, a7a8q).
std::ostream &operator<<(std::ostream &, const move &);

}  // namespace testudo

#endif  // include guard
