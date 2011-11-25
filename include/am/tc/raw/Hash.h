#ifndef AM_TC_RAW_HASH_H
#define AM_TC_RAW_HASH_H
/**
 * @file am/tc/raw/Hash.h
 * @author Ian Yang
 * @date Created <2009-09-02 14:11:06>
 * @date Updated <2009-09-10 10:37:42>
 * @brief Raw AM wrapper of tokyo cabinet hash database, which only can store
 * izenelib::am::raw::Buffer
 */
#include <am/concept/DataType.h>
#include <am/raw/Buffer.h>
#include <am/tc/String.h>
#include <am/range/IterNextRange.h>
#include <am/range/GetNextRange.h>

#include <tcutil.h>
#include <tchdb.h>

#include <boost/optional.hpp>

namespace izenelib {
namespace am {
namespace tc {
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

    typedef IterNextRange<Hash> exclusive_range_type;
    typedef GetNextRange<Hash> range_type;

    typedef boost::optional<key_type> cursor_type;

    enum {
        READER = ::HDBOREADER,
        WRITER = ::HDBOWRITER,
        CREAT  = ::HDBOCREAT,
        TRUNC  = ::HDBOTRUNC,
        NOLCK  = ::HDBONOLCK,
        LCKNB  = ::HDBOLCKNB,
        TSYNC  = ::HDBOTSYNC,
    };

    enum { DEFAULT_OPEN_MODE = WRITER | CREAT };

    explicit Hash(const std::string& file = "")
        : hdb_(::tchdbnew()), isOpened_(false), file_(file)
    {
        checkHandle_(hdb_);
    }

    ~Hash()
    {
        close();
        if (hdb_)
        {
            ::tchdbdel(hdb_);
        }
    }

    //@{
    //@brief tuning functions used before opening, refer to the manual of
    //tokyo cabinet

    bool setmutex()
    {
        return checkHandle_(hdb_) && ::tchdbsetmutex(hdb_);
    }
    bool setcache(int32_t rcnum)
    {
        return checkHandle_(hdb_) && ::tchdbsetcache(hdb_, rcnum);
    }
    bool tune(int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts)
    {
        return checkHandle_(hdb_) && ::tchdbtune(hdb_, bnum, apow, fpow, opts);
    }
    bool setxmsiz(int64_t xmsiz)
    {
        return checkHandle_(hdb_) && ::tchdbsetxmsiz(hdb_, xmsiz);
    }
    bool setdfunit(int32_t dfunit)
    {
        return checkHandle_(hdb_) && ::tchdbsetdfunit(hdb_, dfunit);
    }
    //@}

    bool open(const std::string& file, int mode = DEFAULT_OPEN_MODE)
    {
        file_ = file;
        return open(mode);
    }
    bool open(int mode = DEFAULT_OPEN_MODE)
    {
        close(); // close first if opened

        if (checkHandle_(hdb_) && !file_.empty() &&
            ::tchdbopen(hdb_, file_.c_str(), mode))
        {
            isOpened_ = true;
        }

        return isOpened_;
    }
    std::string getFileName() const
    {
        return file_;
    }
    void close()
    {
        if (hdb_ && isOpened_)
        {
            ::tchdbclose(hdb_);
            isOpened_ = false;
        }
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
        return checkHandle_(hdb_) && isOpened() && ::tchdbsync(hdb_);
    }

    size_type size() const
    {
        return hdb_ ? ::tchdbrnum(hdb_) : 0;
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
        return checkHandle_(hdb_) && isOpened() && ::tchdbvanish(hdb_);
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
        return checkHandle_(hdb_) && isOpened() && ::tchdbputkeep(
            hdb_,
            key.data(), key.size(),
            value.data(), value.size()
        );
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
        return checkHandle_(hdb_) && isOpened() && ::tchdbput(
            hdb_,
            key.data(), key.size(),
            value.data(), value.size()
        );
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
        void* buffer;
        int size;
        if (checkHandle_(hdb_) && isOpened() &&
            (buffer = ::tchdbget(hdb_, key.data(), key.size(), &size)))
        {
            value.attach(static_cast<char*>(buffer),
                         static_cast<std::size_t>(size),
                         &::tcfree);

            return true;
        }

        return false;
    }

    bool exist(const Buffer& key) const
    {
        return checkHandle_(hdb_) && isOpened() &&
            (-1 != ::tchdbvsiz(hdb_, key.data(), key.size()));
    }

    bool del(const Buffer& key)
    {
        return checkHandle_(hdb_) && isOpened() &&
            ::tchdbout(hdb_, key.data(), key.size());
    }

    //@{
    //@brief iteration

