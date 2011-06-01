/**
 * @file CacheContainer.h
 * @brief The header file of CacheContainer.
 *
 * This file defines class CacheContainer.
 */


#ifndef CACHECONTAINER_H
#define CACHECONTAINER_H

#include "CacheInfo.h"
#include "cm_basics.h"

#include <ctime>
#include <map>
#include <ext/hash_map>

using  namespace std;
using  namespace   __gnu_cxx;
using namespace boost;


namespace izenelib
{

namespace cache
{


/**
* 	\brief Cache Container for CacheInfo.
*
*	It can deal with different type of key-data pair using different Replacement
*	Policies to store caching info.
*
*   KeyType and DataType    :   the type of what we want to cache.
*	RelacementPolciy        :   it can be lruCmp, lfuCmp or slruCmp, which are defined in CacheInfo.h.
*
*/
template <class KeyType, class ValueType, class ReplacementPolicy>
class CacheContainer
{
public:
    enum {LIFE_TIME = 86400*7};//set the lifetime of the cache item,  a week.
    typedef map<CacheInfo<KeyType>, bool, ReplacementPolicy >  CacheInfoKeyMap;
    typedef hash_map<KeyType, CacheInfo<KeyType>, HashFun<KeyType> > CmHashMap;
    typedef typename map<CacheInfo<KeyType>, bool, ReplacementPolicy  > :: iterator MIT;
    typedef typename hash_map<KeyType, CacheInfo<KeyType>, HashFun<KeyType> > ::iterator HIT;
    //typedef izenelib::am::DataType<KeyType,ValueType> DataType;
public:
    CacheContainer()
    {
        seqNo_ = 0;
    }

    /**
    *	\brief It determines if an item exits in CacheContainer.
    */
    bool find(const KeyType& key)
    {
        return CacheInfoHash_.find(key) != CacheInfoHash_.end();
    }
    /**
    *	\brief It gets the first item, i.e the item with lowest priority.
    */
    MIT begin()
    {
        return keyInfoMap_.begin();
    }
    /**
    *	\brief It gets the last item.
    */
    MIT end()
    {
        return keyInfoMap_.end();
    }
    /**
    *	\brief It finds the next item.
    */
    MIT next(MIT& it)
    {
        return ++it;
    }
    /**
    *	\brief It determines  if an item is out of date in CacheContainer.
    */
    bool isOutOfDate(const KeyType& key)
    {
        return keyInfoMap_[CacheInfoHash_[key]];
    }

    void replace(const KeyType& key);
    void firstInsert(const KeyType& key);
    void del(const KeyType& key);

    void insert( const KeyType& key)
    {
        if ( find(key) )
        {
            replace(key);
        }
        else
        {
            firstInsert(key);
        }
    }
    size_t size()
    {
        return CacheInfoHash_.size();
    }

    void display()
    {
        cout<<"CacheInfoHash size:"<<CacheInfoHash_.size()<<endl;
        cout<<"keyInfoMap_ size:"<<keyInfoMap_.size()<<endl;
    }

    void setTimeToLive(const KeyType &key, time_t lifeTime);
    CacheInfo<KeyType>& getCacheInfo(const KeyType& key)
    {
        return CacheInfoHash_[key];
    }
    void UpdateKeyInfoMap();
    void printKeyInfoMap();


