#ifndef CCCR_TYPE_H_
#define CCCR_TYPE_H_

#include <types.h>
#include <am/am.h>
#include <am/concept/DataType.h>

#include <am/util/DbObj.h>
//#include <am/util/Wrapper.h>
#include <am/util/SdbUtil.h>
#include <am/util/Serialization.h>

#include <util/RefCount.h>
#include <util/hashFunction.h>
#include <util/izene_serialization.h>

//#include <boost/memory.hpp>
#include <boost/static_assert.hpp>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>


using namespace std;

using namespace izenelib::am::util;

using namespace izenelib::util;

//MAKE_FEBIRD_SERIALIZATION(int);

//MAKE_FEBIRD_SERIALIZATION(string);

NS_IZENELIB_AM_BEGIN

#define PAGE_EXPANDING 1
#define EXACT_EXPANDING 0
#define INIT_BUCKET_SIZE 64

NS_IZENELIB_AM_END


#endif /*CCCR_TYPE_H_*/
