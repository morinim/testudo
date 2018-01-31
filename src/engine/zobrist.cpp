/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "zobrist.h"
#include "state.h"

namespace testudo
{

namespace zobrist
{

hash_t hash(const state &s) noexcept
{
  hash_t ret(0);

  for (square i(0); i < 64; ++i)
  {
    const piece p(s[i]);
    if (p != EMPTY)
      ret ^= p.hash(i);
  }
  if (!s.side())
    ret ^= side();
  if (s.en_passant() != -1)
    ret ^= ep(file(s.en_passant()));
  if (s.castle())
    ret ^= castle(s.castle());

  return ret;
}

}  // namespace zobrist

}  // namespace testudo
