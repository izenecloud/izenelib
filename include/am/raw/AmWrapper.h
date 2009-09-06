#ifndef AM_RAW_AM_WRAPPER_H
#define AM_RAW_AM_WRAPPER_H
/**
 * @file am/raw/AmWrapper.h
 * @author Ian Yang
 * @date Created <2009-09-06 22:50:37>
 * @date Updated <2009-09-06 23:36:52>
 * @brief Integrate raw am and serialization framework
 */

#include "Buffer.h"
#include "WriteImage.h"

namespace izenelib {
namespace am {
namespace raw {

namespace detail {
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


template<typename Derived>
class AmWrapper
{
public:
    typedef typename Derived::key_type key_type;
    typedef typename Derived::value_type value_type;
    typedef typename Derived::data_type data_type;
    typedef typename Derived::size_type size_type;

    typedef typename Derived::raw_am_type raw_am_type;

    bool open()
    {
        return rawAm().open();
    }
    bool open(int mode)
    {
        return rawAm().open(mode);
    }
    bool open(const std::string& file);
    {
        return rawAm().open(file);
    }
    bool open(const std::string& file, int mode);
    {
        return rawAm().open(file, mode);
    }
    void close()
    {
        rawAm().close();
    }
    bool isOpened() const
    {
        return rawAm().isOpened();
    }
    bool flush()
    {
        return rawAm().flush();
    }
    size_type size() const
    {
        return rawAm().size();
    }
    bool empty() const
    {
        return rawAm().empty();
    }
    bool insert(const KeyType& key, const ValueType& value)
    {
        Buffer keyBuffer;
        izenelib::util::izene_serialization<KeyType> izsKey(key);
        write_image(izsKey, keyBuffer);

        Buffer valueBuffer;
        izenelib::util::izene_serialization<ValueType> izsValue(value);
        write_image(izsValue, valueBuffer);

        return rawAm().insert(keyBuffer, valueBuffer);
    }
    bool insert(const data_type& date)
    {
        return insert(data.get_key(), data.get_value());
    }
    bool update(const KeyType& key, const ValueType& value)
    {
        Buffer keyBuffer;
        izenelib::util::izene_serialization<KeyType> izsKey(key);
        write_image(izsKey, keyBuffer);

        Buffer valueBuffer;
        izenelib::util::izene_serialization<ValueType> izsValue(value);
        write_image(izsValue, valueBuffer);

        return rawAm().update(keyBuffer, valueBuffer);
    }
    bool update(const data_type& data)
    {
        return update(data.get_key(), data.get_value());
    }
    bool get(const KeyType& key, ValueType& value) const
    {
        Buffer keyBuffer;
        izenelib::util::izene_serialization<KeyType> izsKey(key);
        write_image(izsKey, keyBuffer);

        Buffer valueBuffer;
        bool status = rawAm().get(keyBuffer, valueBuffer);

        if (status)
        {
            izenelib::util::izene_deserialization<ValueType> izdValue(
                valueBuffer.data(), valueBuffer.size()
            );
            izdValue.read_image(value);
        }

        return status;
    }
    bool exist(const KeyType& key) const
    {
        Buffer keyBuffer;

        izenelib::util::izene_serialization<KeyType> izsKey(key);
        write_image(izsKey, keyBuffer);

        return rawAm().exist(keyBuffer);
    }
    bool del(const KeyType& key)
    {
        Buffer keyBuffer;
        izenelib::util::izene_serialization<KeyType> izsKey(key);
        write_image(izsKey, keyBuffer);

        return rawAm().del(keyBuffer);
    }

private:
    const raw_am_type& rawAm() const
    {
        return detail::AmWrapperAccess<raw_am_type>::rawAm(
            *(static_cast<const Derived*>(this))
        );
    }

    raw_am_type& rawAm()
    {
        return detail::AmWrapperAccess<raw_am_type>::rawAm(
            *(static_cast<Derived*>(this))
        );
    }
};

}}} // namespace izenelib::am::raw

#endif // AM_RAW_AM_WRAPPER_H
