#ifndef AM_RAW_AM_WRAPPER_H
#define AM_RAW_AM_WRAPPER_H
/**
 * @file am/raw/AmWrapper.h
 * @author Ian Yang
 * @date Created <2009-09-06 22:50:37>
 * @date Updated <2009-09-08 13:08:52>
 * @brief Integrate raw am and serialization framework
 */

#include "Buffer.h"
#include "WriteImage.h"

#include <am/concept/DataType.h>
#include <iostream>
namespace izenelib
{
namespace am
{
namespace raw
{

namespace detail
{
struct AmWrapperAccess
{
    template<typename RawAm, typename Derived>
    static inline RawAm& rawAm(Derived& d)
    {
        return d.rawAm();
    }

    template<typename RawAm, typename Derived>
    static inline const RawAm& rawAm(const Derived& d)
    {
        return d.rawAm();
    }
};
} // namespace detail


template<
typename Derived,
typename RawAm,
typename KeyType,
typename ValueType,
typename SizeType = typename RawAm::size_type
>
class AmWrapper
{
public:
    typedef RawAm raw_am_type;

    typedef KeyType key_type;
    typedef ValueType value_type;
    typedef DataType<KeyType, ValueType> data_type;
    typedef SizeType size_type;

    typedef typename raw_am_type::cursor_type cursor_type;

    enum{ AMWrapperType = true };

    bool open()
    {
        return rawAm().open();
    }
    bool open(int mode)
    {
        return rawAm().open(mode);
    }
    bool open(const std::string& file)
    {
        return rawAm().open(file);
    }
    bool open(const std::string& file, int mode)
    {
        return rawAm().open(file, mode);
    }
    std::string getFileName() const
    {
        return rawAm().getFileName();
    }
    void close()
    {
        rawAm().close();
    }
    bool isOpened() const
    {
        return rawAm().isOpened();
    }
    /**
     * @deprecated
     */
    bool is_open() const
    {
        return rawAm().is_open();
    }
    bool flush()
    {
        return rawAm().flush();
    }
    size_type size() const
    {
        return rawAm().size();
    }
    /**
     * @deprecated
     */
    size_type num_items() const
    {
        return rawAm().size();
    }
    bool empty() const
    {
        return rawAm().empty();
    }
    bool clear()
    {
        return rawAm().clear();
    }
    bool insert(const key_type& key, const value_type& value)
    {
        Buffer keyBuffer;
        izenelib::util::izene_serialization<key_type> izsKey(key);
        write_image(izsKey, keyBuffer);

        Buffer valueBuffer;
        izenelib::util::izene_serialization<value_type> izsValue(value);
        write_image(izsValue, valueBuffer);

        return rawAm().insert(keyBuffer, valueBuffer);
    }
    bool insert(const data_type& data)
    {
        return insert(data.get_key(), data.get_value());
    }
    bool update(const key_type& key, const value_type& value)
    {
        Buffer keyBuffer;
        izenelib::util::izene_serialization<key_type> izsKey(key);
        write_image(izsKey, keyBuffer);

        Buffer valueBuffer;
        izenelib::util::izene_serialization<value_type> izsValue(value);
        write_image(izsValue, valueBuffer);

        return rawAm().update(keyBuffer, valueBuffer);
    }
    bool update(const data_type& data)
    {
        return update(data.get_key(), data.get_value());
    }
    bool append(const key_type& key, const value_type& value)
    {
        Buffer keyBuffer;
        izenelib::util::izene_serialization<key_type> izsKey(key);
        write_image(izsKey, keyBuffer);

        Buffer valueBuffer;
        izenelib::util::izene_serialization<value_type> izsValue(value);
        write_image(izsValue, valueBuffer);

        return rawAm().append(keyBuffer, valueBuffer);
    }
    bool get(const key_type& key, value_type& value) const
    {
        Buffer keyBuffer;
        izenelib::util::izene_serialization<key_type> izsKey(key);
        write_image(izsKey, keyBuffer);

        Buffer valueBuffer;
        bool status = rawAm().get(keyBuffer, valueBuffer);

        if (status)
        {
            izenelib::util::izene_deserialization<value_type> izdValue(
                valueBuffer.data(), valueBuffer.size()
            );
            izdValue.read_image(value);
        }

        return status;
    }
    bool exist(const key_type& key) const
    {
        Buffer keyBuffer;

        izenelib::util::izene_serialization<key_type> izsKey(key);
        write_image(izsKey, keyBuffer);

        return rawAm().exist(keyBuffer);
    }
    bool del(const key_type& key)
    {
        Buffer keyBuffer;
        izenelib::util::izene_serialization<key_type> izsKey(key);
        write_image(izsKey, keyBuffer);

        return rawAm().del(keyBuffer);
    }


    bool iterInit()
    {
        return rawAm().iterInit();
    }

    bool iterInit(const KeyType& key)
    {
        izenelib::am::raw::Buffer keyBuffer;
        izenelib::util::izene_serialization<key_type> izsKey(key);
        write_image(izsKey, keyBuffer);

        return rawAm().iterInit(keyBuffer);
    }

    bool iterNext(KeyType& key)
    {
        izenelib::am::raw::Buffer keyBuffer;
        izenelib::util::izene_serialization<KeyType> izsKey(key);
        write_image(izsKey, keyBuffer);

        if (rawAm().iterNext(keyBuffer))
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

        if (rawAm().iterNext(keyBuffer, valueBuffer))
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

    cursor_type begin() const
    {
        return rawAm().begin();
    }

    cursor_type rbegin() const
    {
        return rawAm().rbegin();
    }

    cursor_type begin(const KeyType& key) const
    {
        izenelib::am::raw::Buffer keyBuffer;
        izenelib::util::izene_serialization<key_type> izsKey(key);
        write_image(izsKey, keyBuffer);
        return rawAm().begin(keyBuffer);
    }

    bool fetch(cursor_type& cursor,key_type& key, value_type& value)
    {
        Buffer keyBuffer;
        Buffer valueBuffer;
        bool status = rawAm().fetch(cursor,keyBuffer, valueBuffer);

        if (status)
        {
            izenelib::util::izene_deserialization<key_type> izdKey(
                keyBuffer.data(), keyBuffer.size()
            );
            izdKey.read_image(key);
            izenelib::util::izene_deserialization<value_type> izdValue(
                valueBuffer.data(), valueBuffer.size()
            );
            izdValue.read_image(value);
        }
        return status;
    }

    bool iterNext(cursor_type& cursor)
    {
        return rawAm().iterNext(cursor);
    }

    bool iterPrev(cursor_type& cursor)
    {
        return rawAm().iterPrev(cursor);
    }

    bool getFirst(KeyType& key) const
    {
        izenelib::am::raw::Buffer keyBuffer;

        if (rawAm().getFirst(keyBuffer))
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

        if (rawAm().getFirst(keyBuffer, valueBuffer))
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

        if (rawAm().getNext(keyBuffer))
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

        if (rawAm().getNext(keyBuffer, valueBuffer))
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


private:
    const raw_am_type& rawAm() const
    {
        return detail::AmWrapperAccess::rawAm<raw_am_type>(
                   *(static_cast<const Derived*>(this))
               );
    }

    raw_am_type& rawAm()
    {
        return detail::AmWrapperAccess::rawAm<raw_am_type>(
                   *(static_cast<Derived*>(this))
               );
    }
};

}
}
} // namespace izenelib::am::raw

#endif // AM_RAW_AM_WRAPPER_H
