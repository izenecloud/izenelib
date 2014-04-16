#ifndef SF1COMMON_INTTYPES_H
#define SF1COMMON_INTTYPES_H

#ifndef SF1R_NO_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#else
# ifdef HAVE_STDINT_H
#  include <stdint.h>
# else
#  ifdef HAVE_SYS_TYPES_H
#   include <sys/types.h>
#  endif
#  ifdef HAVE_STDDEF_H
#   include <stddef.h>
#  endif
# endif
#endif

namespace izenelib{

typedef uint32_t termid_t;
typedef uint32_t docid_t;
typedef uint32_t propertyid_t;
typedef uint32_t labelid_t;
typedef uint32_t loc_t;
typedef uint32_t count_t;
typedef uint32_t collectionid_t;
typedef uint32_t workerid_t;
typedef uint64_t wdocid_t;

typedef float score_t;

} // namespace izenelib

#endif

