/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "testudo.h"
#include "cecp.h"

#include "thirdparty/cxxopts.hpp"


#if !defined(TESTUDO_CONFIG_TEST)
int main()
{
  testudo::log::setup_stream("testudo");
  testudo::CECP::loop();
}
#endif
