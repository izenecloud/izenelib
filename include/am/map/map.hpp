/**
   @file map.hpp
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef MAP_HPP
#define MAP_HPP

#include <types.h>
#include <am/am.h>
#include <am/concept/DataType.h>

#include <util/hashFunction.h>
#include <boost/static_assert.hpp>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <istream>
#include <iostream>

#ifdef _DEBUG
#include "debug_new.h"
#endif

using namespace std;

using namespace izenelib::am::util;

class simple_hash
{
public:
    static uint32_t getValue(const char* p, size_t len)
    {
        return izenelib::util::sdb_hash_fun(p,len);
    }

    static uint32_t getValue(const unsigned int& key, size_t x=0)
    {
        return key;

    }

};

NS_IZENELIB_AM_BEGIN
/**
   @class Map
 *@brief This is an implementation of open addressing and linear probing hash table for integer key.
 *
 **/
template<
class KeyType = uint32_t,
      class ValueType = uint64_t,
      size_t ENTRY_POW = 16,//!< entry size will be 2^ENTRY_POW
      class HASH_FUNCTION = simple_hash,
      int INIT_MAP_BUCKET_SIZE=64//!< initial size of bucket
      >
class Map : public AccessMethod<KeyType, ValueType>
{
    enum {ENTRY_SIZE = (2<<ENTRY_POW)};
    enum {ENTRY_MASK = ENTRY_SIZE-1};
    enum {ENTRY_GROW = 5};

    typedef izenelib::am::DataType<KeyType,ValueType> DataType;
    typedef Map<KeyType, ValueType, ENTRY_POW, HASH_FUNCTION> SelfType;

    typedef boost::archive::binary_iarchive iarchive;
    typedef boost::archive::binary_oarchive oarchive;

    struct KEY_
    {
        KeyType k_;
        inline KEY_()
        {
            k_ = (KeyType)-1;
        }

        inline KEY_(const KeyType& k)
        {
            k_ = k;
        }

        bool operator != (const KeyType& k)const
        {
            return k_!=k;
        }

        bool operator == (const KeyType& k)const
        {
            return k_==k;
        }

    };

    struct ENTRY_
    {
        KEY_* pK_;
        size_t*  pIdx_;
    };

public:
    Map():entry_size_(ENTRY_SIZE)
    {
        entry_.pK_ = (KEY_*) malloc( entry_size_ * sizeof(KEY_) );
        entry_.pIdx_ = (size_t*) malloc( entry_size_ * sizeof(size_t) );
        count_ = 0;
    }

    ~Map()
    {
        free(entry_.pK_);
        free(entry_.pIdx_);
        count_ = 0;
        dataVec_.clear();
    }

    void clear()
    {
        free(entry_.pK_);
        free(entry_.pIdx_);
        count_ = 0;
        dataVec_.clear();
    }

    bool insert(const DataType& data)
    {
        return insert(data.get_key(), data.get_value());
    }

    bool update(const DataType& dat)
    {
        return update(dat.get_key(), dat.get_value());
    }

    int num_items() const
    {
        return count_;
    }

    void display(std::ostream & os=std::cout)
    {
    }

    bool insert(const KeyType& key, const ValueType& v)
    {
        KeyType idx = key & ENTRY_MASK;

        while (idx<entry_size_ && entry_.pK_[idx] != (KeyType)-1)
        {
            if (entry_.pK_[idx] == key)
                return false;

            idx ++;
        }

        if (idx == entry_size_)
        {
            entry_.pK_ = (KEY_*)realloc(entry_.pK_, (entry_size_+ENTRY_GROW)*sizeof(KEY_));

            for (KeyType i=0; i<ENTRY_GROW; i++)
                entry_.pK_[entry_size_+i] = (KeyType)-1;

            entry_.pIdx_ =  (size_t*)realloc((void*)entry_.pIdx_, (entry_size_+ENTRY_GROW)*sizeof(size_t));

            entry_size_ += ENTRY_GROW;
        }

        entry_.pK_[idx] = key;
        entry_.pIdx_[idx] = dataVec_.size();
        dataVec_.push_back(v);
        count_++;

        return true;
    }

