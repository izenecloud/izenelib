/**
 * @file IzeneCache.h
 * @brief The header file of IzeneCache.
 * @Author Peisheng Wang
 *
 * This file defines class IzeneCache.
 */

#ifndef IzeneCache_H
#define IzeneCache_H

#include "cm_basics.h"
#include "CacheHash.h"
#include "IzeneCacheTraits.h"


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
 *  KeyType and ValueType    :  the type of what we want to cache.
 *	Hash                    :   Canadiates are LinearHashTable,ExtendibleHash, or others.
 *	ThreadSafeLock          :   it can be NullLock or ReadWriteLock, which are defined in ylib/lock.h. If using NullLock, then
 *				                No threa dsafe.
 */

template <class KeyType, class ValueType, class ThreadSafeLock = NullLock,
          HASH_TYPE hash_type = RDE_HASH, REPLACEMENT_TYPE policy = LRU>
class IzeneCache
{
    typedef IzeneCacheTrait<KeyType, ValueType, hash_type, policy>
    IzeneCacheType;
    typedef typename IzeneCacheContainerTrait<KeyType, ValueType, policy, hash_type>::LIT
    LIT;
    typedef typename IzeneCacheType::CacheInfoListType CacheInfoListType;
    typedef typename IzeneCacheType::ContainerType ContainerType;
    typedef typename IzeneCacheType::CachedDataType CachedDataType;

public:
    /**
     *  \brief Constuctor1: default fileName for fileHash of Hash_ is "./index.dat".
     */
    IzeneCache(unsigned int cacheSize = 1000)
        : cacheSize_(cacheSize)
        , startingTime_(time(0))
        , nTotal_(0)
        , nHit_(0)
        , hitRatio_(0.0)
        , workload_(0.0)
        , hash_(cacheSize)
        , cacheContainer_()
        , cache_(hash_, cacheContainer_)
    {
    }

    IzeneCache(const IzeneCache& obj)
        : cacheSize_(obj.cacheSize_)
        , hash_(obj.hash_)
        , startingTime_(obj.startingTime_)
        , nTotal_(obj.nTotal_)
        , nHit_(obj.nHit_)
        , hitRatio_(obj.hitRatio_)
        , workload_(obj.workload_)
    {
    }

    unsigned int getCacheSize()
    {
        return cacheSize_;
    }
    void setCacheSize(unsigned int cacheSize)
    {
        cacheSize_ = cacheSize;
        hash_.setHashSize(cacheSize_);
    }

    bool updateValue(const KeyType& key, const ValueType& val) // insert an new item into MCache
    {
        ScopedWriteLock<ThreadSafeLock> lock(lock_);
        if (cache_.hasKey(key))
        {
            CachedDataType **pd1 = hash_.find(key);
            (*pd1)->data = val;
            cache_.replace_(key);
        }
        else
        {
            if (!hash_.full())
            {
                cache_.firstInsert_(key, val);
            }
            else
            {
                cache_.evict_();
                cache_.firstInsert_(key, val);
            }
        }
        return true;
    }

    bool updateValue(const DataType<KeyType,ValueType>& dat) // insert an new item into MCache
    {
        return updateValue(dat.key, dat.value);
    }
    bool getValue(const KeyType& key, ValueType& value); // may insert upon no cache depending on the policies
    bool insertValue(const DataType<KeyType,ValueType>& dat) // insert an new item into MCache
    {
        return insertValue(dat.key, dat.value);
    }
    bool insertValue(const KeyType& key, const ValueType& value);

    int numItems()
    {
        ScopedReadLock<ThreadSafeLock> lock(lock_);
        return hash_.numItems();
    }

    void displayHash()
    {
        ScopedReadLock<ThreadSafeLock> lock(lock_);
        hash_.displayHash();
    }

    void displayCacheInfos(std::ostream& os = std::cout)
    {
        ScopedReadLock<ThreadSafeLock> lock(lock_);
        for (LIT it = cacheContainer_.begin();
                it != cacheContainer_.end(); it++)
        {
            //os<<"("<<it->first<<", "<<it->second<<")->";
            //os<<*it<<" -> ";
        }
        os << std::endl;
        hash_.displayHash();
    }

    bool getValueNoInsert(const KeyType& key, ValueType& value); //  not insert even if not found.
    bool getValueWithInsert(const KeyType& key, ValueType& value); //  insert if not found.
    bool hasKey(const KeyType& key)
    {
        ScopedReadLock<ThreadSafeLock> lock(lock_);
        return cache_.hasKey(key);
    }

