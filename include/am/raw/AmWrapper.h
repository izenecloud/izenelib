#ifndef AM_RAW_AM_WRAPPER_H
#define AM_RAW_AM_WRAPPER_H
/**
 * @file am/raw/AmWrapper.h
 * @author Ian Yang
 * @date Created <2009-09-06 22:50:37>
 * @date Updated <2009-09-07 10:05:04>
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
    bool open(const std::string& file)
    {
        return rawAm().open(file);
    }
    bool open(const std::string& file, int mode)
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

}}} // namespace izenelib::am::raw

#endif // AM_RAW_AM_WRAPPER_H
