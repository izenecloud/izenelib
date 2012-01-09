/**
 * @file CacheHash.h
 * @brief The header file of CacheHash.
 *
 * This file defines class CacheHash and its derived class CacheExtHash, which is storage of Cache.
 */

#ifndef CACHEHASH_H
#define CACHEHASH_H

#include "cm_basics.h"

namespace izenelib
{
namespace cache
{


/**
 * \brief Cache  Storage Class.

 * A hybrid hash that combines two hash, mostly one FirstHash is memory hash(M_LH or M_EH), the other is file hash(F_EH or F_LH).
 *
 * Note that FirstHash and SecondHash should have the same interface as CacheHash.
 */
template <class KeyType, class ValueType, class Hash>
class CacheHash
{
    //typedef izenelib::am::DataType<KeyType,ValueType> DataType;
public:
    /**
     *	\brief default constructor.
     *
     *        Set ratio = 1, i.e. only memory hash.
     */
    CacheHash()
    {
        hashSize_ = 0;
        //memSizeOfValue_ = 0;
    }

    /**
     *	\brief  constructor.
     *
     *	\param hashSize capacity of hash.
     *	\param ratio if ratio = 0.4, then size of first hash is 0.4*hashSize,
     *         and the second hash will be 0.6*hashSize.
     *
     */
    CacheHash(unsigned int hashSize)
    {
        hashSize_ = hashSize;
    }


    unsigned int getHashSize()
    {
        return hashSize_;
    }
    void setHashSize(unsigned int hashSize)
    {
        hashSize_ = hashSize;
    }

    bool hasKey(const KeyType& key)
    {
        return memHash_.hasKey(key);
    }

    ValueType* find(const KeyType& key)
    {
        return memHash_.find(key);
    }

    bool get(const KeyType& key, ValueType& val)
    {
        return memHash_.get(key, val);
    }

    bool insert(const KeyType& key, const ValueType& value)
    {
        return memHash_.num_items() < (int) hashSize_
            && memHash_.insert(key, value);
    }
    bool insert(const DataType<KeyType, ValueType>& data)
    {
        return memHash_.num_items() < (int) hashSize_
            && memHash_.insert(data);
    }

    bool full()
    {
        return (memHash_.num_items() >= (int) hashSize_);
    }
    bool del(const KeyType& key)
    {
        return memHash_.del(key);
    }

    bool update(const DataType<KeyType,ValueType>& data)
    {
        return memHash_.update(data);
    }

    /**
     *	\brief get the total number of item in the Cache.
     */
    int numItems()
    {
        return memHash_.num_items();
    }
    /**
     *   	\brief  display the num of items in first hash and second hash.
     */
    void displayHash()
    {
        cout << "Hash: numItem = " << memHash_.num_items();

    }

    template<class Archive>
    void save(Archive & ar,
              const unsigned int version = 0)
    {
        memHash_.save(ar);
    }

    template<class Archive>
    void load(Archive & ar,
              const unsigned int version = 0)
    {
        memHash_.load(ar);
    }

private:
    unsigned int hashSize_;
    Hash memHash_;
    //size_t memSizeOfValue_;
};


/**
 * \brief Cache  Storage Class.

 * A hybrid hash that combines two hash, mostly one FirstHash is memory hash(M_LH or M_EH), the other is file hash(F_EH or F_LH).
 *
 * Note that FirstHash and SecondHash should have the same interface as CacheHash.
 */
template <class KeyType, class ValueType, class FirstHash, class SecondHash>
class CacheExtHash
{
    //typedef izenelib::am::DataType<KeyType,ValueType> DataType;
public:
    /**
     *	\brief default constructor.
     *
     *        Set ratio = 1, i.e. only memory hash.
     */
    CacheExtHash()
    {
        hashSize_ = 0;
        ratio_ = 1.0;
        //memSizeOfValue_ = 0;
    }

