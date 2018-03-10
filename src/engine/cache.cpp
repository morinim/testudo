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
#include "search.h"

namespace testudo
{

// Fills the slot with the given information. The procedure may keep some of
// the existing information if they're about the same position.
inline constexpr void cache::slot::save(hash_t h, move m, int d, score_type t,
                                        score v, std::uint8_t a) noexcept
{
  assert(std::numeric_limits<decltype(value_)>::min() <= v);
  assert(v <= std::numeric_limits<decltype(value_)>::max());

  // Preserve any existing move for the same position.
  if (m || hash_ != h)
    best_move_ = m;

  hash_  = h;
  draft_ = d;
  value_ = v;
  type_  = t;
  age_   = a;
}

// Looks up a position in the cache. Returns pointer to a `slot` if the
// position is found. Otherwise, returns `nullptr`.
// If available, we prefer the information of the always-replace slot.
// The logic follows:
// > Draft is a highly over-valued property of TT entries. Of course a hit on
// > an entry of very high draft (with OK bounds) can save you a lot of work,
// > much more than on an entry with low draft. But the point that is often
// > overlooked, is that hash probes that need high draft are much less
// > frequent than probes for which a low draft already suffices.
// > From the viewpoint of the hash slot, having a slot store a single d=20
// > entry on which you get 1 hit does less good than a slot that stored 1000
// > different d=10 entries on which you get 5 hits each. So it is in fact not
// > obvious that preferring high drafts buys you anything at all.
// > A much more important effect is that the distribution of hash hits on a
// > given position decreases in time. Immediately after the entry was created,
// > there is a large probability it will be quickly revisited, through a
// > transposition of recent moves (i.e. moves close to the leaves). After that
// > you become dependent on transpositions with moves closer to the root, and
// > such moves have a much slower turnover.
// (H.G. Muller)
const cache::slot *cache::find(hash_t h) noexcept
{
  auto &elem(tt_[get_index(h)]);

  if (elem.first.hash() == h)
  {
    // The replace-always slot doesn't use the age information.
    // elem.first.age(age_);
    return &elem.first;
  }

  if (elem.second.hash() == h)
  {
    elem.second.age(age_);
    return &elem.second;
  }

  return nullptr;
}

// When you get a result from a search, and you want to store an element in the
// table, it's important to record how deep the search went (`draft`). If you
// searched this position with a 3-ply search and you come along later planning
// to do a 10-ply search, you can't assume that the information in this hash
// element is accurate. So the search draft for the sub-tree is recorded.
//
// In an alpha-beta search, you rarely get an exact value when you search a
// node. "alpha" and "beta" exist to help you prune out useless sub-trees, but
// the minor disadvantage to using alpha-beta is that you don't often know
// exactly how bad or good a node is, you just know that it is bad enough or
// good enough that you don't need to waste any more time on it.
//
// Of course, this raises the question as to what value you store in the hash
// element and what you can do with it when you retrieve it. The answer is to
// store a value (`v`) and a flag (`t`) that indicates what the value means.
// If you store, let's say, a 16 in the `value` field, and `exact` in the
// `type` field, this means that the value of the node was exactly 16. If you
// store `fail_low` in the `type` field, the value of the node was at most 16.
// If you store `fail_high`, the value is at least 16.
//
// As for the replacement strategy we use a two-tier system (the strategy was
// devised by Ken Thompson and Joe Condon): for each table entry there is a
// always-replace and depth-preferred slot.
void cache::insert(hash_t h, const move &m, int draft, score_type t,
                   score v) noexcept
{
  // Adjusts mate scores.
  // > Mate scores are weird because they change depending upon where in the
  // > tree they are found.  When stored in the hash table, additional
  // > weirdness can result. This problem can be solved by converting any mate
  // > scores to bounds, then storing the bounds.
  // > A few weird cases were mates that are failing low, and "-mates" that are
  // > failing high. These are stored without any bound information, so it's
  // > not possible to cut off on these later.
  // (Bruce Moreland)
  if (v >= MATE)
  {
    if (t == score_type::fail_low)  // failing low on MATE
      v = +INF;                     // don't allow a cutoff later
    else
    {
      t = score_type::fail_high;    // exact/fail-high, turned into a fail-high
      v = MATE;
    }
  }
  else if (v <= -MATE)
  {
    if (t == score_type::fail_high) // fail high on -MATE
      v = -INF;                     // dont't allow cutoff later
    else
    {
      t = score_type::fail_low;     // exact/fail-low, turned into a fail-low
      v = -MATE;
    }
  }

  auto &elem(tt_[get_index(h)]);

  // Always replace slot.
  elem.first.save(h, m, draft, t, v, age_);

  // Depth preferred slot.
  // Here, using a "replace if deeper or same depth" scheme, the cache might
  // eventually fill up with outdated deep nodes. The solution to this is add a
  // "age" field to the slot, so the replacement scheme becomes: "replace if
  // same depth, deeper or the element pertains to an ancient search".
  if (elem.second.age() != age_
      || elem.second.draft() <= draft)
    elem.second.save(h, m, draft, t, v, age_);
}

}  // namespace testudo
