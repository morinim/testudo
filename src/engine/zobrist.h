/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_ZOBRIST_H)
#define      TESTUDO_ZOBRIST_H

#include <array>

#include "piece.h"

namespace testudo
{

class state;

using hash_t = std::uint64_t;

// Zobrist hashing is a hash function construction used in computer programs
// that play board games to implement transposition tables.
//
// At program initialization, we generate arrays of pseudorandom numbers:
// - one number for each piece at each square (see piece.h);
// - one number to indicate the side to move is BLACK;
// - sixteen numbers to indicate the castling rights;
// - eight numbers to indicate the file of a valid en passant square, if any.
//
// If we now want to get the Zobrist hash code of a certain position, we
// initialize the hash key by xoring all random numbers linked to the given
// feature.
namespace zobrist
{

extern std::array<std::array<hash_t, 64>, piece::sup_id> piece;
extern const hash_t side;
extern const std::array<hash_t, 8> ep;
extern const std::array<hash_t, 16> castle;

hash_t hash(const state &) noexcept;

}  // namespace zobrist

}  // namespace testudo

#endif  // include guard
