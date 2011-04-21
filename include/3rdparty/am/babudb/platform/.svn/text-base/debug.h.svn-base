// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_DEBUG_H
#define YIELD_PLATFORM_DEBUG_H

namespace YIELD
{
#ifdef _WIN32
  extern "C"
  {
    __declspec( dllimport ) void __stdcall DebugBreak();
  }
#else
  inline void DebugBreak()
  {
    *((int*)0) = 0xabadcafe;
  }
#endif
}

#endif

