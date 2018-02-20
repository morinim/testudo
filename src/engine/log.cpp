/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <chrono>
#include <fstream>
#include <iomanip>

#include "log.h"

namespace testudo
{

namespace
{

std::tm tm_now()
{
  const auto now(std::chrono::system_clock::now());
  const auto tt(std::chrono::system_clock::to_time_t(now));

  return *std::localtime(&tt);
}

}  // unnamed namespace

log::level log::reporting_level = log::ALL;
std::unique_ptr<std::ostream> log::stream = nullptr;

// Creates a `log` object.
log::log() : os(), level_(INFO) {}

// Sets the logging level of a message.
//
// The following code:
//
//     log().get(level) << "Hello " << username;
//
// creates a `log` object with the `level` logging level, fetches its
// `std::stringstream` object, formats and accumulates the user-supplied data
// and, finally:
// - prints the resulting string on `std::cout`;
// - persists the resulting string into the log file (if specified).
std::ostringstream &log::get(level l)
{
  level_ = std::min(FATAL, l);
  return os;
}

log::~log()
{
  static const std::string tags[] =
  {
    "ALL", "DEBUG", "INFO", "OUTPUT", "WARNING", "ERROR", "FATAL", ""
  };

  if (stream && level_ >= reporting_level)
  {
    const auto lt(tm_now());

    *stream << std::put_time(&lt, "%T") << '\t' << tags[level_] << '\t'
            << os.str() << std::endl;
  }

  if (level_ == OUTPUT)
    std::cout << os.str() << std::endl;
}

// Sets the `log::stream` variable with a convenient value.
//
// Given the `/home/doe/app` arguments associates the `log::stream` variable
// with the `app_123_18_30_00.log` file (the numbers represents the current:
// day of the year, hours, minutes, seconds) in the `/home/doe/` directory.
void log::setup_stream(const std::string &base)
{
  const auto lt(tm_now());

  std::ostringstream fn;
  fn << base << std::put_time(&lt, "_%j_%H_%M_%S") << ".log";

  stream = std::make_unique<std::ofstream>(fn.str());
}

}  // namespace testudo
