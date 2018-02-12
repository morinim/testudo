/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "move.h"
#include <sstream>

namespace testudo
{

// Move in coordinate notation (g1f3, a7a8q).
std::ostream &operator<<(std::ostream &o, const move &m)
{
  if (m.is_sentry())
    return o << '-';

  std::ostringstream ss;

  ss << char('a' + file(m.from)) << 1 + rank(m.from)
     << char('a' + file(m.to)) << 1 + rank(m.to);

  if (m.flags & move::promotion)
    ss << piece(BLACK, m.promote());

  return o << ss.str();
}

}  // namespace testudo
