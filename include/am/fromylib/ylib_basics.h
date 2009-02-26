#ifndef YLIB_BASIC_H_
#define YLIB_BASIC_H_

#include <types.h>
#include <am/am.h>
#include <am/concept/DataType.h>

#include <am/util/DbObj.h>
#include <am/util/Wrapper.h>
#include <util/RefCount.h>
#include <util/hashFunction.h>

#include <boost/memory.hpp>
#include <boost/static_assert.hpp>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>

using namespace std;
using namespace izenelib::am::util;
using namespace izenelib::util;

NS_IZENELIB_AM_BEGIN

const int SEGMENT_SIZE = 2048;
const int DIRECTORY_SIZE = 1024;

const double MIN_LOAD_FACTOR = 0.5;
const double MAX_LOAD_FACTOR = 1; // the paper says up to 5 is fine.

NS_IZENELIB_AM_END

#endif

