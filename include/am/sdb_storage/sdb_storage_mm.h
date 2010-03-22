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
#ifndef SDB_STORAGE_MM_H
#define SDB_STORAGE_MM_H

#include <string>
#include <iostream>
#include <types.h>
#include <sys/stat.h>
#include <stdio.h>

//#include "sdb_storage_header.h"
#include "sdb_storage_types.h"

#include <am/sdb_btree/sdb_btree.h>
#include <am/sdb_hash/sdb_hash.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem/operations.hpp>  
#include <boost/filesystem.hpp>

NS_IZENELIB_AM_BEGIN

class memory_map {
private:
	std::string path_; // path to the current file
	size_t offSet_; // still left in the file
	size_t mapSize_;
	boost::iostreams::mapped_file m_file_; // current memory map source   
public:
	memory_map(const std::string& path, long offSet=0, size_t mapSize=1024*1024*1024):
	path_(path),offSet_(offSet),mapSize_(mapSize),
	m_file_(path, ios_base::in | ios_base::out, mapSize) {		
	}

	size_t read (long offSet, void * buf, size_t num_bytes) {
		if(offSet + num_bytes> mapSize_)
		return size_t(-1);
		memcpy(buf, m_file_.data()+offSet, num_bytes);
		return num_bytes;
	}

	size_t write(void * buf, size_t num_bytes) {
		if(offSet_+num_bytes < mapSize_) {
			memcpy(m_file_.data()+offSet_, buf, num_bytes);
			offSet_ += num_bytes;
			return num_bytes;
		}
		return size_t(-1);
	}

	void setOffset(size_t off) {
		offSet_ = off;
	}
	
	size_t getOffset() {
		return offSet_;
	}

	void* header() {
		return m_file_.data();
	}

	void display() {
		cout<<"memory_map info:"<<endl;
		cout<<"path_:" <<path_<<endl;
		cout<<"offset_: "<<offSet_<<endl;
	}
};

struct mm_header {
	int idx;
	size_t mapSize;
	long offset;
	size_t tsz;

	mm_header() :
		idx(0), offset(sizeof(mm_header)), tsz(sizeof(mm_header)) {
		mapSize = 1024*1024*1024;
	}
	void read(void* fileBuffer) {
		memcpy(this, fileBuffer, sizeof(mm_header));
	}
	void write(void* fileBuffer) {
		memcpy(fileBuffer, this, sizeof(mm_header));
	}

	void display(std::ostream& os = std::cout) {
		cout<<"idx: "<<idx<<endl;
		cout<<"mapSize: "<<mapSize<<endl;
		cout<<"offset: "<<offset<<endl;
		cout<<"tsz: "<<tsz<<endl;
	}
};

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

