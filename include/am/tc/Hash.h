#ifndef AM_TC_HASH_H
#define AM_TC_HASH_H
/**
 * @file am/tc/Hash.h
 * @author Ian Yang
 * @date Created <2009-09-06 23:37:24>
 * @date Updated <2009-09-10 10:37:58>
 * @brief TokyoCabinet Hash DB wrapper
 */

#include "raw/Hash.h"

#include <am/raw/AmWrapper.h>
#include <am/range/IterNextRange.h>
#include <am/range/GetNextRange.h>

namespace izenelib {
namespace am {
namespace tc {

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

    typedef IterNextRange<self_type> exclusive_range_type;
    typedef GetNextRange<self_type> range_type;

    enum {
        READER = raw::Hash::READER,
        WRITER = raw::Hash::WRITER,
        CREAT  = raw::Hash::CREAT,
        TRUNC  = raw::Hash::TRUNC,
        NOLCK  = raw::Hash::NOLCK,
        LCKNB  = raw::Hash::LCKNB,
        TSYNC  = raw::Hash::TSYNC,
    };

    enum { DEFAULT_OPEN_MODE = raw::Hash::DEFAULT_OPEN_MODE };

    explicit Hash(const std::string& file = "")
    : hash_(file)
    {
    }

    //@{
    //@brief tuning functions used before opening, refer to the manual of
    //tokyo cabinet

    bool setmutex()
    {
        return hash_.setmutex();
    }
    bool setcache(int32_t rcnum)
    {
        return hash_.setcache(rcnum);
    }
    bool tune(int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts)
    {
        return hash_.tune(bnum, apow, fpow, opts);
    }
    bool setxmsiz(int64_t xmsiz)
    {
        return hash_.setxmsiz(xmsiz);
    }
    bool setdfunit(int32_t dfunit)
    {
        return hash_.setdfunit(dfunit);
    }
    //@}

    //@{
    //@brief tc special functions

    //@}

    void all(range_type& range)
    {
        range.attach(*this);
    }
    void exclusiveAll(exclusive_range_type& range)
    {
        range.attach(*this);
    }

    const char* errorMessage() const
    {
        return hash_.errorMessage();
    }
    static inline const char* errorMessage(int code)
    {
        return raw::Hash::errorMessage(code);
    }
    int errorCode() const
    {
        return hash_.errorCode();
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

}}} // namespace izenelib::am::tc

#endif // AM_TC_HASH_H
