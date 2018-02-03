/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cassert>

#include "cache.h"

namespace testudo
{

const cache::info *cache::find(hash_t h) const noexcept
{
  const auto &p(tt_[get_index(h)]);

  if (p.first.hash == h)
    return &p.first;
  else if (p.second.hash == h)
    return &p.second;

  return nullptr;
}

// When you get a result from a search, and you want to store an element in the
// table, it's important to record how deep the search went. If you searched
// this position with a 3-ply search, and you come along later planning to do a
// 10-ply search, you can't assume that the information in this hash element is
// accurate. So the search draft for the sub-tree is also recorded.
// In an alpha-beta search, rarely do you get an exact value when you search a
// node.  "Alpha" and "beta" exist to help you prune out useless sub-trees, but
// the minor disadvantage to using alpha-beta is that you don't often know
// exactly how bad or good a node is, you just know that it is bad enough or
// good enough that you don't need to waste any more time on it.
//
// Of course, this raises the question as to what value you store in the hash
// element, and what you can do with it when you retrieve it. The answer is to
// store a value, and a flag that indicates what the value means.
// If you store, let's say, a 16 in the `value` field, and `exact` in the
// `type` field, this means that the value of the node was exactly 16. If you
// store `fail_low` in the `type` field, the value of the node was at most 16.  // If you store `fail_high`, the value is at least 16.
void cache::insert(hash_t h, const move &m, int draft, score_type t,
                   score v) noexcept
{
  // Adjusts mate scores.
  // > Mate scores are weird because they change depending upon where in the
  // > tree they are found.  When stored in the hash table, additional
  // > weirdness can result. This problem can be solved by converting any mate
  // > scores to bounds, then storing the bounds.
  // > A few weird cases were mates that are failing low, and -MATES that are
  // > failing high. These are stored without any bound information, so it's
  // > not possible to cut off on these later.
  // (Bruce Moreland)
  if (v >= MATE - 500)
  {
    if (t == score_type::fail_low)  // failing low on MATE
      t = score_type::ignore;       // don't allow a cutoff later
    else
    {
      t = score_type::fail_high;    // exact/fail-high, turned into a fail-high
      v = MATE - 500;
    }
  }
  else if (v <= -MATE + 500)
  {
    if (t == score_type::fail_high) // fail high on -MATE
      t = score_type::ignore;       // dont't allow cutoff later
    else
    {
      t = score_type::fail_low;     // exact/fail-low, turned into a fail-low
      v = -MATE + 500;
    }
  }

  info i(h, m, draft, t, v, age_);

  auto &entry(tt_[get_index(h)]);
  if (entry.first.draft <= i.draft || entry.first.age != age_)
    entry.first = i;
  else
    entry.second = i;
}

}  // namespace testudo
