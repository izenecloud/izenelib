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
int compare_tc(const char* aptr, int asize, const char* bptr, int bsize, void* op=0) {
	DbObjPtr ptr, ptr1;
	KeyType key, key1;
	izenelib::am::CompareFunctor<KeyType> _comp;
	
	ptr.reset(new DbObj(aptr,asize));
	ptr1.reset(new DbObj(bptr, bsize));
	read_image(key, ptr);
	read_image(key1, ptr1);
	return _comp(key,key1);
}

template< typename KeyType, typename ValueType, typename LockType =NullLock> class tc_btree :
public AccessMethod<KeyType, ValueType, LockType>
{
public:
	//typedef std::pair<bucket_chain*, char*> SDBCursor;
	typedef DataType<KeyType,ValueType> DataType;

public:
	/**
	 *   constructor
	 */
	tc_btree(const string& fileName = "tc_btree.dat"): fileName_(fileName) {
		
	}

	/**
	 *   deconstructor, close() will also be called here.
	 */
	virtual ~tc_btree() {
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

		return tcbdbputkeep(bdb_, ptr->getData(), ptr->getSize(), ptr1->getData(), ptr1->getSize());
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
		value = tcbdbget(bdb_, (void*)(ptr->getData()), ptr->getSize(), &sp);
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

		return tcbdbout(bdb_, ptr->getData(), ptr->getSize() );
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

		return tcbdbput(bdb_, ptr->getData(), ptr->getSize(), ptr1->getData(), ptr1->getSize());

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
	int num_items() {
		return tcbdbrnum(bdb_);
	}

public:
	/**
	 *   db must be opened to be used.
	 */
	bool open(bool setCmpFun = false ) {
		bdb_ = tcbdbnew();
		if(setCmpFun){
			tcbdbsetcmpfunc(bdb_, compare_tc<KeyType>, NULL);
		}
		return tcbdbopen(bdb_, fileName_.c_str(), 15);
	}
	/**
	 *   db should be closed after open, and  it will automatically called in deconstuctor.
	 */
	bool close()
	{
		return tcbdbclose(bdb_);
	}
	/**
	 *  write the dirty buckets to disk, not release the memory
	 *  
	 */
	void commit() {
		tcbdbsync(bdb_);
	}
	/**
	 *   Write the dirty buckets to disk and also free up most of the memory.
	 *   Note that, for efficieny, entry_[] is not freed up.
	 */
	void flush() {
		commit();
	}
	/**
	 *  display the info of tc_btree
	 */
	void display(std::ostream& os = std::cout) {

	}

	TCBDB* getHandle() {
		return bdb_;
	}

private:
	string fileName_;
	TCBDB* bdb_;

};

NS_IZENELIB_AM_END

#endif
