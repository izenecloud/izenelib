/**
   @file cccr_hash.h
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef CCCR_HASH_H
#define CCCR_HASH_H

#include "cccr_type.h"

NS_IZENELIB_AM_BEGIN
/**
 * @class cccr_hash
 * @brief cccr_hash stands for Cache-Conscious Collision Resolution String Hash Table.
 *
 *  This is based on work of Nikolas Askitis and Justin Zobel, 'Cache-Conscious Collision Resolution
 *     in String Hash Tables' .On typical current machines each cache miss incurs a delay of
 *  hundreds of clock cycles while data is fetched from memory. This approach both
 *  saves space and eliminates a potential cache miss at each node access, at little cost.
 *  In experiments with large sets of strings drawn from real-world data, we show that,
 *  in comparison to standard chaining, compact-chain hash tables can yield both space
 *  savings and reductions in per-string access times.
 *
 *
 **/

template<
typename KeyType = string,
typename ValueType = NullType,
size_t ENTRY_POW = 17
>class cccr_hash : public AccessMethod<KeyType, ValueType>
{
	enum {ENTRY_SIZE = (2<<ENTRY_POW)};
	enum {ENTRY_MASK = ENTRY_SIZE-1};
	enum {EXPAND = PAGE_EXPANDING};
	//typedef DataType<KeyType,ValueType> DataType;
public:
    /**
       @brief a constructor
     */
	cccr_hash()
	{
	    entry_ = new char*[ENTRY_SIZE];
		for(size_t i=0; i<ENTRY_SIZE; i++)
		{
			entry_[i] = NULL;
		}
		count_ = 0;
	}
    
    /**
       @brief a destructor
     */
	~cccr_hash()
	{
		for(size_t i=0; i<ENTRY_SIZE; i++)
		{
			if (entry_[i]!=NULL) delete entry_[i];
			entry_[i] = NULL;
		}
		delete entry_;
	}

	bool insert(const DataType<KeyType,ValueType> & data) {
		return insert(data.get_key(), data.get_value());
	}

	bool update(const DataType<KeyType,ValueType> & dat) {
		return update(dat.get_key(), dat.get_value());
	}

	int num_items() const
	{
		return count_;
	}

	void display(std::ostream & os=std::cout)
	{
		for(size_t i=0; i<ENTRY_SIZE; i++)
		{
			char* pBkt = entry_[i];

			if (pBkt ==NULL)
			continue;

			os<<endl<<i<<" Entry===========\n";

			uint32_t bs = *(uint32_t*)(pBkt);
			uint32_t content_len = *((uint32_t*)(pBkt)+1);
			os<<"bucket size: "<<bs<<endl;
			os<<"content len: "<<content_len<<endl;

			uint32_t p = sizeof(uint32_t)*2;;
			while (p < content_len)
			{
				size_t len = *((size_t*)(pBkt+p));
				p += sizeof (size_t);
				os<<"("<<len<<")";

				for (size_t j=0; j<len; j++)
				os<<pBkt[p+j];
				p += len;

				os<<"=>"<<*((uint64_t*)(pBkt+p))<<endl;
				p += sizeof (uint64_t);
			}
		}
	}

	bool insert(const KeyType& key, const ValueType& v)
	{
		char* str;
		std::size_t ksize;
		//write_image(key, str, ksize);	
		izene_serialization<KeyType> izs(key);
		izs.write_image(str, ksize);

		uint32_t idx = izenelib::util::sdb_hash_fun(str,ksize) & ENTRY_MASK;

		uint64_t value = dataVec_.size();
		dataVec_.push_back(v);

		char* pBkt = entry_[idx];

		if (pBkt == NULL)
		{
			entry_[idx] = pBkt = new char[INIT_BUCKET_SIZE];
			*(uint32_t*)(pBkt) = 64;
			*((uint32_t*)(pBkt)+1) = 2*sizeof(uint32_t);
		}
		else
		{//does it exist?
			uint32_t content_len = *((uint32_t*)pBkt+1);
			uint32_t p = sizeof(uint32_t)*2;;
			while (p < content_len)
			{
				size_t len = *((size_t*)(pBkt+p));

				if (len != ksize )
				{
					p += sizeof (size_t) + len + sizeof(uint64_t);
					continue;
				}

				p += sizeof (size_t);

				size_t j=0;
				for (; j<len; j++)
				if (str[j] != pBkt[p+j])
				break;

				if (j == len)
				{
					//*(uint64_t*)(pBkt+p+j) = value;
					return false;
				}

				p += len + sizeof (uint64_t);
			}
		}

		uint32_t bs = *(uint32_t*)(pBkt);
		uint32_t content_len = *((uint32_t*)pBkt+1);

		if (bs-content_len<ksize+sizeof(size_t)+sizeof(uint64_t))
		{
			bs += EXPAND ==PAGE_EXPANDING? INIT_BUCKET_SIZE: ksize+sizeof(size_t)+sizeof(uint64_t);
			//content_len += str.length()+sizeof(uint32_t);
			pBkt = new char[bs];
			memcpy(pBkt, entry_[idx], *(uint32_t*)(entry_[idx]) );
			delete entry_[idx];
			entry_[idx] = pBkt;
		}

		*((size_t*)(pBkt+content_len)) = ksize;
		content_len += sizeof(size_t);
		memcpy(pBkt+content_len, str, ksize);
		content_len += ksize;
		*((uint64_t*)(pBkt+content_len)) = value;
		content_len += sizeof(uint64_t);

		*(uint32_t*)(pBkt) = bs;
		*((uint32_t*)pBkt+1) = content_len;

		count_++;
		return true;
		//LDBG_<<str;

	}
	
	bool get(const KeyType&key, ValueType& value){
		ValueType* pv= find(key);
		if( pv){
			value = *find(key);
			return true;
		}
		return false;
	}
	

	ValueType* find(const KeyType& key)
	{
		char* str;
		size_t ksize;
		//write_image(key, str, ksize);
		izene_serialization<KeyType> izs(key);
		izs.write_image(str, ksize);

		uint32_t idx = izenelib::util::sdb_hash_fun(str,ksize) & ENTRY_MASK;

		char* pBkt = entry_[idx];
		if (pBkt == NULL)
		return NULL;

		uint32_t content_len = *((uint32_t*)(pBkt)+1);

		uint32_t p = sizeof(uint32_t)*2;;
		while (p < content_len)
		{
			size_t len = *((size_t*)(pBkt+p));
			p += sizeof (size_t);
			if (ksize != len)
			{
				p += len + sizeof (uint64_t);
				continue;
			}

			size_t i = 0;
			for (; i<len; i++)
			{
				if (pBkt[p+i]!=str[i])
				break;
			}
			if (i != len)
			{
				p += len + sizeof (uint64_t);
				continue;
			}

			p += len;

			if (*((uint64_t*)(pBkt+p))!=(uint64_t)-1)
			return &(dataVec_[*((uint64_t*)(pBkt+p))]);
			else
			return NULL;

		}

		return NULL;

	}

	bool del(const KeyType& key)
	{

		char* str;
		size_t ksize;
		//write_image(key, str, ksize);
		izene_serialization<KeyType> izs(key);
		izs.write_image(str, ksize);

		uint32_t idx = izenelib::util::sdb_hash_fun(str,ksize) & ENTRY_MASK;

		char* pBkt = entry_[idx];
		if (pBkt ==NULL)
		return -1;

		uint32_t content_len = *((uint32_t*)(pBkt)+1);

		uint32_t p = sizeof(uint32_t)*2;;
		while (p < content_len)
		{
			size_t len = *((size_t*)(pBkt+p));
			p += sizeof (size_t);
			if (ksize != len)
			{
				p += len + sizeof (uint64_t);
				continue;
			}

			size_t i = 0;
			for (; i<len; i++)
			{
				if (pBkt[p+i]!=str[i])
				break;
			}
			if (i != len)
			{
				p += len + sizeof (uint64_t);
				continue;
			}

			p += len;

			*((uint64_t*)(pBkt+p)) = (uint64_t)-1;

			if (count_!=0)
				count_ --;
			return true;
		}

		return false;
	}

	bool update(const KeyType& key, const ValueType& v)
	{

		char* str;
		size_t ksize;
		//write_image(key, str, ksize);
		izene_serialization<KeyType> izs(key);
		izs.write_image(str, ksize);

		uint32_t idx = izenelib::util::sdb_hash_fun(str,ksize) & ENTRY_MASK;

		char* pBkt = entry_[idx];

		if (pBkt ==NULL)
		return -1;

		uint32_t content_len = *((uint32_t*)(pBkt)+1);

		uint32_t p = sizeof(uint32_t)*2;;
		while (p < content_len)
		{
			size_t len = *((size_t*)(pBkt+p));
			p += sizeof (size_t);
			if (ksize != len)
			{
				p += len + sizeof (uint64_t);
				continue;
			}

			size_t i = 0;

			for (; i<len; i++)
			{
				if (pBkt[p+i]!=str[i])
				break;
			}
			if (i != len)
			{
				p += len + sizeof (uint64_t);
				continue;
			}

			p += len;

			if (*((uint64_t*)(pBkt+p)) != (uint64_t)-1)
			{
				dataVec_[*((uint64_t*)(pBkt+p))] = v;
				return true;
			}
			else
			return false;
		}

		return false;
	}

	/*bool save(const string& keyFilename, const string& valueFilename)
	 {
	 FILE* f = fopen(keyFilename.c_str(), "w+");
	 if (f ==NULL)
	 {
	 cout<<"\nCan't open file: "<<keyFilename<<endl;
	 return false;
	 }

	 fseek(f, 0, SEEK_SET);

	 for(size_t i=0; i<ENTRY_SIZE; i++)
	 {
	 char* pBkt = entry_[i];

	 if (pBkt ==NULL)
	 continue;

	 if ( fwrite(&i, sizeof(size_t), 1, f)!=1)
	 return false;

	 uint32_t bs = *(uint32_t*)(pBkt);

	 if ( fwrite(pBkt, bs, 1, f)!=1)
	 return false;

	 }

	 fclose(f);

	 ofstream of(valueFilename.c_str());
	 oarchive oa(of);
	 size_t size = dataVec_.size();
	 oa << size;

	 for(typename vector<ValueType>::iterator i =dataVec_.begin(); i!=dataVec_.end(); i++)
	 {
	 oa<<(*i);
	 }

	 of.close();

	 return true;
	 }

	 bool load(const string& keyFilename, const string& valueFilename)
	 {
	 FILE* f = fopen(keyFilename.c_str(), "r");
	 if (f ==NULL)
	 {
	 cout<<"\nCan't open file: "<<keyFilename<<endl;
	 return false;
	 }

	 fseek(f, 0, SEEK_SET);
	 size_t s;

	 while(fread(&s, sizeof(size_t), 1, f)==1)
	 {
	 uint32_t bs;
	 if(fread(&bs, sizeof(uint32_t), 1, f)!=1)
	 return false;

	 entry_[s] = new char[bs];
	 char* pBkt = entry_ [s];
	 *(uint32_t*)pBkt = bs;
	 if(fread(pBkt+sizeof(uint32_t),bs-sizeof(uint32_t), 1, f)!=1)
	 return false;

	 }

	 fclose(f);

	 dataVec_.clear();

	 ifstream ifs(valueFilename.c_str());
	 iarchive ia(ifs);
	 size_t size;
	 ia>>size;

	 for (size_t i =0; i<size; i++)
	 {
	 ValueType v;
	 ia>>v;
	 dataVec_.push_back(v);
	 }

	 ifs.close();

	 return true;
	 }*/

protected:
	char** entry_;//!< entry of table
	vector<ValueType> dataVec_;//!< store values
	int count_;//!< count of records in there
};

template< typename KeyType =string, typename ValueType =NullType > class cccr_small_hash :
	public cccr_hash<KeyType, ValueType, 6> {

};

NS_IZENELIB_AM_END

#endif
