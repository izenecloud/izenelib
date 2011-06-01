/**
 * @file MCache.h
 * @brief The header file of MCache.
 *
 * This file defines class MCache.
 */

#ifndef MCACHE_H
#define MCACHE_H

#include "cm_basics.h"
#include "CacheHash.h"
#include "CacheInfo.h"
#include "CacheContainer.h"

#include <ctime>
#include <list>
#include <map>
#include <fstream>
#include <ext/hash_map>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

using namespace std;
using namespace __gnu_cxx;
using namespace boost;

namespace izenelib
{
namespace cache
{
/**
 * 	\brief A basics Cache, only resides in memory.
 *
 *	It can deal with different type of key-data pair using different Replacement
 *	Policies and using different storage policies through template template parameters.
 *
 *  KeyType and DataType    :   the type of what we want to cache.
 *	RelacementPolciy        :   it can be lruCmp, lfuCmp or slruCmp, which are defined in CacheInfo.h.
 *	Hash                    :   Canadiates are LinearHashTable,ExtendibleHash, or others.
 *	LockType          :   it can be NullLock or ReadWriteLock, which are defined in ylib/lock.h. If using NullLock, then
 *				                No threa dsafe.
 */

//static ofstream loggerFile1("./mcache_logger");
template <class KeyType, class ValueType, class ReplacementPolicy,
class Hash = izenelib::am::rde_hash<KeyType, ValueType>,
class LockType =NullLock> class MCache
{
    typedef typename map<CacheInfo<KeyType>, bool, ReplacementPolicy> :: iterator
    MIT;
    typedef typename hash_map<KeyType, CacheInfo<KeyType>, HashFun<KeyType> > ::iterator
    HIT;
    LockType lock;
    //typedef izenelib::am::DataType<KeyType,ValueType> DataType;
public:
    /**
     *  \brief Constuctor1: default fileName for fileHash of Hash_ is "./index.dat".
     */
    MCache(unsigned int cacheSize) :
            hash_(cacheSize), startingTime_(time(0)), nTotal_(0), nHit_(0),
            hitRatio_(0.0), workload_(0.0)
    {
        cacheSize_ = cacheSize;
        isArchive_ = 0;
    } //CacheExtHash with ratio = 1.0.

    unsigned int getCacheSize()
    {
        return cacheSize_;
    }
    void setCacheSize(unsigned int cacheSize)
    {
        cacheSize_ = cacheSize;
        hash_.setHashSize(cacheSize_);
    }

    bool getValue(const KeyType& key, ValueType& value); // may insert upon no cache depending on the policies
    void insertValue(const DataType<KeyType,ValueType>& value); // insert an new item into MCache

    void insertValue(const KeyType& key, const ValueType& value) // insert an new item into MCache
    {
        insertValue(DataType<KeyType, ValueType>(key, value) );
    }

    bool updateValue(const DataType<KeyType,ValueType>& dat) // insert an new item into MCache
    {
        lock.acquire_write_lock();
        KeyType key = dat.get_key();
        if (hash_.update(dat) )
        {
            if (cacheContainer_.find(key))
            {
                cacheContainer_.replace(key);
            }
            else
            {
                cacheContainer_.firstInsert(key); //Insert the corresponding CacheInfo into KeyInfoMap_.
            }
        }
        return true;
        lock.release_write_lock();

    }
    bool updateValue(const KeyType& key, const ValueType& value) // insert an new item into MCache
    {
        return updateValue(DataType<KeyType, ValueType>(key, value) );
    }

    bool getValueNoInsert(const KeyType& key, ValueType& value); //  not insert even if not found.
    bool getValueWithInsert(const KeyType& key, ValueType& value); //  insert if not found.
    bool hasKey(const KeyType& key);
    int numItems();

    void displayHash();
    void printKeyInfoMap();

    /**
     *      \brief It gets the info of one item in Cache.
     */
    CacheInfo<KeyType>& getCacheInfo(const KeyType& key)
    {
        return cacheContainer_.getCacheInfo(key);
    }

    /**
     *  	\brief It sets the TimeToLive of one item in Cache, can be used to indicate the item is out of date.
     *
     */
    void setTimeToLive(const KeyType &key, unsigned int lifeTime)
    {
        lock.acquire_write_lock();
        cacheContainer_.setTimeToLive(key, lifeTime);
        lock.release_write_lock();
    }

    void UpdateKeyInfoMap()
    {
        lock.acquire_write_lock();
        cacheContainer_.UpdateKeyInfoMap();
        lock.release_write_lock();
    }
    void flush(const KeyType& key);
    void flush();
    void dump(); //to be implemented;
    void clear();

    /**
     *	\brief return the approximate size of memory of cache.
     *
     *	It is approximate. We only calculate it by sum(size of (value))
     */
    /*size_t getMemSizeOfValue() {
     return hash_.getMemSizeOfValue();
     }*/

