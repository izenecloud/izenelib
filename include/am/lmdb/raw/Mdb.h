#ifndef AM_LMDB_RAW_MDB_H
#define AM_LMDB_RAW_MDB_H

#include <am/concept/DataType.h>
#include <am/raw/Buffer.h>

#include <3rdparty/am/lmdb/lmdb.h>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <glog/logging.h>
#include <list>

namespace izenelib {
namespace am {
namespace lmdb {
namespace raw {

using izenelib::am::raw::Buffer;
using namespace boost::filesystem;
namespace bfs = boost::filesystem;

struct MdbEx: public std::runtime_error {MdbEx (std::string message): std::runtime_error (message) {}};

class Mdb
    : boost::noncopyable
{
public:
    typedef Buffer key_type;
    typedef Buffer value_type;
    typedef DataType<Buffer, Buffer> data_type;
    typedef int size_type;
    typedef boost::shared_ptr<MDB_cursor> cursor_type;
    typedef Mdb* raw_type_ptr;
    typedef boost::interprocess::unique_ptr<MDB_txn, void(*)(MDB_txn*)> transaction_type;

    explicit Mdb(const std::string& fullPath = "" , unsigned int maxSizeInMb = 1024 )
        : dbi_(NULL),file_(fullPath)
    {
        bfs::create_directories(fullPath);
        MDB_env* env = 0; 
        int rc = ::mdb_env_create (&env);
        if (rc) throw MdbEx (std::string ("mdb_env_create: ") + ::strerror (rc));
        env_.reset (env, ::mdb_env_close);
        rc = ::mdb_env_set_mapsize (env, maxSizeInMb * 1024 * 1024);
        if (rc) throw MdbEx (std::string ("mdb_env_set_mapsize: ") + ::strerror (rc));
        envConf (env);
        rc = ::mdb_env_open (env, fullPath.c_str(), MDB_NOSYNC, 0664);
        open ();
    }

    ~Mdb()
    {
        close();
    }

    transaction_type beginTransaction (unsigned flags = 0) const
    {
        MDB_txn* txn = 0; int rc = ::mdb_txn_begin (env_.get(), NULL, flags, &txn);
        if (rc) throw MdbEx (std::string ("mdb_txn_begin: ") + ::strerror (rc));
        return transaction_type (txn, ::mdb_txn_abort);
    }

    void commitTransaction (transaction_type& txn) const
    {
        int rc = ::mdb_txn_commit (txn.get());
        txn.release(); // Must prevent `mdb_txn_abort` from happening (even if rc != 0).
        if (rc) throw MdbEx (std::string ("mdb_txn_commit: ") + ::strerror (rc));
    }

    /** Used before `mdb_env_open`. By default sets the number of database to 32. */
    void envConf (MDB_env* env) 
    {
        int rc = ::mdb_env_set_maxdbs (env, 32);
        if (rc) throw MdbEx (std::string ("envConf: ") + ::strerror (rc));
    }

    bool open(const std::string& file)
    {
        file_ = file;
        return open();
    }

    bool open()
    {
        close(); // close first if opened

        transaction_type txn = beginTransaction();
        unsigned flags = MDB_CREATE;
        flags |= MDB_DUPSORT;
        int rc = ::mdb_open (txn.get(), file_.c_str(), flags, &dbi_);
        if (rc) throw MdbEx (std::string ("mdb_open (") + file_ + "): " + ::strerror (rc));
        commitTransaction (txn);

        return true;
    }
    std::string getFileName() const
    {
        return file_;
    }
    void close()
    {
        if (dbi_) {::mdb_close (env_.get(), dbi_); dbi_ = 0;}
    }

    bool isOpened() const
    {
        return checkHandle_(dbi_);
    }
    /**
     * @deprecated
     */
    bool is_open() const
    {
        return checkHandle_(dbi_);
    }

    bool flush()
    {
        // reopen to flush cached data.
        return checkHandle_(dbi_);
    }

    size_type size() const
    {
        //Not supported now, use iterator
        size_type size = 0;
        if (checkHandle_(dbi_))
        {
            /*
            ::leveldb::ReadOptions options;
            options.fill_cache = false;
            ::leveldb::Iterator* it = dbi_->NewIterator(options);
            for (it->SeekToFirst(); it->Valid(); it->Next())
            {
                ++size;
            }
            delete it;
            */
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
        transaction_type txn (beginTransaction());
        MDB_val mkey = {key.size(), (void*) key.data()};
        MDB_val mvalue = {value.size(), (void*) value.data()};
        int rc = ::mdb_put (txn.get(), dbi_, &mkey, &mvalue, 0);
        if (rc) return false;
        commitTransaction (txn);
        return true;
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
        transaction_type txn (beginTransaction (MDB_RDONLY));;
    
        MDB_val mkey = {key.size(), (void*) key.data()};
        MDB_val mvalue;
        int rc = ::mdb_get (txn.get(), dbi_, &mkey, &mvalue);
        if (rc == MDB_NOTFOUND) return false;
        if (rc) return false;
        value.attach(static_cast<char*>(mvalue.mv_data),
                    static_cast<std::size_t>(mvalue.mv_size));
        commitTransaction (txn);
        return true;
    }

    bool del(const Buffer& key)
    {
        transaction_type txn (beginTransaction());
        MDB_val mkey = {key.size(), (void*) key.data()};
        int rc = ::mdb_del (txn.get(), dbi_, &mkey, NULL);
        if (rc == MDB_NOTFOUND) return false;
        if (rc) return false;
        commitTransaction (txn);
        return true;
    }

    cursor_type begin() const
    {
        if (isOpened())
        {
              transaction_type txn (beginTransaction());
              MDB_cursor* cur = NULL; 
              int rc = ::mdb_cursor_open (txn.get(), dbi_, &cur);
              if (rc) throw MdbEx ("mdb_cursor_open");
              int position = ::MDB_FIRST;
              MDB_val _key = {0, 0}, _val = {0, 0};			  
              rc = ::mdb_cursor_get (cur, &_key, &_val, (MDB_cursor_op) position);
              if (rc) throw MdbEx ("mdb_cursor_get");
              cursor_type cursor(cur, ::mdb_cursor_close);
              return cursor;
        }
        return cursor_type();
    }

    cursor_type begin(Buffer& key) const
    {
        if (isOpened())
        {
              transaction_type txn (beginTransaction());
              MDB_cursor* cur = NULL; 
              int rc = ::mdb_cursor_open (txn.get(), dbi_, &cur);
              if (rc) throw MdbEx ("mdb_cursor_open");
              int position = ::MDB_FIRST;
              MDB_val _key = {key.size(), (void*) key.data()};
              MDB_val _val = {0, 0};			  
              rc = ::mdb_cursor_get (cur, &_key, &_val, (MDB_cursor_op) position);
              if (rc) throw MdbEx ("mdb_cursor_get");
              cursor_type cursor(cur, ::mdb_cursor_close);
              return cursor;
        }
        return cursor_type();
    }

    cursor_type rbegin() const
    {
        if (isOpened())
        {
              transaction_type txn (beginTransaction());
              MDB_cursor* cur = NULL; 
              int rc = ::mdb_cursor_open (txn.get(), dbi_, &cur);
              if (rc) throw MdbEx ("mdb_cursor_open");
              int position = ::MDB_LAST;
              MDB_val _key = {0, 0}, _val = {0, 0};			  
              rc = ::mdb_cursor_get (cur, &_key, &_val, (MDB_cursor_op) position);
              if (rc) throw MdbEx ("mdb_cursor_get");
              cursor_type cursor(cur, ::mdb_cursor_close);
              return cursor;
        }
        return cursor_type();
    }

    bool fetch(cursor_type& cursor, Buffer& key, Buffer& value)
    {
        if (isOpened() && cursor.get())
        {
            MDB_val mkey;
            MDB_val mvalue;
            int rc = ::mdb_cursor_get (cursor.get(), &mkey, &mvalue, ::MDB_GET_CURRENT); 
            if (rc == MDB_NOTFOUND) return false;
            key.attach(static_cast<char*>(mkey.mv_data),
                    static_cast<std::size_t>(mkey.mv_size));
            value.attach(static_cast<char*>(mvalue.mv_data),
                    static_cast<std::size_t>(mvalue.mv_size));
            return true;
        }
        return false;
    }

    bool iterNext(cursor_type& cursor)
    {
        if (isOpened() && cursor.get())
        {
            MDB_val mkey;
            MDB_val mvalue;
            int rc = ::mdb_cursor_get (cursor.get(), &mkey, &mvalue, ::MDB_NEXT); 
            if(rc)
            {
                cursor.reset();
                return false;
            }
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
        if (isOpened() && cursor.get())
        {
            MDB_val mkey;
            MDB_val mvalue;
            int rc = ::mdb_cursor_get (cursor.get(), &mkey, &mvalue, ::MDB_PREV); 
            if(rc)
            {
                cursor.reset();
                return false;
            }
            return true;
        }
        else
        {
            cursor.reset();
            return false;
        }
    }

private:
    static bool checkHandle_(MDB_dbi h)
    {
        return h;
    }

    boost::shared_ptr<MDB_env> env_;

    MDB_dbi dbi_;

    std::string file_;
};

}}}} // namespace izenelib::am::lmdb::raw

#endif // AM_LEVELDB_RAW_TABLE_H
