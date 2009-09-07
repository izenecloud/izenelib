#ifndef AM_TOKYO_CABINET_RAW_HASH_H
#define AM_TOKYO_CABINET_RAW_HASH_H
/**
 * @file am/tokyo_cabinet/RawHash.h
 * @author Ian Yang
 * @date Created <2009-09-02 14:11:06>
 * @date Updated <2009-09-02 17:41:08>
 * @brief Raw AM wrapper of tokyo cabinet hash database, which only can store
 * izenelib::am::tc::Buffer
 */

namespace izenelib {
namespace am {
namespace tc {

class RawHash
    : boost::noncopyable
{
public:
    typedef Buffer key_type;
    typedef Buffer value_type;
    typedef DataType<Buffer, Buffer> data_type;

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

    explicit RawHash(const std::string& file)
    : hdb_(::tchdbnew()), isOpened_(false), file_(file)
    {
        checkHandle_(hdb_);
    }

    ~RawHash()
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
        return checkHandle_() && ::tchdbsetmutex(hdb_);
    }
    bool setcache(int32_t rcnum)
    {
        return checkHandle_() && ::tchdbsetcache(hdb_, rcnum);
    }
    bool tune(int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts)
    {
        return checkHandle_() && ::tchdbtune(hdb_, bnum, apow, fpow, opts);
    }
    bool setxmsiz(int64_t xmsiz)
    {
        return checkHandle_() && ::tchdbsetxmsiz(hdb_, xmsiz);
    }
    bool setdfunit(int32_t dfunit)
    {
        return checkHandle_() && ::tchdbsetdfunit(hdb_, dfunit);
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

        if (checkHandle_() && ::tchdbopen(hdb_, file_.c_str(), mode))
        {
            isOpened_ = true;
        }

        return isOpened_;
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

    bool flush()
    {
        return checkHandle_() && ::tchdbsync(hdb_);
    }

    size_t size() const
    {
        return hdb_ ? ::tchdbrnum(hdb_) : 0;
    }
    bool empty() const
    {
        return size() == 0;
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
        return checkHandle_() && ::tchdbputkeep(
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
        return checkHandle_() && ::tchdbput(
            hdb_,
            key.data(), key.size(),
            value.data(0), value.data(0)
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

	bool get(const Buffer& key, Buffer& value)
	{
        void* buffer;
        int size;
        if (checkHandle_() &&
            (buffer = ::tchdbget(hdb_, key.data(), key.size(), &size)))
        {
            value.attach(buffer,
                         static_cast<std::size_t>(size),
                         &::tcfree);

            return true;
        }

        return false;
	}



    bool exist(const Buffer& key) const
    {
        return checkHandle_() &&
            (-1 != ::tchdbvsiz(hdb_, key.data(), key.size()));
    }

    bool del(const Buffer& key)
    {
        return checkHandle_() &&
            ::tchdbout(hdb_, key.data(), key.size());
    }

    const char* errorMessage() const
    {
        errorMessage(errorCode());
    }
    static const char* errorMessage(int code)
    {
        ::tchdberrmsg(code);
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

    bool isOpened_;
    ::TCHDB* hdb_;
    std::string file_;
};

}}} // namespace izenelib::am::tc

#endif // AM_TOKYO_CABINET_RAW_HASH_H
