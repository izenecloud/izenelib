/**
 * @file MFCache.h
 * @brief The header file of MFCache.
 *
 * This file defines class MFCache.
 */

#ifndef MFCACHE_H
#define MFCACHE_H

#include "cm_basics.h"
#include "CacheHash.h"
#include "CacheInfo.h"
#include "CacheContainer.h"

#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <list>


namespace izenelib
{
namespace cache
{

/**
 * 	\brief A Cache resides both in memory and file.
 *
 *	It can deal with different type of key-data pair using different Replacement
 *	Policies and using different storage policies through template template parameters.
 *
 *  KeyType and ValueType    :   the type of what we want to cache.
 *	RelacementPolciy        :   it can be lruCmp, lfuCmp or slruCmp, which are defined in CacheInfo.h.
 *	FirstHash and SecondHash:   it detemines the type of hybird  hash as storage of cache.
 *				                And Canadiates are LinearHashTable,ExtendibleHash,ExtendibleHashFile, LinearHashFile, or others.
 *	LockType          :   it can be NullLock or ReadWriteLock, which are defined in ylib/lock.h. If using NullLock, then
 *				                No threadsafe.
 */

//static ofstream loggerFile("./mfcache_logger");
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType = NullLock>
class MFCache
{
    enum { EVICT_NUM = 30 };
    typedef typename std::map<CacheInfo<KeyType>, bool, ReplacementPolicy>::iterator
    MIT;
    typedef typename boost::unordered_map<KeyType, CacheInfo<KeyType>, HashFun<KeyType> >::iterator
    HIT;

    LockType lock;
    //typedef izenelib::am::DataType<KeyType,ValueType> DataType;
public:
    /**
     *  \brief Constuctor1: default fileName for fileHash of Hash_ is "./index.dat".
     */
    MFCache(unsigned int cacheSize, double ratio, int dumpOption)
        : hash_(cacheSize, ratio)
        , cacheSize_(cacheSize)
        , ratio_(ratio)
        , dumpOption_(dumpOption)
        , startingTime_(time(0))
        , nTotal_(0)
        , nHit_(0)
        , hitRatio_(0.0)
        , workload_(0.0)
        , isArchive_(false)
    {
    }
    /**
     *  \brief Constructor2
     *
     */
    MFCache(unsigned int cacheSize, double ratio, int dumpOption, const char* fileName)
        : hash_(cacheSize, ratio, fileName)
        , cacheSize_(cacheSize)
        , ratio_(ratio)
        , dumpOption_(dumpOption)
        , startingTime_(time(0))
        , nTotal_(0)
        , nHit_(0)
        , hitRatio_(0.0)
        , workload_(0.0)
        , isArchive_(false)
    {
    }

    /**
     *  \brief get Cache Size
     */
    unsigned int getCacheSize()
    {
        return cacheSize_;
    }
    /**
     *  \brief set Cache Size
     */
    void setCacheSize(unsigned int cacheSize)
    {
        cacheSize_ = cacheSize;
        hash_.setHashSize(cacheSize_);
    }
    /**
     *  \brief get ratio
     */
    double getRatio()
    {
        return ratio_;
    }

    /**
     *  \brief set the ratio
     */
    void setRatio(double ratio)
    {
        ratio_ = ratio;
        hash_.setRatio(ratio);
    }

    bool getValue(const KeyType& key, ValueType& value); // may insert upon no cache depending on the policies
    void insertValue(const KeyType& key, const ValueType& value) // insert an new item into MFCache
    {
        insertValue( DataType<KeyType,ValueType>(key, value) );
    }
    void insertValue(const DataType<KeyType,ValueType>& dat);

    bool getValueNoInsert(const KeyType& key, ValueType& value); //  not insert even if not found.
    bool getValueWithInsert(const KeyType& key, ValueType& value); //  insert if not found.
    bool hasKey(const KeyType& key);
    int numItems();

    /**
     *  	\brief Get Hash_STATUS, there are 4 cases: both full, memory full, file full, both not full.
     */
    HASH_STATUS getHashStatus()
    {
        return hash_.getStatus();
    }

    /**
     *	\brief Display the number of items in memory hash and file hash.
     */
    void displayHash()
    {
        lock.acquire_read_lock();
        hash_.displayHash();
        lock.release_read_lock();
    }
    void printKeyInfoMap();

    /**
     *      \brief It gets the info of one item in Cache.
     */
    CacheInfo<KeyType>& getCacheInfo(const KeyType& key)
    {
        return cacheContainer_.getCacheInfo();
    }

    /**
     *  	\brief It sets the TimeToLive of one item in Cache, can be used to indicate the item is out of date.
     *
     */
    void setTimeToLive(const KeyType &key, time_t lifeTime)
    {
        lock.acquire_write_lock();
        cacheContainer_.setTimeToLive(key, lifeTime);
        lock.release_write_lock();
    }

    /**
     *	\brief It detemines which item is out of date by the CacheInfos.
     */
    void UpdateKeyInfoMap();
    /**
     *  \brief delete the item with give key.
     */

