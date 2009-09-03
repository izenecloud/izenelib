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

#include <boost/optional.hpp>

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
	typedef boost::optional<KeyType> SDBCursor;

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
	bool insert(const DataType<KeyType,ValueType> & dat) {
		return insert(dat.get_key(), dat.get_value() );
	}

	/**
	 *  insert an item in key/value pair
	 */
	bool insert(const KeyType& key, const ValueType& value) {

		char* ptr;
		char* ptr1;
		size_t ksize;
		size_t vsize;
		izene_serialization<KeyType> izs(key);
		izene_serialization<ValueType> izs1(value);
		izs.write_image(ptr, ksize);
		izs1.write_image(ptr1, vsize);

		return tchdbputkeep(hdb_, ptr, ksize, ptr1, vsize);
	}

	/**
	 *  find an item, return pointer to the value.
	 *  Note that, there will be memory leak if not delete the value
	 */
	ValueType* find(const KeyType & key) {
		char* ptr;
		size_t ksize;
		izene_serialization<KeyType> izs(key);
		izs.write_image(ptr, ksize);

		int sp;
		void* value = tchdbget(hdb_, ptr, ksize, &sp);
		if( !value )return NULL;
		else {
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
		void* pv = tchdbget(hdb_, ptr, ksize, &sp);
		if( !pv )return false;
		else {
			izene_deserialization<ValueType> izd((char*)pv, (size_t)sp);
			izd.read_image(value);
			free(pv);
			return true;
		}
	}

	/**
	 *  delete  an item
	 */
	bool del(const KeyType& key) {
		char* ptr;
		size_t ksize;
		izene_serialization<KeyType> izs(key);
		izs.write_image(ptr, ksize);

		//ptr->display();
		return tchdbout(hdb_, ptr, ksize);
	}

	/**
	 *  update  an item through DataType data
	 */
	bool update(const DataType<KeyType,ValueType> & dat)
	{
		return update( dat.get_key(), dat.get_value() );
	}

	/**
	 *  update  an item by key/value pair
	 */
	bool update(const KeyType& key, const ValueType& value) {
		char* ptr;
		char* ptr1;
		size_t ksize;
		size_t vsize;
		izene_serialization<KeyType> izs(key);
		izene_serialization<ValueType> izs1(value);
		izs.write_image(ptr, ksize);
		izs1.write_image(ptr1, vsize);

		return tchdbput(hdb_, ptr, ksize, ptr1, vsize);

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
		return SDBCursor();
	}

	bool get(const SDBCursor& locn, KeyType& key, ValueType& value)
	{
		char* ptr = 0;
		size_t ksize = 0;
		if (locn)
		{
			izene_serialization<KeyType> izs(locn.get());
			izs.write_image(ptr, ksize);
		}

		const char* value_buff = 0;
		int value_size = 0;
		int ret_key_size = 0;
		char* ret_key_buff = tchdbgetnext3(
				hdb_, // handler
				ptr, ksize, // key
				&ret_key_size, // returned key size
				&value_buff, &value_size // returned value
		);

		if (ret_key_buff)
		{
			izene_deserialization<KeyType> izd_key(
					ret_key_buff,
					static_cast<size_t>(ret_key_size)
			);
			izd_key.read_image(key);

			izene_deserialization<ValueType> izd_value(
					value_buff,
					static_cast<size_t>(value_size)
			);
			izd_value.read_image(value);

			tcfree(ret_key_buff);
			return true;
		}

		return false;
	}

	/**
	 *  get an item from given SDBCursor
	 */
	bool get(const SDBCursor& locn, DataType<KeyType,ValueType> & rec) {
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
		if( sdir == ESD_FORWARD) {
			DataType<KeyType,ValueType> rec;
			if (get(locn, rec))
			{
				locn = rec.get_key();
				return true;
			}
			else
			{
				return false;
			}
		} else {
			assert( false);
			return false;
		}
	}

	bool iterInit()
	{
		return tchdbiterinit(hdb_);
	}

	bool iterNext(KeyType& key, ValueType& value)
	{
		TCXSTR* ptcKey = tcxstrnew();
		TCXSTR* ptcValue = tcxstrnew();

		bool b = tchdbiternext3(hdb_, ptcKey, ptcValue);
		if(!b) return false;
		char* cpKey = (char*)tcxstrptr(ptcKey);
		char* cpValue = (char*)tcxstrptr(ptcValue);

		izene_deserialization<KeyType> izdk(cpKey, tcxstrsize(ptcKey));
		izdk.read_image(key);
		izene_deserialization<ValueType> izdv(cpValue, tcxstrsize(ptcValue));
		izdv.read_image(value);
		tcxstrdel(ptcKey);
		tcxstrdel(ptcValue);

		return true;
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
		return tchdbopen(hdb_, fileName_.c_str(), HDBOCREAT | HDBOWRITER);
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
