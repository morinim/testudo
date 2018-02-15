/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "eval.h"
#include "parameters.h"

namespace testudo
{

score eval(const state &s)
{
  score_vector e;

  for (square i(0); i < 64; ++i)
    if (s[i] != EMPTY)
    {
      e.material[s[i].color()] += s[i].value();

      if (e.material[!s[i].color()] > 1200)
        e.pcsq[s[i].color()] += db.pcsq_m(s[i], i);
      else
        e.pcsq[s[i].color()] += db.pcsq_e(s[i], i);
    }

  return e.material[s.side()] - e.material[!s.side()]
         + e.pcsq[s.side()] - e.pcsq[!s.side()];
}

}  // namespace testudo