    bool get(const KeyType&key, ValueType& value)
    {
        ValueType* ret = find(key);
        if(!ret)
        {
            value = *ret;
            return true;
        }
        else return false;
    }

    ValueType* find(const KeyType& key)
    {
        KeyType idx = key & ENTRY_MASK;

        while (idx<entry_size_)
        {
            if (entry_.pK_[idx] == (KeyType)-1)
                return NULL;

            if (entry_.pK_[idx] == key)
            {
                return &dataVec_[entry_.pIdx_[idx]];
            }

            idx ++;
        }

        return NULL;
    }

    bool del(const KeyType& key)
    {

        KeyType idx = key & ENTRY_MASK;

        while (idx<entry_size_)
        {
            if (entry_.pK_[idx] == (KeyType)-1)
                return false;

            if (entry_.pK_[idx] == key)
            {
                entry_.pK_[idx] = (KeyType)-1;
                count_--;
                return true;
            }

            idx ++;
        }

        return false;
    }

    bool update(const KeyType& key, const ValueType& v)
    {
        KeyType idx = key & ENTRY_MASK;

        while (idx<entry_size_)
        {
            if (entry_.pK_[idx] == (KeyType)-1)
                return false;

            if (entry_.pK_[idx] == key)
            {
                entry_.pIdx_[idx] = dataVec_.size();
                dataVec_.push_back(v);
                return true;
            }

            idx ++;
        }

        return true;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version)  const
    {
        ar & entry_size_;
        ar.save_binary(entry_.pK_, entry_size_*sizeof(KEY_));
        ar.save_binary(entry_.pIdx_, entry_size_*sizeof(size_t));
        ar & count_;
        ar & dataVec_;
    }


    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        clear();

        ar & entry_size_;
        entry_.pK_ = (KEY_*)malloc(entry_size_*sizeof(KEY_));
        entry_.pIdx_ = (size_t*)malloc(entry_size_*sizeof(size_t));

        ar.load_binary(entry_.pK_, entry_size_*sizeof(KEY_));
        ar.load_binary(entry_.pIdx_, entry_size_*sizeof(size_t));

        ar & count_;
        ar & dataVec_;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    void save(const string& file)  const
    {
        ofstream of(file.c_str(), ios_base::trunc);
        oarchive oa(of);
        oa<<*this;
        of.close();
    }

    void load(const string& file)
    {
        ifstream of(file.c_str());
        iarchive ia(of);
        ia >> *this;
        of.close();
    }

protected:
    ENTRY_ entry_;
    vector<ValueType> dataVec_;
    KeyType entry_size_;
    int count_;

};

