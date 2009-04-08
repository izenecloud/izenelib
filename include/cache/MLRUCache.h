/**
 * @file MCache.h
 * @brief The header file of MCache.
 *
 * This file defines class MCache.
 */

#ifndef MLRUCACHE_H
#define MLRUCACHE_H

#include "cm_basics.h"
#include "CacheHash.h"

#include <ctime>
#include <list>
#include <fstream>

using namespace std;
using namespace __gnu_cxx;
using namespace boost;


namespace izenelib{
namespace cache{
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
template <class KeyType, class ValueType, class Hash,
		class ThreadSafeLock=NullLock> class MLRUCache {

	typedef list<KeyType> CacheInfoList;
	typedef typename list<KeyType> :: iterator LIT;

	template<class D> struct _CachedData {
		D data;
		LIT lit;
		//const K get_key() const {
		//	return (const K)data.get_key();
		//}
	};
	typedef _CachedData<ValueType> CachedData;
	//typedef ExtendibleHash<KeyType, CachedData, NullLock> extHash;
	typedef izenelib::am::LinearHashTable<KeyType, CachedData, NullLock> linHash;	
	typedef izenelib::am::CCCR_StrHashTable<KeyType, CachedData, 8192*16> cccrHash;
	typedef izenelib::am::DataType<KeyType,ValueType> DataType;
public:
	/**
	 *  \brief Constuctor1: default fileName for fileHash of Hash_ is "./index.dat".
	 */
	MLRUCache(unsigned int cacheSize) :
		hash_(cacheSize, 1.0), startingTime_(time(0)), nTotal_(0), nHit_(0),
				hitRatio_(0.0), workload_(0.0) {
		cacheSize_ = cacheSize;
		//	isArchive_ = 0;
	} //CacheExtHash with ratio = 1.0.

	unsigned int getCacheSize() {
		return cacheSize_;
	}
	void setCacheSize(unsigned int cacheSize) {
		cacheSize_ = cacheSize;
		hash_.setHashSize(cacheSize_);
	}

	bool getValue(const KeyType& key, ValueType& value); // may insert upon no cache depending on the policies
	void insertValue(const DataType& value); // insert an new item into MCache
	void insertValue(const KeyType& key, const ValueType& value)
	{
		insertValue( DataType(key, value) );
	}
	bool getValueNoInsert(const KeyType& key, ValueType& value); //  not insert even if not found.		
	bool getValueWithInsert(const KeyType& key, ValueType& value); //  insert if not found.	
	bool hasKey(const KeyType& key);
	int numItems();

	void displayHash() {
		hash_.displayHash();
	}
	void flush(const KeyType& key);
	void clear();

