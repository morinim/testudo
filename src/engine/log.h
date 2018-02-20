/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_LOG_H)
#define      TESTUDO_LOG_H

#include <memory>
#include <sstream>

#include "move.h"

namespace testudo
{

// A basic logger.
//
// This is derived from the code presented in "Logging in C++" by Petru
// Marginean (DDJ Sep 2007)
class log
{
public:
  // The log level.
  //
  // * `DEBUG`   - Only interesting for developers
  // * `INFO`    - I say something but I don't expect you to listen
  // * `OUTPUT`  - Standard program's console output
  // * `WARNING` - I can continue but please have a look
  // * `ERROR`   - Something really wrong... but you could be lucky
  // * `FATAL`   - The program cannot continue
  // * `OFF`     - Disable output
  //
  // The `DEBUG` log level can be switched on only if the `NDEBUG` macro is
  // defined.
  enum level : unsigned {ALL, DEBUG, INFO, OUTPUT, WARNING, ERROR, FATAL, OFF};

  /// Messages with a lower level aren't logged / printed.
  static level reporting_level;

  /// Log stream.
  static std::unique_ptr<std::ostream> stream;
  static void setup_stream(const std::string &base);

  explicit log();
  log(const log &) = delete;
  log &operator=(const log &) = delete;

  virtual ~log();

  std::ostringstream &get(level = INFO);

protected:
  std::ostringstream os;

private:
  level level_;  // current log level
};

// A little trick that makes the code, when logging is not necessary, almost
// as fast as the code with no logging at all.
//
// Logging will have a cost only if it actually produces output; otherwise,
// the cost is low (and actually immeasurable in most cases). This lets you
// control the trade-off between fast execution and detailed logging.
//
// Macro-related dangers should be avoided> we shouldn't forget that the
// logging code might not be executed at all, subject to the logging level in
// effect. This is what we actually wanted and is actually what makes the code
// efficient. But as always, "macro-itis" can introduce subtle bugs. In this
// example:
//
//     testudoPRINT(log::INFO) << "A number of " << NotifyClients()
//                             << " were notified.";
//
// the clients will be notified only if the logging level detail will be
// `log::INFO` and greater. Probably not what was intended! The correct code
// should be:
//
//     const int notifiedClients = NotifyClients();
//     testudoPRINT(log::INFO) << "A number of " << notifiedClients
//                             << " were notified.";
//
//
// When the `NDEBUG` is defined all the debug-level logging is eliminated at
// compile time.
#if defined(NDEBUG)
#define testudoPRINT(level) if (level == log::DEBUG);\
                            else if (level < log::reporting_level);\
                            else log().get(level)
#else
#define testudoPRINT(level) if (level < log::reporting_level);\
                            else log().get(level)
#endif

#define testudoFATAL   testudoPRINT(log::FATAL)
#define testudoDEBUG   testudoPRINT(log::DEBUG)
#define testudoERROR   testudoPRINT(log::ERROR)
#define testudoINFO    testudoPRINT(log::INFO)
#define testudoOUTPUT  testudoPRINT(log::OUTPUT)
#define testudoWARNING testudoPRINT(log::WARNING)

}  // namespace state

#endif  // include guard