    bool iterInit()
    {
        return checkHandle_(hdb_) && isOpened() &&
            ::tchdbiterinit(hdb_);
    }
    /**
     * @brief initialize iterator to the corresponding \a key.
     * @return \c true if success. \c false if failed or cannot find the key.
     */
    bool iterInit(const Buffer& key)
    {
        return checkHandle_(hdb_) && isOpened() &&
            ::tchdbiterinit2(hdb_, key.data(), key.size());
    }
    bool iterNext(Buffer& key)
    {
        if (checkHandle_(hdb_) && isOpened())
        {
            int size = 0;
            void* result = ::tchdbiternext(hdb_, &size);
            if (result)
            {
                key.attach(static_cast<char*>(result),
                           static_cast<std::size_t>(size),
                           &::tcfree);
                return true;
            }
        }

        return false;
    }
    bool iterNext(Buffer& key, Buffer& value)
    {
        if (! (checkHandle_(hdb_) && isOpened()))
        {
            return false;
        }

        String tmpKey;
        String tmpValue;
        if (::tchdbiternext3(hdb_,
                             detail::StringAccessor::handle(tmpKey),
                             detail::StringAccessor::handle(tmpValue)))
        {
            tmpKey.toBuffer(key);
            tmpValue.toBuffer(value);

            return true;
        }

        return false;
    }
    bool iterNext(data_type& data)
    {
        return iterNext(data.get_key(), data.get_value());
    }

    //@{
    //@brief iteration
    cursor_type begin() const
    {
        ::tchdbiterinit(hdb_);
        return cursor_type();
    }

    cursor_type begin(Buffer& key) const
    {
        ::tchdbiterinit2(hdb_, key.data(), key.size());
        return cursor_type();
    }

    bool fetch(cursor_type& cursor, Buffer& key, Buffer& value)
    {
        return iterNext(key,value);
    }

    bool iterNext(cursor_type& cursor)
    {
        return true;
    }

    bool iterPrev(cursor_type& cursor)
    {
        return false;
    }

    bool getFirst(Buffer& key) const
    {
        Buffer empty;
        if (getNext(empty))
        {
            key.swap(empty);
            return true;
        }

        return false;
    }
    bool getFirst(Buffer& key, Buffer& value) const
    {
        Buffer empty;
        if (getNext(empty, value))
        {
            key.swap(empty);
            return true;
        }

        return false;
    }
    bool getFirst(data_type& data) const
    {
        return getFirst(data.get_key(), data.get_value());
    }
    bool getNext(Buffer& key) const
    {
        if (! (checkHandle_(hdb_) && isOpened()))
        {
            return false;
        }

        int size = 0;
        void* keyBuf = ::tchdbgetnext(hdb_, key.data(), key.size(), &size);
        if (keyBuf)
        {
            key.attach(static_cast<char*>(keyBuf),
                       static_cast<std::size_t>(size),
                       &::tcfree);
            return true;
        }

        return false;
    }
    bool getNext(Buffer& key, Buffer& value) const
    {

        if (! (checkHandle_(hdb_) && isOpened()))
        {
            return false;
        }

        int keySize = 0;
        int valueSize = 0;
        const char* valueBuf = 0;

        char* keyBuf = ::tchdbgetnext3(
            hdb_,
            key.data(), key.size(),
            &keySize,
            &valueBuf, &valueSize
        );

        if (keyBuf && valueBuf)
        {
            key.attach(static_cast<char*>(keyBuf),
                       static_cast<std::size_t>(keySize),
                       &::tcfree);
            value.copyAttach(static_cast<const char*>(valueBuf),
                             static_cast<std::size_t>(valueSize));

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
    void exclusiveAll(exclusive_range_type& range)
    {
        range.attach(*this);
    }

    //@{
    //@brief tc special functions

    bool append(const Buffer& key, const Buffer& value)
    {
        return checkHandle_(hdb_) && isOpened() &&
            ::tchdbputcat(
                hdb_,
                key.data(), key.size(),
                value.data(), value.size()
            );
    }

    bool updateAsync(const Buffer& key, const Buffer& value)
    {
        return checkHandle_(hdb_) && isOpened() &&
            ::tchdbputasync(
                hdb_,
                key.data(), key.size(),
                value.data(), value.size()
            );
    }

    //@}

    const char* errorMessage() const
    {
        return errorMessage(errorCode());
    }
    static inline const char* errorMessage(int code)
    {
        return ::tchdberrmsg(code);
    }
    int errorCode() const
    {
        if (hdb_)
        {
            return ::tchdbecode(hdb_);
        }

        return ::TCEOPEN;
    }

private:

    static bool checkHandle_(::TCHDB* h)
    {
        BOOST_ASSERT(h);
        // if (!h) // todo: add logs
        return h;
    }

    ::TCHDB* hdb_;
    bool isOpened_;
    std::string file_;
};

}}}} // namespace izenelib::am::tc::raw

#endif // AM_TC_RAW_HASH_H
