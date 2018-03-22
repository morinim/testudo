/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TESTUDO_UTIL_H)
#define      TESTUDO_UTIL_H

#include <string>

namespace testudo
{

// Ensures something gets run at the end of a scope.
//
// An alternative, hacky, implementation is:
//
//     template<class F>
//     auto finally(F f) noexcept(noexcept(F(std::move(f))))
//     {
//       auto x([f = std::move(f)](void *){ f(); });
//       return std::unique_ptr<void, decltype(x)>((void*)1, std::move(x));
//     }
//
template <class F>
class final_action
{
public:
  explicit final_action(F f) noexcept : f_(std::move(f)) {}

  final_action &operator=(const final_action &) = delete;

  ~final_action() noexcept { f_(); }

private:
  F f_;
};

// Convenience functions to generate a `final_action`.
template <class F>
inline final_action<F> finally(F f) noexcept
{
  return final_action<F>(f);
}

std::string trim(const std::string &);

}  // namespace testudo

#endif  // include guard
