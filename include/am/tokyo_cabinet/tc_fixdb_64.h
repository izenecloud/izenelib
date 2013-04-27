/**
 * @file tc_fixdb.h
 * @brief wrapp tokyo cabinet's fixed db
 * @author Jia Guo
 *
 *
 * This file defines class tc_fixdb.
 */
#ifndef TC_FIXDB_64_H
#define TC_FIXDB_64_H

#include "tc_types.h"
#include <util/Exception.h>
#include <boost/optional.hpp>

NS_IZENELIB_AM_BEGIN

/**
 *  \brief wrap tokyo/cabinet for CacheDB and SDB.
 *
 *
 */

template<typename ValueType, typename LockType =NullLock>
class tc_fixdb
{

public:
    /**
     *   constructor
     */
    tc_fixdb(const string& fileName): fileName_(fileName),fdb_(NULL),isOpen_(false)
    {
        initHandle_();
        tune();
    }

    /**
     *   deconstructor, close() will also be called here.
     */
    virtual ~tc_fixdb()
    {
        if ( isOpen() )
        {
            try
            {
                close();
            }
            catch (std::exception& ex)
            {
                std::cout<<"error while close tc_fixdb : "<<fileName_<<std::endl;
            }
        }
        freeHandle_();
    }

    bool isOpen() const
    {
        return isOpen_;
    }


    void tune(int32_t width = sizeof(ValueType), int64_t limsiz = std::numeric_limits< uint32_t >::max())
    {
        bool op = tcfdbtune(fdb_, width, limsiz);
        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_fixdb tune on "+fileName_+" : "+tcfdberrmsg(errcode));
        }

    }


    /**
     *  \brief return the file name of the SequentialDB
     */
    std::string getFileName() const
    {
        return fileName_;
    }

    /**
     *  insert an item in key/value pair
     */
    bool insert(int64_t key, const ValueType& value)
    {
        if ( !isOpen() ) return false;

        char* ptr1;
        size_t vsize;
        izene_serialization<ValueType> izs1(value);
        izs1.write_image(ptr1, vsize);

        bool op = tcfdbputkeep(fdb_, key, ptr1, vsize);

        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_fixdb insert on "+fileName_+" : "+tcfdberrmsg(errcode));
        }
        return op;
    }

    bool insert(int64_t key, char* value, std::size_t vsize)
    {
        if ( !isOpen() ) return false;

        bool op = tcfdbputkeep(fdb_, key, value, vsize);

        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_fixdb insert on "+fileName_+" : "+tcfdberrmsg(errcode));
        }
        return op;
    }

    bool get(int64_t key, ValueType& value)
    {
        if ( !isOpen() ) return false;

        int sp;
        void* pv = tcfdbget(fdb_, key, &sp);
        if ( pv == NULL )
        {
            return false;
        }
        else
        {
            izene_deserialization<ValueType> izd((char*)pv, (size_t)sp);
            izd.read_image(value);
            free(pv);
            return true;
        }
    }

    char* get(int64_t key, int& sp)
    {
        if ( !isOpen() ) return NULL;

        void* pv = tcfdbget(fdb_, key, &sp);
        if ( pv == NULL )
        {
            return NULL;
        }
        else
        {
            return (char*)pv;

        }
    }


    /**
     *  update  an item by key/value pair
     */
    bool update(int64_t key, const ValueType& value)
    {
        if ( !isOpen() ) return false;
        char* ptr1;
        size_t vsize;
        izene_serialization<ValueType> izs1(value);
        izs1.write_image(ptr1, vsize);

        bool op = tcfdbput(fdb_, key, ptr1, vsize);

        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_fixdb update on "+fileName_+" : "+tcfdberrmsg(errcode));
        }
        return op;

    }

    bool update(int64_t key, char* value, std::size_t vsize)
    {
        if ( !isOpen() ) return false;

        bool op = tcfdbput(fdb_, key, value, vsize);

        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_fixdb update on "+fileName_+" : "+tcfdberrmsg(errcode));
        }
        return op;

    }


    /**
     *  get the num of items
     */
    uint64_t num_items() const
    {
        if ( !isOpen() ) return 0;
        uint64_t r = tcfdbrnum(fdb_);
        return r;
    }

    uint64_t numItems() const
    {
        return num_items();
    }

public:
    /**
     *   db must be opened to be used.
     */
    bool open()
    {
        if ( isOpen() ) return false;
        bool ret = tcfdbopen(fdb_, fileName_.c_str(), FDBOCREAT | FDBOWRITER);
        if ( !ret )
        {
            int errcode = ecode();
            //if ( errcode != TCESUCCESS )
            {
                IZENELIB_THROW("tc_fixdb open on "+fileName_+" : "+tcfdberrmsg(errcode));
            }
        }
        isOpen_ = true;
        return ret;
    }
    /**
     *  db should be closed after open, and  it will automatically called in deconstuctor.
     */
    bool close()
    {
        if ( !isOpen() ) return false;
        commit();
        bool ret = tcfdbclose(fdb_);
        if ( !ret )
        {
            int errcode = ecode();
            //if ( errcode != TCESUCCESS )
            {
                IZENELIB_THROW("tc_fixdb close on "+fileName_+" : "+tcfdberrmsg(errcode));
            }
        }
        isOpen_ = false;
        return ret;
    }

    /**
     *  write the dirty buckets to disk, not release the memory
     *
     */
    void commit()
    {
        if ( !isOpen() ) return;
        bool ret = tcfdbsync(fdb_);
        if ( !ret )
        {
            int errcode = ecode();
            //if ( errcode != TCESUCCESS )
            {
                IZENELIB_THROW("tc_fixdb commit on "+fileName_+" : "+tcfdberrmsg(errcode));
            }
        }
    }
    /**
     *  Write the dirty buckets to disk.
     */
    void flush()
    {
        commit();
    }

    int ecode()
    {
        return tcfdbecode(fdb_);
    }

    /**
     *  Remove all records of a fixed-length database
     */
    bool clear()
    {
        if ( !isOpen() ) return false;
        bool ret = tcfdbvanish(fdb_);
        if ( !ret )
        {
            int errcode = ecode();
            //if ( errcode != TCESUCCESS )
            {
                IZENELIB_THROW("tc_fixdb clear on "+fileName_+" : "+tcfdberrmsg(errcode));
            }
        }
        return ret;
    }
    /**
     *  We can directly process fdb_ by this handle.
     */
    TCFDB* getHandle()
    {
        return fdb_;
    }

private:
    void initHandle_()
    {
        if ( fdb_ == NULL )
        {
            fdb_ = tcfdbnew();
            if ( fdb_ == NULL )
            {
                int errcode = ecode();
                //if ( errcode != TCESUCCESS )
                {
                    IZENELIB_THROW("tc_fixdb new on "+fileName_+" : "+tcfdberrmsg(errcode));
                }
            }
        }
    }

    void freeHandle_()
    {
        if ( fdb_ != NULL )
        {
            tcfdbdel(fdb_);
            fdb_ = NULL;
        }
    }

private:
    string fileName_;

    TCFDB* fdb_;
    bool isOpen_;

};

NS_IZENELIB_AM_END

#endif
