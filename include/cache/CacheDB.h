/**
 * @file CacheDB.h
 * @brief The header file of CacheDB.
 *
 * This file defines class CacheDB.
 */

#ifndef CacheDB_H
#define CacheDB_H

#include "CacheInfo.h"
#include "cm_basics.h"

#include <ctime>
#include <list>
#include <map>
#include <ext/hash_map>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

using namespace std;
using namespace __gnu_cxx;
using namespace boost;

namespace izenelib {
namespace cache {

/**
 * 	\brief  CacheDB,  file-based data structure for storage. 
 *	
 *	 It is persistent in that it stores all the key-value pairs in file. Plus, it also caches most of the items being used in memory such that
 *  	 it provides fast lookup to most of the retrieval calls. It has all the replacement policy, storage policy, and others. CacheDB can be used
 *  	 as our base DB for key-value pairs. There are many open source platforms like this, such as berkeley DB and gdbm. Our version is based on
 *  	 Linear Hashtable and supports efficient caching explicitly (instead of relying on OS virtual memory/swapping system). It also support
 *  	 multi-threads and locking/concurrency. 
 *	
 *
 *       KeyType and DataType    :   the type of what we want to cache.
 *	RelacementPolciy        :   it can be lruCmp, lfuCmp or slruCmp, which are defined in CacheInfo.h.
 *	MCache			:   Using the MCache to cache the most frequent items in DataHash to improve the efficency. 
 *       DataHash		:   Store the data, can be extendibleHashFile, linearHashFile.				  
 *	LockType          :   it can be NullLock or ReadWriteLock, which are defined in ylib/lock.h. If using NullLock, then 
 *				    No threa dsafe. 	 
 */

template <class KeyType, class ValueType, class ReplacementPolicy,
		class MCache, class DataHash, class LockType =NullLock> class CacheDB {

	typedef izenelib::am::DataType<KeyType,ValueType> DataType;
public:
	/**
	 *  \brief Constuctor1: default fileName for dataHash is "./index.dat".
	 */
	CacheDB(unsigned int cacheSize) :
		mCache_(cacheSize) {
	}

	CacheDB(unsigned int cacheSize, const char* fileName) :
		mCache_(cacheSize), dataHash_(fileName) {
		dataHash_.open();

	}

	void setCacheSize(unsigned int cacheSize) {
		mCache_.setCacheSize(cacheSize);
	}
	unsigned int getCacheSize() {
		return mCache_.getCacheSize();
	}

	bool del(const KeyType& key); // if no data return false, else successful delete.	
	bool getValue(const KeyType& key, ValueType& value); // may insert upon no cache depending on the policies
	void insertValue(const DataType& value); // insert an new item into CacheDB
	void insertValue(const KeyType& key, const ValueType& value) // insert an new item into CacheDB
	{
		insertValue(DataType(key, value) );
	}

	void updateValue(const DataType& dat) {
		lock.acquire_write_lock();
		if ( dataHash_.update(dat) ) {
			mCache_.updateValue(dat);
		}
		lock.release_write_lock();
	}

	void updateValue(const KeyType& key, const ValueType& value) // insert an new item into CacheDB
	{
		updateValue( DataType(key, value) );
	}

	bool hasKey(const KeyType& key);
	int numItems();

	bool getValueNoInsert(const KeyType& key, ValueType& value); //  not insert even if not found.		
	bool getValueWithInsert(const KeyType& key, ValueType& value); //  insert if not found.	

	void displayHash();

	void flush(const KeyType& key);
	//void flush();
	void dump();//to be implemented
	void clear();

	/**
	 *	\brief  monitor the performance of Cache.
	 *	
	 *	 Hit ratio will be calculated in the time from starting time to now.  
	 *
	 *	 workload is calculated by "(number of lookups) / (time) ", 
	 *        where, "time" is in units of seconds. 
	 */

	void getEfficiency(double& hitRatio, double& workload) {
		mCache_.getEfficiency(hitRatio, workload);

	}
	/**
	 *	\brief reset the StartingTime to monitor the performance of cache.
	 *		and record hitRatio and workload. 
	 *
	 */

