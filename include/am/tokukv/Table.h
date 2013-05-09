#ifndef AM_TOKUKV_TABLE_H
#define AM_TOKUKV_TABLE_H

#include "raw/Table.h"

#include <am/raw/AmWrapper.h>

#include <functional>

#include <boost/assert.hpp>

namespace izenelib
{
namespace am
{
namespace tokukv
{
//using namespace izenelib::am::tokukv::raw;

template<typename KeyType>
class TwoPartComparator
{
public:
    int Compare (DB * db, const DBT * a, const DBT * b)
    {
        //return a.compare(b);
        izenelib::am::CompareFunctor<KeyType> _comp;
        KeyType keyA, keyB;
        izene_deserialization<KeyType> izdA(a->data, a->size);
        izene_deserialization<KeyType> izdB(b->data, b->size);
        izdA.read_image(keyA);
        izdB.read_image(keyB);
        return _comp(keyA, keyB);
    }

    // Ignore the following methods for now:
    const char* Name() const
    {
        return "TwoPartComparator";
    }
};


template<typename KeyType,
         typename ValueType,
         typename Comp=TwoPartComparator<KeyType> >
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

    explicit Table(const std::string& file = "")
        : table_(file), comp_()
    {
    }

private:
    raw::Table<Comp> table_;
    Comp comp_;

    raw::Table<Comp>& rawAm()
    {
        return table_;
    }
    const raw::Table<Comp>& rawAm() const
    {
        return table_;
    }

    friend struct izenelib::am::raw::detail::AmWrapperAccess;
};
/**/
}
}
} // namespace izenelib::am::tokukv

#endif // AM_tokukv_TABLE_H
