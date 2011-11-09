#ifndef AM_BEANSDB_RAW_HASH_H
#define AM_BEANSDB_RAW_HASH_H
/**
 * @file am/beansdb/raw/Hash.h
 * @author Ian Yang
 * @date Created <2009-09-02 14:11:06>
 * @date Updated <2009-09-10 10:37:42>
 * @brief Raw AM wrapper of beansdb hash database, which only can store
 * izenelib::am::raw::Buffer
 */
#include <am/concept/DataType.h>
#include <am/raw/Buffer.h>

#include <3rdparty/am/beansdb/hstore.h>

#include <boost/optional.hpp>

namespace izenelib {
namespace am {
namespace beansdb {
namespace raw {

using izenelib::am::raw::Buffer;
	
class Hash
    : boost::noncopyable
{
public:
    typedef Buffer key_type;
    typedef Buffer value_type;
    typedef DataType<Buffer, Buffer> data_type;
    typedef int size_type;
    typedef boost::optional<key_type> cursor_type;

    explicit Hash(const std::string& file)
    {
        hdb_ = hs_open(file.c_str(), 1, 0);
        isOpened_ = true;
    }

    ~Hash()
    {
        close();
    }

    bool open()
    {
        return true;
    }

    void close()
    {
        if (hdb_ && isOpened_)
        {
            hs_close(hdb_);
            hdb_ = NULL;
            isOpened_ = false;
        }
    }

    bool optimize()
    {
        bool limit = 10000;
        return checkHandle_(hdb_)&&(hs_optimize(hdb_, limit));
    }

    bool isOpened() const
    {
        return isOpened_;
    }
    /**
     * @deprecated
     */
    bool is_open() const
    {
        return isOpened_;
    }

    bool flush()
    {
        int flush_limit = 1024 * 2;
        hs_flush(hdb_, flush_limit);
        return true;
    }

    size_type size() const
    {
        return 0;
    }
    /**
     * @deprecated
     */
    size_type num_items() const
    {
        return size();
    }
    bool empty() const
    {
        return size() == 0;
    }
    bool clear()
    {
        return false;
    }

    /**
     * @brief Insert new data into database.
     *
     * If a record with the same key exists in the database, this function has no effect.
     *
     * @param key Key Buffer
     * @param value Value Buffer
     *
     * @return If successful, the return value is @c true, else, it is @c false.
     */
    bool insert(const Buffer& key, const Buffer& value)
    {
        unsigned flag = 0;
        return hs_set(hdb_, (char*)key.data(), (char*)value.data(), value.size(), flag, 0);
    }
    /**
     * @brief Insert new data into database.
     *
     * If a record with the same key exists in the database, this function has no effect.
     *
     * @param data data record
     *
     * @return If successful, the return value is @c true, else, it is @c false.
     */
    bool insert(const data_type& data)
    {
        return insert(data.get_key(), data.get_value());
    }

    /**
     * @brief Insert new data into database or update the existing record.
     *
     * @param key Key Buffer
     * @param value Value Buffer
     *
     * @return If successful, the return value is @c true, else, it is @c false.
     */
    bool update(const Buffer& key, const Buffer& value)
    {
        unsigned flag = 0;
        return hs_set(hdb_, (char*)key.data(), (char*)value.data(), value.size(), flag, 0);
    }
    /**
     * @brief Insert new data into database or update the existing record.
     *
     * @param data data record
     *
     * @return If successful, the return value is @c true, else, it is @c false.
     */
    bool update(const data_type& data)
    {
        return update(data.get_key(), data.get_value());
    }

    bool get(const Buffer& key, Buffer& value) const
    {
        int size;
        unsigned flag;
        char *buffer = hs_get(hdb_, (char*)key.data(), &size, &flag);
        if (buffer)
        {
            value.attach(buffer,
                         static_cast<std::size_t>(size),
                         &std::free);

            return true;
        }

        return false;
    }

    bool exist(const Buffer& key) const
    {
        return false;
    }

    bool del(const Buffer& key)
    {
        return checkHandle_(hdb_) && isOpened() &&
            ::hs_delete(hdb_, (char*)key.data());
    }

    bool append(const Buffer& key, const Buffer& value)
    {
        return hs_append(hdb_, (char*)key.data(), (char*)value.data(), value.size());
    }

private:

    static bool checkHandle_(::HStore* h)
    {
        return h;
    }

    ::HStore* hdb_;

    bool isOpened_;
};

}}}} // namespace izenelib::am::tc::raw

#endif // AM_TC_RAW_HASH_H