    /**
     *	\brief  constructor.
     *
     *	\param hashSize capacity of hash.
     *	\param ratio if ratio = 0.4, then size of first hash is 0.4*hashSize,
     *         and the second hash will be 0.6*hashSize.
     *
     */
    CacheExtHash(unsigned int hashSize, double ratio)
    {
        hashSize_ = hashSize;
        ratio_=ratio;
        RatioCheck();
        //memSizeOfValue_ = 0;
    }
    /**
     *	\brief  constructor.
     *
     *	\param hashSize capacity of hash.
     *	\param ratio if ratio = 0.4, then size of first hash is 0.4*hashSize,
     and the second hash will be 0.6*hashSize.
     *	\param fileName for the second hash(file hash).
     */
    CacheExtHash(unsigned int hashSize, double ratio, const char* fileName)
        : fileHash_(fileName)
    {
        hashSize_ = hashSize;
        ratio_ = ratio;
        RatioCheck();
        fileHash_.open();
        //memSizeOfValue_ = 0;
    }

    unsigned int getHashSize()
    {
        return hashSize_;
    }
    void setHashSize(unsigned int hashSize)
    {
        hashSize_ = hashSize;
    }

    double getRatio()
    {
        return ratio_;
    }
    void setRatio(double ratio)
    {
        ratio_ = ratio;
        RatioCheck();
    }

    ValueType* find(const KeyType& key);

    bool getValue(const KeyType& key, ValueType& val)
    {
        if (! memHash_.get(key, val))
            return fileHash_.get(key, val);
    }
    bool insert(const KeyType& key, const ValueType& value)
    {
        return insert(DataType<KeyType,ValueType>(key,value));
    }
    bool insert(const DataType<KeyType, ValueType>& data);
    bool del(const KeyType& key);

    bool update(const DataType<KeyType,ValueType>& data)
    {
        return memHash_.update(data)
               &&  fileHash_.update(data) ;
    };

    bool update(const KeyType& key, const ValueType& value)
    {
        return update(DataType<KeyType,ValueType>(key,value));
    }


    /**
     *	\brief get the total number of item in the Cache.
     */
    int numItems()
    {
        return memHash_.num_items() + fileHash_.num_items();
    }

    FOUND_RESULT find(const KeyType& key, ValueType &dat);
    HASH_STATUS getStatus();
    DUMP_RESULT dump(const KeyType& key);
    /**
     *   	\brief  display the num of items in first hash and second hash.
     */
    void displayHash()
    {
        cout << "memHash: numItem = " << memHash_.num_items()
             << "; fileHash:numItem = " << fileHash_.num_items() << endl;
    }

    template <class Archive>
    void save(Archive & ar,
              const unsigned int version = 0)
    {
        memHash_.save(ar);
    }

    template <class Archive>
    void load(Archive & ar,
              const unsigned int version = 0)
    {
        memHash_.load(ar);
    }

private:
    unsigned int hashSize_;
    double ratio_; // Range: 0~1. 1--all memory;  0--all file.
    FirstHash memHash_;
    SecondHash fileHash_;
    bool RatioCheck();

