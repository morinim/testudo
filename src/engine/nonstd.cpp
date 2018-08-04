/*
 *  This file is part of TESTUDO.
 *
 *  Copyright (C) 2018 Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if defined(UNIX)
#  include <unistd.h>
#  include <sys/time.h>
#  include <sys/types.h>
#elif defined(WIN32)
#  include <sys/timeb.h>
#  include <windows.h>
#endif

#include "nonstd.h"

namespace testudo
{

bool input_available()
{
#if defined(UNIX)
  fd_set readfds;
  timeval tv;

  FD_ZERO(&readfds);
  FD_SET(0, &readfds);
  tv.tv_sec  = 0;
  tv.tv_usec = 0;
  select(16, &readfds, nullptr, nullptr, &tv);

  return FD_ISSET(0, &readfds);
#elif defined(WIN32)
  DWORD dw;
  static bool init(false);
  static HANDLE inh(GetStdHandle(STD_INPUT_HANDLE));
  static int pipe(!GetConsoleMode(inh, &dw));

  if (!init)
  {
    init = true;

    if (!pipe)
    {
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(inh);
      FlushConsoleInputBuffer(inh);
    }
  }

  if (pipe)
    return !PeekNamedPipe(inh, nullptr, 0, nullptr, &dw, nullptr) ? 1 : dw;

  GetNumberOfConsoleInputEvents(inh, &dw);
  return dw > 1;
#else
#  error Missing implementation of the input_available() function
#endif
}

}  // namespace testudo