    /**
     *	\brief  monitor the performance of Cache.
     *
     *	 Hit ratio is calculated  from starting time to now.
     *
     *	 workload is calculated by "(number of lookups) / (time) ",
     *        where, "time" is in units of seconds.
     */

    void getEfficiency(double& hitRatio, double& workload)
    {
        if (nHit_ != 0)
        {
            hitRatio_ = double(nHit_) / double (nTotal_);
        }
        if (time(0) != startingTime_)
        {
            workload_ = double( nTotal_) / double(time(0) - startingTime_);
        }
        hitRatio = hitRatio_;
        workload = workload_;

    }
    /**
     *	\brief reset the StartingTime to monitor the performance of cache.
     *		and record hitRatio and workload.
     *
     */
    void resetStartingTime()
    {
        time_t now = time(0);
        getEfficiency(hitRatio_, workload_);

        /*loggerFile1 <<"From " <<ctime( &startingTime_);
         loggerFile1 <<" to "<<ctime( &now)<<endl;
         loggerFile1 << "nTotal: " << nTotal_<<endl;
         loggerFile1 << "nHit: " << nHit_<<endl;
         loggerFile1 << "hitRatio: " << hitRatio_<<endl;
         loggerFile1 << "workload: " << workload_<<endl;
         loggerFile1 << endl;*/

        nTotal_ = nHit_ = 0;
        hitRatio_ = workload_ = 0.0;
        startingTime_ = now;
    }

    template<class Archive> void save(Archive & ar,
                                      const unsigned int version = 0)
    {

        /*DataType<KeyType,ValueType> dat;
         int num = numItems();
         ar & num;
         for (MIT it=cacheContainer_.begin(); it != cacheContainer_.end(); it++) {
         FOUND_RESULT result = hash_.find(it->first.key, dat);
         if (result == FOUND_IN_MEM)
         ar & dat;
         }*/
        hash_.save(ar);
        //ar & cacheContainer_;

    }

    template<class Archive> void load(Archive & ar,
                                      const unsigned int version = 0)
    {
        /*int num;
         ar & num;

         for (int i=0; i<num; i++) {
         DataType<KeyType,ValueType> dat;
         ar & dat;
         insertValue(dat);
         }*/
        hash_.load(ar);
        isArchive_ = 1;
        //ar & cacheContainer_;
    }

private:
    CacheHash<KeyType, ValueType, Hash> hash_; // Use hash for Storage
    CacheContainer<KeyType, ValueType, ReplacementPolicy> cacheContainer_;
    unsigned int cacheSize_; // Capacity of Cache

    time_t startingTime_;
    int nTotal_;
    int nHit_;
    double hitRatio_;
    double workload_;

