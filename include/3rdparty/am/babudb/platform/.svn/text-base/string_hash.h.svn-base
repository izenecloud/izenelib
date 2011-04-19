// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_STRING_HASH_H
#define YIELD_PLATFORM_STRING_HASH_H

#include "yield/platform/platform_types.h"


#define YIELD_STRING_HASH_NEXT( c, hash ) hash = hash ^ ( ( hash << 5 ) + ( hash >> 2 ) + c )


namespace YIELD
{
  inline uint32_t string_hash( const char* str, uint32_t hash = 0 )
  {
    while ( *str != 0 )
    {
      YIELD_STRING_HASH_NEXT( *str, hash );
      str++;
    }

    return hash;
  }

  inline uint32_t string_hash( const char* str, size_t str_len, uint32_t hash )
  {
    size_t str_i = 0;

    while ( str_i < str_len )
    {
      YIELD_STRING_HASH_NEXT( str[str_i], hash );
      str_i++;
    }

    return hash;
  }
}

#endif