	void resetStartingTime() {
		mCache_.resetStartingTime();
	}

protected:
	MCache mCache_;
	DataHash dataHash_;
	LockType lock;//use threadsafe only when dataHash_ is not threadSafe.	
};

/**
 *  	\brief Get an item from CacheDB
 *
 *	@return TRUE if found, otherwise return faulse
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
		class MCache, class DataHash, class LockType> bool CacheDB<KeyType,
		ValueType, ReplacementPolicy, MCache, DataHash, LockType>::getValue(
		const KeyType& key, ValueType& value) {
	lock.acquire_write_lock();
	if (mCache_.getValue(key, value) ) {
		lock.release_write_lock();
		return TRUE;
	} else {
		ValueType* p;
		p = dataHash_.find(key);
		if (p) {
			value = *p;
			delete p;
			p = 0;
			mCache_.insertValue(key, value);
		}
		lock.release_write_lock();
		return FALSE;
	}

}

/**
 *   \brief insert an new item into CacheDB
 */

template <class KeyType, class ValueType, class ReplacementPolicy,
		class MCache, class DataHash, class LockType> void CacheDB<KeyType,
		ValueType, ReplacementPolicy, MCache, DataHash, LockType>::insertValue(
		const DataType& dat) {
	lock.acquire_write_lock();
	if (dataHash_.insert(dat) ) {
		mCache_.insertValue(dat);
	}
	lock.release_write_lock();
}

/**
 * 	\brief not insert even if not found
 * 	
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
		class MCache, class DataHash, class LockType> bool CacheDB<KeyType,
		ValueType, ReplacementPolicy, MCache, DataHash, LockType>::getValueNoInsert(
		const KeyType& key, ValueType& value) {
	return getValue(key, value);
}

/**
 *	 \brief insert if not found
 * 
 *         @return TRUE if hits, othewise reture False and insert into the new item.	
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
		class MCache, class DataHash, class LockType> bool CacheDB<KeyType,
		ValueType, ReplacementPolicy, MCache, DataHash, LockType>::getValueWithInsert(
		const KeyType& key, ValueType& value) {

	if (getValue(key, value) )
		return TRUE;
	else {
		insertValue(key, value);
		return FALSE;
	}

}

/**
 *  	\brief delete an item from CacheDB.
 */

template <class KeyType, class ValueType, class ReplacementPolicy,
		class MCache, class DataHash, class LockType> bool CacheDB<KeyType,
		ValueType, ReplacementPolicy, MCache, DataHash, LockType>::del(
		const KeyType& key) {
	lock.acquire_write_lock();
	if (dataHash_.del(key) ) {
		mCache_.flush(key);
		lock.release_write_lock();
		return 1;
	} else {
		lock.release_write_lock();
		return 0;
	}
}

/**
 *	\brief to determine if an item exists in CacheDB.    
 * 
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
		class MCache, class DataHash, class LockType> bool CacheDB<KeyType,
		ValueType, ReplacementPolicy, MCache, DataHash, LockType>::hasKey(
		const KeyType& key) {
	lock.acquire_read_lock();
	bool is = mCache_.hasKey(key) || (dataHash_.find(key) != NULL);
	lock.release_read_lock();

	return is;

}

/** 
 * 	\brief get the number of the items in tha CacheDB. 
 *
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
		class MCache, class DataHash, class LockType> int CacheDB<KeyType,
		ValueType, ReplacementPolicy, MCache, DataHash, LockType>::numItems() {
	lock.acquire_read_lock();
	int num = dataHash_.num_items();
	lock.release_read_lock();

	return num;
}

/**
 *	\brief It displays the number of items in memory Cache.
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
		class MCache, class DataHash, class LockType> void CacheDB<KeyType,
		ValueType, ReplacementPolicy, MCache, DataHash, LockType>::displayHash() {
	mCache_.displayHash();
	cout<<"dataHash: numItems ="<< numItems()<<endl;
}

/**
 *  	\brief Flush(delete) the item in the cache that need to be updated.
 * 	
 */
template <class KeyType, class ValueType, class ReplacementPolicy,
		class MCache, class DataHash, class LockType> void CacheDB<KeyType,
		ValueType, ReplacementPolicy, MCache, DataHash, LockType>::flush(
		const KeyType& key) {
	mCache_.flush(key);
}

/**
 *  	\brief Flush(delete) all the item in the cache that need to be updated.
 * 	
 */
/*template <class KeyType, class DataType, class ReplacementPolicy, class MCache,
 class DataHash, class LockType> void CacheDB<KeyType, DataType,
 ReplacementPolicy, MCache, DataHash, LockType>::flush() {
 mCache_.flush();
 }*/

/**
 *	\brief it saves the most frequent item in memory to file. 
 *
 */
template <class KeyType, class DataType, class ReplacementPolicy, class MCache,
		class DataHash, class LockType> void CacheDB<KeyType, DataType,
		ReplacementPolicy, MCache, DataHash, LockType>::dump() {
	//to be implemented
}

/**
 *	\brief it deletes all the items in memory. 
 *
 */
template <class KeyType, class DataType, class ReplacementPolicy, class MCache,
		class DataHash, class LockType> void CacheDB<KeyType, DataType,
		ReplacementPolicy, MCache, DataHash, LockType>::clear() {
	dataHash_.flush();
	mCache_.clear();
}

}
}
#endif //CacheDB
