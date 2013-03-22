#ifndef AM_LEVELDB_RAW_TABLE_H
#define AM_LEVELDB_RAW_TABLE_H
/**
 * @file am/leveldb/Table.h
 * @author Yingfeng Zhang
 * @date Created <2011-05-10 10:37:24>
 * @date Updated <2011-11-08 20:01:44>
 * @brief LevelDB wrapper
 */

#include <am/concept/DataType.h>
#include <am/raw/Buffer.h>

#include <3rdparty/am/leveldb/db.h>
#include <3rdparty/am/leveldb/comparator.h>
#include <glog/logging.h>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include <list>
namespace izenelib {
namespace am {
namespace leveldb {
namespace raw {

using izenelib::am::raw::Buffer;

template<typename Comp>
class Table
    : boost::noncopyable
{
public:
    typedef Buffer key_type;
    typedef Buffer value_type;
    typedef DataType<Buffer, Buffer> data_type;
    typedef int size_type;
    typedef boost::shared_ptr< ::leveldb::Iterator> cursor_type;
    typedef ::leveldb::DB*  raw_type_ptr;

    explicit Table(const std::string& file = "")
        : db_(NULL),comp_(), file_(file)
    {
    }

    ~Table()
    {
        close();
    }

    bool open(const std::string& file)
    {
        file_ = file;
        return open();
    }

    bool open()
    {
        close(); // close first if opened

        ::leveldb::Options options;
        options.create_if_missing = true;
        options.comparator = &comp_;

        ::leveldb::Status status;

        status = ::leveldb::DB::Open(options, file_, &db_);

        if (status.ok())
            return true;

        return false;
    }
    std::string getFileName() const
    {
        return file_;
    }
    void close()
    {
        if (db_)
        {
            checkHandle_(db_);
            delete db_;
            db_ = NULL;
        }
    }

    bool isOpened() const
    {
        return checkHandle_(db_);
    }
    /**
     * @deprecated
     */
    bool is_open() const
    {
        return checkHandle_(db_);
    }

    bool flush()
    {
        // reopen to flush cached data.
        return checkHandle_(db_);
    }

    size_type size() const
    {
        //Not supported now, use iterator
        size_type size = 0;
        if (checkHandle_(db_))
        {
            ::leveldb::ReadOptions options;
            options.fill_cache = false;
            ::leveldb::Iterator* it = db_->NewIterator(options);
            for (it->SeekToFirst(); it->Valid(); it->Next())
            {
                ++size;
            }
            delete it;
        }
        return size;
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

    /*
     * @attention:
     * clear method will destroy the leveldb instance,
     * be sure all DB iterators have been destroyed
     * before calling this method
     */
    bool clear()
    {
        if (isOpened())
        {
            close();
            boost::filesystem::remove_all(file_);
            open();
        }
        return true;
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
        return checkHandle_(db_) && (db_->Put(
            ::leveldb::WriteOptions(),
            ::leveldb::Slice(key.data(),key.size()),
            ::leveldb::Slice(value.data(),value.size())).ok()
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
        return insert(key, value);
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
    bool append(const Buffer& key, const Buffer& value)
    {
        ///not supported yet
        return false;
    }
    bool get(const Buffer& key, Buffer& value) const
    {
        if (checkHandle_(db_) )
        {
            ::leveldb::Status s = db_->Get(::leveldb::ReadOptions(), ::leveldb::Slice(key.data(),key.size()), &(value.strbuffer()));
            if (s.ok())
            {
                value.attach();
                return true;
            }
        }

        return false;
    }

    bool del(const Buffer& key)
    {
        return checkHandle_(db_) &&
            db_->Delete(
                    ::leveldb::WriteOptions(),
                    ::leveldb::Slice(key.data(),key.size())).ok();
    }

    cursor_type begin() const
    {
        if (isOpened())
        {
            ::leveldb::ReadOptions options;
            options.fill_cache = false;
            cursor_type cursor(db_->NewIterator(options));
            cursor->SeekToFirst();
            return cursor;
        }
        return cursor_type();
    }

    cursor_type begin(Buffer& key) const
    {
        if (isOpened())
        {
            ::leveldb::ReadOptions options;
            options.fill_cache = false;
            cursor_type cursor(db_->NewIterator(options));
            cursor->Seek(::leveldb::Slice(key.data(),key.size()));
            return cursor;
        }
        return cursor_type();
    }

    cursor_type rbegin() const
    {
        if (isOpened())
        {
            ::leveldb::ReadOptions options;
            options.fill_cache = false;
            cursor_type cursor(db_->NewIterator(options));
            cursor->SeekToLast();
            return cursor;
        }
        return cursor_type();
    }

    bool fetch(cursor_type& cursor, Buffer& key, Buffer& value)
    {
        if (isOpened() && cursor.get() && cursor->Valid())
        {
            key.attach(const_cast<char*>(cursor->key().data()),
                    static_cast<std::size_t>(cursor->key().size()));
            value.attach(const_cast<char*>(cursor->value().data()),
                    static_cast<std::size_t>(cursor->value().size()));
            return true;
        }
        return false;
    }

    bool iterNext(cursor_type& cursor)
    {
        if (isOpened() && cursor.get() && cursor->Valid())
        {
            cursor->Next();
            return true;
        }
        else
        {
            cursor.reset();
            return false;
        }
    }

    bool iterPrev(cursor_type& cursor)
    {
        if (isOpened() && cursor.get() && cursor->Valid())
        {
            cursor->Prev();
            return true;
        }
        else
        {
            cursor.reset();
            return false;
        }
    }

private:
    static bool checkHandle_(::leveldb::DB* h)
    {
        return h;
    }

    ::leveldb::DB* db_;

    Comp comp_;

    std::string file_;
};

}}}} // namespace izenelib::am::leveldb::raw

#endif // AM_LEVELDB_RAW_TABLE_H