    template<typename Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & keyInfoMap_;
        ar & CacheInfoHash_;
    }



private:
    CmHashMap CacheInfoHash_;	//Map key to the corresponding CacheInfo.
    CacheInfoKeyMap keyInfoMap_;	//Container of CacheInfo.
    unsigned int  seqNo_;
};

/**
*	\brief display  KeyInfoMap_ inside, for Debug.
*
*/
template <class KeyType, class ValueType, class ReplacementPolicy>
void  CacheContainer<KeyType, ValueType ,ReplacementPolicy>:: printKeyInfoMap()
{
    cout<<"KeyInforMap_ Info chain: "<<endl;
    for (MIT pos = keyInfoMap_.begin(); pos != keyInfoMap_.end(); pos++)
    {
        KeyType key = pos->first.key;
        cout<<"->"<<CacheInfoHash_[key].key<< "(" << CacheInfoHash_[key].iCount
            <<" "<<CacheInfoHash_[key].FirstAccessTime <<" "<<CacheInfoHash_[key].LastAccessTime
            <<" "<<CacheInfoHash_[key].TimeToLive<< ")";
    }
    cout<<endl;
}


/**
*	 \brief When get an item from the cacheContainer, update the corresponding CacheInfoHash_ and keyInfoMap_.
*
*/
template <class KeyType, class ValueType, class ReplacementPolicy>
void  CacheContainer<KeyType, ValueType ,ReplacementPolicy>:: replace(const KeyType& key)
{

    CacheInfo<KeyType> oldCacheInfo = CacheInfoHash_[key];
    CacheInfo<KeyType> newCacheInfo;

    newCacheInfo.isHit = 1;
    newCacheInfo.key = key;
    //newCacheInfo.docSize= oldCacheInfo.docSize;
    newCacheInfo.FirstAccessTime = oldCacheInfo.FirstAccessTime;
    newCacheInfo.LastAccessTime = ++seqNo_; //for debug, not add 1 actually.
    newCacheInfo.TimeToLive = oldCacheInfo.TimeToLive;
    newCacheInfo.iCount = oldCacheInfo.iCount + 1;

    CacheInfoHash_[key] = newCacheInfo;
    keyInfoMap_.erase(oldCacheInfo);
    keyInfoMap_[newCacheInfo] = 0;

}

/**
* 	\brief When insert an new item into the cache, update the corresponding CacheInfoHash_ and keyInfoMap_.
*/
template <class KeyType, class ValueType, class ReplacementPolicy>
void  CacheContainer<KeyType, ValueType ,ReplacementPolicy>:: firstInsert(const KeyType& key)
{
    CacheInfo<KeyType> newCacheInfo;
    newCacheInfo.key = key;
    //newCacheInfo.docSize = size of(hash_[key]);
    newCacheInfo.isHit = 0;
    newCacheInfo.FirstAccessTime = newCacheInfo.LastAccessTime = ++seqNo_;
    newCacheInfo.TimeToLive = LIFE_TIME;
    newCacheInfo.iCount = 1;

    keyInfoMap_[newCacheInfo] = 0;
    CacheInfoHash_[key] = newCacheInfo;
}

/**
* 	\brief When delele an item from the cache, update the corresponding CacheInfoHash_ and keyInfoMap_.
*/
template <class KeyType, class ValueType, class ReplacementPolicy>
void  CacheContainer<KeyType, ValueType ,ReplacementPolicy>:: del(const KeyType& key)
{
    //if(keyInfoMap_.erase(CacheInfoHash_[key]))
    keyInfoMap_.erase(CacheInfoHash_[key]);
    CacheInfoHash_.erase(key);

    //keyInfoMap_[CacheInfoHash_[key]] = 0;
    //CacheInfoHash_[key] = 0;
    //Remove the corresponding CacheInfo.
}


/**
*  	\brief It sets the TimeToLive of one item in Cache, can be used to indicate the item is out of date.
*
*/
template <class KeyType, class ValueType, class ReplacementPolicy>
void  CacheContainer<KeyType, ValueType ,ReplacementPolicy>::setTimeToLive(const KeyType &key, clock_t lifeTime)
{
    CacheInfoHash_[key].TimeToLive = lifeTime;
}


/**
 *	\brief To detemine which item is out of date by the CacheInfos.
 */
template <class KeyType, class ValueType, class ReplacementPolicy>
void  CacheContainer<KeyType, ValueType ,ReplacementPolicy>:: UpdateKeyInfoMap()
{

    MIT it = keyInfoMap_.begin();
    while ( it != keyInfoMap_.end() )
    {
        MIT it1 = it;
        it++;
        KeyType key = it1->first.key;
        unsigned int TimeToLive = CacheInfoHash_[key].TimeToLive;
        unsigned int FirstAccessTime = it1->first.FirstAccessTime;
        unsigned int LastAccessTime = it1->first.LastAccessTime;

        //Diffenent rule to detemine a item is out of date can be implemented here.

        if ( (LastAccessTime - FirstAccessTime) > TimeToLive )
            keyInfoMap_[CacheInfoHash_[key]] = 1;
        else
        {
            //cout<<key<<" "<<Now<< (Now + LastAccessTime - 2*FirstAccessTime > TimeToLive*2) <<endl;
        }

    }

}

}

}
#endif //CacheContainer