template< typename KeyType, typename ValueType, typename LockType =NullLock,
		typename AmType=sdb_btree<KeyType, unsigned int, LockType>,
		bool UseCompress = true > class sdb_storage_mm :
	public AccessMethod<KeyType, ValueType, LockType> {
public:
	typedef AmType KeyHash;
	typedef typename AmType::SDBCursor SDBCursor;
	//enum {MMAP_SIZE = 1024*1024*1024};
	//enum {BATCH_READ_NUM = 16};
	//enum {FILE_HEAD_SIZE = 1024};
public:
	/**
	 *   constructor
	 */
	sdb_storage_mm(const string& fileName = "sdb_storage") :
		ssh_(), fileName_(fileName+"_value.dat"),
				keyHash_(fileName+ "_key.dat") {
		//dataFile_ = 0;
		isOpen_ = false;
	}

	/**
	 *   deconstructor, close() will also be called here.
	 */
	virtual ~sdb_storage_mm() {
		if (dataFile_)
			close();
	}

	void setMapSize(size_t mapSize) {
		assert(isOpen_ == false);
		ssh_.mapSize = mapSize;

	}

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
	void setCacheSize(size_t cacheSize) {
		//ssh_.cacheSize = cacheSize;
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
		if (keyHash_.search(key, locn))
			return false;
		else {
			keyHash_.insert(key, ssh_.offset);
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
		if ( !keyHash_.get(key, npos) )
			return NULL;
		else {
			ValueType *pv = new ValueType;
			if (readValue_(npos, *pv) )
				return pv;
			else {
				delete pv;
				pv = 0;
				return NULL;
			}
		}

	}

	bool get(const KeyType& key, ValueType& val) {
		unsigned int npos;
		if ( !keyHash_.get(key, npos) )
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
	bool update(const DataType<KeyType,ValueType> & dat) {
		return update(dat.get_key(), dat.get_value() );
	}

	/**
	 *  update  an item by key/value pair
	 */
	bool update(const KeyType& key, const ValueType& val) {
		unsigned int npos;
		if ( !keyHash_.get(key, npos) )
			return insert(key, val);
		else {
			keyHash_.update(key, ssh_.offset);
			appendValue_(val);
		}
		return true;
	}

	/**
	 *  search an item
	 * 
	 *   @return SDBCursor
	 */
	SDBCursor search(const KeyType& key) {
		return keyHash_.search(key);
	}

	/**
	 *    another search function, flushCache_() will be called at the beginning,
	 * 
	 */
	bool search(const KeyType &key, SDBCursor &locn) {
		return keyHash_.search(key, locn);
	}

	/**
	 *  get the SDBCursor of first item in the first not empty bucket.
	 */
	SDBCursor get_first_locn() {
		return keyHash_.get_first_locn();
	}

	SDBCursor get_last_locn() {
		return keyHash_.get_last_locn();
	}

	bool get(const SDBCursor& locn, KeyType& key, ValueType& value) {
		unsigned int npos;
		bool ret =keyHash_.get(locn, key, npos);
		if (ret) {
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
	bool seq(SDBCursor& locn, ESeqDirection sdir=ESD_FORWARD) {
		return keyHash_.seq(locn, sdir);
	}

	bool seq(SDBCursor& locn, KeyType& key, ESeqDirection sdir=ESD_FORWARD) {
		unsigned int npos;
		return keyHash_.seq(locn, key, npos, sdir);
	}

	bool seq(SDBCursor& locn, KeyType& key, ValueType& value,
			ESeqDirection sdir=ESD_FORWARD) {
		unsigned int npos;
		if (keyHash_.seq(locn, key, npos, sdir) )
			return readValue_(npos, value);
		else
			return false;
	}

	bool seq(SDBCursor& locn, DataType<KeyType,ValueType> & rec,
			ESeqDirection sdir=ESD_FORWARD) {
		return seq(locn, rec.key, rec.value, sdir);

	}

	/**
	 *   get the num of items 
	 */
	int num_items() {
		return keyHash_.num_items();
	}

public:
	bool is_open() {
		return isOpen_;
	}
	/**
	 *   db must be opened to be used.
	 */
	bool open() {
		if (isOpen_)
			return true;

		keyHash_.open();
		if (boost::filesystem::exists(fileName_) ) {
			DLOG(INFO)<<"open exist...\n"<<endl;			
			mms_.reset(new memory_map(fileName_, ssh_.offset, ssh_.mapSize));
			ssh_.read(mms_->header() );
			mms_->setOffset(ssh_.offset);	
			dataFile_ = fopen(fileName_.c_str(), "r+b");

			//			ssh_.tsz = 1024*1024*1024+ssh_.offset;
			//			syncHeader_();

			//			int idx=1;
			//			char ch[5];
			//			sprintf(ch, "_%d", idx);
			//			ff = fileName_ + ch;
			//			while (boost::filesystem::exists( ff ) ) {
			//				mms_.resize(idx+1);
			//				mms_.reset(new memory_map( ff ));
			//				++idx;
			//				sprintf(ch, "_%d", idx);
			//				ff = fileName_ + ch;
			//			}
			//			mms_[ssh_.idx]->setOffset(ssh_.offset);
		} else {
			DLOG(INFO)<<"creat new...\n"<<endl;

			//			dataFile_=fopen(fileName_.c_str(), "w+");
			//			fseek(dataFile_, 1024*1024*1024, SEEK_SET);
			//			putw(0, dataFile_);
			//mms_.resize(1);
			genInitFile_();		
			mms_.reset(new memory_map(fileName_, 0, ssh_.mapSize));
			mms_->setOffset(ssh_.offset);		
			syncHeader_();
			dataFile_=fopen(fileName_.c_str(), "r+b");
		}
		keyHash_.fillCache();
		
		//		struct stat statbuf;
		//		bool creating = stat(fileName_.c_str(), &statbuf);
		//
		//		keyHash_.open();
		//		dataFile_ = fopen(fileName_.c_str(), creating ? "w+b" : "r+b");
		//		if ( 0 == dataFile_) {
		//			DLOG(ERROR)<<"Error in open(): open file failed"<<endl;
		//			return false;
		//		}
		//		bool ret = false;
		//		if (creating) {
		//			// We're creating if the file doesn't exist.
		//			DLOG(INFO)<<"creating...\n"<<endl;
		//			//ssh_.toFile(dataFile_);
		//			ret = true;
		//		} else {
		//			if ( !ssh_.fromFile(dataFile_) ) {
		//				return false;
		//			} else {
		//				if (ssh_.magic != 0x061561) {
		//					cout<<"Error, read wrong file header_\n"<<endl;
		//					return false;
		//				}
		//				DLOG(INFO)<<"open exist...\n"<<endl;
		//				//ssh_.display( LOG(INFO) );
		//			}
		//			keyHash_.fillCache();
		//		}
#ifdef DEBUG 
		ssh_.display();
#endif
		isOpen_ = true;
		return true;
	}
	/**
	 *   db should be closed after open, and  it will automatically called in deconstuctor.
	 */
	bool close() {
		if ( !isOpen_)
			return false;
		flush();
		fclose(dataFile_);
		dataFile_ = 0;
		isOpen_ = false;
		return true;
	}
	/**
	 *  write the dirty buckets to disk, not release the memory
	 *  
	 */
	void commit() {
		keyHash_.commit();
		//ssh_.toFile(dataFile_);
		fflush(dataFile_);
	}
	/**
	 *   Write the dirty buckets to disk and also free up most of the memory.
	 *   Note that, for efficieny, entry_[] is not freed up.
	 */
	void flush() {
		if ( !dataFile_)
			return;
		keyHash_.flush();
		//ssh_.toFile(dataFile_);
		fflush(dataFile_);
	}
	/**
	 *  display the info of sdb_storage
	 */
	void display(std::ostream& os = std::cout, bool onlyheader = true) {
		ssh_.display(os);
		keyHash_.display(os);
		//os<<"dataFile loadFactor: " <<loadFactor()<<endl;
	}

private:
	///SsHeader ssh_;
	mm_header ssh_;
	string fileName_;
	FILE* dataFile_;
	KeyHash keyHash_;
	bool isOpen_;

private:
	LockType fileLock_;
	map<unsigned int, ValueType> readCache_;
	boost::shared_ptr<memory_map> mms_;
	
	void syncHeader_() {
		memcpy(mms_->header(), &ssh_, sizeof(ssh_));
	}

	void genInitFile_() {
		//cout<<"genInitFile: "<<fileName_<<endl;
		FILE *fp=fopen(fileName_.c_str(), "w+");
		fseek(fp, 1024*1024*1024, SEEK_SET);
		putw(0, fp);
		fclose(fp);
	}

	/**
	 *   Allocate an bucket_chain element 
	 */
	inline bool appendValue_(const ValueType& val) {
		char* ptr;
		size_t vsize;

		izene_serialization<ValueType> izs(val);
		izs.write_image(ptr, vsize);

		if (UseCompress) {
			int vsz = 0;
			ptr = (char*)_tc_bzcompress(ptr, vsize, &vsz);
			vsize = vsz;
		}

		if ( ( (size_t)ssh_.offset < ssh_.mapSize) && ( (size_t)ssh_.offset
				+sizeof(size_t)+vsize >= ssh_.mapSize ) ){
			ssh_.offset = ssh_.mapSize;
		    ssh_.tsz = ssh_.offset;
		}
		
		//cout<<"write Value_:  "<<ssh_.offset<<endl;	
		//cout<<"to "<<mms_->getOffset()<<endl;
		
		if (ssh_.offset < ssh_.mapSize) {
			mms_->write(&vsize, sizeof(size_t));
			mms_->write(ptr, vsize);
			ssh_.tsz += sizeof(size_t) + vsize;
			ssh_.offset += sizeof(size_t) + vsize;
		} else {
			if ( 0 != fseek(dataFile_, ssh_.offset, 
			SEEK_SET) ) {
				if (UseCompress) {
					free(ptr);
				}
				return false;
			}
			if ( 1 != fwrite(&vsize, sizeof(size_t), 1, dataFile_) ) {
				if (UseCompress) {
					free(ptr);
				}
				return false;
			}
			if ( 1 != fwrite(ptr, vsize, 1, dataFile_) ) {
				if (UseCompress) {
					free(ptr);
				}
				return false;
			}
			//ssh_.nPage += (sizeof(size_t)+vsize)/ssh_.pageSize + 1;
			ssh_.tsz += sizeof(size_t) + vsize;
			ssh_.offset += sizeof(size_t)+vsize;

			//			++ssh_.idx;
			//			mms_.resize(ssh_.idx+1);
			//			char ch[5];
			//			sprintf(ch, "_%d", ssh_.idx);
			//			string ff = fileName_+ch;
			//			genInitFile_(ff);
			//			mms_[ssh_.idx].reset(new memory_map(ff ));
			//			mms_[ssh_.idx]->write(&vsize, sizeof(size_t));
			//			mms_[ssh_.idx]->write(ptr, vsize);
			//			ssh_.tsz = ssh_.idx*MMAP_SIZE+sizeof(size_t) + vsize;
			//			ssh_.offset = sizeof(size_t) + vsize;
			//			ssh_.tsz
		}
		syncHeader_();
		
		
		//mms_[ssh_.idx]->display();
		//	ScopedWriteLock<LockType> lock(fileLock_);

		//	if ( 0 != fseek(dataFile_, ssh_.nPage*ssh_.pageSize+sizeof(SsHeader), 
		//	SEEK_SET) ) {
		//		if (UseCompress) {
		//			free(ptr);
		//		}
		//		return false;
		//	}
		//	if ( 1 != fwrite(&vsize, sizeof(size_t), 1, dataFile_) ) {
		//		if (UseCompress) {
		//			free(ptr);
		//		}
		//		return false;
		//	}
		//	if ( 1 != fwrite(ptr, vsize, 1, dataFile_) ) {
		//		if (UseCompress) {
		//			free(ptr);
		//		}
		//		return false;
		//	}
		//  ssh_.nPage += (sizeof(size_t)+vsize)/ssh_.pageSize + 1;

		if (UseCompress) {
			free(ptr);
		}
		return true;
	}

	inline bool readValue_(unsigned int npos, ValueType& val) {
		//	ScopedWriteLock<LockType> lock(fileLock_);
		//
		//	if ( 0 != fseek(dataFile_, npos*ssh_.pageSize+sizeof(SsHeader), 
		//	SEEK_SET) )
		//		return false;
		//
		//	size_t vsize;
		//	if ( 1 != fread(&vsize, sizeof(size_t), 1, dataFile_) )
		//		return false;
		//	char* ptr = new char[vsize];
		//	if ( 1 != fread(ptr, vsize, 1, dataFile_) )
		//		return false;
		// cout<<"read from off: "<<npos<<endl;

		char* ptr;
		size_t vsize = 0;
		if (npos < ssh_.mapSize) {
			//int idx = npos/MMAP_SIZE;
			//size_t offset = npos - idx*MMAP_SIZE;
			//cout<<"readValue_:  "<<npos<<endl;			
			mms_->read(npos, &vsize, sizeof(size_t));
			//cout<<"vsz="<<vsize<<endl;	
			assert(vsize> 0);	
			ptr = new char[vsize];
			mms_->read(npos+sizeof(size_t), ptr, vsize);

		} else {
			if ( 0 != fseek(dataFile_, npos, SEEK_SET) )
				return false;
			if ( 1 != fread(&vsize, sizeof(size_t), 1, dataFile_) )
				return false;
			ptr = new char[vsize];
			if ( 1 != fread(ptr, vsize, 1, dataFile_) )
				return false;
		}

		if (UseCompress) {
			int vsz=0;
			char *p =(char*)_tc_bzdecompress(ptr, vsize, &vsz);
			vsize = vsz;
			delete ptr;
			ptr = p;
		}

		izene_deserialization<ValueType> izd(ptr, vsize);
		izd.read_image(val);

		if (UseCompress) {
			free(ptr);
		} else {
			delete ptr;
			ptr = 0;
		}
		return true;
	}

};

NS_IZENELIB_AM_END

#endif
