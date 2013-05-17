#ifndef AM_TOKUKV_RAW_TABLE_H
#define  AM_TOKUKV_RAW_TABLE_H

#include <stdint.h>
#include <config.h>
#include <inttypes.h>
#ifdef BDB
#include <sys/types.h>
#include <db.h>
#define DIRSUF bdb
#else
#include <tokudb.h>
#define DIRSUF tokudb
#endif
#include <glog/logging.h>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <list>
#include <am/concept/DataType.h>
#include <am/raw/Buffer.h>
#include <glog/logging.h>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <pthread.h>
#include <list>
using namespace izenelib::am;
using izenelib::am::raw::Buffer;

namespace izenelib
{
namespace am
{
namespace tokukv
{
namespace raw
{

template<typename Comp>
class Table
    : boost::noncopyable
{

public:
    typedef Buffer key_type;
    typedef Buffer value_type;
    typedef DataType<Buffer, Buffer> data_type;
    typedef int size_type;
    typedef DBC* cursor_type;
    typedef DB*  raw_type_ptr;

    explicit Table(const std::string& file = "")
        : comp_(),db_(NULL),parenttid_(NULL),tid_(NULL),file_(file)
    {
    }

    ~Table()
    {
        close();
    }

    bool open(const std::string& file)
    {
        file_ = file;
        dbenv_->set_default_bt_compare(dbenv_,&comp_.Compare);
        return open();
    }

    bool open ()
    {
        parenttid_=0;
        tid_=0;
        dbenv_=NULL;
        string sdbdir=(file_+"/bench.tokudb");
        string sdbfilename=(file_+"/bench.db");
        const char* dbdir=sdbdir.c_str();
        const char *dbfilename =sdbfilename.c_str();
        if (!boost::filesystem::is_directory(file_))
        {
            boost::filesystem::create_directories(file_);
        }
        int r;
        bool do_append=false;
        string strdbdir(dbdir);
        if (boost::filesystem::is_directory(dbdir))
        {
            do_append=true;
        }
        if (!do_append)
        {
            char unlink_cmd[strlen(dbdir) + strlen("rm -rf ") + 1];
            snprintf(unlink_cmd, sizeof(unlink_cmd), "rm -rf %s", dbdir);
            int r=system(unlink_cmd);

            if (strcmp(dbdir, ".") != 0)
            {
                r = mkdir(dbdir, 0644);
                cout<< "mkdir"<<r<<endl;
                assert(r == 0);
            }
        }
        bool do_txns=true;
        r = db_env_create(&dbenv_, 0);
        if (dbenv_->set_cachesize)
        {
            r = dbenv_->set_cachesize(dbenv_, 1, 0, 1);
            if (r != 0)
                printf("WARNING: set_cachesize %d\n", r);
        }
        r = dbenv_->open(dbenv_, dbdir, DB_CREATE|DB_PRIVATE|DB_INIT_MPOOL|DB_INIT_TXN , 0644);
        r = db_create(&db_, dbenv_, 0);
        assert(r==0);
        if (do_txns)
        {
            r = dbenv_->txn_begin(dbenv_, 0, &tid_, 0);
        }
        r = db_->open(db_, tid_, dbfilename, NULL, DB_BTREE, DB_CREATE, 0);
        r = db_->pre_acquire_table_lock(db_, tid_);
        assert(r==0);
        return true;//r==0;
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
            db_->close(db_, 0);
            db_ = NULL;
        }
    }

    void close(cursor_type cursor)
    {
        cursor->c_close(cursor);
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
        //TODO
        size_type size = 0;
        if (checkHandle_(db_))
        {
            for (cursor_type cursor=begin(); cursor; iterNext(cursor))
            {
                ++size;
            }
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
     * clear method will destroy the tokukv instance,
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
        return checkHandle_(db_) && (
                   db_->put(db_, tid_, fill_dbt_(key.data(),key.size()), fill_dbt_(value.data(),value.size()), 0)==0);
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
            DBT val;
            memset(&val, 0, sizeof val);
            int r = db_->get(db_, tid_, fill_dbt_(key.data(),key.size()), &val, 0);
            if(r==0)
            {
                value.attach(const_cast<char*>((char*)val.data),
                             static_cast<std::size_t>(val.size));
                return true;
            }
            else
            {
                return false;
            }
        }
        return false;
    }

