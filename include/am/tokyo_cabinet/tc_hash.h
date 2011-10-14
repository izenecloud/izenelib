#ifndef TC_HASH_H
#define TC_HASH_H

#ifdef __x86_64
#include "tc_hash_64.h"
#else
#include "tc_hash_32.h"
#endif

#endif
