/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <algorithm>

#include "util.h"

namespace testudo
{

std::string trim(const std::string &s)
{
  const auto front(
    std::find_if_not(s.begin(), s.end(),
                     [](auto c) { return std::isspace(c); }));

  // The search is limited in the reverse direction to the last non-space value
  // found in the search in the forward direction.
  const auto back(
    std::find_if_not(s.rbegin(), std::make_reverse_iterator(front),
                     [](auto c) { return std::isspace(c); }).base());

  return {front, back};
}

}  // namespace testudo
