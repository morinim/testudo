/**
 *  \file
 *  \remark This file is part of MMMCTS.
 *
 *  \copyright Copyright (C) 2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(LIGHT_MCTS_TIMER_H)
#define      LIGHT_MCTS_TIMER_H

#include <chrono>

namespace light_mcts
{
///
/// We always run into the task of measuring the time between two points.
///
/// The timer class cuts down the verbose syntax needed to measure the elapsed
/// time. It's similar to `boost::cpu_timer` but tailored to our needs (so less
/// general).
///
/// The simplest and most common use is:
///
///     int main()
///     {
///       timer t;
///
///       do_stuff_and_burn_some_time();
///
///       std::cout << "Elapsed (ms): " << t.elapsed().count() << '\n'
///
///     }
///
/// \warning
/// A useful recommendation is to never trust timings unless they are:
/// * at least 100 times longer than the CPU time resolution
/// * run multiple times
/// * run on release builds.
/// .. and results that are too good to be true need to be investigated
/// skeptically.
///
/// \remark
/// The original idea is of Kjellkod (http://kjellkod.wordpress.com).
///

class timer
{
public:
  timer() : start_(std::chrono::steady_clock::now()) {}

  void restart() { start_ = std::chrono::steady_clock::now(); }

  ///
  /// \return time elapsed in milliseconds (as would be measured by a clock
  ///         on the wall. It's NOT the processor time)
  ///
  std::chrono::milliseconds elapsed() const
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_);
  }

  bool elapsed(std::chrono::milliseconds m) const
  {
    return m > std::chrono::milliseconds(0) && elapsed() > m;
  }

private:
  std::chrono::steady_clock::time_point start_;
};

}  // namespace light_mcts

#endif  // include guard