    bool isArchive_;

private:
    void evict(unsigned int num);//evict the oldest items.
};

/**
 *  	\brief Get an item from MCache
 *
 *	@return true if found, otherwise return faulse
 */
template <class KeyType, class ValueType, class ReplacementPolicy, class Hash,
class LockType> bool MCache<KeyType, ValueType, ReplacementPolicy,
Hash, LockType>::getValue(const KeyType& key, ValueType& value)
{
    lock.acquire_write_lock();
    nTotal_++;
    ValueType* pd = hash_.find(key);
    if (pd)
        value = *pd;
    if (cacheContainer_.find(key) )
    {
        cacheContainer_.replace(key); //Update the corresponding  CacheInfo.
        nHit_++;
        lock.release_write_lock();
        return true;
    }
    else if (isArchive_ && hash_.find(key) )
    {
        cacheContainer_.firstInsert(key); //Update the corresponding  CacheInfo.
        nHit_++;
        lock.release_write_lock();
        return true;
    }
    else
    {
        lock.release_write_lock();
        return false;
    }
}

/**
 *   \brief insert an new item into MCache, if storage(hash_) is full, evict 1 items accordint to the replacement policy.
 *
 */

template <class KeyType, class ValueType, class ReplacementPolicy, class Hash,
class LockType> void MCache<KeyType, ValueType, ReplacementPolicy,
Hash, LockType>::insertValue(const DataType<KeyType,ValueType>& dat)
{
    lock.acquire_write_lock();
    KeyType key = dat.get_key();
    if (hasKey(key) )
        return;
    if (hash_.insert(dat) )
    {
        if (cacheContainer_.find(key))
        {
            cacheContainer_.replace(key);
        }
        else
        {
            cacheContainer_.firstInsert(key); //Insert the corresponding CacheInfo into KeyInfoMap_.
        }
    }
    else //Insert failed, i.e. CacheHash is full.
    {
        evict(1); //evict the oldest item.
        if (hash_.insert(dat) ) //Insert into the hash_.
        {
            cacheContainer_.firstInsert(key); //Insert the corresponding CacheInfo into KeyInfoMap_.

        }
        else
        {
            //assert(false);
        }
    }
    lock.release_write_lock();
}

/**
 * 	\brief not insert even if not found
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy, class Hash,
class LockType> bool MCache<KeyType, ValueType, ReplacementPolicy,
Hash, LockType>::getValueNoInsert(const KeyType& key, ValueType& value)
{
    return getValue(key, value);
}

/**
 *	\brief  insert if not found
 *
 *         @return true if hits, othewise reture False and insert into the new item.
 */
template <class KeyType, class ValueType, class ReplacementPolicy, class Hash,
class LockType> bool MCache<KeyType, ValueType, ReplacementPolicy,
Hash, LockType>::getValueWithInsert(const KeyType& key, ValueType& value)
{

    if (getValue(key, value) )
        return true;
    else
    {
        insertValue(key, value);
        return false;
    }

}

/**
 *	\brief to determine if an item exists in MCache.
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy, class Hash,
class LockType> bool MCache<KeyType, ValueType, ReplacementPolicy,
Hash, LockType>::hasKey(const KeyType& key)
{
    lock.acquire_read_lock();
    bool isFound = cacheContainer_.find(key) || hash_.find(key);
    lock.release_read_lock();
    return (isFound );
}

/**
 * 	\brief get the number of the items in tha MCache.
 *
 */
template <class KeyType, class DataType, class ReplacementPolicy, class Hash,
class LockType> int MCache<KeyType, DataType, ReplacementPolicy, Hash,
LockType>::numItems()
{
    lock.acquire_read_lock();
    int num = hash_.numItems();
    lock.release_read_lock();
    return num;
}

/**
 *	\brief Display the number of items in memory hash and file hash.
 */
template <class KeyType, class DataType, class ReplacementPolicy, class Hash,
class LockType> void MCache<KeyType, DataType, ReplacementPolicy, Hash,
LockType>::displayHash()
{
    lock.acquire_read_lock();
    cout<<"MCache_hash numItems:"<<hash_.numItems()<<endl;
    cacheContainer_.display();
    lock.release_read_lock();
}

/**
 *	\brief display  KeyInfoMap_ inside, for Debug.
 *
 */
template <class KeyType, class DataType, class ReplacementPolicy, class Hash,
class LockType> void MCache<KeyType, DataType, ReplacementPolicy, Hash,
LockType>::printKeyInfoMap()
{
    lock.acquire_read_lock();
    cacheContainer_.printKeyInfoMap();
    lock.release_read_lock();
}

/**
 *	\brief Evict the  oldest items  overall.
 *
 */
template <class KeyType, class DataType, class ReplacementPolicy, class Hash,
class LockType> void MCache<KeyType, DataType, ReplacementPolicy, Hash,
LockType>::evict(unsigned int num)
{
    MIT it = cacheContainer_.begin();
    while (it != cacheContainer_.end() && num>0)
    {
        //cout<<it->first.key<<endl;
        MIT it1 = it; //reserverd iterator it, or cacheContainer_.del(key) would delete it also
        it = cacheContainer_.next(it);
        if (hash_.del(it1->first.key) ) //Remove an item from the cache.
        {
            cacheContainer_.del(it1->first.key); //Update the corresponding  CacheInfo.
            num--;
        }
        else
        {
            cout<<"\nevict failed\n\n";
            exit(1);
        }
    }
}

/**
 *  	\brief Flush(delete) the item that need to be updated.
 *
 */
template <class KeyType, class DataType, class ReplacementPolicy, class Hash,
class LockType> void MCache<KeyType, DataType, ReplacementPolicy, Hash,
LockType>::flush(const KeyType& key)
{
    lock.acquire_write_lock();
    if (hash_.del(key))
        cacheContainer_.del(key);
    lock.release_write_lock();
}

/**
 *  	\brief Flush(delete) all the items that need to be updated, detemined by keyInfoMap_.
 *
 */

template <class KeyType, class DataType, class ReplacementPolicy, class Hash,
class LockType> void MCache<KeyType, DataType, ReplacementPolicy, Hash,
LockType>::flush()
{
    lock.acquire_write_lock();
    UpdateKeyInfoMap();
    MIT it = cacheContainer_.begin();
    while (it != cacheContainer_.end() )
    {
        MIT it1 = it;
        it = cacheContainer_.next(it);
        KeyType key = it1->first.key;
        if (cacheContainer_.isOutOfDate(key) )
            flush(key);
    }
    lock.release_write_lock();
}

/**
 *  	\brief delete all the items in Cache.
 *
 */

template <class KeyType, class DataType, class ReplacementPolicy, class Hash,
class LockType> void MCache<KeyType, DataType, ReplacementPolicy, Hash,
LockType>::clear()
{
    lock.acquire_write_lock();
    UpdateKeyInfoMap();
    MIT it = cacheContainer_.begin();
    while (it != cacheContainer_.end() )
    {
        MIT it1 = it;
        it = cacheContainer_.next(it);
        flush(it1->first.key);
    }
    lock.release_write_lock();
}

}
}
#endif //MCache