    void flush(const KeyType& key);
    /**
     *  \brief	Flush(delete) the item that need to be updated.
     *
     */
    void flush();

    void dump();
    /**
     *   \brief clear all items in  cache.
     */
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
     *	 Hit ratio is calculated from starting time to now.
     *
     *	 workload is calculated by "(number of lookups) / (time) ",
     *        where, "time" is in units of seconds.
     */

    void getEfficiency(double& hitRatio, double& workload)
    {

        if (nHit_ != 0)
        {
            hitRatio_ = double(nHit_) / double (nTotal_ );
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

        /*getEfficiency(hitRatio_, workload_);
         loggerFile <<"From " <<ctime( &startingTime_);
         loggerFile <<" to "<<ctime( &now)<<endl;
         loggerFile << "nTotal: " << nTotal_<<endl;
         loggerFile << "nHit: " << nHit_<<endl;
         loggerFile << "hitRatio: " << hitRatio_<<endl;
         loggerFile << "workload: " << workload_<<endl;
         loggerFile << endl;*/

        nTotal_ = nHit_ = 0;
        hitRatio_ = workload_ = 0.0;
        startingTime_ = now;
    }
    /**
     *  \brief MFCache serialization
     */

    template<class Archive>
    void save(Archive & ar,
              const unsigned int version = 0)
    {
        hash_.save(ar);
        //ar & cacheContainer_;

        /*	ValueType dat;
         int num = int(numItems()*ratio_);
         ar & num;
         for (MIT it=cacheContainer_.begin(); it != cacheContainer_.end(); it++) {
         FOUND_RESULT result = hash_.find(it->first.key, dat);
         if (result == FOUND_IN_MEM)
         ar & dat;
         }*/
    }
    /**
     *  \brief MFCache serialization
     */

    template<class Archive>
    void load(Archive & ar,
              const unsigned int version = 0)
    {
        /*int num;
         ar & num;

         for (int i=0; i<num; i++) {
         ValueType dat;
         ar & dat;
         insertValue(dat);
         }*/

        hash_.load(ar);
        isArchive_ = 1;
        //ar & cacheContainer_;
    }

private:
    CacheExtHash<KeyType, ValueType, FirstHash, SecondHash> hash_; // Use hash for Storage
    CacheContainer<KeyType, ValueType, ReplacementPolicy> cacheContainer_;

    unsigned int cacheSize_; // Capacity of Cache
    double ratio_; // The proportion of memory and file in terms of cache
    // items in the cache storage.

    int dumpOption_; //dump option
    time_t startingTime_;
    int nTotal_;
    int nHit_;
    double hitRatio_;
    double workload_;

    bool isArchive_;

private:
    void evict_memory(unsigned int num);//evict the oldest items in memory.
    void evict(unsigned int num);//evict the oldest items.

    std::list<KeyType> dumpList_;
    DUMP_RESULT dump_f2m(const KeyType& key);
};

/**
 *  	\brief Get an item from MFCache
 *
 *	@return true if found, otherwise return faulse
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
bool MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
getValue(const KeyType& key, ValueType& value)
{

    lock.acquire_write_lock();
    nTotal_++;
    if (cacheContainer_.find(key) )
    {
        //if( !hash_.find(key) )
        //	cout<<"Found? "<< hash_.find(key) <<endl;
        FOUND_RESULT result = hash_.find(key, value);
        if (result == NOT_FOUND)
        {
            cout<<"Error, unconsistence between CacheInfo and CacheHash"<<endl;

            lock.release_write_lock();
            return false;
            //exit(1);
        }
        if (result == FOUND_IN_FILE)
        {
            if (dumpOption_ == ONLY_DUMP || dumpOption_ == DUMP_EVICT)
                dump_f2m(key);
            if (dumpOption_ == DUMP_LATER)
                dumpList_.push_back(key);
        }
        cacheContainer_.replace(key); //Update the corresponding  CacheInfo.
        nHit_++;
        lock.release_write_lock();
        return true;
    } //else if (isArchive_ && hash_.find(key) )
    else if ( hash_.find(key) )
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
 *   \brief insert an new item into MFCache, if storage(hash_) is full, evict several items accoding to DUMP policy.
 *
 */

template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
void MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
insertValue(const DataType<KeyType,ValueType>& dat)
{
    lock.acquire_write_lock();
    KeyType key = dat.get_key();//value should have get_key() method.
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
        if (dumpOption_ == DUMP_EVICT || dumpOption_ == ONLY_EVICT)
        {
            evict(EVICT_NUM); //evict the EVICT_NUM oldest items.
            if (hash_.insert(dat) ) //Insert into the hash_.
            {
                cacheContainer_.firstInsert(key); //Insert the corresponding CacheInfo into cacheContainer_.
            }
        }
        else //for dumpOption == NO_DUMP and dumpOtption == ONLY_DUMP;
        {
            evict(1); //evict the oldest item.
            if (hash_.insert(dat) ) //Insert into the hash_.
            {
                cacheContainer_.firstInsert(key); //Insert the corresponding CacheInfo into cacheContainer_.
            }
        }

    }
    lock.release_write_lock();
}

/**
 * 	\brief not insert even if not found
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
bool MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
getValueNoInsert(const KeyType& key, ValueType& value)
{
    return getValue(key, value);
}

/**
 *	 \brief insert if not found
 *
 *         @return true if hits, othewise reture False and insert into the new item.
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
bool MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
getValueWithInsert(const KeyType& key, ValueType& value)
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
 *	\brief to determine if an item exists in MFCache.
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
bool MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
hasKey(const KeyType& key)
{
    lock.acquire_read_lock();
    bool isFound = cacheContainer_.find(key);
    lock.release_read_lock();
    return (isFound );

}

/**
 * 	\brief get the number of the items in tha MFCache.
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
int MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
numItems()
{
    lock.acquire_read_lock();
    int num = hash_.numItems();
    lock.release_read_lock();
    return num;
}

/**
 *	\brief display  cacheContainer_ inside, for Debug.
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
void MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
printKeyInfoMap()
{
    lock.acquire_read_lock();
    cacheContainer_.printKeyInfoMap();
    lock.release_read_lock();
}

/**
 *	\brief  Evict the oldest items only reside in memory.
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
void MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
evict_memory(unsigned int num)
{

    ValueType dat;
    MIT it = cacheContainer_.begin();
    while (it != cacheContainer_.end() && num>0)
    {

        MIT it1 = it; //reserverd iterator it, or cacheContainer_.del(key) would delete it also
        it = cacheContainer_.next(it);
        FOUND_RESULT result = hash_.find(it1->first.key, dat);
        if (result == FOUND_IN_MEM)
        {
            if (hash_.del(it1->first.key) ) //Remove an item from the cahe.
            {
                cacheContainer_.del(it1->first.key); //Update the corresponding  CacheInfo.
                num--;
            }
            else
            {
                cout<<"\nevict_mem failed\n\n";
                exit(1);
            }
        }
    }

}

/**
 *	\brief  Evict the  oldest items in overall
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
void MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
evict(unsigned int num)
{

    MIT it = cacheContainer_.begin();
    while (it != cacheContainer_.end() && num>0)
    {
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
 *	 \brief dump from file to memory for efficency.
 *	 precondition -- key is in fileHash.
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
DUMP_RESULT MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
dump_f2m(const KeyType& key)
{

    HASH_STATUS hs = getHashStatus();
    if (hs & 1)// memory is not full.
    {
        return hash_.dump(key);
    }
    else // memHash_ is full.
    {

        for (MIT it = cacheContainer_.begin(); it != cacheContainer_.end(); it++)
        {
            ValueType val, firstVal;
            FOUND_RESULT result = hash_.find(it->first.key, firstVal);

            if (result == FOUND_IN_MEM) //found the oldest item in memory.
            {
                if (hash_.find(key, val) == FOUND_IN_FILE) //check the precondition -- key is in fileHash
                {
                    hash_.del(it->first.key); //delete an item from the memory;
                    hash_.insert(it->first.key, firstVal); //insert dat with handle of key into memory;
                    hash_.insert(key, val); //insert into the item that previously deleted.
                    return DUMP_F2M;
                }

                else
                {
                    cout<<"\ndump_f2m: Key is not in the fileHash_ \n\n";
                    exit(1);
                }
            }
        }
        return DUMP_FAILED;
    }

}

template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
void MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
UpdateKeyInfoMap()
{
    lock.acquire_write_lock();
    cacheContainer_.UpdateKeyInfoMap();
    lock.release_write_lock();
}

/**
 *  	Flush(delete) the item that need to be updated.
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
void MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
flush(const KeyType& key)
{
    lock.acquire_write_lock();
    if ( hash_.del(key) )
        //hash_.del(key);
        cacheContainer_.del(key);
    lock.release_write_lock();
}

/**
 *  	Flush(delete) all the items that need to be updated, detemined by cacheContainer_.
 *
 */

template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
void MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
flush()
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
 *  	dump the item in dumpList_ from file to memory.
 * 	Suggestion: Call it when cpu is idle.
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
void MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
dump()
{
    lock.acquire_write_lock();
    typename std::list<KeyType>::iterator it, it1;
    it = dumpList_.begin();
    while (it != dumpList_.end() )
    {
        //cout<<*it<<endl;
        dump_f2m(*it);
        it1 = it; //reserverd iterator it, or cacheContainer_.del(key) would delete it also
        it++;
        dumpList_.erase(it1);
    }
    lock.release_write_lock();
    //cout<<dumpList_.size()<<endl;
}

/**
 *  	\brief delete all the items in Cache.
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
          class FirstHash, class SecondHash, class LockType>
void MFCache<KeyType, ValueType, ReplacementPolicy, FirstHash, SecondHash, LockType>::
clear()
{
    lock.acquire_write_lock();
    UpdateKeyInfoMap();
    MIT it = cacheContainer_.begin();
    while (it != cacheContainer_.end() )
    {
        MIT it1 = it;
        it->cacheContainer_.next(it);
        flush(it1->first.key);
    }
    lock.release_write_lock();
}

}
}

#endif //MFCACHE
