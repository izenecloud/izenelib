/**
 * @file DHashTableConstants.h
 * @brief The header file of Linear Hash Table constants.
 *
 * Used by LinearHashTable, EfficientLHT, StringELHT classes.
 */

#ifndef LHT_TRAIT_H
#define LHT_TRAIT_H

#include "ylib_basics.h"

NS_IZENELIB_AM_BEGIN

struct lht_trait{
	
      static const int SEGMENT_SIZE = 2048;
      static const int DIRECTORY_SIZE = 1024;

      static const double MIN_LOAD_FACTOR = 0.5;
      static const double MAX_LOAD_FACTOR = 1; // the paper says up to 5 is fine.
   
}
          
NS_IZENELIB_AM_END

#endif
