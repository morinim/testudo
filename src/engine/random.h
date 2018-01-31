/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <random>

#if !defined(TESTUDO_RANDOM_H)
#define      TESTUDO_RANDOM_H

namespace testudo
{

namespace random
{
inline std::mt19937 &engine()
{
  static thread_local std::mt19937 random_engine;

  return random_engine;
}

template<class T>
T between(T min, T max)
{
  std::uniform_int_distribution<T> d(min, max);
  return d(engine());
}

template<class T>
T number()
{
  return between<T>(0, std::numeric_limits<T>::max());
}

template<class C>
C fill()
{
  C ret;

  for (auto &v : ret)
    v = number<typename C::value_type>();

  return ret;
}

template<class C>
C fill2d()
{
  C ret;

  for (auto &v : ret)
    v = fill<typename C::value_type>();

  return ret;
}

}  // namespace random

}  // namespace testudo

#endif // include guard
