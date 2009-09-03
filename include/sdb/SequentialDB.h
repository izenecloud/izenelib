/**
 * @file SEQUENTIALDB.h
 * @brief The header file of SequentialDB.
 *
 * This file defines Class SequentialDB.
 */

#ifndef SEQUENTIALDB_H_
#define SEQUENTIALDB_H_

#include <am/sdb_hash/sdb_hash.h>
#include <am/sdb_hash/sdb_fixedhash.h>
#include <am/sdb_btree/sdb_btree.h>
#include <am/tokyo_cabinet/tc_hash.h>

/*#ifdef EXTERNAL_TOKYO_CABINET
 #include <am/tokyo_cabinet/tc_hash.h>
 #endif*/
//#include <am/btree/BTreeFile.h>

using namespace izenelib::am;
using namespace std;

namespace izenelib {

namespace sdb {

/**
 *
 * 	\brief  SequentialDB,  file-based DB
 *
 *	 It is persistent that it stores all the key-value pairs in file. Plus,
 *   it supports random acess and sequentail acess and query.
 *
 *   It can use btree, hash, tc_hash and all kindes of access methods as underlying data strucure
 *
 *
 *  KeyType and DataType  :  the type of what we want to cache.
 *	LockType        	  :  it can be NullLock or ReadWriteLock. If using NullLock, then
 *				                No threadsafe.
 *
 *
 */
template<
		typename KeyType =string,
		typename ValueType=NullType,
		typename LockType =NullLock,
		typename ContainerType=izenelib::am::sdb_btree<KeyType, ValueType, LockType>,
		typename Alloc=std::allocator<DataType<KeyType,ValueType> > > class SequentialDB {
public:
	//typedef DataType<KeyType, ValueType> DataType;
	typedef typename ContainerType::SDBCursor SDBCursor;
	typedef KeyType SDBKeyType;
	typedef ValueType SDBValueType;

public:
	/**
	 *  \brief constructor1, the default fileName is "SeqentialDB.dat"
	 *
	 *  @fileName filename of db data file. For SDB/btree, if its file name ends with '#', eg
	 *   'sdb.dat#', it means cc-b-tree, otherwise its cc-b*-btree. cc-b*-btree perform better in
	 *  reading and use less disk space, but perform much worse for ascending access.
	 *
	 */
	SequentialDB(const string& fileName = "SequentialDB.dat") :
		container_(fileName) {
	}

	//=============for sdb/btree======================
	/**
	 *
	 *  \brief set degree for btree.
	 *  @degree, mostly degree from 8~16 is preffered, but for ascending inserting,
	 *  big degree(even, 32, 64) can perform better.
	 *
	 */
	void setDegree(int degree) {
		container_.setDegree(degree);
	}

	/**
	 *  \brief set page size for both sdb/hash and sdb/btree
	 *
	 */

	void setPageSize(size_t pageSize) {
		container_.setPageSize(pageSize);
	}
	//=================================================


	//==============for sdb/hash ======================
	/**
	 *  \brief set directorySize only for sdb/hash
	 *
	 *   if not called, default is 65536.
	 */

	void setDirectorySize(size_t directorySize) {
		container_.setDirectorySize(directorySize);
	}
	/**
	 *  \brief set bucketSize only for sdb/hash
	 *  if not called, default is 1024.
	 *
	 */
	void setBucketSize(size_t bucketSize) {
		container_.setBucketSize(bucketSize);
	}

	//=================================================
	/**
	 * 	\brief set the cache size.
	 *
	 */
	void setCacheSize(size_t sz) {
		container_.setCacheSize(sz);
	}

	/**
	 *  \brief open the db
	 *  @return true if open success, otherwise false
	 */
	bool open() {
		return container_.open();
	}
	/**
	 *  \brief close the db
	 *
	 */
	bool close() {
		return container_.close();
	}

	/**
	 *  \brief read an item from SequentialDB by key.
	 *
	 *	@return TRUE if found, otherwise return faulse
	 */
	bool getValue(const KeyType& key, DataType<KeyType,ValueType> & data) {
		ValueType val;
		if (getValue(key, val) ) {
			data = DataType<KeyType, ValueType>(key, val);
			return true;
		}
		return false;
	}

	/**
	 *  \brief read an item from SequentialDB by key.
	 *
	 *	@return TRUE if found, otherwise return faulse
	 */
	bool getValue(const KeyType& key, ValueType& value);

	/**
	 *   \brief insert an new item into SequentialDB
	 *   @data wrap both key/value pair
	 */
	bool insertValue(const DataType<KeyType,ValueType> & data);

	/**
	 *   \brief insert an new item into SequentialDB
	 */
	bool insertValue(const KeyType& key, const ValueType& value) {
		return insertValue(DataType<KeyType, ValueType>(key, value) );
	}

	/**
	 *  \brief if not found, insert them
	 */
	bool getValueWithInsert(const KeyType& key, ValueType& value) {
		if (getValue(key, value) )
			return true;
		else {
			insertValue(key, value);
			//container_.flush();
			return false;
		}
	}
	/**
	 *  	\brief It deletes an item from SequentialDB.
	 */
	bool del(const KeyType& key);

	/**
	 * \brief update an item with given key, if it not exist, insert it directly.
	 *
	 */
	bool update(const DataType<KeyType,ValueType> & data);

	/**
	 * \brief update an item with given key, if it not exist, insert it directly.
	 *
	 */
	bool update(const KeyType& key, const ValueType& value) {
		return update(DataType<KeyType, ValueType>(key, value) );
	}

	bool dump(SequentialDB& other) {
		ContainerType &otherContainer = other.getContainer();
		return container_.dump(otherContainer);
	}

	bool dump(const string& fileName) {
		return container_.dump(fileName);
	}

	/**
	 *  \brief write the dirtypage to the disk.
	 *
	 */
	void commit() {
		lock_.acquire_write_lock();
		container_.commit();
		lock_.release_write_lock();
	}

	SDBCursor get_first_Locn() {
		return container_.get_first_locn();
	}

	/**
	 * 	\brief get the next or prev item.
	 *
	 *  \locn when locn is default value, it will start with firt element when sdri=ESD_FORWARD
	 *   and start with last element when sdir = ESD_BACKWARD
	 */
	bool seq(SDBCursor& locn, KeyType& key, ValueType& value, ESeqDirection sdir=ESD_FORWARD)
	{
	    bool ret = seq(locn);
	    get(locn, key, value);
	    return ret;
	}
	bool seq(SDBCursor& locn, DataType<KeyType, ValueType>& dat, ESeqDirection sdir=ESD_FORWARD)
    {
		return seq(locn, dat.key, dat.value, sdir);
    }

	
	bool seq(SDBCursor& locn, ESeqDirection sdir = ESD_FORWARD) {
		return container_.seq(locn, sdir);
	}

	/**
	 *   \brief get the cursor for given key
	 *
	 *   @param locn is cursor of key.
	 *   @return true if key exists otherwise false.
	 *
	 */
	bool search(const KeyType& key, SDBCursor& locn) {
		lock_.acquire_read_lock();
		bool ret = container_.search(key, locn);
		lock_.release_read_lock();
		return ret;
	}

	/**
	 *   \brief get the cursor for given key
	 *
	 *   @return locn is cursor of key if key exist, otherwise return default value.			 *
	 *
	 */
	SDBCursor search(const KeyType& key) {
		SDBCursor locn;
		search(key, locn);
		return locn;
	}

	/**
	 *  \brief get the item of given Locn.	 *
	 */
	bool get(const SDBCursor& locn, KeyType& key, ValueType& value) {
		lock_.acquire_read_lock();
		bool ret = container_.get(locn, key, value);
		lock_.release_read_lock();
		return ret;
	}

	/**
	 *  \brief get an item of given Locn.	 *
	 */
	bool get(const SDBCursor& locn, DataType<KeyType,ValueType> & dat) {
		lock_.acquire_read_lock();
		bool ret = container_.get(locn, dat);
		lock_.release_read_lock();
		return ret;
	}

	/**
	 *	\brief It determines if an item exists in SequentialDB.
	 *
	 */
	bool hasKey(const KeyType& key);

	/**
	 *
	 *    \brief It gets the number of the items in  SequentialDB.
	 *
	 */
	int numItems();
	/**
	 * 	\brief It display some infomation of SequentialDB.
	 */
	void display(std::ostream& os = std::cout) {
		lock_.acquire_read_lock();
		container_.display(os);
		lock_.release_read_lock();
	}
	/**
	 * 	\brief clear the memeory and write the dirty page back to disk.
	 */
	void flush() {
		lock_.acquire_write_lock();
		container_.flush();
		lock_.release_write_lock();
	}

	/// Note that,  getnext, getPrev, getNearest, getValueForwar,getValueBackWard, getValuePrefix, getValueBetween
	/// only for BTree

	/**
	 *   \brief get the  next key
	 */
	KeyType getNext(const KeyType& key) {
		SDBCursor locn;
		DataType<KeyType,ValueType> dat;
		KeyType rk;
		lock_.acquire_read_lock();
		if (container_.search(key, locn) ) {
			if (container_.seq(locn, ESD_FORWARD) ) {
				get(locn, dat);
			}
			lock_.release_read_lock();
			return dat.key;
		} else {
			container_.get(locn, dat);
			rk = dat.get_key();
			lock_.release_read_lock();
			return rk;
		}
	}

	/**
	 *   \brief get the previous key
	 */
	KeyType getPrev(const KeyType& key) {
		SDBCursor locn;
		DataType<KeyType,ValueType> dat;
		KeyType rk;
		lock_.acquire_read_lock();
		container_.search(key, locn);
		if (container_.seq(locn, ESD_BACKWARD) ) {
			container_.get(locn, dat);
			rk = dat.get_key();
		}
		lock_.release_read_lock();
		return rk;
	}

	/**
	 *  if input key exists, get the key itself, otherwise get the smallest
	 *  key that bigger than input key.
	 *
	 */
	KeyType getNearest(const KeyType& key) {
		SDBCursor locn;
		lock_.acquire_read_lock();
		container_.search(key, locn);
		DataType<KeyType,ValueType> dat;
		container_.get(locn, dat);
		KeyType rk = dat.get_key();
		lock_.release_read_lock();
		return rk;
	}

	/**
	 *  \brief It reads count items from SequentialD  start with key foward.
	 *
	 * 	@param key starting index, if it is "", it reads from the minKey.
	 * 	@param count the number of the keys read, if there is not enough items, stop early.
	 *  @param result read the items to result.
	 *	@return TRUE if all get the items, otherwise return faulse
	 */
	bool getValueForward(const int count,
			vector<DataType<KeyType,ValueType> >& result, const KeyType& key);
	/**
	 *   \brief It reads count items from SequentialD  start with the first key.
	 */
	bool getValueForward(int count, vector<DataType<KeyType,ValueType> >& result) {
		KeyType key;
		return getValueForward(count, result, key);
	}

	/**
	 *  \brief It reads count items from SequentialD  start with key backfoward.
	 *
	 * 	@param key starting index, if it it is "" , it reads from the maxkey.
	 * 	@param count the number of the keys read, if there is not enough items, stop early.
	 *  @param result read the items to result.
	 *	@return TRUE if all get the items, otherwise return false
	 */

	bool getValueBackward(const int count,
			vector<DataType<KeyType,ValueType> >& result, const KeyType& key);

	/**
	 *   \brief It reads count items from SequentialDB backward from the last key.
	 */
	bool getValueBackward(int count, vector<DataType<KeyType,ValueType> >& result) {
		KeyType key;
		return getValueBackward(count, result, key);
	}

	/**
	 * 	\brief get value with key between lowKey and highkey, including value with lowkey and highKey.
	 *
	 */

	bool getValueBetween(vector<DataType<KeyType,ValueType> >& result,
			const KeyType& lowKey, const KeyType& highKey);

	/**
	 *  \brief get all the values with prefix equal to given key.
	 *
	 */
	void getValuePrefix(const KeyType& key,
			vector<DataType<KeyType,ValueType> >& result) {
		SDBCursor locn;
		search(key, locn);
		DataType<KeyType,ValueType> dat;
		lock_.acquire_read_lock();
		while (key.isPrefix(dat.get_key()) && container_.seq(locn, ESD_FORWARD) ) {
			if (get(locn, dat) )
				result.push_back(dat);
		}
		lock_.release_read_lock();
	}

	void getValuePrefix(const KeyType& key, vector<KeyType>& result) {
		SDBCursor locn;
		search(key, locn);
		DataType<KeyType,ValueType> dat;
		get(locn, dat);
		lock_.acquire_read_lock();
		while (key.isPrefix(dat.get_key()) && container_.seq(locn, ESD_FORWARD) ) {
			if (get(locn, dat))
				result.push_back(dat.get_key());
		}
		lock_.release_read_lock();
	}

	ContainerType& getContainer() {
		return container_;
	}
private:
	ContainerType container_;
	LockType lock_; // for multithread access.
	izenelib::am::CompareFunctor<KeyType> comp_;
};

template<typename KeyType, typename ValueType, typename LockType,
		typename ContainerType, typename Alloc> bool SequentialDB< KeyType,
		ValueType, LockType, ContainerType, Alloc>::getValue(
		const KeyType& key, ValueType& val) {
	lock_.acquire_read_lock();
	bool ret = container_.get(key, val);
	if (ret) {
		lock_.release_read_lock();
		return true;
	}
	lock_.release_read_lock();
	return false;
}

template<typename KeyType, typename ValueType, typename LockType,
		typename ContainerType, typename Alloc> bool SequentialDB< KeyType,
		ValueType, LockType, ContainerType, Alloc>::insertValue(
		const DataType<KeyType,ValueType> & data) {
	lock_.acquire_write_lock();
	bool ret = container_.insert(data);
	lock_.release_write_lock();
	return ret;
}

template<typename KeyType, typename ValueType, typename LockType,
		typename ContainerType, typename Alloc> bool SequentialDB< KeyType,
		ValueType, LockType, ContainerType, Alloc>::del(const KeyType& key) {
	lock_.acquire_write_lock();
	bool ret = container_.del(key);
	lock_.release_write_lock();
	return ret;
}

template<typename KeyType, typename ValueType, typename LockType,
		typename ContainerType, typename Alloc> bool SequentialDB< KeyType,
		ValueType, LockType, ContainerType, Alloc>::update(
		const DataType<KeyType,ValueType> & data) {
	lock_.acquire_write_lock();
	bool ret = container_.update(data);
	lock_.release_write_lock();
	return ret;
}

template<typename KeyType, typename ValueType, typename LockType,
		typename ContainerType, typename Alloc> bool SequentialDB< KeyType,
		ValueType, LockType, ContainerType, Alloc>::hasKey(const KeyType& key) {
	lock_.acquire_read_lock();
	ValueType* pv = container_.find(key);
	bool ret = (pv != NULL );
	lock_.release_read_lock();
	return ret;

}

template<typename KeyType, typename ValueType, typename LockType,
		typename ContainerType, typename Alloc> int SequentialDB< KeyType,
		ValueType, LockType, ContainerType, Alloc>::numItems() {
	lock_.acquire_read_lock();
	int num = container_.num_items();
	lock_.release_read_lock();
	return num;
}

//including the current item.
template<typename KeyType, typename ValueType, typename LockType,
		typename ContainerType, typename Alloc> bool SequentialDB< KeyType,
		ValueType, LockType, ContainerType, Alloc>::getValueForward(
		const int count, vector<DataType<KeyType,ValueType> >& result,
		const KeyType& key) {

	DataType<KeyType,ValueType> rec;
	int i =0;
	SDBCursor locn;
	lock_.acquire_read_lock();
	container_.search(key, locn);
	if (container_.get(locn, rec)) {
		result.push_back(rec);
		i++;
	}
	for (; i<count; i++) {
		if (container_.seq(locn) ) {
			//locn.first->display();
			if (get(locn, rec) )
				result.push_back(rec);
		} else {
			lock_.release_read_lock();
			return false;
		}
	}
	lock_.release_read_lock();
	return 1;

}

template<typename KeyType, typename ValueType, typename LockType,
		typename ContainerType, typename Alloc> bool SequentialDB< KeyType,
		ValueType, LockType, ContainerType, Alloc>::getValueBackward(
		const int count, vector<DataType<KeyType,ValueType> >& result,
		const KeyType& key) {
	DataType<KeyType,ValueType> rec;
	int i=0;
	SDBCursor locn;
	KeyType temp = getPrev(key);
	lock_.acquire_read_lock();
	if (container_.search(key, locn) ) {
		if (container_.get(locn, rec)) {
			result.push_back(rec);
			i++;
		}
	} else {
		if ( !container_.search(temp, locn) ) {
			lock_.release_read_lock();
			return false;
		} else {
			if (container_.get(locn, rec)) {
				result.push_back(rec);
				i++;
			}
		}
	}
	for (; i<count; i++) {
		if (container_.seq(locn, ESD_BACKWARD) ) {
			if (get(locn, rec))
				result.push_back(rec);
		} else {
			lock_.release_read_lock();
			return false;
		}
	}
	lock_.release_read_lock();
	return 1;

}

template<typename KeyType, typename ValueType, typename LockType,
		typename ContainerType, typename Alloc> bool SequentialDB< KeyType,
		ValueType, LockType, ContainerType, Alloc>::getValueBetween(
		vector<DataType<KeyType,ValueType> >& result, const KeyType& lowKey,
		const KeyType& highKey) {
	if (comp_(lowKey, highKey)> 0) {
		return false;
	}
	SDBCursor locn;
	DataType<KeyType,ValueType> rec;

	lock_.acquire_read_lock();
	container_.search(lowKey, locn);
	if (container_.get(locn, rec)) {
		if (comp_(rec.get_key(), highKey) <= 0) {
			result.push_back(rec);
		} else
			return true;
	}

	//if lowKey not exist in database, it starts from the lowest key.
	//container_.search(newLowKey, locn);
	do {
		if (container_.seq(locn, ESD_FORWARD) ) {
			if (get(locn, rec) ) {
				if (comp_(rec.get_key(), highKey) <= 0) {
					result.push_back(rec);
				} else {
					break;
				}
			}
		} else {
			lock_.release_read_lock();
			return false;
		}
	} while (true);
	lock_.release_read_lock();
	return true;
}

template< typename KeyType =string, typename ValueType=NullType,
		typename LockType =NullLock > class unordered_sdb_fixed :
	public SequentialDB<KeyType, ValueType, LockType, sdb_fixedhash<KeyType, ValueType, LockType> > {
public:
	unordered_sdb_fixed(const string& sdbname) :
		SequentialDB<KeyType, ValueType, LockType,
				sdb_fixedhash<KeyType, ValueType, LockType> >(sdbname) {

	}
};

template< typename KeyType =string, typename ValueType=NullType,
		typename LockType =NullLock > class unordered_sdb_1 :
	public SequentialDB<KeyType, ValueType, LockType, tc_hash<KeyType, ValueType, LockType> > {
public:
	unordered_sdb_1(const string& sdbname) :
		SequentialDB<KeyType, ValueType, LockType,
				tc_hash<KeyType, ValueType, LockType> >(sdbname) {

	}
};

template< typename KeyType =string, typename ValueType=NullType,
		typename LockType =NullLock > class unordered_sdb :
	public SequentialDB<KeyType, ValueType, LockType, sdb_hash<KeyType, ValueType, LockType> > {
public:
	unordered_sdb(const string& sdbname) :
		SequentialDB<KeyType, ValueType, LockType,
				sdb_hash<KeyType, ValueType, LockType> >(sdbname) {

	}
};

template< typename KeyType =string, typename ValueType=NullType,
		typename LockType =NullLock > class ordered_sdb :
	public SequentialDB<KeyType, ValueType, LockType, sdb_btree<KeyType, ValueType, LockType> > {
public:
	ordered_sdb(const string& sdbname) :
		SequentialDB<KeyType, ValueType, LockType,
				sdb_btree<KeyType, ValueType, LockType> >(sdbname) {

	}
};

}

}
#endif /*SEQUENTIALDB_H_*/
