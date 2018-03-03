/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_EVAL_H)
#define      TESTUDO_EVAL_H

#include "state.h"

namespace testudo
{

struct score_vector
{
  explicit score_vector(const state &);

  int phase;

  score material[2];

  score pcsq_e[2];
  score pcsq_m[2];
};

extern score eval(const state &);

}  // namespace testudo

#endif  // include guard
