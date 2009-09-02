/**
 * @file sdb_hash.h
 * @brief The header file of sdb_hash.
 * @author peisheng wang
 *
 * @history
 * ==========================
 * 1. 2009-02-16 first version.
 *
 *
 * This file defines class sdb_hash.
 */
#ifndef SDB_HASH_H
#define SDB_HASH_H

#include <string>
#include <queue>
#include <map>
#include <iostream>
#include <types.h>
#include <sys/stat.h>
#include <util/ClockTimer.h>

//#include <util/log.h>
#include <stdio.h>

#include "sdb_hash_types.h"
#include "sdb_hash_header.h"
#include "bucket_chain.h"

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

template< typename KeyType, typename ValueType, typename LockType =NullLock> class sdb_hash :
public AccessMethod<KeyType, ValueType, LockType>
{
	enum {unloadByRss = false};
	enum {unloadAll = false};
	enum {orderedCommit =true};
	enum {delayFlush = true};
	enum {quickFlush = false};
public:
	//SDBCursor is like db cursor
	typedef bucket_chain_<LockType> bucket_chain;
	typedef std::pair<bucket_chain*, char*> SDBCursor;
public:
	/**
	 *   constructor
	 */
	sdb_hash(const string& fileName = "sdb_hash.dat"):sfh_(), fileName_(fileName) {
		directorySize_ = (1<<sfh_.dpow);
		dmask_ = directorySize_ - 1;
		dataFile_ = 0;
		isOpen_ = false;
		activeNum_ = 0;
		dirtyPageNum_ = 0;
		cacheSize_ = 0;

		if(unloadByRss)
		{
			unsigned long vm = 0;
			unsigned long rlimit;
			ProcMemInfo::getProcMemInfo(vm, initRss_, rlimit);
		}
	}

	/**
	 *   deconstructor, close() will also be called here.
	 */
	virtual ~sdb_hash() {
		if(dataFile_)
		close();
	}

	bool is_open() {
		return isOpen_;
	}
	/**
	 *  \brief set bucket size of fileHeader
	 *
	 *   if not called use default size 8192
	 */
	void setBucketSize(size_t bucketSize) {
		assert(isOpen_ == false);
		sfh_.bucketSize = bucketSize;
	}

	/**
	 *  \brief set bucket size of fileHeader
	 *
	 *   if not called use default size 8192
	 */
	void setPageSize(size_t pageSize) {
		setBucketSize( pageSize );
	}

	/**
	 *  set directory size if fileHeader
	 *
	 *  if not called use default size 4096
	 */
	/*void setDirectorySize(size_t dirSize) {
	 assert(isOpen_ == false);
	 sfh_.directorySize = dirSize;
	 }*/

	void setDegree(size_t dpow) {
		assert(isOpen_ == false);
		sfh_.dpow = dpow;
		directorySize_ = (1<<dpow);
		dmask_ = directorySize_ - 1;

	}

	/**
	 *  set cache size, if not called use default size 100000
	 */
	void setCacheSize(size_t cacheSize)
	{
		sfh_.cacheSize = cacheSize;
		cacheSize_ = cacheSize;
		//if(sfh_.cacheSize < directorySize_)
		//sfh_.cacheSize = directorySize_;
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
	bool insert(const DataType<KeyType,ValueType>& dat) {
		return insert(dat.get_key(), dat.get_value() );
	}

	/**
	 *  insert an item in key/value pair
	 */
	bool insert(const KeyType& key, const ValueType& value) {

		SDBCursor locn;
		if( search(key, locn) )
		return false;
		else {
			char* ptr = 0;
			char* ptr1 = 0;
			size_t ksize;
			size_t vsize;
			izene_serialization<KeyType> izs(key);
			izene_serialization<ValueType> izs1(value);
			izs.write_image(ptr, ksize);
			izs1.write_image(ptr1, vsize);

			bucket_chain* sa = locn.first;
			char* p = locn.second;

			//entry_[idx] is even NULL.
			if(locn.first == NULL) {
				uint32_t idx = sdb_hashing::hash_fun(ptr, ksize) & dmask_;;

				if (bucketAddr[idx] == 0) {
					entry_[idx] = allocateBlock_();
					bucketAddr[idx] = entry_[idx]->fpos;

					sa = entry_[idx];
					p = entry_[idx]->str;
				}
			}
			else
			{
				assert(locn.second != NULL);
				size_t gap = sizeof(long)+sizeof(int)+ksize+vsize+2*sizeof(size_t);

				//add an extra size_t to indicate if reach the end of  bucket_chain.
				if ( size_t(p - sa->str)> sfh_.bucketSize-gap-sizeof(size_t) ) {
					if (sa->next == 0) {
						//sa->isDirty = true;
						setDirty_(sa);
						sa->next = allocateBlock_();
					}
					sa = loadNext_( sa );
					p = sa->str;
				}
			}

			memcpy(p, &ksize, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(p, &vsize, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(p, ptr, ksize);
			p += ksize;
			memcpy(p, ptr1, vsize);
			p += vsize;

			assert( size_t (p-sa->str) + sizeof(long) + sizeof(int) <= sfh_.bucketSize);

			//sa->isDirty = true;
			setDirty_(sa);
			sa->num++;
			++sfh_.numItems;
			return true;
		}

	}

	/**
	 *  find an item, return pointer to the value.
	 *  Note that, there will be memory leak if not delete the value
	 */
	ValueType* find(const KeyType & key) {

		SDBCursor locn;
		if( !search(key, locn) )
		return NULL;
		else {
			char *p = locn.second;
			size_t ksz, vsz;
			memcpy(&ksz, p, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(&vsz, p, sizeof(size_t));
			p += sizeof(size_t);

			ValueType *val = new ValueType;
			izene_deserialization<ValueType> isd(p+ksz, vsz);
			isd.read_image(*val);
			return val;
		}
	}

	bool get(const KeyType& key, ValueType& value)
	{
		SDBCursor locn;
		if( !search(key, locn) )
		return false;
		else {
			char *p = locn.second;
			size_t ksz, vsz;
			memcpy(&ksz, p, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(&vsz, p, sizeof(size_t));
			p += sizeof(size_t);

			izene_deserialization<ValueType> isd(p+ksz, vsz);
			isd.read_image(value);
			return true;
		}
	}

	/**
	 *  delete  an item
	 */
	bool del(const KeyType& key) {

		SDBCursor locn;
		if( !search(key, locn) )
		return false;
		else
		{
			char *p = locn.second;
			size_t ksz, vsz;
			memcpy(&ksz, p, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(&vsz, p, sizeof(size_t));
			p += sizeof(size_t);
			memset(p, 0xff, ksz);

			//this is much slower.
			/*vsz += ksz+1;
			 ksz = -1;
			 memcpy(p-2*sizeof(size_t), &ksz, sizeof(size_t) );
			 memcpy(p-sizeof(size_t), &vsz, sizeof(size_t) );*/

			//locn.first->isDirty = true;
			setDirty_(locn.first);
			--sfh_.numItems;
			return true;
		}

	}

	/**
	 *  update  an item through DataType data
	 */
	bool update(const DataType<KeyType,ValueType>& dat)
	{
		return update( dat.get_key(), dat.get_value() );
	}

	/**
	 *  update  an item by key/value pair
	 */
	bool update(const KeyType& key, const ValueType& value) {
		SDBCursor locn;
		if( !search(key, locn) )
		return insert(key, value);
		else
		{
			char* ptr;
			char* ptr1;
			size_t ksize;
			size_t vsize;
			izene_serialization<KeyType> izs(key);
			izene_serialization<ValueType> izs1(value);
			izs.write_image(ptr, ksize);
			izs1.write_image(ptr1, vsize);

			bucket_chain* sa = locn.first;
			char *p = locn.second;
			size_t ksz, vsz;
			memcpy(&ksz, p, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(&vsz, p, sizeof(size_t));
			p += sizeof(size_t);
			if(vsz == vsize)
			{
				//sa->isDirty = true;
				setDirty_(sa);
				memcpy(p+ksz, ptr1, vsz);
				return true;
			}
			else
			{
				//sa->isDirty = true;
				setDirty_(sa);
				//delete it first!
				memset(p, 0xff, ksize);
				return insert(key, value);
			}
			return true;
		}
	}

	template<typename AM>
	bool dump(AM& other) {
		if (!is_open() )
		open();
		if( !other.is_open() )
		{
			if( !other.open() )
			return false;
		}
		SDBCursor locn = get_first_locn();
		KeyType key;
		ValueType value;	
		while( seq(locn, key, value) ) {				
			if( !other.insert(key, value) )
			return false;;
		}
		return true;
	}

	bool dump2f(const string& fileName)
	{
		sdb_hash other(fileName);
		if( !other.open() )
		return false;
		return dump( other );
	}

	/**
	 *  search an item
	 *
	 *   @return SDBCursor
	 */
	SDBCursor search(const KeyType& key)
	{
		SDBCursor locn;
		search(key, locn);
		return locn;
	}

	/**
	 *    another search function, flushCache_() will be called at the beginning,
	 *
	 */
	bool search(const KeyType &key, SDBCursor &locn)
	{
		flushCache_();

		locn.first = NULL;
		locn.second = NULL;

		char* ptr=0;
		size_t ksize;
		izene_serialization<KeyType> izs(key);
		izs.write_image(ptr, ksize);

		uint32_t idx = sdb_hashing::hash_fun(ptr, ksize) & dmask_;
		locn.first = entry_[idx];

		if (entry_[idx] == NULL) {
			return false;
		} else {
			int i = 0;
			bucket_chain* sa = entry_[idx];

			if( !sa->isLoaded ) {
				entry_[idx]->read(dataFile_);
				++activeNum_;
			}

			char* p = sa->str;

			while ( sa ) {
				locn.first = sa;
				//cout<<"search level: "<<sa->level<<endl;
				p = sa->str;
				//if( !p )return false;
				for (i=0; i<sa->num; i++) {
					size_t ksz, vsz;
					memcpy(&ksz, p, sizeof(size_t));
					p += sizeof(size_t);
					memcpy(&vsz, p, sizeof(size_t));
					p += sizeof(size_t);

					//cout<<ksz<<endl;
					//cout<<vsz<<endl;

					if (ksz != ksize) {
						p += ksz + vsz;
						continue;
					}

					char *pd = ptr;
					size_t j=0;
					for (; j<ksz; j++) {
						//cout<<pd[j]<<" vs "<<p[j]<<endl;
						if (pd[j] != p[j]) {
							break;
						}
					}

					if (j == ksz) {
						locn.second = p-2*sizeof(size_t);
						//cout<<key<<" found"<<endl;
						return true;
					}
					p += ksz + vsz;
				}
				sa = loadNext_(sa);
			}
			locn.second = p;
		}
		return false;
	}

	/**
	 *  get the SDBCursor of first item in the first not empty bucket.
	 */
	SDBCursor get_first_locn()
	{
		SDBCursor locn;
		for(size_t i=0; i<directorySize_; i++)
		{
			if( entry_[i] )
			{
				if (!entry_[i]->isLoaded ) {
					entry_[i]->read(dataFile_);
					++activeNum_;
				}
				locn.first = entry_[i];
				locn.second = entry_[i]->str;
				break;
			}
		}
		return locn;
	}

	bool get(const SDBCursor& locn, KeyType& key, ValueType& value)
	{
		DataType<KeyType,ValueType> dat;
		bool ret =get(locn, dat);
		if(ret) {
			key = dat.get_key();
			value = dat.get_value();
		}
		return ret;
	}
	/**
	 *  get an item from given SDBCursor
	 */
	bool get(const SDBCursor& locn, DataType<KeyType,ValueType>& rec) {

		bucket_chain* sa = locn.first;
		char* p = locn.second;

		if(sa == NULL)return false;
		if(p == NULL)return false;

		size_t ksize, vsize;
		//DbObjPtr ptr, ptr1;
		//char* ptr, ptr1;

		KeyType key;
		ValueType val;

		memcpy(&ksize, p, sizeof(size_t));
		p += sizeof(size_t);
		memcpy(&vsize, p, sizeof(size_t));
		p += sizeof(size_t);

		izene_deserialization<KeyType> izs(p, ksize);
		izs.read_image(key);
		p += ksize;

		izene_deserialization<ValueType> izs1(p, vsize);
		izs1.read_image(val);
		p += vsize;

		rec.key = key;
		rec.value = val;

		return true;

	}

	/**
	 *   \brief sequential access method
	 *
	 *   @param locn is the current SDBCursor, and will replaced next SDBCursor when route finished.
	 *   @param rec is the item in SDBCursor locn.
	 *   @param sdir is sequential access direction, for hash is unordered, we only implement forward case.
	 *
	 */

	bool seq(SDBCursor& locn, DataType<KeyType,ValueType>& rec, ESeqDirection sdir=ESD_FORWARD)
	{
		return seq(locn, rec.key, rec.value, sdir);
	}

	bool seq(SDBCursor& locn, KeyType& key, ValueType& val, ESeqDirection sdir=ESD_FORWARD) {
		flushCache_(locn);
		if( sdir == ESD_FORWARD ) {
			bucket_chain* sa = locn.first;
			char* p = locn.second;

			if(sa == NULL)return false;
			if(p == NULL)return false;

			size_t ksize, vsize;
			char* ptr;
			size_t poff;

			while(true) {

				memcpy(&ksize, p, sizeof(size_t));
				p += sizeof(size_t);
				memcpy(&vsize, p, sizeof(size_t));
				p += sizeof(size_t);

				bool isContinue = false;
				//to determine if encountered item is deleted.
				char *a = new char[ksize];
				memset(a, 0xff, ksize);
				if(memcmp(p, a, ksize) == 0) {
					delete a;
					a = 0;
					isContinue = true;
				}
				delete a;
				a = 0;

				ptr = p;
				poff = ksize;
				izene_deserialization<KeyType> izd(p, ksize);
				izd.read_image(key);
				p += ksize;

				izene_deserialization<ValueType> izd1(p, vsize);
				izd1.read_image(val);
				p += vsize;

				memcpy(&ksize, p, sizeof(size_t));
				if( ksize == 0 ) {
					sa = loadNext_(sa);
					if( sa ) {
						p = sa->str;
					}
					else
					{
						uint32_t idx = sdb_hashing::hash_fun(ptr, poff) & dmask_;
						while( !entry_[++idx] ) {
							if( idx >= directorySize_-1 )
							break;
							//return false;
						}

						if (entry_[idx] && !entry_[idx]->isLoaded ) {
							entry_[idx]->read(dataFile_);
							++activeNum_;
						}

						//get next bucket;
						sa = entry_[idx];
						if( sa ) p = sa->str;
						else
						p = NULL;
					}
				}

				if( isContinue )continue;

				//cout<<"!!!! seq "<<key<<endl;
				//rec.key = key;
				//rec.value = val;
				locn.first = sa;
				locn.second = p;

				return true;

			}
		} else
		{
			//it seems unecessary, for items are unordered.
			return false;
		}
	}

	/**
	 *   get the num of items
	 */
	int num_items() {
		return sfh_.numItems;
	}

public:
	/**
	 *   db must be opened to be used.
	 */
	bool open() {

		if(isOpen_) return true;
		struct stat statbuf;
		bool creating = stat(fileName_.c_str(), &statbuf);

		dataFile_ = fopen(fileName_.c_str(), creating ? "w+b" : "r+b");
		if ( 0 == dataFile_) {
			cout<<"Error in open(): open file failed"<<endl;
			return false;
		}
		bool ret = false;
		if (creating) {

			// We're creating if the file doesn't exist.
#ifdef DEBUG
			cout<<"creating...\n"<<endl;
			sfh_.display();
#endif
			bucketAddr = new long[directorySize_];
			entry_ = new bucket_chain*[directorySize_];

			//initialization
			memset(bucketAddr, 0, sizeof(long)*directorySize_);
			memset(entry_ , 0, sizeof(bucket_chain*)*directorySize_);

			sfh_.toFile(dataFile_);
			ret = true;
		} else {
			if ( !sfh_.fromFile(dataFile_) ) {
				return false;
			} else {
				if (sfh_.magic != 0x061561) {
					cout<<"Error, read wrong file header_\n"<<endl;
					return false;
				}
				if(cacheSize_ != 0)
				sfh_.cacheSize = cacheSize_;
#ifdef DEBUG
				cout<<"open exist...\n"<<endl;
				sfh_.display();
#endif
				directorySize_ = (1<<sfh_.dpow);
				dmask_ = directorySize_ - 1;

				bucketAddr = new long[directorySize_];
				entry_ = new bucket_chain*[directorySize_];
				memset(bucketAddr, 0, sizeof(long)*directorySize_);
				memset(entry_ , 0, sizeof(bucket_chain*)*directorySize_);

				if (directorySize_ != fread(bucketAddr, sizeof(long),
								directorySize_, dataFile_))
				return false;

				for (size_t i=0; i<directorySize_; i++) {
					if (bucketAddr[i] != 0) {
						entry_[i] = new bucket_chain(sfh_.bucketSize, fileLock_);
						entry_[i]->fpos = bucketAddr[i];
						//entry_[i]->read(dataFile_);
						//activeNum_++;
					}
				}
				ret = true;
			}
		}
		isOpen_ = true;
		return ret;
	}
	/**
	 *   db should be closed after open, and  it will automatically called in deconstuctor.
	 */
	bool close()
	{
		flush();
		/*	for (size_t i=0; i<directorySize_; i++)
		 {
		 while(entry_[i])
		 {
		 bucket_chain* sc = entry_[i]->next;
		 delete entry_[i];
		 entry_[i] = 0;
		 entry_[i] = sc;
		 }

		 }
		 delete [] entry_;
		 entry_ = 0;*/

		delete [] bucketAddr;
		bucketAddr = 0;
		fclose(dataFile_);
		dataFile_ = 0;
		return true;
	}
	/**
	 *  write the dirty buckets to disk, not release the memory
	 *
	 */
	void commit() {
		sfh_.toFile(dataFile_);
		if (directorySize_ != fwrite(bucketAddr, sizeof(long),
						directorySize_, dataFile_) )
		return;
		if (orderedCommit) {
			typedef map<long, bucket_chain*> COMMIT_MAP;
			typedef typename COMMIT_MAP::iterator CMIT;
			COMMIT_MAP toBeWrited;
			queue<bucket_chain*> qnode;
			for (size_t i=0; i<directorySize_; i++) {
				qnode.push( entry_[i]);
			}
			while (!qnode.empty() ) {
				bucket_chain* popNode = qnode.front();
				if ( popNode && popNode->isLoaded && popNode-> isDirty)
				toBeWrited.insert(make_pair(popNode->fpos, popNode) );
				qnode.pop();
				if (popNode && popNode->next ) {
					qnode.push( popNode->next );
				}
			}

			CMIT it = toBeWrited.begin();
			for (; it != toBeWrited.end(); it++) {
				if ( it->second->write( dataFile_ ) )
				--dirtyPageNum_;
			}

		}
		else {

			for (size_t i=0; i<directorySize_; i++) {
				bucket_chain* sc = entry_[i];
				while (sc) {
					if ( sc->write(dataFile_) ) {
						sc = sc->next;
					} else {
						//sc->display();
						assert(0);
					}
				}
			}
		}
		fflush(dataFile_);
	}
	/**
	 *   Write the dirty buckets to disk and also free up most of the memory.
	 *   Note that, for efficieny, entry_[] is not freed up.
	 */
	void flush() {
#ifdef DEBUG
		izenelib::util::ClockTimer timer;
#endif
		commit();
#ifdef DEBUG
		printf("commit elapsed 1 ( actually ): %lf seconds\n",
				timer.elapsed() );
#endif
		unload_();
		if (unloadByRss) {
			unsigned long vm = 0;
			unsigned long rlimit;
			ProcMemInfo::getProcMemInfo(vm, initRss_, rlimit);
		}
	}

	void unload_() {
		for (size_t i=0; i<directorySize_; i++) {
			if (entry_[i]) {
				//bucket_chain* sc = entry_[i]->next;
				bucket_chain* sc = entry_[i];
				bucket_chain* sa = 0;
				while (sc) {
					sa = sc->next;
					if (sc) {
						if (sc->unload())
						--activeNum_;
						//delete sc;
						//sc = 0;
					}
					sc = sa;
				}
			}
		}

	}
	/**
	 *  display the info of sdb_hash
	 */
	void display(std::ostream& os = std::cout, bool onlyheader = true) {
		sfh_.display(os);
		os<<"activeNum: "<<activeNum_<<endl;
		os<<"dirtyPageNum "<<dirtyPageNum_<<endl;
		//os<<"loadFactor: "<<loadFactor()<<endl;

		if ( !onlyheader) {
			for (size_t i=0; i<directorySize_; i++) {
				os<<"["<<i<<"]: ";
				if (entry_[i])
				entry_[i]->display(os);
				os<<endl;
			}
		}

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
	double loadFactor() {
		int nslot = 0;
		for (size_t i=0; i<directorySize_; i++) {
			bucket_chain* sc = entry_[i];
			while (sc) {
				nslot += sc->num;
				sc = loadNext_(sc);
			}
		}
		if (nslot == 0)
		return 0.0;
		else
		return double(sfh_.numItems)/nslot;

	}

protected:
	bucket_chain** entry_;

	//bucketAddr stores fpos for entry_ and it was store in disk after fileHeader.
	long *bucketAddr;

	//levle->bucket_chain* map, used for caching
	multimap<int, bucket_chain*, greater<int> > sh_cache_;
	typedef typename multimap<int, bucket_chain*, greater<int> >::iterator CacheIter;

private:
	ShFileHeader sfh_;
	string fileName_;
	FILE* dataFile_;
	bool isOpen_;

	unsigned int activeNum_;
	unsigned int dirtyPageNum_;
	LockType fileLock_;
private:
	size_t directorySize_;
	size_t dmask_;
	size_t cacheSize_;

	unsigned long initRss_;
	unsigned int flushCount_;

	void setDirty_(bucket_chain* bucket) {
		if( !bucket->isDirty ) {
			++dirtyPageNum_;
			bucket->isDirty = true;
		}
	}
	/**
	 *   Allocate an bucket_chain element
	 */
	bucket_chain* allocateBlock_() {
		//cout<<"allocateBlock idx="<<sfh_.nBlock<<endl;
		bucket_chain* newBlock;
		newBlock = new bucket_chain(sfh_.bucketSize, fileLock_);
		newBlock->str = new char[sfh_.bucketSize-sizeof(long)-sizeof(int)];
		memset(newBlock->str, 0, sfh_.bucketSize-sizeof(long)-sizeof(int));
		newBlock->isLoaded = true;
		newBlock->isDirty = true;

		newBlock->fpos = sizeof(ShFileHeader) + sizeof(long)*directorySize_
		+ sfh_.bucketSize*sfh_.nBlock;

		++sfh_.nBlock;
		++activeNum_;
		++dirtyPageNum_;

		return newBlock;
	}

	bucket_chain* loadNext_(bucket_chain* current) {
		bool loaded = false;
		bucket_chain* next = current->loadNext(dataFile_, loaded);
		if (loaded)
		activeNum_++;
		return next;
	}

	/**
	 *  when cache is full, it was called to reduce memory usage.
	 *
	 */
	void flushCache_() {
		if (unloadByRss) {
			++flushCount_;

			if ( (flushCount_ & 0xffff) == 0) {

				unsigned long vm = 0, rss;
				unsigned long rlimit;
				ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

				//if( _activeNodeNum> _sfh.cacheSize/1024 )
				if (rss - initRss_> sfh_.cacheSize*sfh_.bucketSize) {
#ifdef DEBUG
					cout<<"\n!!! memory occupied: "<<rss<<" bytes"<<endl;
#endif
					flushCacheImpl_();
#ifdef DEBUG
					ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
					cout<<"\n!!! after unload.  Memory occupied: "<<rss<<" bytes"<<endl;
#endif
				}

				//cout<<activeNum_<<" vs "<<sfh_.cacheSize <<endl;
				//if( activeNum_> sfh_.cacheSize )
				//{
				//	flushCacheImpl_(quickFlush);
				//}
			}
		} else {
			if (activeNum_> sfh_.cacheSize) {
				flushCacheImpl_();
			}
		}
	}

	void flushCache_(SDBCursor &locn) {
		if (unloadByRss) {
			++flushCount_;
			if ( (flushCount_ & 0xffff) == 0) {
				unsigned long vm = 0, rss;
				unsigned long rlimit;
				ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

				if (rss - initRss_> sfh_.cacheSize * sfh_.bucketSize) {
					KeyType key;
					ValueType value;
					get(locn, key, value);
					flushCacheImpl_();
					search(key, locn);
				}
			}
		} else {
			if (activeNum_> sfh_.cacheSize) {
				KeyType key;
				ValueType value;
				get(locn, key, value);
				flushCacheImpl_();
				search(key, locn);
			}
		}

	}

	void flushCacheImpl_() {
#ifdef DEBUG
		cout<<"cache is full..."<<endl;
		sfh_.display();
		cout<<activeNum_<<" vs "<<sfh_.cacheSize <<endl;
		cout<<"dirtyPageNum: "<<dirtyPageNum_<<endl;
#endif
//		cout<<"begin activePageNum"<<activeNum_<<",dirtyPageNum: "<<dirtyPageNum_<<std::endl;

        bool commitCondition = dirtyPageNum_ >= (activeNum_ * 0.5);

		if (unloadAll) {
			flush();
#ifdef DEBUG
			cout<<"\n====================================\n"<<endl;
			cout<<"cache is full..."<<endl;
			sfh_.display();
			cout<<activeNum_<<" vs "<<sfh_.cacheSize <<endl;
			cout<<"dirtyPageNum: "<<dirtyPageNum_<<endl;
#endif
			return;
		} else {

			if( delayFlush && commitCondition)
                commit();
			for (size_t i=0; i<directorySize_; i++) {
				bucket_chain* sc = entry_[i];
				while (sc) {
                    if (sc->isLoaded && !sc->isDirty)
					sh_cache_.insert(make_pair(sc->level, sc));
					sc = sc->next;
				}
			}

			for (CacheIter it = sh_cache_.begin(); it != sh_cache_.end(); it++) {
//				if (quickFlush)
//                    it->second->write(dataFile_);
				if ( it->second->unload() )
                    --activeNum_;
//				if (quickFlush && activeNum_ < max(sfh_.cacheSize/2, directorySize_) ) {
//					fflush(dataFile_);
//					sh_cache_.clear();
//					return;
//				}
			}

            if(delayFlush && commitCondition)
                fflush(dataFile_);
            sh_cache_.clear();
		}
//		cout<<"stop activePageNum"<<activeNum_<<",dirtyPageNum: "<<dirtyPageNum_<<std::endl;
		//cout<<" !!!! "<<activeNum_<<" vs "<<sfh_.cacheSize <<endl;
		//display();
	}

};

NS_IZENELIB_AM_END

#endif
