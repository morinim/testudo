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
  score material[2] = {0, 0};
  score pcsq_score[2] = {0, 0};

  for (square i(0); i < 64; ++i)
    if (s[i] != EMPTY)
    {
      material[s[i].color()] += s[i].value();

      if (material[!s[i].color()] > 1200)
        pcsq_score[s[i].color()] += db.pcsq_m(s[i], i);
      else
        pcsq_score[s[i].color()] += db.pcsq_e(s[i], i);
    }

  return material[s.side()] - material[!s.side()]
         + pcsq_score[s.side()] - pcsq_score[!s.side()];
}

}  // namespace testudo
