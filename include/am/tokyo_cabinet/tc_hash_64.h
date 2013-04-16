/**
 * @file tc_hash.h
 * @brief wrapp tokyo cabinet's hash for SDB, CacheDB
 * @author peisheng wang
 *
 *
 * This file defines class tc_hash.
 */
#ifndef TC_HASH_64_H
#define TC_HASH_64_H

#include "tc_types.h"
#include <util/Exception.h>
#include <boost/optional.hpp>

NS_IZENELIB_AM_BEGIN

/**
 *  \brief wrap tokyo/cabinet for CacheDB and SDB.
 *
 *
 */

template< typename KeyType, typename ValueType, typename LockType =NullLock> class tc_hash :
            public AccessMethod<KeyType, ValueType, LockType>
{
public:
    typedef boost::optional<KeyType> SDBCursor;

public:
    /**
     *   constructor
     */
    tc_hash(const string& fileName = "tc_hash.dat"): fileName_(fileName + ".tch"),hdb_(NULL),isOpen_(false), cacheSize_(0)
    {
        initHandle_();
    }

    /**
     *   deconstructor, close() will also be called here.
     */
    virtual ~tc_hash()
    {
        if ( isOpen() )
        {
            try
            {
                close();
            }
            catch (std::exception& ex)
            {
                std::cout<<"error while close tc_hash : "<<fileName_<<std::endl;
            }
        }
        freeHandle_();
    }

    bool isOpen() const
    {
        return isOpen_;
    }

    /**
     *  set cache size, if not called use default size 100000
     */
    void setCacheSize(size_t cacheSize)
    {
        bool op = tchdbsetcache(hdb_, cacheSize);
        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_hash setCacheSize on "+fileName_+" : "+tchdberrmsg(errcode));
        }
        cacheSize_ = cacheSize;
    }

    void tune(int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts)
    {
        bool op = tchdbtune(hdb_, bnum, apow, fpow, opts);
        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_hash tune on "+fileName_+" : "+tchdberrmsg(errcode));
        }

    }

    void setXMSize(int64_t xmsize)
    {
        bool op = tchdbsetxmsiz(hdb_, xmsize);
        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_hash setXMSize on "+fileName_+" : "+tchdberrmsg(errcode));
        }
    }

    /**
     * 	\brief return the file name of the SequentialDB
     */
    std::string getFileName() const
    {
        return fileName_;
    }

    /**
     *  insert an item of DataType
     */
    bool insert(const DataType<KeyType,ValueType> & dat)
    {
        if ( !isOpen() ) return false;
        return insert(dat.get_key(), dat.get_value() );
    }

    /**
     *  insert an item in key/value pair
     */
    bool insert(const KeyType& key, const ValueType& value)
    {
        if ( !isOpen() ) return false;
        char* ptr;
        char* ptr1;
        size_t ksize;
        size_t vsize;
        izene_serialization<KeyType> izs(key);
        izene_serialization<ValueType> izs1(value);
        izs.write_image(ptr, ksize);
        izs1.write_image(ptr1, vsize);

        bool op = tchdbputkeep(hdb_, ptr, ksize, ptr1, vsize);

        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_hash insert on "+fileName_+" : "+tchdberrmsg(errcode));
        }
        return op;
    }

    bool insertValue(const KeyType& key, const ValueType& value)
    {
        if ( !isOpen() ) return false;
        return insert(key, value);
    }

    /**
     *  find an item, return pointer to the value.
     *  Note that, there will be memory leak if not delete the value
     */
    ValueType* find(const KeyType & key)
    {
        if ( !isOpen() ) return NULL;
        char* ptr;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        int sp;
        void* value = tchdbget(hdb_, ptr, ksize, &sp);

        if ( value == NULL )
        {
        }
        if ( value == NULL )return NULL;
        else
        {
            ValueType *val = new ValueType;
            izene_deserialization<ValueType> izd((char*)value, (size_t)sp);
            izd.read_image(*val);
            free(value);
            return val;
        }
    }

    bool get(const KeyType& key, ValueType& value)
    {
        if ( !isOpen() ) return false;
        char* ptr;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        int sp;
        void* pv = tchdbget(hdb_, ptr, ksize, &sp);
        if ( pv == NULL )
        {
        }
        if ( !pv )return false;
        else
        {
            izene_deserialization<ValueType> izd((char*)pv, (size_t)sp);
            izd.read_image(value);
            free(pv);
            return true;
        }
    }

    char* get(const KeyType& key, int& sp)
    {
        if ( !isOpen() ) return false;
        char* ptr;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);
        void* pv = tchdbget(hdb_, ptr, ksize, &sp);
        if ( pv == NULL )
        {
            return NULL;
        }
        else
        {
            return(char*)pv;
        }
    }

    bool getValue(const KeyType& key, ValueType& value)
    {
        if ( !isOpen() ) return false;
        return get(key, value);
    }

    /**
     *  delete  an item
     */
    bool del(const KeyType& key)
    {
        if ( !isOpen() ) return false;
        char* ptr;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        //ptr->display();
        bool op = tchdbout(hdb_, ptr, ksize);

        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_hash del on "+fileName_+" : "+tchdberrmsg(errcode));
        }
        return op;
    }

    /**
     *  update  an item through DataType data
     */
    bool update(const DataType<KeyType,ValueType> & dat)
    {
        if ( !isOpen() ) return false;
        return update( dat.get_key(), dat.get_value() );
    }

    /**
     *  update  an item by key/value pair
     */
    bool update(const KeyType& key, const ValueType& value)
    {
        if ( !isOpen() ) return false;
        char* ptr;
        char* ptr1;
        size_t ksize;
        size_t vsize;
        izene_serialization<KeyType> izs(key);
        izene_serialization<ValueType> izs1(value);
        izs.write_image(ptr, ksize);
        izs1.write_image(ptr1, vsize);

        bool op = tchdbput(hdb_, ptr, ksize, ptr1, vsize);

        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_hash update on "+fileName_+" : "+tchdberrmsg(errcode));
        }
        return op;

    }

    bool update(const KeyType& key, char* value, std::size_t vsize)
    {
        if ( !isOpen() ) return false;
        char* ptr;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        bool op = tchdbput(hdb_, ptr, ksize, value, vsize);

        if ( !op )
        {
            int errcode = ecode();
            IZENELIB_THROW("tc_fixdb update on "+fileName_+" : "+tcfdberrmsg(errcode));
        }
        return op;

    }

    bool search(const KeyType&key, SDBCursor& locn)
    {
        if ( !isOpen() )
            return false;

        if ( hasKey_(key) )
        {
            locn = key;
            return true;
        }

        return false;
    }

    /**
     *  get the SDBCursor of first item in the first not empty bucket.
     */

    SDBCursor get_first_locn()
    {
        return SDBCursor();
    }

    bool get(const SDBCursor& locn, KeyType& key, ValueType& value)
    {
        if ( !isOpen() ) return false;
        char* ptr = 0;
        size_t ksize = 0;
        if (locn)
        {
            izene_serialization<KeyType> izs(locn.get());
            izs.write_image(ptr, ksize);
        }

        const char* value_buff = 0;
        int value_size = 0;
        int ret_key_size = 0;
        char* ret_key_buff = tchdbgetnext3(
                                 hdb_, // handler
                                 ptr, ksize, // key
                                 &ret_key_size, // returned key size
                                 &value_buff, &value_size // returned value
                             );

        if ( ret_key_buff == NULL )
        {
        }
        if (ret_key_buff)
        {
            izene_deserialization<KeyType> izd_key(
                ret_key_buff,
                static_cast<size_t>(ret_key_size)
            );
            izd_key.read_image(key);

            izene_deserialization<ValueType> izd_value(
                value_buff,
                static_cast<size_t>(value_size)
            );
            izd_value.read_image(value);

            tcfree(ret_key_buff);
            return true;
        }

        return false;
    }

    /**
     *  get an item from given SDBCursor
     */
    bool get(const SDBCursor& locn, DataType<KeyType,ValueType> & rec)
    {
        if ( !isOpen() ) return false;
        return get(locn, rec.get_key(), rec.get_value() );
    }

    /**
     *   \brief sequential access method
     *
     *   @param locn is the current SDBCursor, and will replaced next SDBCursor when route finished.
     *   @param rec is the item in SDBCursor locn.
     *   @param sdir is sequential access direction, for hash is unordered, we only implement forward case.
     *
     */
    bool seq(SDBCursor& locn, KeyType& key, ValueType& value, ESeqDirection sdir=ESD_FORWARD)
    {
        if ( !isOpen() ) return false;
        bool ret = seq(locn);
        get(locn, key, value);
        return ret;
    }
    bool seq(SDBCursor& locn, DataType<KeyType, ValueType>& dat, ESeqDirection sdir=ESD_FORWARD)
    {
        if ( !isOpen() ) return false;
        return seq(locn, dat.key, dat.value, sdir);
    }


    bool seq(SDBCursor& locn, ESeqDirection sdir = ESD_FORWARD)
    {
        if ( !isOpen() ) return false;
        if ( sdir == ESD_FORWARD)
        {
            DataType<KeyType,ValueType> rec;
            if (get(locn, rec))
            {
                locn = rec.get_key();
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            assert( false);
            return false;
        }
    }

    bool iterInit()
    {
        if ( !isOpen() ) return false;
        bool op = tchdbiterinit(hdb_);
        if ( !op )
        {
            int errcode = ecode();
            //if ( errcode != TCESUCCESS )
            {
                IZENELIB_THROW("tc_hash iterInit on "+fileName_+" : "+tchdberrmsg(errcode));
            }
        }
        return op;
    }

    bool iterNext(KeyType& key, ValueType& value)
    {
        if ( !isOpen() ) return false;
        TCXSTR* ptcKey = tcxstrnew();
        TCXSTR* ptcValue = tcxstrnew();

        bool b = tchdbiternext3(hdb_, ptcKey, ptcValue);
        if ( !b )
        {
            int errcode = ecode();
            //if ( errcode != TCESUCCESS )
            {
                IZENELIB_THROW("tc_hash iterNext on "+fileName_+" : "+tchdberrmsg(errcode));
            }
        }
        if (!b) return false;
        char* cpKey = (char*)tcxstrptr(ptcKey);
        char* cpValue = (char*)tcxstrptr(ptcValue);

        izene_deserialization<KeyType> izdk(cpKey, tcxstrsize(ptcKey));
        izdk.read_image(key);
        izene_deserialization<ValueType> izdv(cpValue, tcxstrsize(ptcValue));
        izdv.read_image(value);
        tcxstrdel(ptcKey);
        tcxstrdel(ptcValue);

        return true;
    }

    /**
     *   get the num of items
     */
    uint64_t num_items() const
    {
        if ( !isOpen() ) return 0;
        uint64_t r = tchdbrnum(hdb_);
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
        bool ret = tchdbopen(hdb_, fileName_.c_str(), HDBOCREAT | HDBOWRITER);
        if ( !ret )
        {
            int errcode = ecode();
            //if ( errcode != TCESUCCESS )
            {
                IZENELIB_THROW("tc_hash open on "+fileName_+" : "+tchdberrmsg(errcode));
            }
        }
        isOpen_ = true;
        return ret;
    }
    /**
     *   db should be closed after open, and  it will automatically called in deconstuctor.
     */
    bool close()
    {
        if ( !isOpen() ) return false;
        commit();
        bool ret = tchdbclose(hdb_);
        if ( !ret )
        {
            int errcode = ecode();
            //if ( errcode != TCESUCCESS )
            {
                IZENELIB_THROW("tc_hash close on "+fileName_+" : "+tchdberrmsg(errcode));
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
        bool ret = tchdbsync(hdb_);
        if ( !ret )
        {
            int errcode = ecode();
            //if ( errcode != TCESUCCESS )
            {
                IZENELIB_THROW("tc_hash commit on "+fileName_+" : "+tchdberrmsg(errcode));
            }
        }
    }
    /**
    *   Write the dirty buckets to disk.
    */
    void flush()
    {
        if ( !isOpen() ) return;
        try{
            close();
            open();
        }catch(const std::exception& e){
            std::cerr << "flush tc hash error.";
        }
        //commit();
    }

    /**
    *   Write the dirty buckets to disk and also free up most of the memory.
    */
    void release()
    {
        close();
        freeHandle_();
        initHandle_();
        setCacheSize(cacheSize_);
        open();
    }

    int ecode()
    {
        return tchdbecode(hdb_);
    }

    /**
     *  display the info of tc_hash
     */
    void display(std::ostream& os = std::cout)
    {

    }
    /**
     *  We can directly process hdb_ by this handle.
     */
    TCHDB* getHandle()
    {
        return hdb_;
    }

private:
    void initHandle_()
    {
        if ( hdb_ == NULL )
        {
            hdb_ = tchdbnew();
            if ( hdb_ == NULL )
            {
                int errcode = ecode();
                //if ( errcode != TCESUCCESS )
                {
                    IZENELIB_THROW("tc_hash new on "+fileName_+" : "+tchdberrmsg(errcode));
                }
            }
        }
    }

    void freeHandle_()
    {
        if ( hdb_ != NULL )
        {
            tchdbdel(hdb_);
            hdb_ = NULL;
        }
    }

    bool hasKey_(const KeyType & key)
    {
        char* ptr;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        int sp;
        void* value = tchdbget(hdb_, ptr, ksize, &sp);
        if ( value )
        {
            free(value);
            return true;
        }

        return false;
    }

private:
    string fileName_;

    TCHDB* hdb_;
    bool isOpen_;
    size_t cacheSize_;
};

NS_IZENELIB_AM_END

#endif
