/**
 * @file tc_hash.h
 * @brief wrapp tokyo cabinet's hash for SDB, CacheDB
 * @author peisheng wang 
 * 
 *
 * This file defines class tc_hash.
 */
#ifndef TC_HASH_H
#define TC_HASH_H

#include "tc_types.h"

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
	//typedef std::pair<bucket_chain*, char*> NodeKeyLocn;
	typedef DataType<KeyType,ValueType> DataType;
public:
	/**
	 *   constructor
	 */
	tc_hash(const string& fileName = "tc_hash.dat"): fileName_(fileName) {		
	}

	/**
	 *   deconstructor, close() will also be called here.
	 */
	virtual ~tc_hash() {
		close();
		tchdbdel(hdb_);
	}

	/**
	 *  set cache size, if not called use default size 100000
	 */
	void setCacheSize(size_t cacheSize)
	{
		tchdbsetcache(hdb_, cacheSize);
	}

	/**
	 * 	\brief return the file name of the SequentialDB
	 */
	std::string getFileName() const {
		return fileName_;
	}

	/**
	 *  insert an item of DataType 
	 */
	bool insert(const DataType& dat) {
		return insert(dat.get_key(), dat.get_value() );
	}

	/**
	 *  insert an item in key/value pair
	 */
	bool insert(const KeyType& key, const ValueType& value) {
		DbObjPtr ptr, ptr1;
		ptr.reset(new DbObj);
		ptr1.reset(new DbObj);
		write_image(key, ptr);
		write_image(key, ptr1);

		return tchdbputkeep(hdb_, ptr->getData(), ptr->getSize(), ptr1->getData(), ptr1->getSize());
	}

	/**
	 *  find an item, return pointer to the value.
	 *  Note that, there will be memory leak if not delete the value 
	 */
	ValueType* find(const KeyType & key) {
		DbObjPtr ptr, ptr1;
		ptr.reset(new DbObj);
		write_image(key, ptr);

		void* value =NULL;
		int sp;
		value = tchdbget(hdb_, (void*)(ptr->getData()), ptr->getSize(), &sp);
		if( !value )return NULL;
		else {
			ptr1.reset(new DbObj(value, sp));
			ValueType *val = new ValueType;
			read_image(*val, ptr1);
			return val;
		}
	}

	/**
	 *  delete  an item
	 */
	bool del(const KeyType& key) {
		DbObjPtr ptr;
		ptr.reset(new DbObj);
		write_image(key, ptr);

		return tchdbout(hdb_, ptr->getData(), ptr->getSize() );
	}

	/**
	 *  update  an item through DataType data
	 */
	bool update(const DataType& dat)
	{
		return update( dat.get_key(), dat.get_value() );
	}

	/**
	 *  update  an item by key/value pair
	 */
	bool update(const KeyType& key, const ValueType& value) {
		DbObjPtr ptr, ptr1;
		ptr.reset(new DbObj);
		ptr1.reset(new DbObj);
		write_image(key, ptr);
		write_image(key, ptr1);

		return tchdbput(hdb_, ptr->getData(), ptr->getSize(), ptr1->getData(), ptr1->getSize());

	}

	/**
	 *  search an item
	 * 
	 *   @return NodeKeyLocn
	 */
	//NodeKeyLocn search(const KeyType& key)
	//{
	////	NodeKeyLocn locn;
	//	search(key, locn);
	//	return locn;
	//}

	/**
	 *    another search function, flushCache_() will be called at the beginning,
	 * 
	 */

	//bool search(const KeyType&key, NodeKeyLocn& locn)
	//{
	//	return false;
	//}

	/**
	 *  get the NodeKeyLocn of first item in the first not empty bucket.
	 */

	//NodeKeyLocn get_first_locn()
	//{
	//	
	//}

	//bool get(const NodeKeyLocn& locn, KeyType& key, ValueType& value)
	//{
	//	return false;
	//}
	/**
	 *  get an item from given NodeKeyLocn
	 */
	//bool get(const NodeKeyLocn& locn, DataType& rec) {
	//	
	//	return true;

	//}

	/**
	 *   \brief sequential access method
	 * 
	 *   @param locn is the current NodeKeyLocn, and will replaced next NodeKeyLocn when route finished. 
	 *   @param rec is the item in NodeKeyLocn locn.
	 *   @param sdir is sequential access direction, for hash is unordered, we only implement forward case.
	 *   
	 */
	//bool seq(NodeKeyLocn& locn, DataType& rec, ESeqDirection sdir=ESD_FORWARD) {
	//	return false;

	//}

	/**
	 *   get the num of items 
	 */
	int num_items() {
		return tchdbrnum(hdb_);
	}

public:
	/**
	 *   db must be opened to be used.
	 */
	bool open() {
		hdb_ = tchdbnew();
		return tchdbopen(hdb_, fileName_.c_str(), 15);
	}
	/**
	 *   db should be closed after open, and  it will automatically called in deconstuctor.
	 */
	bool close()
	{
		return tchdbclose(hdb_);
	}
		
	/**
	 *  write the dirty buckets to disk, not release the memory
	 *  
	 */
	void commit() {
		tchdbsync(hdb_);
	}
	/**
	 *   Write the dirty buckets to disk and also free up most of the memory.
	 *   Note that, for efficieny, entry_[] is not freed up.
	 */
	void flush() {
		commit();
	}
	/**
	 *  display the info of tc_hash
	 */
	void display(std::ostream& os = std::cout) {

	}

	TCHDB* getHandle() {
		return hdb_;
	}

private:
	string fileName_;
	TCHDB* hdb_;

};

NS_IZENELIB_AM_END

#endif