    //size_t memSizeOfValue_;
};

/**
 * 	\brief It checks the range of ratio.
 *	If ratio is out of range(0~1.0),  set ratio_ to 1.
 *
 */

template <class KeyType, class ValueType, class FirstHash, class SecondHash>
bool CacheExtHash<KeyType, ValueType, FirstHash, SecondHash>::RatioCheck()
{
    if ((ratio_ >= 0) && (ratio_ <= 1))
        return 1;
    else
    {
        ratio_ = 1;
        return 0;
    }
}

/**
 * 	\brief Find an element.
 *
 *	@param key the handle of value
 *	@return  pointer to the value, otherwise return NULL if not found.
 */

template <class KeyType, class ValueType, class FirstHash, class SecondHash>
ValueType* CacheExtHash<KeyType, ValueType, FirstHash, SecondHash>::find(const KeyType& key)
{
    ValueType *p;
    p = memHash_.find(key);
    if (p)
        return p; //If hit from memoryHash_, then not find from file.
    else
        return fileHash_.find(key);

}

/**
 *
 * 	\brief Find an element from the Hybrid hash.
 *
 *	@return FOUND_RESULT:NOT_FOUND, FOUND_IN_MEM, FOUND_IN_FILE.
 */
template <class KeyType, class ValueType, class FirstHash, class SecondHash>
FOUND_RESULT CacheExtHash<KeyType, ValueType, FirstHash, SecondHash>::find(
        const KeyType& key, ValueType &dat)
{
    ValueType *pv;
    pv = memHash_.find(key);
    if (pv)
    {
        dat = *pv;
        return FOUND_IN_MEM; //If found from mem, then not find from file.
    }
    else
    {
        pv = fileHash_.find(key);
        if (pv)
        {
            dat = *pv;
            delete pv;
            pv = 0;
            return FOUND_IN_FILE;
        }
        else
            return NOT_FOUND;
    }

}

/**
 *	\brief Insert an element to the Hybrid hash.
 *
 *	@param data the content want to cache.
 *	@return true if succeeds, otherwise return false when insert failed, eg. when hash is full.
 */

template <class KeyType, class ValueType, class FirstHash, class SecondHash>
bool CacheExtHash<KeyType, ValueType, FirstHash, SecondHash>::insert(
        const DataType<KeyType, ValueType>& data)
{

#if 0
    cout<<"Before CacheExtHash insert: "<<memHash_.num_items()<<"  "
        <<fileHash_.num_items()<<endl;
    //cout<<"insert  "<<data<<endl;
#endif

    const KeyType& key = data.get_key();
    if (find(key))
        return true;

    if (memHash_.num_items() + 1 <= hashSize_ * ratio_)
    {
        //memSizeOfValue_ += CacheDataType<DataType>::getSize(data);
        //memSizeOfValue_ += data.size();
        return memHash_.insert(data);
    }
    else
    {
        if (fileHash_.num_items() + 1 <= hashSize_ * (1 - ratio_))
        {
            return fileHash_.insert(data);
        }
        else
        {
            return false;
        }
    }

}

/**
 *	\brief Delete an element from the Hybrid hash.
 *
 *	@return true if succeeds, otherwise return false when insert failed, eg. when hash is empty.
 */
template <class KeyType, class ValueType, class FirstHash, class SecondHash>
bool CacheExtHash<KeyType, ValueType, FirstHash, SecondHash>::del(const KeyType& key)
{

#if 0
    cout<<"Before extHash del :"<<memHash_.num_items()<<"  "
        <<fileHash_.num_items()<<endl;
#endif

    ValueType *pd = find(key);
    ValueType data;
    if (pd)
    {
        //cout<<*pd<<endl;
        data = *pd;
    }
    else
    {
        return 0;
    }
    if (memHash_.del(key))
    {
        //memSizeOfValue_ -= CacheDataType<DataType>::getSize(data);
        //memSizeOfValue_ -= data.size();
        return 1;
    }
    else
    {
        return fileHash_.del(key);
    }
}

/**
 *  	\brief Get the status of the hash
 * 		@return HASH_STATUS	0----both full
 *		1----file full
 *		2----memory full
 *		3----both not full
 */
template <class KeyType, class ValueType, class FirstHash, class SecondHash>
HASH_STATUS CacheExtHash<KeyType, ValueType, FirstHash, SecondHash>::getStatus()
{
    int mNum = memHash_.num_items();
    int fNum = fileHash_.num_items();
    return HASH_STATUS(int(mNum < hashSize_*ratio_) + int(fNum < hashSize_ * (1 - ratio_)) * 2);
}

/**
 *	\brief dump from file to memory or from memory to file.
 *
 *	@return DUMP_RESULT:not dump, dump from memory to file, dump from file to memory, dump failed(eg,when targe hash is full).
 */

template <class KeyType, class ValueType, class FirstHash, class SecondHash>
DUMP_RESULT CacheExtHash<KeyType, ValueType, FirstHash, SecondHash>::dump(const KeyType& key)
{
    ValueType val;
    FOUND_RESULT result = find(key, val);

    switch (result)
    {
    case NOT_FOUND:
        return NO_DUMP; //return not dump
        break;

    case FOUND_IN_MEM:
        //Make sure filehash_ is not full;
        if (fileHash_.num_items() < hashSize_ * (1 - ratio_) && memHash_.del(key))
        {
            if (fileHash_.insert(key, val))
                return DUMP_M2F;// return dump from memory to file.
        }
        break;

    case FOUND_IN_FILE:
        //make sure memHash_ is not full
        if (memHash_.num_items() < hashSize_ * ratio_ && fileHash_.del(key))
        {
            if (memHash_.insert(key, val))
                return DUMP_F2M;// return dump from file to memory.
        }
        break;

    default:
        break;
    }
    return DUMP_FAILED; //return dump failed.
}

}

}
#endif
