/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_CACHE_H)
#define      TESTUDO_CACHE_H

#include "move.h"
#include "zobrist.h"

namespace testudo
{

enum class score_type : std::uint8_t
{ignore, exact, fail_high /* cut */, fail_low};

// The cache class (aka transposition table) consists of a power of 2 number of
// `slot`-`slot` pairs. Each non-empty slot contains information about exactly
// one position.
//
// NOTE
// A problem that happens when you start using a transposition hash table, if
// you allow the search to cut off based upon elements in the table, is that
// your search suffers from instability.
// You get instability for at least a couple of reasons:
// - you might be trying to do a 6-ply search here, but if you have results for
//   a 10-ply search in your hash element, you might cut off based upon these.
//   Later on you come back and the element has been over-written, so you get a
//   different value back for this node.
// - the hash-key doesn't take into account the path taken to get to a node.
//   Not every path is the same. It's possible that the score in a hash element
//   might be based upon a path that would contain a repetition if encountered
//   at some other point in the tree. A repetition might result in a draw
//   score, or at least a different score.
//
// There is nothing that can be done about this.
class cache
{
public:
  class slot
  {
  public:
    constexpr slot(hash_t h = 0, move m = move::sentry(), int d = 0,
                   score_type t = score_type::ignore, score v = 0,
                   std::uint8_t a = 0) noexcept
    : hash_(h), best_move_(m), draft_(d), value_(v), type_(t), age_(a)
    {
      assert(std::numeric_limits<decltype(value_)>::min() <= v);
      assert(v <= std::numeric_limits<decltype(value_)>::max());
    }

    constexpr void save(hash_t, move, int, score_type, score,
                        std::uint8_t) noexcept;

    constexpr const hash_t &hash() const noexcept { return hash_; }
    constexpr const move &best_move() const noexcept { return best_move_; }
    constexpr int draft() const noexcept { return draft_; }
    constexpr score_type type() const noexcept { return type_; }
    constexpr score value() const noexcept { return value_; }
    constexpr std::uint8_t age() const noexcept { return age_; }

    void age(std::uint8_t a) noexcept { age_ = a; }

  private:
    hash_t        hash_;
    move     best_move_;
    int          draft_;
    std::int16_t value_;
    score_type    type_;
    std::uint8_t   age_;
  };

  explicit cache(std::uint8_t bits = 19) : tt_(1 << bits), age_(0) {}

  const slot *find(hash_t) noexcept;
  void insert(hash_t, const move &, int, score_type, score) noexcept;

  void inc_age() { ++age_; }

private:
  std::size_t get_index(hash_t h) const noexcept
  { return h & (tt_.size() - 1); }

  std::vector<std::pair<slot, slot>> tt_;
  decltype(slot::age_) age_;
};

}  // namespace testudo

#endif  // include guard
