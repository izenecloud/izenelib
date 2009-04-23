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
	typedef KeyType SDBCursor;
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
		write_image(value, ptr1);

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

		int sp;
		void* value = tchdbget(hdb_, (void*)(ptr->getData()), ptr->getSize(), &sp);
		if( !value )return NULL;
		else {			
			ptr1.reset(new DbObj(value, sp));			
			ValueType *val = new ValueType;			
			read_image(*val, ptr1);		
			free(value);
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
		
		//ptr->display();
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
		write_image(value, ptr1);
		return tchdbput(hdb_, ptr->getData(), ptr->getSize(), ptr1->getData(), ptr1->getSize());

	}

	/**
	 *  search an item
	 * 
	 *   @return SDBCursor
	 */
	SDBCursor search(const KeyType& key)
	{
		SDBCursor cur;
		if( find(key) ) {
			cur = key;
		}
		return cur;
	}

	/**
	 *    another search function, flushCache_() will be called at the beginning,
	 * 
	 */

	bool search(const KeyType&key, SDBCursor& locn)
	{
		if( find(key) ) {
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
		SDBCursor cur;
		return cur;
	}

	bool get(const SDBCursor& locn, KeyType& key, ValueType& value)
	{
		ValueType* pv = find(locn);
		if( pv )
		{
			key = locn;
			value = *pv;
			return true;
		}
		return false;
	}

	/**
	 *  get an item from given SDBCursor
	 */
	bool get(const SDBCursor& locn, DataType& rec) {
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
	bool seq(SDBCursor& locn, DataType& rec, ESeqDirection sdir=ESD_FORWARD) {
		DbObjPtr ptr, ptr1;
		ptr.reset(new DbObj);

		write_image(locn, ptr);
		ValueType* pv;
		pv = find(locn);
		if( pv )
		{
			rec.key = locn;
			rec.value = *pv;
		}
		else
		{
			ptr.reset(new DbObj);
		}
		int sp;
		void *buf;
		buf = tchdbgetnext(hdb_, ptr->getData(), ptr->getSize(),&sp);
		if(buf != NULL)
		{
			ptr1.reset(new DbObj(buf, sp));
			read_image(locn, ptr1);
			return true;
		}
		return false;
	}

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
		commit();
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
	/**
	 *  We can directly process hdb_ by this handle.
	 */
	TCHDB* getHandle() {
		return hdb_;
	}

private:
	string fileName_;
	TCHDB* hdb_;
};

NS_IZENELIB_AM_END

#endif
