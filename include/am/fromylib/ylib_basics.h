#ifndef YLIB_BASIC_H_
#define YLIB_BASIC_H_

#include <types.h>
#include <am/am.h>
#include <am/concept/DataType.h>

#include <am/util/DbObj.h>
#include <am/util/Wrapper.h>
#include <util/RefCount.h>
#include <util/hashFunction.h>

#include <3rdparty/boost/memory.hpp>
#include <boost/static_assert.hpp>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>

using namespace std;
using namespace izenelib::am::util;
using namespace izenelib::util;

NS_IZENELIB_AM_BEGIN

const int IZENE_SEGMENT_SIZE = 2048;
const int IZENE_DIRECTORY_SIZE = 1024;

const double IZENE_MIN_LOAD_FACTOR = 0.5;
const double IZENE_MAX_LOAD_FACTOR = 1; // the paper says up to 5 is fine.

NS_IZENELIB_AM_END

#endif