    bool del(const KeyType& key)
    {
        ScopedWriteLock<ThreadSafeLock> lock(lock_);
        return cache_.del(key);

    }
    void clear()
    {
        ScopedWriteLock<ThreadSafeLock> lock(lock_);
        cache_.clear();
    }

    CacheInfoListType getKeyList()
    {
        return cacheContainer_;
    }

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
            workload_ = double(nTotal_) / double(time(0) - startingTime_);
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
        nTotal_ = nHit_ = 0;
        hitRatio_ = workload_ = 0.0;
        startingTime_ = now;
    }

private:

    unsigned int cacheSize_; // Capacity of Cache
    time_t startingTime_;
    int nTotal_;
    int nHit_;
    double hitRatio_;
    double workload_;

    ContainerType hash_; // Use hash for Storage
    CacheInfoListType cacheContainer_;
    IzeneCacheType cache_;
    ThreadSafeLock lock_;

}; // end - class IzeneCache

/**
 *  	\brief Get an item from MCache
 *
 *	@return true if found, otherwise return faulse
 */
template <class KeyType, class ValueType, class ThreadSafeLock,
          HASH_TYPE hash_type, REPLACEMENT_TYPE policy>
bool IzeneCache<KeyType, ValueType, ThreadSafeLock, hash_type, policy>::getValue(
        const KeyType& key, ValueType& value)
{
    ScopedWriteLock<ThreadSafeLock> lock(lock_);
    ++nTotal_;
    if (cache_.get(key, value))
    {
        cache_.replace_(key); //Update the corresponding  CacheInfo.
        nHit_++;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 *   \brief insert an new item into IzeneCache, if storage(hash_) is full, evict 1 items accordint to the replacement policy.
 *
 */

template <class KeyType, class ValueType, class ThreadSafeLock,
          HASH_TYPE hash_type, REPLACEMENT_TYPE policy>
bool IzeneCache<KeyType, ValueType, ThreadSafeLock, hash_type, policy>::insertValue(
        const KeyType& key, const ValueType& val)
{
    ScopedWriteLock<ThreadSafeLock> lock(lock_);
    if (cache_.hasKey(key))
    {
        cache_.replace_(key); //Insert the corresponding CacheInfo into KeyInfoMap_.
        return false;
    }
    else //Insert failed, i.e. CacheHash is full.
    {
        if (!hash_.full())
        {
            cache_.firstInsert_(key, val);
        }
        else
        {
            cache_.evict_();
            cache_.firstInsert_(key, val);
        }
        return true;
    }

}

/**
 * 	\brief not insert even if not found
 *
 */
template <class KeyType, class ValueType, class ThreadSafeLock,
          HASH_TYPE hash_type, REPLACEMENT_TYPE policy>
bool IzeneCache<KeyType, ValueType, ThreadSafeLock, hash_type, policy>::getValueNoInsert(
        const KeyType& key, ValueType& value)
{
    return getValue(key, value);
}

/**
 *	\brief  insert if not found
 *
 *         @return true if hits, othewise reture False and insert into the new item.
 */
template < class KeyType, class ValueType, class ThreadSafeLock,
HASH_TYPE hash_type, REPLACEMENT_TYPE policy>
bool IzeneCache<KeyType, ValueType, ThreadSafeLock, hash_type, policy>::getValueWithInsert(
        const KeyType& key, ValueType& value)
{
    if (getValue(key, value))
        return true;
    else
    {
        insertValue(key, value);
        return false;
    }

}

template <class KeyType = string,
          class ValueType = NullType,
          class LockType = NullLock>
class ILRUCache : public IzeneCache<KeyType, ValueType, LockType, RDE_HASH, LRU>
{
public:
    ILRUCache(size_t cacheSize = 1000)
        : IzeneCache<KeyType, ValueType, LockType, RDE_HASH, LRU>(cacheSize)
    {
    }
};

template <class KeyType = string,
          class ValueType = NullType,
          class LockType = NullLock>
class ILFUCache : public IzeneCache<KeyType, ValueType, LockType, RDE_HASH, LFU>
{
public:
    ILFUCache(size_t cacheSize = 1000)
        : IzeneCache<KeyType, ValueType, LockType, RDE_HASH, LFU>(cacheSize)
    {
    }
};

}
}

#endif //IzeneCache
