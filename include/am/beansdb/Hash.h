#ifndef AM_BEANSDB_H
#define AM_BEANSDB_H

#include "raw/Hash.h"

#include <am/raw/AmWrapper.h>

namespace izenelib {
namespace am {
namespace beansdb {

template<typename KeyType, typename ValueType>
class Hash
    : public izenelib::am::raw::AmWrapper<Hash<KeyType, ValueType>,
                                          raw::Hash,
                                          KeyType,
                                          ValueType>
{
    typedef Hash<KeyType, ValueType> self_type;

public:
    typedef KeyType key_type;
    typedef ValueType value_type;
    typedef DataType<KeyType, ValueType> data_type;
    typedef typename raw::Hash::size_type size_type;

    typedef raw::Hash raw_am_type;

    explicit Hash(const std::string& file )
    : hash_(file)
    {
    }

    bool optimize()
    {
	return hash_.optimize();
    }

private:
    raw::Hash hash_;

    raw::Hash& rawAm()
    {
        return hash_;
    }
    const raw::Hash& rawAm() const
    {
        return hash_;
    }

    friend struct izenelib::am::raw::detail::AmWrapperAccess;
};

}}} // namespace izenelib::am::beansdb

#endif

