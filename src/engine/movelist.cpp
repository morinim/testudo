/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <sstream>

#include "movelist.h"

namespace testudo
{

std::ostream &operator<<(std::ostream &o, const movelist &ml)
{
  std::ostringstream s;

  for (std::size_t i(0); i < ml.size(); ++i)
  {
    if (i)
      s << ' ';

    s << ml[i];
  }

  return o << s.str();
}

}  // namespace testudo
