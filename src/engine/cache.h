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

// cache class (aka transposition table)
//
// Every element of the cache has two slots:
// - the first slot operates via a "replace if deeper or more recent search"
//   replacement scheme.
//   Every time a new search is started, a sequence number is incremented. If
//   you try to insert an element into the table, it is inserted if the
//   sequence number now is different than the one associated with the element
//   in the table. If the sequence numbers are the same, an element will be
//   inserted only if the depth associated with the element you are trying to
//   add is greater than the depth associated with the element already here;
// - the second slot operates via a "replace always" scheme. If you try to
//   insert an element into the table, it always is inserted.
//
// This scheme is due to Ken Thompson. His idea is that deep searches can stick
// around for a while, while stuff that is very recent can also stay around,
// even if it is shallow.
//
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
  struct info
  {
    info(hash_t h = 0, move m = move::sentry(), int d = 0,
         score_type t = score_type::fail_low, score v = -INF,
         std::uint8_t a = 0)
      : hash(h), best_move(m), draft(d), type(t), value(v), age(a)
    {
      assert(std::numeric_limits<decltype(value)>::min() <= v);
      assert(v <= std::numeric_limits<decltype(value)>::max());
    }

    hash_t         hash;
    move      best_move;
    int           draft;
    std::uint16_t value;
    score_type     type;
    std::uint8_t    age;
  };

  explicit cache(std::uint8_t bits = 18) : tt_(1 << bits), age_(0) {}

  const info *find(hash_t) const noexcept;
  void insert(hash_t, const move &, int, score_type, score) noexcept;

  void inc_age() { ++age_; }

private:
  std::size_t get_index(hash_t h) const noexcept
  { return h & (tt_.size() - 1); }

  std::vector<std::pair<info, info>> tt_;
  decltype(info::age) age_;
};

}  // namespace testudo

#endif  // include guard
