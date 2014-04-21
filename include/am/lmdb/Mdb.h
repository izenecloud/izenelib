#ifndef AM_LMDB_MDB_H
#define AM_LMDB_MDB_H

#include "raw/Mdb.h"

#include <am/raw/AmWrapper.h>

#include <functional>

#include <boost/assert.hpp>

namespace izenelib {
namespace am {
namespace lmdb {

template<typename KeyType,
         typename ValueType >
class Mdb
    : public izenelib::am::raw::AmWrapper<Mdb<KeyType, ValueType>,
                                          raw::Mdb,
                                          KeyType,
                                          ValueType>
{
    typedef Mdb<KeyType, ValueType> self_type;

public:
    typedef KeyType key_type;
    typedef ValueType value_type;
    typedef DataType<KeyType, ValueType> data_type;
    typedef typename raw::Mdb::size_type size_type;

    typedef raw::Mdb raw_am_type;

    explicit Mdb(const std::string& file = "", unsigned int maxSizeInMb = 1024)
    : table_(file, maxSizeInMb)
    {
    }

private:
    raw::Mdb table_;

    raw::Mdb& rawAm()
    {
        return table_;
    }
    const raw::Mdb& rawAm() const
    {
        return table_;
    }

    friend struct izenelib::am::raw::detail::AmWrapperAccess;
};

}}} // namespace izenelib::am::lmdb

#endif // AM_LMDB_MDB_H