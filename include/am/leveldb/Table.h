#ifndef AM_LEVELDB_TABLE_H
#define AM_LEVELDB_TABLE_H
/**
 * @file am/leveldb/Table.h
 * @author Ian Yang
 * @date Created <2009-09-06 23:37:24>
 * @date Updated <2009-09-10 20:50:44>
 * @brief TokyoCabinet Table DB wrapper
 */

#include "raw/Table.h"

#include <am/raw/AmWrapper.h>
#include <am/range/IterNextRange.h>
#include <am/range/GetNextRange.h>

#include <functional>

#include <boost/assert.hpp>

namespace izenelib {
namespace am {
namespace leveldb {

class TwoPartComparator : public ::leveldb::Comparator {
public:
    // Three-way comparison function:
    //   if a < b: negative result
    //   if a > b: positive result
    //   else: zero result
    int Compare(const ::leveldb::Slice& a, const ::leveldb::Slice& b) const {
        return a.compare(b);
    }

    int compare(const char* dataA, int sizeA,
                const char* dataB, int sizeB) const
    {
        // never go here
        BOOST_ASSERT(false);
        return 0;
    }
	
    // Ignore the following methods for now:
    const char* Name() const { return "TwoPartComparator"; }
    void FindShortestSeparator(std::string*, const ::leveldb::Slice&) const { }
    void FindShortSuccessor(std::string*) const { }
};

template<typename KeyType,
         typename ValueType,
         typename Comp=TwoPartComparator >
class Table
    : public izenelib::am::raw::AmWrapper<Table<KeyType, ValueType, Comp>,
                                          raw::Table<Comp>,
                                          KeyType,
                                          ValueType>
{
    typedef Table<KeyType, ValueType, Comp> self_type;

public:
    typedef KeyType key_type;
    typedef ValueType value_type;
    typedef DataType<KeyType, ValueType> data_type;
    typedef typename raw::Table<Comp>::size_type size_type;

    typedef raw::Table<Comp> raw_am_type;
    typedef IterNextRange<self_type> exclusive_range_type;
    typedef IterNextRange<self_type> range_type;

    explicit Table(const std::string& file = "")
    : hash_(file), comp_()
    {
    }

    static int compare(const char* dataA, int sizeA,
                       const char* dataB, int sizeB,
                       void* p)
    {
        Comp* pHash = static_cast<Comp*>(p);
        return pHash->compare(dataA, sizeA, dataB, sizeB);
    }

    void all(range_type& range)
    {
        range.attach(*this);
    }

    void exclusiveAll(exclusive_range_type& range)
    {
        range.attach(*this);
    }

private:
    raw::Table<Comp> hash_;
    Comp comp_;

    raw::Table<Comp>& rawAm()
    {
        return hash_;
    }
    const raw::Table<Comp>& rawAm() const
    {
        return hash_;
    }

    friend struct izenelib::am::raw::detail::AmWrapperAccess;
};

}}} // namespace izenelib::am::leveldb

#endif // AM_LEVELDB_TABLE_H
