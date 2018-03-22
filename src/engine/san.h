/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_SAN_H)
#define      TESTUDO_SAN_H

#include <string>

#include "move.h"

namespace testudo
{

class state;

namespace SAN
{

move from(std::string, const state &);

}  // namespace SAN

} //namespace testudo

#endif  // include guard