    bool del(const Buffer& key)
    {
        return checkHandle_(db_) &&
               db_->del(db_, tid_, fill_dbt_(key.data(),key.size()), 0)==0;
    }

    cursor_type begin() const
    {
        if (isOpened())
        {
            cursor_type cursor;
            int r =db_->cursor(db_,tid_, &cursor, 0);
            IASSERT(r==0);			
            DBT k,v;
            memset(&k, 0, sizeof(k));
            memset(&v, 0, sizeof(v));
            r = cursor->c_get(cursor, &k, &v,  DB_FIRST);
            return cursor;
        }
        return cursor_type();
    }

    cursor_type reset(cursor_type& cursor) const
    {

        cursor=begin();
        return cursor;
    }

    cursor_type begin(Buffer& key) const
    {

        if (isOpened())
        {
            cursor_type cursor;

            int r = db_->cursor(db_, tid_, &cursor, 0);
            IASSERT(r==0);
            DBT k,v;//TODO
            memset(&k, 0, sizeof(k));
            memset(&v, 0, sizeof(v));
            cursor->c_get(cursor, fill_dbt_(key.data(),key.size()), &v, 0);
            return cursor;
        }
        return cursor_type();
    }

    cursor_type rbegin() const
    {
        if (isOpened())
        {
            cursor_type cursor;
            int r = db_->cursor(db_, tid_, &cursor, 0);
            DBT k,v;
            memset(&k, 0, sizeof(k));
            memset(&v, 0, sizeof(v));
            r = cursor->c_get(cursor, &k, &v, DB_LAST);
            IASSERT(r==0);
            return cursor;
        }
        return cursor_type();
    }

    bool fetch(cursor_type& cursor, Buffer& key, Buffer& value)
    {
        if (isOpened()&&cursor)
        {
            DBT k,v;
            memset(&k, 0, sizeof(k));
            memset(&v, 0, sizeof(v));
            cursor->c_get(cursor, &k, &v, DB_CURRENT);
            key.attach(const_cast<char*>((char*)k.data),
                       static_cast<std::size_t>(k.size));
            value.attach(const_cast<char*>((char*)v.data),
                         static_cast<std::size_t>(v.size));
            return true;
        }
        return false;
    }

    bool iterNext(cursor_type& cursor)
    {
        if (isOpened())
        {
            DBT k,v;
            memset(&k, 0, sizeof(k));
            memset(&v, 0, sizeof(v));
            int r = cursor->c_get(cursor, &k, &v, DB_NEXT);
            if(r==DB_NOTFOUND)
            {
                cursor=NULL;
                return false;
            }
            else
                return true;
        }
        else
        {
            reset(cursor);
            return false;
        }
    }

    bool iterPrev(cursor_type& cursor)
    {
        if (isOpened())
        {
            DBT k,v;
            memset(&k, 0, sizeof(k));
            memset(&v, 0, sizeof(v));
            int r = cursor->c_get(cursor, &k, &v, DB_PREV);
            if(r==DB_NOTFOUND)
            {
                cursor=NULL;
                return false;
            }
            else
                return true;
        }
        else
        {
            reset(cursor);
            return false;
        }
    }



private:

    static DBT *fill_dbt_(const void *data, int size)
    {
        DBT *dbt=new DBT();
        dbt->size = size;
        dbt->data = (void *) data;
        return dbt;
    }

    static bool checkHandle_(DB* h)
    {
        return h;
    }
private:
    Comp comp_;

    DB_ENV *dbenv_;
    DB *db_;
    DB_TXN *parenttid_;
    DB_TXN *tid_;

    std::string file_;

};
}
}
}
}
// namespace izenelib::am::raw

#endif // AM_tokukv_RAW_TABLE_H
