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

struct BTreeLexicalCmp
{
    int compare(const char* dataA, int sizeA,
                const char* dataB, int sizeB) const
    {
        // never go here
        BOOST_ASSERT(false);
        return 0;
    }
};

template<typename KeyType>
struct BTreeLessCmp
{
    int compare(const char* dataA, int sizeA,
                const char* dataB, int sizeB) const
    {
        KeyType a;
        izenelib::util::izene_deserialization<KeyType> izdA(
            dataA, sizeA
        );
        izdA.read_image(a);

        KeyType b;
        izenelib::util::izene_deserialization<KeyType> izdB(
            dataB, sizeB
        );
        izdB.read_image(b);

        if (a < b)
        {
            return -1;
        }
        else if (b < a)
        {
            return 1;
        }

        return 0;
    }
};

template<typename KeyType,
         typename ValueType,
         typename Comp=BTreeLexicalCmp >
class Table
    : public izenelib::am::raw::AmWrapper<Table<KeyType, ValueType, Comp>,
                                          raw::Table,
                                          KeyType,
                                          ValueType>
{
    typedef Table<KeyType, ValueType, Comp> self_type;

public:
    typedef KeyType key_type;
    typedef ValueType value_type;
    typedef DataType<KeyType, ValueType> data_type;
    typedef typename raw::Table::size_type size_type;

    typedef raw::Table raw_am_type;
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
    raw::Table hash_;
    Comp comp_;

    raw::Table& rawAm()
    {
        return hash_;
    }
    const raw::Table& rawAm() const
    {
        return hash_;
    }

    friend struct izenelib::am::raw::detail::AmWrapperAccess;
};

}}} // namespace izenelib::am::leveldb

#endif // AM_LEVELDB_TABLE_H
