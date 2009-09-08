#ifndef AM_TC_HASH_H
#define AM_TC_HASH_H
/**
 * @file am/tc/Hash.h
 * @author Ian Yang
 * @date Created <2009-09-06 23:37:24>
 * @date Updated <2009-09-08 11:06:20>
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

    typedef IterNextRange<self_type> internal_range_type;
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

    bool iterInit()
    {
        return hash_.iterInit();
    }
    bool iterInit(const KeyType& key)
    {
        izenelib::am::raw::Buffer keyBuffer;
        izenelib::util::izene_serialization<key_type> izsKey(key);
        write_image(izsKey, keyBuffer);

        return hash_.iterInit(keyBuffer);
    }
    bool iterNext(KeyType& key)
    {
        izenelib::am::raw::Buffer keyBuffer;
        izenelib::util::izene_serialization<KeyType> izsKey(key);
        write_image(izsKey, keyBuffer);

        if (hash_.iterNext(keyBuffer))
        {
            izenelib::util::izene_deserialization<KeyType> izdKey(
                keyBuffer.data(), keyBuffer.size()
            );
            izdKey.read_image(key);

            return true;
        }

        return false;
    }
    bool iterNext(KeyType& key, ValueType& value)
    {
        izenelib::am::raw::Buffer keyBuffer;
        izenelib::util::izene_serialization<KeyType> izsKey(key);
        write_image(izsKey, keyBuffer);
        izenelib::am::raw::Buffer valueBuffer;

        if (hash_.iterNext(keyBuffer, valueBuffer))
        {
            izenelib::util::izene_deserialization<KeyType> izdKey(
                keyBuffer.data(), keyBuffer.size()
            );
            izdKey.read_image(key);

            izenelib::util::izene_deserialization<ValueType> izdValue(
                valueBuffer.data(), valueBuffer.size()
            );
            izdValue.read_image(value);

            return true;
        }

        return false;
    }
    bool iterNext(data_type& data)
    {
        return iterNext(data.get_key(), data.get_value());
    }

    bool getFirst(KeyType& key) const
    {
        izenelib::am::raw::Buffer keyBuffer;

        if (hash_.getFirst(keyBuffer))
        {
            izenelib::util::izene_deserialization<KeyType> izdKey(
                keyBuffer.data(), keyBuffer.size()
            );
            izdKey.read_image(key);

            return true;
        }

        return false;
    }
    bool getFirst(KeyType& key, ValueType& value) const
    {
        izenelib::am::raw::Buffer keyBuffer;
        izenelib::am::raw::Buffer valueBuffer;

        if (hash_.getFirst(keyBuffer, valueBuffer))
        {
            izenelib::util::izene_deserialization<KeyType> izdKey(
                keyBuffer.data(), keyBuffer.size()
            );
            izdKey.read_image(key);

            izenelib::util::izene_deserialization<ValueType> izdValue(
                valueBuffer.data(), valueBuffer.size()
            );
            izdValue.read_image(value);

            return true;
        }

        return false;
    }
    bool getFirst(data_type& data) const
    {
        return getFirst(data.get_key(), data.get_value());
    }

    bool getNext(KeyType& key) const
    {
        izenelib::am::raw::Buffer keyBuffer;
        izenelib::util::izene_serialization<KeyType> izsKey(key);
        write_image(izsKey, keyBuffer);

        if (hash_.getNext(keyBuffer))
        {
            izenelib::util::izene_deserialization<KeyType> izdKey(
                keyBuffer.data(), keyBuffer.size()
            );
            izdKey.read_image(key);

            return true;
        }

        return false;
    }
    bool getNext(KeyType& key, ValueType& value) const
    {
        izenelib::am::raw::Buffer keyBuffer;
        izenelib::util::izene_serialization<KeyType> izsKey(key);
        write_image(izsKey, keyBuffer);

        izenelib::am::raw::Buffer valueBuffer;

        if (hash_.getNext(keyBuffer, valueBuffer))
        {
            izenelib::util::izene_deserialization<KeyType> izdKey(
                keyBuffer.data(), keyBuffer.size()
            );
            izdKey.read_image(key);

            izenelib::util::izene_deserialization<ValueType> izdValue(
                valueBuffer.data(), valueBuffer.size()
            );
            izdValue.read_image(value);

            return true;
        }

        return false;
    }
    bool getNext(data_type& data) const
    {
        return getNext(data.get_key(), data.get_value());
    }

    //@}

    void all(range_type& range)
    {
        range.attach(*this);
    }
    void internalAll(internal_range_type& range)
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