//*****************************************************************************
/**
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

#define PAGE_EXPANDING 1
#define EXACT_EXPANDING 0

template<
class ValueType,
      size_t ENTRY_POW,
      class HASH_FUNCTION,
      int INIT_MAP_BUCKET_SIZE
      >
class Map<string, ValueType, ENTRY_POW, HASH_FUNCTION, INIT_MAP_BUCKET_SIZE>
    : public AccessMethod<string, ValueType>
{
    enum {ENTRY_SIZE = (2<<ENTRY_POW)};
    enum {ENTRY_MASK = ENTRY_SIZE-1};
    enum {EXPAND = PAGE_EXPANDING};

    typedef izenelib::am::DataType<string,ValueType> DataType;
    typedef Map<string, ValueType, ENTRY_POW, HASH_FUNCTION> SelfType;

    typedef boost::archive::binary_iarchive iarchive;
    typedef boost::archive::binary_oarchive oarchive;

public:
    Map()
    {
        for(size_t i=0; i<ENTRY_SIZE; i++)
        {
            entry_[i] = NULL;
        }
        count_ = 0;
    }

    ~Map()
    {

        for(size_t i=0; i<ENTRY_SIZE; i++)
        {
            if (entry_[i]!=NULL) delete[] entry_[i];
            entry_[i] = NULL;
        }

    }

    void clear()
    {

        for(size_t i=0; i<ENTRY_SIZE; i++)
        {
            if (entry_[i]!=NULL) delete[] entry_[i];
            entry_[i] = NULL;
        }
        dataVec_.clear();
        count_ = 0;
    }

    bool insert(const DataType& data)
    {
        return insert(data.get_key(), data.get_value());
    }

    bool update(const DataType& dat)
    {
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

    bool insert(const string& str, const ValueType& v)
    {
        size_t ksize = str.length();

        uint32_t idx = HASH_FUNCTION::getValue(str.c_str(),ksize) & ENTRY_MASK;

        uint64_t value = dataVec_.size();

        dataVec_.push_back(v);

        char* pBkt = entry_[idx];

        if (pBkt == NULL)
        {
            entry_[idx] = pBkt = new char[INIT_MAP_BUCKET_SIZE];
            *(uint32_t*)(pBkt) = 64;
            *((uint32_t*)(pBkt)+1) = 2*sizeof(uint32_t);
            good_entries_.push_back(idx);
        }
        else
        {
            //cout<<"does it exist?";
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
                    *(uint64_t*)(pBkt+p+j) = value;
                    return true;
                }

                p += len + sizeof (uint64_t);
            }
        }

        //cout<<"==============\n";
        uint32_t bs = *(uint32_t*)(pBkt);
        uint32_t content_len = *((uint32_t*)pBkt+1);

        if (bs-content_len<ksize+sizeof(size_t)+sizeof(uint64_t))
        {
            bs += EXPAND ==PAGE_EXPANDING? INIT_MAP_BUCKET_SIZE: ksize+sizeof(size_t)+sizeof(uint64_t);
            //content_len += str.length()+sizeof(uint32_t);
            pBkt = new char[bs];
            memcpy(pBkt, entry_[idx], *(uint32_t*)(entry_[idx]) );
            delete[] entry_[idx];
            entry_[idx] = pBkt;
        }

        *((size_t*)(pBkt+content_len)) = ksize;
        content_len += sizeof(size_t);
        memcpy(pBkt+content_len, str.c_str(), ksize);
        content_len += ksize;
        *((uint64_t*)(pBkt+content_len)) = value;
        content_len += sizeof(uint64_t);

        *(uint32_t*)(pBkt) = bs;
        *((uint32_t*)pBkt+1) = content_len;

        count_++;
        return true;

    }

    bool get(const string&key, ValueType& value)
    {
        ValueType* ret = find(key);
        if(!ret)
        {
            value = *ret;
            return true;
        }
        else return false;
    }

    ValueType* find(const string& str)
    {
        size_t ksize = str.length();

        uint32_t idx = HASH_FUNCTION::getValue(str.c_str(),ksize) & ENTRY_MASK;

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

    bool del(const string& str)
    {
        size_t ksize = str.length();

        uint32_t idx = HASH_FUNCTION::getValue(str.c_str(),ksize) & ENTRY_MASK;

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

            count_ --;
            return true;
        }

        return false;
    }

    bool update(const string& str, const ValueType& v)
    {
        size_t ksize = str.length();
        uint32_t idx = HASH_FUNCTION::getValue(str.c_str(),ksize) & ENTRY_MASK;

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

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version)  const
    {
        ar & good_entries_;

        for (uint64_t i=0; i<good_entries_.size(); i++)
        {
            char* pBkt = entry_[good_entries_[i]];

            uint32_t bs = *(uint32_t*)(pBkt);

            ar & bs;
            ar.save_binary(pBkt, bs);
        }

        ar & count_ & dataVec_;
    }


    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {

        ar & good_entries_;

        for (uint64_t i=0; i<good_entries_.size(); i++)
        {
            uint32_t bs;
            ar & bs;

            entry_[good_entries_[i]] = new char[bs];
            ar.load_binary(entry_[good_entries_[i]], bs);

        }

        ar & count_  & dataVec_;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()


    void save(const string& file)  const
    {
        ofstream of(file.c_str(), ios_base::out|ios_base::trunc);

        oarchive oa(of);
        oa<<*this;
        of.close();
    }

    void load(const string& file)
    {
        ifstream of(file.c_str());
        iarchive ia(of);
        ia >> *this;
        of.close();
    }

protected:
    char* entry_[ENTRY_SIZE];
    vector<ValueType> dataVec_;
    vector<uint64_t> good_entries_;
    int count_;

};


NS_IZENELIB_AM_END

#endif
