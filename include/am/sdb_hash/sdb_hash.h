#ifndef SDB_HASH_H
#define SDB_HASH_H

#include <string>
#include <iostream>
#include <types.h>
#include <sys/stat.h>

//#include <util/log.h>
#include <stdio.h>

#include "sh_types.h"
#include "sdb_hash_header.h"
#include "string_chain.h"

using namespace std;

NS_IZENELIB_AM_BEGIN

template< typename KeyType, typename ValueType, typename LockType =NullLock> class sdb_hash :
public AccessMethod<KeyType, ValueType, LockType>
{
public:
	typedef std::pair<string_chain*, char*> NodeKeyLocn;

	typedef DataType<KeyType,ValueType> DataType;
public:
	sdb_hash(const string& fileName = "sdb_hash.dat"):fileName_(fileName) {
		//sfh_.blockSize = 8192;
		//sfh_.maxDataSize = 64;

		for (size_t i=0; i<SH_DIRECTORY_SIZE; i++) {
			entry_[i] = NULL;
			bucketAddr[i] = 0;
		}
	}

	virtual ~sdb_hash() {
		flush();
		for (size_t i=0; i<SH_DIRECTORY_SIZE; i++) {
			string_chain* sc;
			while (entry_[i] != NULL) {
				sc = entry_[i]->next;
				delete entry_[i];
				entry_[i] = sc;
			}
		}
		fclose(dataFile_);
		dataFile_ = 0;
	}

	void setPageSize(size_t blockSize) {
		sfh_.blockSize = blockSize - sizeof(int) - sizeof(long);
	}

	void setCacheSize(size_t cacheSize)
	{
		sfh_.cacheSize = cacheSize;
	}

	bool insert(const DataType& dat) {
		return insert(dat.get_key(), dat.get_value() );
	}

	bool insert(const KeyType& key, const ValueType& value) {

		NodeKeyLocn locn;
		if( search(key, locn) )
		return false;
		else {
			DbObjPtr ptr, ptr1;

			ptr.reset(new DbObj);
			ptr1.reset(new DbObj);
			write_image(key, ptr);
			write_image(value, ptr1);

			size_t ksize = ptr->getSize();
			size_t vsize = ptr1->getSize();

			string_chain* sa = locn.first;
			char* p = locn.second;

			//entry_[idx] is even NULL.
			if(locn.first == NULL) {
				uint32_t idx = sdb_hashing::hash_fun(ptr->getData(), ptr->getSize() )
				% SH_DIRECTORY_SIZE;

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
				
				//add an extra size_t to indicate if reach the end of  string_chain.
				if ( size_t(p - sa->str) >= sfh_.blockSize-ksize-vsize-2*sizeof(size_t)-sizeof(size_t) ) {
					if (sa->next == 0) {
						sa->next = allocateBlock_();
					}
					sa = sa->next;
					p = sa->str;
				}

			}

			memcpy(p, &ksize, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(p, &vsize, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(p, ptr->getData(), ksize);
			p += ksize;
			memcpy(p, ptr1->getData(), vsize);
			p += vsize;

			sa->num++;
			sfh_.numItems++;
			return true;
		}

		/*DbObjPtr ptr, ptr1;
		 ptr.reset(new DbObj);
		 ptr1.reset(new DbObj);
		 write_image(key, ptr);
		 write_image(value, ptr1);

		 size_t ksize = ptr->getSize();
		 size_t vsize = ptr1->getSize();

		 //cout<<(char*)ptr->getData()<<endl;
		 //cout<<ksize<<endl;
		 //cout<<vsize<<endl;
		 //cout<<(char*)ptr1->getData()<<endl;

		 uint32_t idx = sdb_hashing::hash_fun(ptr->getData(), ptr->getSize() )
		 % SH_DIRECTORY_SIZE;

		 //cout<<"idx="<<idx<<endl;
		 if (entry_[idx] == NULL) {
		 if (bucketAddr[idx] == 0) {

		 entry_[idx] = allocateBlock_();
		 bucketAddr[idx] = entry_[idx]->fpos;
		 char* p = entry_[idx]->str;

		 memcpy(p, &ksize, sizeof(size_t));
		 p += sizeof(size_t);
		 memcpy(p, &vsize, sizeof(size_t));
		 p += sizeof(size_t);
		 memcpy(p, ptr->getData(), ksize);
		 p += ksize;
		 memcpy(p, ptr1->getData(), vsize);
		 p += vsize;

		 entry_[idx]->num++;
		 sfh_.numItems++;
		 return true;
		 }
		 } else {

		 int i = 0;
		 string_chain* sa = entry_[idx];
		 char* p = sa->str;

		 while ( sa ) {
		 p = sa->str;
		 //cout<<"sa.num= "<<sa->num<<endl;
		 for (i=0; i<sa->num; i++) {
		 size_t ksz, vsz;
		 memcpy(&ksz, p, sizeof(size_t));
		 //cout<<"existing ksz="<<ksz<<endl;
		 p += sizeof(size_t);

		 memcpy(&vsz, p, sizeof(size_t));
		 //cout<<"existing vsz="<<vsz<<endl;
		 p += sizeof(size_t);

		 if (ksz != ksize) {
		 p += ksz + vsz;
		 continue;
		 }

		 char *pd = (char *)ptr->getData();
		 size_t j=0;
		 for (; j<ksz; j++) {
		 //	cout<<int(pd[j])<<" vs "<<int(p[j])<<endl;
		 if (pd[j] != p[j]) {
		 break;
		 }
		 }
		 //	cout<<j<<" vs "<<ksz<<endl;
		 if (j == ksz) {
		 //cout<<key<<" found"<<endl;
		 return false;
		 }
		 p += ksz + vsz;
		 }
		 //cout<<"shift "<<size_t(p - sa->str)<<endl;
		 if ( size_t(p - sa->str) >= sfh_.blockSize-ksize-vsize-2*sizeof(size_t) ) {
		 if (sa->next == 0) {
		 sa->next = allocateBlock_();
		 sa = sa->next;
		 p = sa->str;
		 break;
		 } else {
		 //cout<<"load next"<<endl;
		 sa = sa->loadNext(dataFile_);
		 }
		 } else
		 break;
		 }

		 assert( size_t(p - sa->str) <= sfh_.blockSize-ksize-vsize-2*sizeof(size_t) );

		 memcpy(p, &ksize, sizeof(size_t));
		 p += sizeof(size_t);
		 memcpy(p, &vsize, sizeof(size_t));
		 p += sizeof(size_t);
		 memcpy(p, ptr->getData(), ksize);
		 p += ksize;
		 memcpy(p, ptr1->getData(), vsize);
		 p += vsize;

		 sa->num++;
		 sfh_.numItems++;

		 return true;

		 }
		 return true;
		 //LDBG_<<str;*/
	}

	ValueType* find(const KeyType & key) {

		NodeKeyLocn locn;
		if( !search(key, locn) )
		return NULL;
		else {
			char *p = locn.second;
			size_t ksz, vsz;
			memcpy(&ksz, p, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(&vsz, p, sizeof(size_t));
			p += sizeof(size_t);

			ValueType* pv;
			ValueType val;

			DbObjPtr ptr;
			ptr.reset( new DbObj(p+ksz, vsz) );
			read_image(val, ptr);
			pv = new ValueType(val);

			return pv;
		}
		/*DbObjPtr ptr;
		 ptr.reset(new DbObj);
		 write_image(key, ptr);

		 size_t ksize = ptr->getSize();

		 uint32_t idx = sdb_hashing::hash_fun(ptr->getData(), ptr->getSize() )
		 % SH_DIRECTORY_SIZE;

		 if (entry_[idx] == NULL) {
		 return NULL;
		 } else {
		 int i = 0;
		 string_chain* sa = entry_[idx];
		 char* p = sa->str;

		 while ( sa ) {
		 p = sa->str;
		 for (i=0; i<sa->num; i++) {
		 size_t ksz, vsz;
		 memcpy(&ksz, p, sizeof(size_t));
		 p += sizeof(size_t);

		 memcpy(&vsz, p, sizeof(size_t));
		 p += sizeof(size_t);

		 if (ksz != ksize) {
		 p += ksz + vsz;
		 continue;
		 }

		 char *pd = (char *)ptr->getData();
		 size_t j=0;
		 for (; j<ksz; j++) {
		 if (pd[j] != p[j]) {
		 break;
		 }
		 }
		 if (j == ksz) {
		 ValuType* pv;
		 ValueType val;
		 DbObjPtr ptr1;
		 ptr1.reset( new DbObj(p, vsz) );
		 read_image(val, ptr1);
		 pv = new ValuType(val);
		 return pv;
		 }
		 p += ksz + vsz;
		 }
		 sa = sa->loadNext(dataFile_);
		 }
		 return NULL;*/
	}

	bool del(const KeyType& key) {

		NodeKeyLocn locn;
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

			memset(p, 0, ksz);
			--locn.first->num;
			--sfh_.numItems;
			return true;

		}

		/*DbObjPtr ptr;
		 ptr.reset(new DbObj);
		 write_image(key, ptr);

		 size_t ksize = ptr->getSize();

		 uint32_t idx = sdb_hashing::hash_fun(ptr->getData(), ptr->getSize() )
		 % SH_DIRECTORY_SIZE;

		 if (entry_[idx] == NULL) {
		 return false;
		 } else {
		 int i = 0;
		 string_chain* sa = entry_[idx];
		 char* p = sa->str;

		 while ( sa ) {
		 p = sa->str;
		 for (i=0; i<sa->num; i++) {
		 size_t ksz, vsz;
		 memcpy(&ksz, p, sizeof(size_t));
		 p += sizeof(size_t);

		 memcpy(&vsz, p, sizeof(size_t));
		 p += sizeof(size_t);

		 if (ksz != ksize) {
		 p += ksz + vsz;
		 continue;
		 }

		 char *pd = (char *)ptr->getData();
		 size_t j=0;
		 for (; j<ksz; j++) {
		 if (pd[j] != p[j]) {
		 break;
		 }
		 }
		 if (j == ksz) {
		 //memset key area in p to zero, to indicate that
		 //it is deleted.
		 memset(p, 0, ksz);
		 --sa->num;
		 --sfh_.numItems;
		 return true;
		 }
		 p += ksz + vsz;
		 }
		 if (size_t(p - sa->str) >= sfh_.blockSize - sfh_.maxDataSize) {
		 sa = sa->loadNext(dataFile_);
		 }
		 else
		 break;
		 }
		 }
		 return false;*/

	}

	bool update(const DataType& dat)
	{
		return update( dat.get_key(), dat.get_value() );
	}

	bool update(const KeyType& key, const ValueType& value) {
		NodeKeyLocn locn;
		if( !search(key, locn) )
		return insert(key, value);
		else
		{
			DbObjPtr ptr, ptr1;
			ptr.reset(new DbObj);
			ptr1.reset(new DbObj);
			write_image(key, ptr);
			write_image(value, ptr1);

			size_t ksize = ptr->getSize();
			size_t vsize = ptr1->getSize();

			char *p = locn.second;
			size_t ksz, vsz;
			memcpy(&ksz, p, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(&vsz, p, sizeof(size_t));
			p += sizeof(size_t);
			if(vsz == vsize)
			{
				memcpy(p+ksz, ptr1->getData(), vsz);
				return true;
			}
			else
			{
				memset(p, 0, ksize);
				return insert(key, value);
			}
			return true;
		}

		/*DbObjPtr ptr, ptr1;
		 ptr.reset(new DbObj);
		 ptr1.reset(new DbObj);
		 write_image(key, ptr);
		 write_image(value, ptr1);

		 size_t ksize = ptr->getSize();
		 size_t vsize = ptr1->getSize();

		 uint32_t idx = sdb_hashing::hash_fun(ptr->getData(), ptr->getSize() )
		 % SH_DIRECTORY_SIZE;

		 if (entry_[idx] == NULL) {
		 return insert(key, value);
		 } else {
		 int i = 0;
		 string_chain* sa = entry_[idx];
		 char* p = sa->str;

		 while ( sa ) {
		 p = sa->str;
		 for (i=0; i<sa->num; i++) {
		 size_t ksz, vsz;
		 memcpy(&ksz, p, sizeof(size_t));
		 p += sizeof(size_t);

		 memcpy(&vsz, p, sizeof(size_t));
		 p += sizeof(size_t);

		 if (ksz != ksize) {
		 p += ksz + vsz;
		 continue;
		 }

		 char *pd = (char *)ptr->getData();
		 size_t j=0;
		 for (; j<ksz; j++) {
		 if (pd[j] != p[j]) {
		 break;
		 }
		 }
		 if (j == ksz) {
		 if(vsz == vsize) {
		 memcpy(p, ptr1->data, vsz);
		 return true;
		 }
		 if( del(key) )
		 return insert(key, value);
		 else
		 return false;
		 }
		 p += ksz + vsz;
		 }
		 if (size_t(p - sa->str) >= sfh_.blockSize - sfh_.maxDataSize) {
		 if (sa->next == 0) {
		 break;
		 } else {
		 sa = sa->loadNext(dataFile_);
		 }
		 } else
		 break;
		 }
		 }
		 return insert(key, value);*/
	}

	bool search(const KeyType&key, NodeKeyLocn& locn)
	{
		locn.first = NULL;
		locn.second = NULL;

		DbObjPtr ptr;
		ptr.reset(new DbObj);
		write_image(key, ptr);

		size_t ksize = ptr->getSize();

		uint32_t idx = sdb_hashing::hash_fun(ptr->getData(), ptr->getSize() )
		% SH_DIRECTORY_SIZE;

		locn.first = entry_[idx];

		if (entry_[idx] == NULL) {
			return false;
		} else {
			int i = 0;
			string_chain* sa = entry_[idx];
			char* p = sa->str;

			while ( sa ) {
				locn.first = sa;
				p = sa->str;
				for (i=0; i<sa->num; i++) {
					size_t ksz, vsz;
					memcpy(&ksz, p, sizeof(size_t));
					p += sizeof(size_t);

					memcpy(&vsz, p, sizeof(size_t));
					p += sizeof(size_t);

					if (ksz != ksize) {
						p += ksz + vsz;
						continue;
					}

					char *pd = (char *)ptr->getData();
					size_t j=0;
					for (; j<ksz; j++) {
						if (pd[j] != p[j]) {
							break;
						}
					}
					if (j == ksz) {
						locn.second = p-2*sizeof(size_t);
						return true;
					}
					p += ksz + vsz;
				}
				sa = sa->loadNext(dataFile_);
			}
			locn.second = p;
		}
		return false;
	}

	bool seq(NodeKeyLocn& locn, DataType& rec, ESeqDirection sdir=ESD_FORWARD) {

		if( sdir == ESD_FORWARD ) {

			string_chain* sa = locn.first;
			char* p = locn.second;

			if(sa == NULL)return false;
			if(p == NULL)return false;

			size_t ksize, vsize;
			DbObjPtr ptr, ptr1;
			KeyType key;
			ValueType val;

			memcpy(&ksize, p, sizeof(size_t));
			p += sizeof(size_t);
			memcpy(&vsize, p, sizeof(size_t));
			p += sizeof(size_t);
			
			ptr.reset(new DbObj(p, ksize));
			read_image(key, ptr);
			p += ksize;
			ptr1.reset(new DbObj(p, vsize));
			read_image(val, ptr1);
			p += vsize;

			rec.key = key;
			rec.value = val;

			memcpy(&ksize, p, sizeof(size_t));
			if(ksize == 0) {
				sa = sa->loadNext(dataFile_);
				p = sa->str;
			}
			locn.first = sa;
			locn.second = p;

			return true;
		} else
		{
			//to be implementd
			return false;
		}
	}

	int num_items() {
		return sfh_.numItems;
	}

public:

	bool open() {
		// We're creating if the file doesn't exist.
		struct stat statbuf;
		bool creating = stat(fileName_.c_str(), &statbuf);

		dataFile_ = fopen(fileName_.c_str(), creating ? "w+b" : "r+b");
		if ( 0 == dataFile_) {
			cout<<"Error in open(): open file failed"<<endl;
			return false;
		}
		bool ret = false;
		if (creating) {

#ifdef DEBUG
			cout<<"creating...\n"<<endl;
			sfh_.display();
#endif

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
#ifdef DEBUG
				cout<<"open exist...\n"<<endl;
				sfh_.display();
#endif
				if (SH_DIRECTORY_SIZE != fread(&bucketAddr[0], sizeof(long),
								SH_DIRECTORY_SIZE, dataFile_))
				return false;
				for (size_t i=0; i<SH_DIRECTORY_SIZE; i++) {
					if (bucketAddr[i] != 0) {
						//cout<<"bucket addr: "<<bucketAddr[i]<<endl;
						entry_[i] = new string_chain(sfh_.blockSize);
						entry_[i]->fpos = bucketAddr[i];
						entry_[i]->read(dataFile_);
					}
				}
				ret = true;
			}
		}
		return ret;
	}

	void flush() {
		sfh_.toFile(dataFile_);
		if (SH_DIRECTORY_SIZE != fwrite(&bucketAddr[0], sizeof(long),
						SH_DIRECTORY_SIZE, dataFile_) )
		return;
		for (size_t i=0; i<SH_DIRECTORY_SIZE; i++) {
			while (entry_[i]) {
				if (entry_[i]->write(dataFile_) ) {
					string_chain* sc = entry_[i]->next;
					entry_[i] = sc;
				} else {
					assert(0);
				}
			}
		}
		fflush(dataFile_);
	}

	void display(std::ostream& os = std::cout) {
		sfh_.display(os);
		for (size_t i=0; i<SH_DIRECTORY_SIZE; i++) {
			if (entry_[i])
			entry_[i]->display(os);
		}
	}

protected:
	string_chain* entry_[SH_DIRECTORY_SIZE];
	long bucketAddr[SH_DIRECTORY_SIZE];

private:
	ShFileHeader sfh_;
	string fileName_;
	FILE* dataFile_;

	vector<string_chain*> diryty_string_chain;

private:
	string_chain* allocateBlock_() {
		string_chain* newBlock;
		newBlock = new string_chain(sfh_.blockSize);
		newBlock->str = new char[sfh_.blockSize];
		memset(newBlock->str, 0, sfh_.blockSize);
		newBlock->isLoaded = true;

		newBlock->fpos = sizeof(ShFileHeader)+sizeof(bucketAddr)
		+ ( sfh_.blockSize+sizeof(int)+ sizeof(long) )*sfh_.nBlock;
		sfh_.nBlock++;

		return newBlock;
	}

	void flushCache_()
	{

	}

};

NS_IZENELIB_AM_END

#endif
