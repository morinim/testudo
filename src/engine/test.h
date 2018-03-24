/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_TEST_H)
#define      TESTUDO_TEST_H

#include "search.h"

namespace testudo
{

bool test(const std::string &, const search::constraints &);

}  // namespace testudo

#endif  // include guard
