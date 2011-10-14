#ifndef SDB_BTREE_TYPES_H_
#define SDB_BTREE_TYPES_H_

#include <types.h>
#include <am/am.h>
#include <am/concept/DataType.h>

#include <am/util/DbObj.h>
//#include <am/util/Wrapper.h>
#include <am/util/SdbUtil.h>
#include <util/izene_serialization.h>
#include <util/ProcMemInfo.h>

//#include <boost/memory.hpp>
//#include <boost/static_assert.hpp>

using namespace izenelib::am::util;

NS_IZENELIB_AM_BEGIN

// the key position in the sdb_node
enum CChildPos
{
    CCP_INTHIS,
    CCP_INLEFT,
    CCP_INRIGHT,
    CCP_NONE,
};

typedef std::pair<size_t, CChildPos> KEYPOS;

/*
const bool unloadbyRss = false;
const bool unloadAll = true;
const bool orderedCommit = true;
const bool quickFlush = false;
*/

NS_IZENELIB_AM_END

#endif /*SDB_TYPES_H_*/
