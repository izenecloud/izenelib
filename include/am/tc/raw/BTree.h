#ifndef AM_TC_RAW_BTREE_H
#define AM_TC_RAW_BTREE_H
/**
 * @file am/tc/raw/BTree.h
 * @author Ian Yang
 * @date Created <2009-09-10 09:37:25>
 * @date Updated <2009-09-10 10:46:40>
 */

#include <am/concept/DataType.h>
#include <am/raw/Buffer.h>
#include <am/tc/String.h>

#include <tcutil.h>
#include <tcbdb.h>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace izenelib {
namespace am {
namespace tc {
namespace raw {

using izenelib::am::raw::Buffer;

class BTree
    : boost::noncopyable
{
public:
    typedef Buffer key_type;
    typedef Buffer value_type;
    typedef DataType<Buffer, Buffer> data_type;
    typedef int size_type;
    typedef boost::shared_ptr<BDBCUR> cursor_type;

    enum {
        READER = ::BDBOREADER,
        WRITER = ::BDBOWRITER,
        CREAT  = ::BDBOCREAT,
        TRUNC  = ::BDBOTRUNC,
        NOLCK  = ::BDBONOLCK,
        LCKNB  = ::BDBOLCKNB,
        TSYNC  = ::BDBOTSYNC,
    };

    enum { DEFAULT_OPEN_MODE = WRITER | CREAT };

    explicit BTree(const std::string& file = "")
    : bdb_(::tcbdbnew()), isOpened_(false), file_(file)
    {
        checkHandle_(bdb_);
    }

    ~BTree()
    {
        close();
        if (bdb_)
        {
            ::tcbdbdel(bdb_);
        }
    }

    //@{
    //@brief tuning functions used before opening, refer to the manual of
    //tokyo cabinet

    bool setmutex()
    {
        return checkHandle_(bdb_) && ::tcbdbsetmutex(bdb_);
    }
    bool setcache(int32_t lcnum, int32_t ncnum)
    {
        return checkHandle_(bdb_) && ::tcbdbsetcache(bdb_, lcnum, ncnum);
    }
    bool tune(int32_t lmemb, int32_t nmemb,
              int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts)
    {
        return checkHandle_(bdb_) &&
            ::tcbdbtune(bdb_, lmemb, nmemb, bnum, apow, fpow, opts);
    }
    bool setxmsiz(int64_t xmsiz)
    {
        return checkHandle_(bdb_) && ::tcbdbsetxmsiz(bdb_, xmsiz);
    }
    bool setdfunit(int32_t dfunit)
    {
        return checkHandle_(bdb_) && ::tcbdbsetdfunit(bdb_, dfunit);
    }
    bool setcmpfunc(TCCMP cmp, void* cmpop)
    {
        return checkHandle_(bdb_) && ::tcbdbsetcmpfunc(bdb_, cmp, cmpop);
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

        if (checkHandle_(bdb_) && !file_.empty() &&
            ::tcbdbopen(bdb_, file_.c_str(), mode))
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
        if (bdb_ && isOpened_)
        {
            flush();
            ::tcbdbclose(bdb_);
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
        return checkHandle_(bdb_) && isOpened() && ::tcbdbsync(bdb_);
    }

    size_type size() const
    {
        return bdb_ ? ::tcbdbrnum(bdb_) : 0;
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
        return checkHandle_(bdb_) && isOpened() && ::tcbdbvanish(bdb_);
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
        return checkHandle_(bdb_) && isOpened() && ::tcbdbputkeep(
            bdb_,
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
        return checkHandle_(bdb_) && isOpened() && ::tcbdbput(
            bdb_,
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
        if (checkHandle_(bdb_) && isOpened() &&
            (buffer = ::tcbdbget(bdb_, key.data(), key.size(), &size)))
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
        return checkHandle_(bdb_) && isOpened() &&
            (-1 != ::tcbdbvsiz(bdb_, key.data(), key.size()));
    }

    bool del(const Buffer& key)
    {
        return checkHandle_(bdb_) && isOpened() &&
            ::tcbdbout(bdb_, key.data(), key.size());
    }

    //@{
    //@brief tc special functions

    bool append(const Buffer& key, const Buffer& value)
    {
        return checkHandle_(bdb_) && isOpened() &&
            ::tcbdbputcat(
                bdb_,
                key.data(), key.size(),
                value.data(), value.size()
            );
    }

    //@{
    //@brief iteration
    cursor_type begin() const
    {
        if(checkHandle_(bdb_) && isOpened())
        {
            cursor_type ans(::tcbdbcurnew(bdb_));
            bool result = ::tcbdbcurfirst(ans.get());
            if(result) return ans;
        }
        return cursor_type();
    }

    cursor_type begin(Buffer& key) const
    {
        if(checkHandle_(bdb_) && isOpened())
        {
            cursor_type ans(::tcbdbcurnew(bdb_));
            //The cursor is set to the first record corresponding the key
            //or the next substitute if completely matching record does not exist.
            bool result = ::tcbdbcurjump(ans.get(), key.data(), key.size());
            if(result) return ans;
        }
        return cursor_type();
    }

    //@{
    //@brief iteration
    cursor_type rbegin() const
    {
        if(checkHandle_(bdb_) && isOpened())
        {
            cursor_type ans(::tcbdbcurnew(bdb_));
            bool result = ::tcbdbcurlast(ans.get());
            if(result) return ans;
        }
        return cursor_type();
    }

    bool fetch(cursor_type& cursor, Buffer& key, Buffer& value)
    {
        if(checkHandle_(bdb_) && isOpened() && cursor.get())
        {
            int size;
            void* buffer = ::tcbdbcurkey(cursor.get(), &size);
            key.attach(static_cast<char*>(buffer),
                                static_cast<std::size_t>(size),
                                &::tcfree);
            buffer = ::tcbdbcurval(cursor.get(), &size);
            value.attach(static_cast<char*>(buffer),
                                static_cast<std::size_t>(size),
                                &::tcfree);
            return true;
        }
        return false;
    }

    bool iterNext(cursor_type& cursor)
    {
        bool ret = ::tcbdbcurnext(cursor.get());
        if(!ret) cursor.reset();
        return ret;
    }

    bool iterPrev(cursor_type& cursor)
    {
        bool ret = ::tcbdbcurprev(cursor.get());
        if(!ret) cursor.reset();
        return ret;
    }

    //@}
    const char* errorMessage() const
    {
        return errorMessage(errorCode());
    }
    static inline const char* errorMessage(int code)
    {
        return ::tcbdberrmsg(code);
    }
    int errorCode() const
    {
        if (bdb_)
        {
            return ::tcbdbecode(bdb_);
        }

        return ::TCEOPEN;
    }

private:

    static bool checkHandle_(::TCBDB* h)
    {
        BOOST_ASSERT(h);
        // if (!h) // todo: add logs
        return h;
    }

    ::TCBDB* bdb_;
    bool isOpened_;
    std::string file_;
};

}}}} // namespace izenelib::am::tc::raw

#endif // AM_TC_RAW_BTREE_H
