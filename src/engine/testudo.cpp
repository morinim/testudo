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
#include "test.h"

#include "thirdparty/docopt/docopt.h"

const char USAGE[] =
 R"(Testudo Chess Engine
Copyright 2018 Manlio Morini
         __    __    __    __
       /__////__////__////__////
      ////__////__////__////__/
     /__////__////__////__////
    ////__////__////__////__/
   /__////__////__////__////
  ////__////__////__////__/
 /__////__////__////__////
////__////__////__////__/

Usage:
  testudo
  testudo [--depth=<d>] [--nodes=<n>] [--time=<sec>] --test TESTSET
  testudo -h | --help
  testudo -v | --version

Options:
  -h --help              shows this screen and exit
  -v --version           shows version and exit
  --test=TESTSET         an EPD test set
  --depth=<d>            maximum allowed search depth
  --nodes=<n>            available number of search nodes
  --time=<sec>           available search time (seconds)
)";

int main(int argc, char *const argv[])
{
  using namespace testudo;

  log::setup_stream("testudo");

  const auto args(docopt::docopt(USAGE,
                                 {argv + 1, argv + argc},
                                 true,      // shows help if requested
                                 VERSION,   // version string
                                 false));   // options first (POSIX compliant)

  const auto testfile(args.at("--test"));
  if (!testfile)
    CECP::loop();
  else
  {
    search::constraints constraints;

    const auto time(args.at("--time"));
    if (time)
      constraints.max_time = std::chrono::seconds(time.asLong());

    const auto depth(args.at("--depth"));
    if (depth)
      constraints.max_depth = depth.asLong();

    const auto nodes(args.at("--nodes"));
    if (nodes)
      constraints.max_nodes = nodes.asLong();

    test(testfile.asString(), constraints);
  }
}
