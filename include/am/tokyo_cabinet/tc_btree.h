/**
 * @file tc_btree.h
 * @brief wrapp tokyo cabinet's hash for SDB, CacheDB
 * @author peisheng wang
 *
 *
 * This file defines class tc_btree.
 */
#ifndef TC_BTREE_H
#define TC_BTREE_H

#include "tc_types.h"

NS_IZENELIB_AM_BEGIN

/**
 *  \brief wrap tokyo/cabinet for CacheDB and SDB.
 *
 *
 */
template<typename KeyType>
int compare_tc(const char* aptr, int asize, const char* bptr, int bsize, void* op=0)
{

    izenelib::am::CompareFunctor<KeyType> _comp;

    /*DbObjPtr ptr, ptr1;
     KeyType key, key1;
     ptr.reset(new DbObj(aptr,asize));
     ptr1.reset(new DbObj(bptr, bsize));
     read_image(key, ptr);
     read_image(key1, ptr1);*/

    //char *ptr, *ptr1;
    KeyType key, key1;
    izene_deserialization<KeyType> izd(aptr, asize);
    izene_deserialization<KeyType> izd1(bptr, bsize);
    izd.read_image(key);
    izd.read_image(key1);

    return _comp(key, key1);
}

template< typename KeyType, typename ValueType, typename LockType =NullLock> class tc_btree :
            public AccessMethod<KeyType, ValueType, LockType>
{
public:
    //typedef std::pair<bucket_chain*, char*> SDBCursor;
    //typedef DataType<KeyType,ValueType> DataType;

public:
    /**
     *   constructor
     */
    tc_btree(const string& fileName = "tc_btree.dat") :
            fileName_(fileName)
    {

    }

    /**
     *   deconstructor, close() will also be called here.
     */
    virtual ~tc_btree()
    {
        close();
        tcbdbdel(bdb_);
    }

    /**
     *  set cache size, if not called use default size 100000
     */
    void setCacheSize(size_t cacheSize)
    {
        tcbdbsetcache(bdb_, cacheSize/2, cacheSize/2);
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
        return insert(dat.get_key(), dat.get_value() );
    }

    /**
     *  insert an item in key/value pair
     */
    bool insert(const KeyType& key, const ValueType& value)
    {

        char* ptr;
        char* ptr1;
        size_t ksize;
        size_t vsize;
        izene_serialization<KeyType> izs(key);
        izene_serialization<ValueType> izs1(value);
        izs.write_image(ptr, ksize);
        izs1.write_image(ptr1, vsize);

        return tcbdbputkeep(bdb_, ptr, ksize, ptr1, vsize);
    }

    /**
     *  find an item, return pointer to the value.
     *  Note that, there will be memory leak if not delete the value
     */
    ValueType* find(const KeyType & key)
    {
        char* ptr;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        int sp;
        void* value = tcbdbget(bdb_, ptr, ksize, &sp);
        if ( !value)
            return NULL;
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

        char* ptr;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        int sp;
        void* pv = tcbdbget(bdb_, ptr, ksize, &sp);
        if ( !pv)
            return false;
        else
        {
            izene_deserialization<ValueType> izd((char*)pv, (size_t)sp);
            izd.read_image(value);
            free(pv);
            return true;
        }
    }

    /**
     *  delete  an item
     */
    bool del(const KeyType& key)
    {
        char* ptr;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        //ptr->display();
        return tcbdbout(bdb_, ptr, ksize);
    }

    /**
     *  update  an item through DataType data
     */
    bool update(const DataType<KeyType,ValueType> & dat)
    {
        return update(dat.get_key(), dat.get_value() );
    }

    /**
     *  update  an item by key/value pair
     */
    bool update(const KeyType& key, const ValueType& value)
    {
        char* ptr;
        char* ptr1;
        size_t ksize;
        size_t vsize;
        izene_serialization<KeyType> izs(key);
        izene_serialization<ValueType> izs1(value);
        izs.write_image(ptr, ksize);
        izs1.write_image(ptr1, vsize);

        return tcbdbput(bdb_, ptr, ksize, ptr1, vsize);

    }

    bool iterInit()
    {
        return false;
    }

    bool iterNext(KeyType& key, ValueType& value)
    {
        return false;
    }

    /**
     *  search an item
     *
     *   @return SDBCursor
     */
    //SDBCursor search(const KeyType& key)
    //{
    ////	SDBCursor locn;
    //	search(key, locn);
    //	return locn;
    //}

    /**
     *    another search function, flushCache_() will be called at the beginning,
     *
     */

    //bool search(const KeyType&key, SDBCursor& locn)
    //{
    //	return false;
    //}

    /**
     *  get the SDBCursor of first item in the first not empty bucket.
     */

    //SDBCursor get_first_locn()
    //{
    //
    //}

    //bool get(const SDBCursor& locn, KeyType& key, ValueType& value)
    //{
    //	return false;
    //}
    /**
     *  get an item from given SDBCursor
     */
    //bool get(const SDBCursor& locn, DataType& rec) {
    //
    //	return true;

    //}

    /**
     *   \brief sequential access method
     *
     *   @param locn is the current SDBCursor, and will replaced next SDBCursor when route finished.
     *   @param rec is the item in SDBCursor locn.
     *   @param sdir is sequential access direction, for hash is unordered, we only implement forward case.
     *
     */
    //bool seq(SDBCursor& locn, DataType& rec, ESeqDirection sdir=ESD_FORWARD) {
    //	return false;

    //}

    /**
     *   get the num of items
     */
    int num_items()
    {
        return tcbdbrnum(bdb_);
    }

public:
    /**
     *   db must be opened to be used.
     */
    bool open(bool setCmpFun = false)
    {
        bdb_ = tcbdbnew();
        if (setCmpFun)
        {
            tcbdbsetcmpfunc(bdb_, compare_tc<KeyType>, NULL);
        }
        return tcbdbopen(bdb_, fileName_.c_str(), BDBOCREAT | BDBOWRITER);
    }
    /**
     *   db should be closed after open, and  it will automatically called in deconstuctor.
     */
    bool close()
    {
        commit();
        return tcbdbclose(bdb_);
    }
    /**
     *  write the dirty buckets to disk, not release the memory
     *
     */
    void commit()
    {
        tcbdbsync(bdb_);
    }
    /**
     *   Write the dirty buckets to disk and also free up most of the memory.
     *   Note that, for efficieny, entry_[] is not freed up.
     */
    void flush()
    {
        commit();
    }
    /**
     *  display the info of tc_btree
     */
    void display(std::ostream& os = std::cout)
    {

    }

    TCBDB* getHandle()
    {
        return bdb_;
    }

private:
    string fileName_;
    TCBDB* bdb_;

};

NS_IZENELIB_AM_END

#endif
