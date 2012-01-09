/**
 * @file CacheInfo.h
 * @brief The header file of CacheInfo.
 *
 * This file defines struct CacheInfo and comparision function object
 *  of CacheInfo to implement different replacement algorithm.
 */

#ifndef CACHEINFO_H
#define CACHEINFO_H


namespace izenelib
{
namespace cache
{

/**
* 	\brief It stores the necessary info for Caching Algorithm.
*    	Other properties can be added if necessary. And differnt comparision
*	function object of CacheInfo are used to implement different replacement a
*	algorthm.
*
*/
template <class KeyType>
struct CacheInfo
{
    KeyType key;
    size_t 	docSize;
    bool isHit; // for SLRU
    unsigned int LastAccessTime;
    unsigned int FirstAccessTime;
    unsigned int TimeToLive;  //reserved for further use.
    unsigned int iCount;

    template<class Archive>
    void serialize(Archive & ar,
                   const unsigned int version)
    {
        ar & key;
        ar & docSize;
        ar & isHit;
        ar & LastAccessTime;
        ar & FirstAccessTime;
        ar & TimeToLive;
        ar & iCount;
    }
};

template <class KeyType>
struct keyCmp
{
    bool operator() (const CacheInfo<KeyType> &lhs, const CacheInfo<KeyType>  &rhs) const
    {
        return lhs.key < rhs.key;
    }
};

/*
*
* 	for lru caching algorithm.
*/
template <class KeyType>
struct lruCmp
{
    bool operator() (const CacheInfo<KeyType> &lhs, const CacheInfo<KeyType> &rhs) const
    {
        return lhs.LastAccessTime < rhs.LastAccessTime
            || (lhs.LastAccessTime == rhs.LastAccessTime && lhs.iCount < rhs.iCount)
            || (lhs.LastAccessTime == rhs.LastAccessTime && lhs.iCount == rhs.iCount && lhs.key < rhs.key);
    }
};

/*
*
* 	for lru caching algorithm.
*/
template <class KeyType>
struct lruCmp1
{
    bool operator() (const CacheInfo<KeyType> &lhs, const CacheInfo<KeyType> &rhs) const
    {
        return lhs.LastAccessTime < rhs.LastAccessTime
            || (lhs.LastAccessTime == rhs.LastAccessTime && lhs.key < rhs.key);
    }
};

/*
*
* 	for lfu caching algorithm.
*/
template <class KeyType>
struct lfuCmp
{
    bool operator() (const CacheInfo<KeyType> &lhs, const CacheInfo<KeyType> &rhs) const
    {
        return lhs.iCount < rhs.iCount
            || (lhs.iCount == rhs.iCount && lhs.LastAccessTime < rhs.LastAccessTime)
            || (lhs.iCount == rhs.iCount && lhs.LastAccessTime == rhs.LastAccessTime && lhs.key < rhs.key);
    }
};


/*
*
* 	for slru caching algorithm.
* 	As for slru, it is assumed that the item hits have higher priority than the items not hits.
*/
template <class KeyType>
struct slruCmp
{

    //The item hits has higher priority than the item not hits.
    bool operator() (const CacheInfo<KeyType> &lhs, const CacheInfo<KeyType> &rhs) const
    {
        return lhs.isHit < rhs.isHit
            || (lhs.isHit == rhs.isHit && lhs.LastAccessTime < rhs.LastAccessTime)
            || (lhs.isHit == rhs.isHit && lhs.LastAccessTime == rhs.LastAccessTime && lhs.iCount < rhs.iCount)
            || (lhs.isHit == rhs.isHit && lhs.LastAccessTime == rhs.LastAccessTime && lhs.iCount == rhs.iCount && lhs.key < rhs.key);
    }
};

}
}

#endif //CacheInfo_H