	/**
	 *	\brief  monitor the performance of Cache.
	 *	
	 *	 Hit ratio is calculated  from starting time to now.  
	 *
	 *	 workload is calculated by "(number of lookups) / (time) ", 
	 *        where, "time" is in units of seconds. 
	 */
	void getEfficiency(double& hitRatio, double& workload) {
		if (nHit_ != 0) {
			hitRatio_ = double(nHit_) / double (nTotal_);
		}
		if (time(0) != startingTime_) {
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
	void resetStartingTime() {
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

	/*	template<class Archive> void save(Archive & ar,
	 const unsigned int version = 0) {	
	 hash_.save(ar);		
	 ar & cacheContainer_;
	 }

	 template<class Archive> void load(Archive & ar,
	 const unsigned int version = 0) {		
	 hash_.load(ar);		
	 ar & cacheContainer_;
	 }*/

private:

	//CacheExtHash<KeyType, CachedData, cccrHash, cccrHash> hash_; // Use hash for Storage
	CacheExtHash<KeyType, CachedData, linHash, linHash> hash_; // Use hash for Storage
	CacheInfoList cacheContainer_;
	unsigned int cacheSize_; // Capacity of Cache	

	time_t startingTime_;
	int nTotal_;
	int nHit_;
	double hitRatio_;
	double workload_;
	//	bool isArchive_;
	ThreadSafeLock lock;
private:
	inline void replace_(const KeyType& key) {
		LIT listTail = cacheContainer_.end();
		--listTail;
		CachedData *pd1 = hash_.find(key);
		CachedData *pd2 = hash_.find( *listTail);

		//swap two keys in list 
		LIT tempit = pd1->lit;
		KeyType tempkey = *listTail;

		*listTail = key;
		*tempit = tempkey;

		//swap LIT iterators of two CachedData
		pd1->lit = listTail;
		if (pd2) {
			pd2->lit = tempit;
		}

	}
	inline void firstInsert_(const DataType& dat) {
		KeyType key = dat.get_key();
		if (hash_.find(key) ) {
			assert(false);
		}
		cacheContainer_.push_back(key);
		LIT lit = cacheContainer_.end();
		--lit;

		CachedData cd;
		cd.data = dat.get_value();
		cd.lit = lit;
		hash_.insert(key, cd);
	}

	void evict_();//evict the oldest items.		
};

/**
 *  	\brief Get an item from MCache
 *
 *	@return TRUE if found, otherwise return faulse
 */
template <class KeyType, class ValueType, class Hash, class ThreadSafeLock> bool MLRUCache<
		KeyType, ValueType, Hash, ThreadSafeLock>::getValue(const KeyType& key,
		ValueType& value) {
	lock.acquire_write_lock();
	nTotal_++;
	CachedData* pd;
	pd = hash_.find(key);
	if (pd) {
		value = pd->data;
		replace_(key); //Update the corresponding  CacheInfo.		
		nHit_++;
		lock.release_write_lock();
		return TRUE;
	} else {
		lock.release_write_lock();
		return FALSE;
	}
}

/**
 *   \brief insert an new item into MLRUCache, if storage(hash_) is full, evict 1 items accordint to the replacement policy.
 *
 */

template <class KeyType, class ValueType, class Hash, class ThreadSafeLock> void MLRUCache<
		KeyType, ValueType, Hash, ThreadSafeLock>::insertValue(
		const DataType& dat) {
	lock.acquire_write_lock();
	KeyType key = dat.get_key();
	if (hash_.find(key) ) {
		replace_(key); //Insert the corresponding CacheInfo into KeyInfoMap_.   
	} else //Insert failed, i.e. CacheHash is full.
	{
		HASH_STATUS status = hash_.getStatus();
		//cout<<status<<endl;
		if (status != BOTH_FULL) //Insert into the hash_.
		{
			firstInsert_(dat);
		} else {
			evict_();
			firstInsert_(dat);
		}
	}
	lock.release_write_lock();
}

/**
 * 	\brief not insert even if not found
 * 	
 */
template <class KeyType, class ValueType, class Hash, class ThreadSafeLock> bool MLRUCache<
		KeyType, ValueType, Hash, ThreadSafeLock>::getValueNoInsert(
		const KeyType& key, ValueType& value) {
	return getValue(key, value);
}

/**
 *	\brief  insert if not found
 * 
 *         @return TRUE if hits, othewise reture False and insert into the new item.	
 */
template <class KeyType, class ValueType, class Hash, class ThreadSafeLock> bool MLRUCache<
		KeyType, ValueType, Hash, ThreadSafeLock>::getValueWithInsert(
		const KeyType& key, ValueType& value) {
	if (getValue(key, value) )
		return TRUE;
	else {
		insertValue(key, value);
		return FALSE;
	}

}

/**
 *	\brief to determine if an item exists in MLRUCache.    
 * 
 */
template <class KeyType, class ValueType, class Hash, class ThreadSafeLock> bool MLRUCache<
		KeyType, ValueType, Hash, ThreadSafeLock>::hasKey(const KeyType& key) {
	lock.acquire_read_lock();
	CachedData* pd = hash_.find(key);
	lock.release_read_lock();
	return (pd != NULL );
}

/** 
 * 	\brief get the number of the items in tha MLRUCache. 
 *
 */
template <class KeyType, class ValueType, class Hash, class ThreadSafeLock> int MLRUCache<
		KeyType, ValueType, Hash, ThreadSafeLock>::numItems() {
	lock.acquire_read_lock();
	int num = hash_.numItems();
	lock.release_read_lock();
	return num;
}

/**
 *	\brief Evict the  oldest items  overall.
 *
 */
template <class KeyType, class ValueType, class Hash, class ThreadSafeLock> void MLRUCache<
		KeyType, ValueType, Hash, ThreadSafeLock>::evict_() {
	KeyType key = cacheContainer_.front();
	cacheContainer_.pop_front();
	hash_.del(key);

}

/**
 *  	\brief Flush(delete) the item that need to be updated.
 * 	
 */
template <class KeyType, class ValueType, class Hash, class ThreadSafeLock> void MLRUCache<
		KeyType, ValueType, Hash, ThreadSafeLock>::flush(const KeyType& key) {
	lock.acquire_write_lock();
	CachedData* pd = hash_.find(key);
	if (pd) {
		cacheContainer_.erase(pd->lit);
		hash_.del(key);
	}
	lock.release_write_lock();
}

/**
 *  	\brief delete all the items in Cache. 
 *	
 */

template <class KeyType, class ValueType, class Hash, class ThreadSafeLock> void MLRUCache<
		KeyType, ValueType, Hash, ThreadSafeLock>::clear() {	
	lock.acquire_write_lock();
	KeyType key;
	while (!cacheContainer_.empty() ) {
		key = cacheContainer_.front();
		cacheContainer_.pop_front();
		hash_.del(key);
	}
	lock.release_write_lock();
}

}
}
#endif //MLRUCache
