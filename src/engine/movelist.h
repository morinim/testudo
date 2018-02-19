/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_MOVELIST_H)
#define      TESTUDO_MOVELIST_H

#include <vector>

#include "move.h"

namespace testudo
{

// The alternative (the commented out `movelist class`) doesn't seem to give a
// performance boost. So KISS.
using movelist = std::vector<move>;

/*
class movelist
{
public:
  using value_type = move;

  movelist() : seq_(), size_() {}

  void push_back(const move &m) { seq_[size_++] = m; }

  const move &operator[](std::size_t i) const noexcept
  { return seq_[i]; }

  const move &front() const noexcept { return seq_.front(); }
  move &front() noexcept { return seq_.front(); }

  bool empty() const noexcept { return !size(); }
  std::size_t size() const noexcept { return size_; }

  auto begin() noexcept { return &seq_[0]; }
  auto begin() const noexcept { return &seq_[0]; }
  auto end() noexcept { return &seq_[size_]; }
  auto end() const noexcept { return &seq_[size_]; }

  move &operator[](std::size_t i) noexcept { return seq_[i]; }

private:
  std::array<move, 256> seq_;
  std::size_t size_ = 0;
};
*/

}  // namespace state

#endif  // include guard
