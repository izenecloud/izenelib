/**
 * @file sdb_storage.h
 * @brief The header file of sdb_hash.
 * @author peisheng wang 
 * 
 * @history
 * ==========================
 * 1. 2009-08-06 first version.
 * 
 *
 * This file defines class sdb_hash.
 */
#ifndef SDB_STORAGE_H
#define SDB_STORAGE_H

#include <string>
#include <iostream>
#include <types.h>
#include <sys/stat.h>
#include <stdio.h>


#include "sdb_storage_header.h"
#include "sdb_storage_types.h"

#include <am/sdb_btree/sdb_btree.h>
#include <am/sdb_hash/sdb_hash.h>

using namespace std;

NS_IZENELIB_AM_BEGIN

/**
 *  \brief  file version of array hash using Cache-Conscious Collision Resolution.
 * 
 *  sdb_hash is built on array hash using Cache-Conscious Collision Resolution.
 * 
 *  For file version, there is a little different, each bucket is now a bucket_chain, and each bucket_chain
 *  hash fixed size.
 *   
 * 
 */

template<
		typename KeyType,
		typename ValueType,
		typename LockType =NullLock,
		typename AmType=sdb_btree<KeyType, unsigned int, LockType> 
> class sdb_storage :public AccessMethod<KeyType, ValueType, LockType>
{
public:
	typedef AmType KeyHash;
	typedef typename AmType::SDBCursor SDBCursor;
public:
	/**
	 *   constructor
	 */
	sdb_storage(const string& fileName = "sdb_storage"):ssh_(), fileName_(fileName+"_value.dat"), keyHash_(fileName+ "_key.dat") {
		dataFile_ = 0;
		isOpen_ = false;
	}

	/**
	 *   deconstructor, close() will also be called here.
	 */
	virtual ~sdb_storage() {
		if(dataFile_)
		close();
	}
	/**
	 *  \brief set bucket size of fileHeader
	 * 
	 *   if not called use default size 4096
	 */

	/*void setPageSize(size_t pageSize) {
	 assert(isOpen_ == false);
	 ssh_.pageSize = pageSize;

	 }*/

	/**
	 *  set directory size if fileHeader
	 * 
	 *  if not called use default size 4096
	 */
	void setDegree(size_t dpow) {
		assert(isOpen_ == false);
		keyHash_.setDegree(dpow);
	}

	/**
	 *  set cache size, if not called use default size 100000
	 */
	void setCacheSize(size_t cacheSize)
	{
		ssh_.cacheSize = cacheSize;
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
	bool insert(const KeyType& key, const ValueType& val) {
		SDBCursor locn;
		if(keyHash_.search(key, locn))return false;
		else {
			keyHash_.insert(key, ssh_.nPage);
			return appendValue_(val);
		}
		return true;

	}

	/**
	 *  find an item, return pointer to the value.
	 *  Note that, there will be memory leak if not delete the value 
	 */
	ValueType* find(const KeyType & key) {
		unsigned int npos;
		if( ! keyHash_.get(key, npos) )
		return NULL;
		else {
			ValueType *pv = new ValueType;
			if(readValue_(npos, *pv) )
			return pv;
			else {
				delete pv;
				pv = 0;
				return NULL;
			}

		}

	}

	bool get(const KeyType& key, ValueType& val)
	{
		unsigned int npos;
		if( ! keyHash_.get(key, npos) )
		return false;
		else {
			return readValue_(npos, val);
		}

	}

	/**
	 *  delete  an item
	 */
	bool del(const KeyType& key) {
		return keyHash_.del(key);
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
	bool update(const KeyType& key, const ValueType& val) {
		unsigned int npos;
		if( ! keyHash_.get(key, npos) )
		return insert(key, val);
		else {
			keyHash_.update(key, ssh_.nPage);
			appendValue_(val);
		}
		return true;
	}

	/**
	 *  search an item
	 * 
	 *   @return SDBCursor
	 */
	SDBCursor search(const KeyType& key)
	{
		return keyHash_.search(key);
	}

	/**
	 *    another search function, flushCache_() will be called at the beginning,
	 * 
	 */
	bool search(const KeyType &key, SDBCursor &locn)
	{
		return keyHash_.search(key, locn);
	}

	/**
	 *  get the SDBCursor of first item in the first not empty bucket.
	 */
	SDBCursor get_first_locn()
	{
		return keyHash_.get_first_locn();
	}

	bool get(const SDBCursor& locn, KeyType& key, ValueType& value)
	{
		unsigned int npos;
		bool ret =keyHash_.get(locn, key, npos);
		if(ret) {
			readValue_(npos, value);
		}
		return ret;
	}
	/**
	 *  get an item from given SDBCursor
	 */
	bool get(const SDBCursor& locn, DataType<KeyType,ValueType> & rec) {
		return get(locn, rec.key, rec.value);
	}

	/**
	 *   \brief sequential access method
	 * 
	 *   @param locn is the current SDBCursor, and will replaced next SDBCursor when route finished. 
	 *   @param rec is the item in SDBCursor locn.
	 *   @param sdir is sequential access direction, for hash is unordered, we only implement forward case.
	 *   
	 */
	bool seq(SDBCursor& locn, ESeqDirection sdir=ESD_FORWARD)
	{	
		return keyHash_.seq(locn,  sdir);
	}
	
	bool seq(SDBCursor& locn, KeyType& key, ESeqDirection sdir=ESD_FORWARD)
	{
		unsigned int npos;
		return keyHash_.seq(locn, key, npos, sdir);
	}

	bool seq(SDBCursor& locn, KeyType& key, ValueType& value, ESeqDirection sdir=ESD_FORWARD)
	{
		unsigned int npos;
		if( keyHash_.seq(locn, key, npos, sdir) )
		return readValue_(npos, value);
		else
		return false;
	}

	bool seq(SDBCursor& locn, DataType<KeyType,ValueType> & rec, ESeqDirection sdir=ESD_FORWARD) {
		return seq(locn, rec.key, rec.value, sdir);

	}

	/**
	 *   get the num of items 
	 */
	int num_items() {
		return keyHash_.num_items();
	}

public:
	/**
	 *   db must be opened to be used.
	 */
	bool open() {
		struct stat statbuf;
		bool creating = stat(fileName_.c_str(), &statbuf);

		keyHash_.open();
		dataFile_ = fopen(fileName_.c_str(), creating ? "w+b" : "r+b");
		if ( 0 == dataFile_) {
			LOG(ERROR)<<"Error in open(): open file failed"<<endl;
			return false;
		}
		bool ret = false;
		if (creating) {
			// We're creating if the file doesn't exist.
			DLOG(INFO)<<"creating...\n"<<endl;
			ssh_.display( DLOG(INFO) );
			ssh_.toFile(dataFile_);
			ret = true;
		} else {
			if ( !ssh_.fromFile(dataFile_) ) {
				return false;
			} else {
				if (ssh_.magic != 0x061561) {
					cout<<"Error, read wrong file header_\n"<<endl;
					return false;
				}
				DLOG(INFO)<<"open exist...\n"<<endl;
				ssh_.display( DLOG(INFO) );

			}
		}
		isOpen_ = true;
		return true;
	}
	/**
	 *   db should be closed after open, and  it will automatically called in deconstuctor.
	 */
	bool close()
	{
		keyHash_.flush();
		fclose(dataFile_);
		dataFile_ = 0;
		return true;
	}
	/**
	 *  write the dirty buckets to disk, not release the memory
	 *  
	 */
	void commit() {
		keyHash_.commit();
		fflush(dataFile_);
	}
	/**
	 *   Write the dirty buckets to disk and also free up most of the memory.
	 *   Note that, for efficieny, entry_[] is not freed up.
	 */
	void flush() {
		keyHash_.flush();
		fflush(dataFile_);
	}
	/**
	 *  display the info of sdb_storage
	 */
	void display(std::ostream& os = std::cout, bool onlyheader = true) {
		ssh_.display(os);
		keyHash_.display(os);
		os<<"dataFile loadFactor: " <<loadFactor()<<endl;
	}

	/**	 
	 * 
	 *    \brief It displays how much space has been wasted in percentage after deleting or updates.   
	 *           
	 *  
	 *    when an item is deleted, we don't release its space in disk but set a flag that
	 *    it have been deleted. And it will lead to low efficiency. Maybe we should dump it 
	 * 	  to another files when loadFactor are low.
	 * 
	 */
	double loadFactor()
	{
		if(ssh_.nPage == 0 )return 0.0;
		else
		return double(ssh_.dPage)/ssh_.nPage;

	}

private:	
	SsHeader ssh_;
	string fileName_;
	FILE* dataFile_;
	KeyHash keyHash_;
	bool isOpen_;
	
	
   // map<unsigned int, ValueType> valueHash_;
	

	/**
	 *   Allocate an bucket_chain element 
	 */
	inline bool appendValue_(const ValueType& val) {
		char* ptr = 0;
		size_t vsize;

		izene_serialization<ValueType> izs(val);
		izs.write_image(ptr, vsize);

		if ( 0 != fseek(dataFile_, ssh_.nPage*ssh_.pageSize+sizeof(SsHeader), SEEK_SET) )
		return false;
		if( 1 != fwrite(&vsize, sizeof(size_t), 1, dataFile_) )
		return false;
		if( 1 != fwrite(ptr, vsize, 1, dataFile_) )
		return false;

		ssh_.nPage += (sizeof(size_t)+vsize)/ssh_.pageSize + 1;
		return true;
	}

	inline bool readValue_(unsigned int npos, ValueType& val) {
		if ( 0 != fseek(dataFile_, npos*ssh_.pageSize+sizeof(SsHeader), SEEK_SET) )
		return false;

		size_t vsize;
		if( 1 != fread(&vsize, sizeof(size_t), 1, dataFile_) )
		return false;
		char* ptr = new char[vsize];
		if( 1 != fread(ptr, vsize, 1, dataFile_) )
		return false;

		izene_deserialization<ValueType> izd(ptr, vsize);
		izd.read_image(val);

		delete ptr;
		ptr = 0;
		return true;
	}

	/**
	 *  when cache is full, it was called to reduce memory usage.
	 * 
	 */
	void flushCache_(bool quickFlush = false)
	{

	}

	void flushCache_(SDBCursor &locn)
	{

	}

	void flushCacheImpl_(bool quickFlush = false)
	{

	}

};

NS_IZENELIB_AM_END

#endif
