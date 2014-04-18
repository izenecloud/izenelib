#ifndef SYSTEM_H
#define SYSTEM_H

#ifndef WIN32

#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <time.h>

#else//for Win32

#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include <fcntl.h>
#include <string>
#include <io.h>
#include <time.h>
#include <errno.h>

#endif

#include <types.h>

#include <ir/index_manager/utility/Exception.h>

#include <util/ustring/UString.h>
//typedef izenelib::util::UString IndexPropString;
typedef std::string IndexPropString;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

#define SF1_VERSION "5.0"

//collection id
typedef uint32_t collectionid_t;
// term id
typedef uint32_t termid_t;
// term position
typedef uint32_t loc_t;
// internal document id encoding.
typedef uint32_t docid_t;


//file offset
typedef int64_t fileoffset_t;
//field id
typedef int32_t fieldid_t;
//document's schema id
typedef int32_t schemaid_t;


// internal term id encoding.
typedef uint32_t count_t;
// integral frequencies
typedef count_t freq_t;

// Added by dohyun 2009.04.22
typedef uint8_t byte;

// document length
typedef uint16_t doclen_t;

#define BAD_POSITION 0xFFFFFFFF
#define BAD_DOCID 0xFFFFFFFF
#define MAX_TERMID 0xFFFFFFFF
#define BAD_PROPERTY_ID 0xFFFFFFFF


#define SKIP_THRESHOLD 1024//40960

#ifdef __APPLE__
#define lseek64 lseek
#define open64 open
#endif

#define DELETED_DOCS "docs.del"
#define BTREE_DELETED_DOCS "btree.del"

} // namespace indexmanager

NS_IZENELIB_IR_END

#endif
