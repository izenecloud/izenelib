/**
 * @file sdb_hash.h
 * @brief The header file of sdb_hash.
 * @author peisheng wang
 *
 * @history
 * ==========================
 * 1. 2009-02-16 first version.
 * 2. 2009-12-10 thread safe
 *
 *
 * This file defines class sdb_hash.
 */
#ifndef SDB_HASH_H
#define SDB_HASH_H

#include <util/ClockTimer.h>
#include <string>
#include <queue>
#include <map>
#include <iostream>
#include <types.h>
#include <sys/stat.h>

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

template< typename KeyType, typename ValueType, typename LockType =NullLock, bool fixed=IsFixed<KeyType, ValueType>::yes> class sdb_hash :
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
    sdb_hash(const string& fileName = "sdb_hash.dat"):sfh_(), fileName_(fileName)
    {
        directorySize_ = (1<<sfh_.dpow);
        dmask_ = directorySize_ - 1;
        dataFile_ = 0;
        isOpen_ = false;
        activeNum_ = 0;
        dirtyPageNum_ = 0;
        cacheSize_ = 0;

        ksize_ = vsize_ = 0;
        if (fixed)
        {
            char* ptr = 0;
            char* ptr1 = 0;
            KeyType key;
            ValueType val;
            izene_serialization<KeyType> izs(key );
            izene_serialization<ValueType> izs1( val );
            izs.write_image(ptr, ksize_);
            izs1.write_image(ptr1, vsize_);
            BucketGap = ksize_+vsize_ + sizeof(long)+sizeof(int)+sizeof(size_t);
        }

    }

    /**
     *   deconstructor, close() will also be called here.
     */
    virtual ~sdb_hash()
    {
        if (dataFile_)
            close();
    }

    void setDataSize(const KeyType& key, const ValueType& val)
    {
        char* ptr = 0;
        char* ptr1 = 0;
        izene_serialization<KeyType> izs(key );
        izene_serialization<ValueType> izs1( val );
        izs.write_image(ptr, ksize_);
        izs1.write_image(ptr1, vsize_);
        BucketGap = ksize_+vsize_ + sizeof(long)+sizeof(int)+sizeof(size_t);
    }

    void clear()
    {
        close();
        std::remove(fileName_.c_str() );
        sfh_.initialize();
        open();
    }

    bool is_open()
    {
        return isOpen_;
    }
    /**
     *  \brief set bucket size of fileHeader
     *
     *   if not called use default size 8192
     */
    void setBucketSize(size_t bucketSize)
    {
        assert(isOpen_ == false);
        sfh_.bucketSize = bucketSize;
    }

    /**
     *  \brief set bucket size of fileHeader
     *
     *   if not called use default size 8192
     */
    void setPageSize(size_t pageSize)
    {
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

    void setDegree(size_t dpow)
    {
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
     *  \brief set file name.
     *
     */
    void setFileName(const std::string& fileName )
    {
        fileName_ = fileName;
    }

    /**
     * 	\brief return the file name of the SequentialDB
     */
    std::string getFileName() const
    {
        return fileName_;
    }

    /**
     *  insert an item of DataType
     */
    bool insert(const DataType<KeyType,ValueType>& dat)
    {
        return insert(dat.get_key(), dat.get_value() );
    }

    /**
     *  insert an item in key/value pair
     */
    bool insert(const KeyType& key, const ValueType& value)
    {
        if ( !isOpen_ )
            return false;
        flushCache_();
        SDBCursor locn;
        if ( search_(key, locn) )
            return false;
        else
        {
            char* ptr = 0;
            char* ptr1 = 0;
            size_t ksize;
            size_t vsize;
            izene_serialization<KeyType> izs(key);
            izene_serialization<ValueType> izs1(value);
            izs.write_image(ptr, ksize);
            izs1.write_image(ptr1, vsize);

            /*if(! fixed ) {
             izs.write_image(ptr, ksize);
             izs1.write_image(ptr1, vsize);

             } else {
             ptr = (char*)&key;
             ptr1 = (char*)&value;
             ksize = sizeof(key);
             vsize =sizeof(value);
             }*/

            bucket_chain* sa = locn.first;
            char* p = locn.second;

            //entry_[idx] is even NULL.
            if (locn.first == NULL)
            {
                uint32_t idx = sdb_hashing::hash_fun(ptr, ksize) & dmask_;;

                if (bucketAddr[idx] == 0)
                {
                    entry_[idx] = allocateBlock_();
                    bucketAddr[idx] = entry_[idx]->fpos;

                    sa = entry_[idx];
                    p = entry_[idx]->str;
                }
            }
            else
            {
                assert(locn.second != NULL);
                if ( !fixed )
                    BucketGap = ksize+vsize + sizeof(long)+sizeof(int) +3*sizeof(size_t);

                //add an extra size_t to indicate if reach the end of  bucket_chain.
                if ( size_t(p - sa->str)> sfh_.bucketSize-BucketGap )
                {
                    if (sa->next == 0)
                    {
                        //sa->isDirty = true;
                        setDirty_(sa);
                        sa->next = allocateBlock_();
                    }
                    sa = loadNext_( sa );
                    p = sa->str;
                }
            }

            if ( !fixed )
            {
                memcpy(p, &ksize, sizeof(size_t));
                p += sizeof(size_t);
                memcpy(p, &vsize, sizeof(size_t));
                p += sizeof(size_t);
                memcpy(p, ptr, ksize);
                p += ksize;
                memcpy(p, ptr1, vsize);
                p += vsize;
            }
            else
            {
                memcpy(p, ptr, ksize);
                p += ksize;
                memcpy(p, ptr1, vsize);
                p += vsize;
            }
            assert( size_t (p-sa->str) + sizeof(long) + sizeof(int) <= sfh_.bucketSize);
            //sa->isDirty = true;
            setDirty_(sa);
            ++sa->num;
            ++sfh_.numItems;
            return true;
        }

    }

    /**
     *  find an item, return pointer to the value.
     *  Note that, there will be memory leak if not delete the value
     */
    ValueType* find(const KeyType & key)
    {
        if ( !isOpen_ )
            return NULL;

        ScopedReadLock<LockType> lock(flushLock_);
        SDBCursor locn;
        if ( !search_(key, locn) )
            return NULL;
        else
        {

            char *p = locn.second;
            size_t ksz, vsz;
            ValueType *pval = new ValueType;

            if (!fixed)
            {
                memcpy(&ksz, p, sizeof(size_t));
                p += sizeof(size_t);
                memcpy(&vsz, p, sizeof(size_t));
                p += sizeof(size_t);

                izene_deserialization<ValueType> isd(p+ksz, vsz);
                isd.read_image(*pval);
            }
            else
            {
                //memcpy(pval, p+ksize_, vsize_);
                izene_deserialization<ValueType> isd(p+ksize_, vsize_);
                isd.read_image(*pval);
            }
            return pval;
        }
    }

    bool get(const KeyType& key, ValueType& value)
    {
        if ( !isOpen_ )
            return false;
        safeFlushCache_();
        ScopedReadLock<LockType> lock(flushLock_);
        SDBCursor locn;
        if ( !search_(key, locn) )
            return false;
        else
        {
            char *p = locn.second;
            if ( !fixed )
            {
                size_t ksz, vsz;
                memcpy(&ksz, p, sizeof(size_t));
                p += sizeof(size_t);
                memcpy(&vsz, p, sizeof(size_t));
                p += sizeof(size_t);

                izene_deserialization<ValueType> isd(p+ksz, vsz);
                isd.read_image(value);
            }
            else
            {
                izene_deserialization<ValueType> isd(p+ksize_, vsize_);
                isd.read_image(value);
                //memcpy(&value, p+ksize_, vsize_);
            }
            return true;
        }
    }

    /**
     *  delete  an item
     */
    bool del(const KeyType& key)
    {
        if ( !isOpen_ )
            return false;

        SDBCursor locn;
        if ( !search(key, locn) )
            return false;
        else
        {
            char *p = locn.second;
            if (!fixed)
            {

                size_t ksz, vsz;
                memcpy(&ksz, p, sizeof(size_t));
                p += sizeof(size_t);
                memcpy(&vsz, p, sizeof(size_t));
                p += sizeof(size_t);
                //memset(p, 0xff, ksz);
                size_t leftSize = sfh_.bucketSize-(2*sizeof(size_t)+ksz+vsz)-(p-locn.first->str);
                memcpy(p-2*sizeof(size_t), p+ksz+vsz, leftSize);
                --locn.first->num;

                //this is much slower.
                /*vsz += ksz+1;
                 ksz = -1;
                 memcpy(p-2*sizeof(size_t), &ksz, sizeof(size_t) );
                 memcpy(p-sizeof(size_t), &vsz, sizeof(size_t) );*/
            }
            else
            {
                size_t leftSize = sfh_.bucketSize-BucketGap-(p-locn.first->str);
                memcpy(p, p+ksize_+vsize_, leftSize);
                --locn.first->num;
            }

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
    bool update(const KeyType& key, const ValueType& value)
    {
        if ( !isOpen_ )
            return false;
        SDBCursor locn;
        if ( !search(key, locn) )
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

            if ( !fixed )
            {

                bucket_chain* sa = locn.first;
                char *p = locn.second;
                size_t ksz, vsz;
                memcpy(&ksz, p, sizeof(size_t));
                p += sizeof(size_t);
                memcpy(&vsz, p, sizeof(size_t));
                p += sizeof(size_t);
                if (vsz == vsize)
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
            }
            else
            {
                //char* ptr1 = (char*)&value;
                //size_t vsize = sizeof(value);

                bucket_chain* sa = locn.first;
                char *p = locn.second;
                {
                    setDirty_(sa);
                    memcpy(p+ksize, ptr1, vsize);
                    return true;
                }
            }

            return true;
        }
    }

    template<typename AM>
    bool dump(AM& other)
    {
        if (!is_open() )
            open();
        if ( !other.is_open() )
        {
            if ( !other.open() )
                return false;
        }
        SDBCursor locn = get_first_locn();
        KeyType key;
        ValueType value;
        while (get(locn, key, value))
        {
            other.insert(key, value);
            if ( !seq(locn) )
                break;
        }
        return true;
    }

    bool dump2f(const string& fileName)
    {
        sdb_hash other(fileName);
        if ( !other.open() )
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
        safeFlushCache_();
        ScopedReadLock<LockType> lock(flushLock_);
        return search_(key, locn);
    }

    bool search_(const KeyType &key, SDBCursor &locn)
    {
        if ( !isOpen_ )
            return false;

        locn.first = NULL;
        locn.second = NULL;

        char* ptr=0;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        /*if( !fixed) {
         izs.write_image(ptr, ksize);}
         else {
         ptr= (char*)&key;
         ksize = ksize_;
         }*/

        uint32_t idx = sdb_hashing::hash_fun(ptr, ksize) & dmask_;

        if ( entry_ == NULL )
            return false;
        locn.first = entry_[idx];

        if ( !entry_[idx] )
        {
            if ( bucketAddr[idx] != 0 )
            {
                entry_[idx] = new bucket_chain(sfh_.bucketSize, fileLock_);
                entry_[idx] ->fpos = bucketAddr[idx];
            }
        }

        if (entry_[idx] == NULL)
        {
            return false;
        }
        else
        {
            int i = 0;
            bucket_chain* sa = entry_[idx];

            load_( sa );

            char* p = sa->str;

            while ( sa )
            {
                locn.first = sa;
                //cout<<"search level: "<<sa->level<<endl;
                p = sa->str;
                //if( !p )return false;

                for (i=0; i<sa->num; i++)
                {
                    char *pd = ptr;
                    size_t j=0;
                    if (!fixed )
                    {
                        size_t ksz, vsz;
                        memcpy(&ksz, p, sizeof(size_t));
                        p += sizeof(size_t);
                        memcpy(&vsz, p, sizeof(size_t));
                        p += sizeof(size_t);

                        //cout<<ksz<<endl;
                        //cout<<vsz<<endl;

                        if (ksz != ksize)
                        {
                            p += ksz + vsz;
                            continue;
                        }

                        for (; j<ksz; j++)
                        {
                            //cout<<pd[j]<<" vs "<<p[j]<<endl;
                            if (pd[j] != p[j])
                            {
                                break;
                            }
                        }

                        if (j == ksz)
                        {
                            locn.second = p-2*sizeof(size_t);
                            //cout<<key<<" found"<<endl;
                            return true;
                        }
                        p += ksz + vsz;
                    }
                    else
                    {
                        for (; j<ksize; j++)
                        {
                            if (pd[j] != p[j])
                            {
                                break;
                            }
                        }
                        if (j == ksize)
                        {
                            locn.second = p;
                            return true;
                        }
                        p += ksize_ + vsize_;
                    }
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
        ScopedReadLock<LockType> lock(flushLock_);
        SDBCursor locn;
        locn.first = NULL;
        locn.second = NULL;
        if ( sfh_.numItems == 0 )
            return locn;

        for (size_t i=0; i<directorySize_; i++)
        {
            if ( !entry_[i] )
            {
                if ( bucketAddr[i] != 0 )
                {
                    entry_[i] = new bucket_chain(sfh_.bucketSize, fileLock_);
                    entry_[i] ->fpos = bucketAddr[i];
                }
            }
            if ( entry_[i] )
            {
                load_( entry_[i] );
                locn.first = entry_[i];
                locn.second = entry_[i]->str;
                break;
            }
        }

        KeyType key;
        ValueType value;
        while ( !get_(locn, key, value) )
        {
            seq_(locn);
        }
        return locn;
    }

    /**
     *  get an item from given SDBCursor
     */
    bool get(const SDBCursor& locn, DataType<KeyType,ValueType>& rec)
    {
        return get(locn, rec.key, rec.value);
    }

    bool get(const SDBCursor& locn, KeyType& key, ValueType& val)
    {
        ScopedReadLock<LockType> lock(flushLock_);
        return get_(locn, key, val);
    }

    bool get_(const SDBCursor& locn, KeyType& key, ValueType& val)
    {
        if ( !isOpen_ )
            return false;

        bucket_chain* sa = locn.first;
        char* p = locn.second;

        if (sa == NULL)return false;
        if (p == NULL)return false;
        if (sa->num == 0)return false;

        if ( !fixed )
        {
            size_t ksize, vsize;
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

        }
        else
        {
            //memcpy(&key, p, ksize_ );
            izene_deserialization<KeyType> izs(p, ksize_);
            izs.read_image(key);
            p += ksize_;

            //memcpy(&val, p, vsize_ );
            izene_deserialization<ValueType> izs1(p, vsize_);
            izs1.read_image(val);
            p += vsize_;
        }

        return true;

    }

    /**
     *   \brief sequential access method
     *
     *   @param locn is the current SDBCursor, and will replaced next SDBCursor when route finished.
     *   @param sdir is sequential access direction, for hash is unordered, we only implement forward case.
     *
     */

    bool seq(SDBCursor& locn, KeyType& key, ValueType& value, util::ESeqDirection sdir=util::ESD_FORWARD)
    {
        bool ret = seq(locn);
        get(locn, key, value);
        return ret;
    }
    bool seq(SDBCursor& locn, DataType<KeyType, ValueType>& dat, util::ESeqDirection sdir=util::ESD_FORWARD)
    {
        return seq(locn, dat.key, dat.value, sdir);
    }

    bool seq(SDBCursor& locn, util::ESeqDirection sdir=util::ESD_FORWARD)
    {
        safeFlushCache_(locn);
        ScopedReadLock<LockType> lock(flushLock_);
        return seq_(locn, sdir);
    }

    bool seq_(SDBCursor& locn, util::ESeqDirection sdir=util::ESD_FORWARD)
    {
        if ( !isOpen_ )
            return false;

        if ( sdir == util::ESD_FORWARD )
        {
            bucket_chain* sa = locn.first;
            char* p = locn.second;

            if (sa == NULL)return false;
            if (p == NULL)return false;

            char* ptr = 0;
            size_t poff = 0;

            while (true)
            {

                if (!fixed)
                {

                    size_t ksize = 0, vsize;

                    if ( sa->num != 0)
                    {
                        memcpy(&ksize, p, sizeof(size_t));
                        p += sizeof(size_t);
                        memcpy(&vsize, p, sizeof(size_t));
                        p += sizeof(size_t);

                        ptr = p;
                        poff = ksize;
                        p += ksize;
                        p += vsize;
                        memcpy(&ksize, p, sizeof(size_t));
                    }
                    if ( ksize == 0 )
                    {
                        sa = loadNext_(sa);
                        while ( sa && sa->num <= 0)
                        {
                            sa = loadNext_(sa);
                        }
                        if ( sa )
                        {
                            p = sa->str;
                        }
                        else
                        {
                            uint32_t idx = sdb_hashing::hash_fun(ptr, poff) & dmask_;

                            while ( isEmptyBucket_(++idx, sa ) )
                            {
                                if ( idx >= directorySize_ -1)
                                {
                                    break;
                                }
                            }

                            if ( sa ) p = sa->str;
                            else
                                p = NULL;
                        }
                    }

                }
                else
                {
                    ptr = p;
                    /*poff = ksize_;
                     p += ksize_;
                     p += vsize_;*/
                    poff = ksize_;
                    p += ksize_;
                    p += vsize_;

                    if ( (unsigned int)(p-locn.first->str) >= (unsigned int)(locn.first->num)*(ksize_ + vsize_) )
                    {
                        sa = loadNext_(sa);
                        while ( sa && sa->num <= 0)
                        {
                            sa = loadNext_(sa);
                        }
                        if ( sa )
                        {
                            p = sa->str;
                        }
                        else
                        {
                            uint32_t idx = sdb_hashing::hash_fun(ptr, poff) & dmask_;
                            while ( isEmptyBucket_(++idx, sa ) )
                            {
                                if ( idx >= directorySize_ -1)
                                {
                                    break;
                                }
                            }

                            //get next bucket;
                            //sa = entry_[idx];
                            if ( sa )
                                p = sa->str;
                            else
                                p = NULL;
                        }
                    }
                }

                locn.first = sa;
                locn.second = p;
                return true;
            }
        }
        else
        {
            //it seems unecessary, for items are unordered.
            return false;
        }
    }

    /**
     *   get the num of items
     */
    int num_items()
    {
        return sfh_.numItems;
    }

    void fillCache()
    {
        if ( !isOpen_ )
            return;

        if (sfh_.numItems == 0)
            return;

        typedef map<long, bucket_chain*> COMMIT_MAP;
        typedef typename COMMIT_MAP::iterator CMIT;
        COMMIT_MAP toBeRead, nextRead;
        for (size_t i=0; i<directorySize_; i++)
        {
            if ( entry_&& entry_[i] && entry_[i]->fpos !=0 )
                toBeRead.insert(make_pair(entry_[i]->fpos, entry_[i]) );
        }
        while ( !toBeRead.empty() )
        {
            CMIT it = toBeRead.begin();
            for (; it != toBeRead.end(); it++)
            {
                load_( it->second);
                if ( activeNum_> sfh_.cacheSize )
                    return;
                if (it->second->next)
                    nextRead.insert(make_pair(it->second->nextfpos, it->second->next) );
            }
            toBeRead = nextRead;
            nextRead.clear();
        }
        //
        //				queue<bucket_chain*> qnode;
        //				for (size_t i=0; i<directorySize_; i++) {
        //					load_( entry_[i] );
        //					if( entry_[i] && entry_[i]->isLoaded )
        //					qnode.push( entry_[i] );
        //				}
        //				while (!qnode.empty() ) {
        //					bucket_chain* popNode = qnode.front();
        //					if ( popNode && popNode->isLoaded)
        //					qnode.pop();
        //					if (popNode && popNode->next ) {
        //						bucket_chain* node = loadNext_(popNode);
        //						if( node )
        //						qnode.push(node );
        //						if( activeNum_> sfh_.cacheSize )
        //						break;
        //					}
        //				}
    }
public:
    /**
     *   db must be opened to be used.
     */
    bool open()
    {

        if (isOpen_) return true;
        struct stat statbuf;
        bool creating = stat(fileName_.c_str(), &statbuf);

        dataFile_ = fopen(fileName_.c_str(), creating ? "w+b" : "r+b");
        if ( 0 == dataFile_)
        {
            cout<<"Error in open(): open file failed"<<endl;
            return false;
        }
        bool ret = false;
        if (creating)
        {

            // We're creating if the file doesn't exist.
#ifdef DEBUG
            cout<<"creating sdb_hash: "<<fileName_<<"...\n"<<endl;
            sfh_.display();
#endif
            bucketAddr = new long[directorySize_];
            entry_ = new bucket_chain*[directorySize_];

            //initialization
            memset(bucketAddr, 0, sizeof(long)*directorySize_);
            memset(entry_ , 0, sizeof(bucket_chain*)*directorySize_);
            commit();
            ret = true;
        }
        else
        {
            if ( !sfh_.fromFile(dataFile_) )
            {
                return false;
            }
            else
            {
                if (sfh_.magic != 0x061561)
                {
                    cout<<"Error, read wrong file header_\n"<<endl;
                    return false;
                }
                if (cacheSize_ != 0)
                    sfh_.cacheSize = cacheSize_;
#ifdef DEBUG
                cout<<"open exist sdb_hash: "<<fileName_<<"...\n"<<endl;
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

                for (size_t i=0; i<directorySize_; i++)
                {
                    if (bucketAddr[i] != 0)
                    {
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
        if ( isOpen_ == false )
            return true;

        isOpen_ = false;
        flush();

        delete [] bucketAddr;
        bucketAddr = 0;
        delete [] entry_;
        entry_ = 0;
        if (dataFile_)
        {
            fclose(dataFile_);
            dataFile_ = 0;
        }
        return true;
    }
    /**
     *  write the dirty buckets to disk, not release the memory
     *
     */
    void commit()
    {
        if ( !dataFile_ )
            return;
        sfh_.toFile(dataFile_);
        if (directorySize_ != fwrite(bucketAddr, sizeof(long),
                                     directorySize_, dataFile_) )
            return;
        if (orderedCommit)
        {
            if ( ! entry_ )
                return;
            typedef map<long, bucket_chain*> COMMIT_MAP;
            typedef typename COMMIT_MAP::iterator CMIT;
            COMMIT_MAP toBeWrited;
            queue<bucket_chain*> qnode;
            for (size_t i=0; i<directorySize_; i++)
            {
                qnode.push( entry_[i]);
            }
            while (!qnode.empty() )
            {
                bucket_chain* popNode = qnode.front();
                if ( popNode && popNode->isLoaded && popNode-> isDirty)
                    toBeWrited.insert(make_pair(popNode->fpos, popNode) );
                qnode.pop();
                if (popNode && popNode->next )
                {
                    qnode.push( popNode->next );
                }
            }

            CMIT it = toBeWrited.begin();
            for (; it != toBeWrited.end(); it++)
            {
                if ( it->second->write( dataFile_ ) )
                    --dirtyPageNum_;
            }
        }
        else
        {
            for (size_t i=0; i<directorySize_; i++)
            {
                bucket_chain* sc = entry_[i];
                while (sc)
                {
                    if ( sc->write(dataFile_) )
                    {
                        sc = sc->next;
                    }
                    else
                    {
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
    void flush()
    {
#ifdef DEBUG
        izenelib::util::ClockTimer timer;
#endif
        commit();
#ifdef DEBUG
        printf("commit elapsed 1 ( actually ): %lf seconds\n",
               timer.elapsed() );
#endif
        ScopedWriteLock<LockType> lock(flushLock_);
        unload_();
    }

    void unload_()
    {
        if (entry_)
        {
            for (size_t i=0; i<directorySize_; i++)
            {
                if (entry_[i])
                {
                    delete entry_[i];
                    entry_[i] = 0;
                }
            }
        }
        activeNum_ = 0;

    }
    /**
     *  display the info of sdb_hash
     */
    void display(std::ostream& os = std::cout, bool onlyheader = true)
    {
        sfh_.display(os);
        os<<"activeNum: "<<activeNum_<<endl;
        os<<"dirtyPageNum "<<dirtyPageNum_<<endl;
        //os<<"loadFactor: "<<loadFactor()<<endl;

        if ( !onlyheader)
        {
            for (size_t i=0; i<directorySize_; i++)
            {
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
    double loadFactor()
    {
        int nslot = 0;
        for (size_t i=0; i<directorySize_; i++)
        {
            bucket_chain* sc = entry_[i];
            while (sc)
            {
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
    LockType flushLock_;
private:
    size_t directorySize_;
    size_t dmask_;
    size_t cacheSize_;

    size_t ksize_;
    size_t vsize_;
    size_t BucketGap;

    unsigned long initRss_;
    unsigned int flushCount_;

    void setDirty_(bucket_chain* bucket)
    {
        if ( !bucket->isDirty )
        {
            ++dirtyPageNum_;
            bucket->isDirty = true;
        }
    }

    bool load_(bucket_chain* bucket)
    {
        if (bucket && !bucket->isLoaded )
        {
            ++activeNum_;
            return bucket->load(dataFile_);

        }
        return false;

    }
    /**
     *   Allocate an bucket_chain element
     */
    bucket_chain* allocateBlock_()
    {
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

    bucket_chain* loadNext_(bucket_chain* current)
    {
        bool loaded = false;
        bucket_chain* next = current->loadNext(dataFile_, loaded);
        if (loaded)
            activeNum_++;
        return next;
    }

    bool isEmptyBucket_(uint32_t idx, bucket_chain* &sa)
    {
        if (idx >= directorySize_)
        {
            sa = NULL;
            return false;
        }

        if ( !entry_[idx] )
        {
            if ( bucketAddr[idx] != 0 )
            {
                entry_[idx] = new bucket_chain(sfh_.bucketSize, fileLock_);
                entry_[idx] ->fpos = bucketAddr[idx];
            }
        }
        sa = entry_[idx];
        load_(sa);
        while (sa && sa->num <= 0)
        {
            sa = loadNext_(sa);
        }
        return sa == NULL;

    }

    /**
     *  when cache is full, it was called to reduce memory usage.
     *
     */
    void safeFlushCache_()
    {
        if (activeNum_> sfh_.cacheSize)
        {
            ScopedWriteLock<LockType> lock(flushLock_);
            flushCacheImpl_();
        }
    }

    void flushCache_()
    {
        if (activeNum_> sfh_.cacheSize)
        {
            flushCacheImpl_();
        }

    }

    void safeFlushCache_(SDBCursor &locn)
    {
        if (activeNum_> sfh_.cacheSize)
        {
            KeyType key;
            ValueType value;
            get(locn, key, value);
            {
                ScopedWriteLock<LockType> lock(flushLock_);
                flushCacheImpl_();
            }
            search(key, locn);
        }
    }

    void flushCacheImpl_()
    {
#ifdef DEBUG
        cout<<"cache is full..."<<endl;
        sfh_.display();
        cout<<activeNum_<<" vs "<<sfh_.cacheSize <<endl;
        cout<<"dirtyPageNum: "<<dirtyPageNum_<<endl;
#endif
        //		cout<<"begin activePageNum"<<activeNum_<<",dirtyPageNum: "<<dirtyPageNum_<<std::endl;

        bool commitCondition = dirtyPageNum_ >= (activeNum_ * 0.5);

        if (unloadAll)
        {
            flush();
#ifdef DEBUG
            cout<<"\n====================================\n"<<endl;
            cout<<"cache is full..."<<endl;
            sfh_.display();
            cout<<activeNum_<<" vs "<<sfh_.cacheSize <<endl;
            cout<<"dirtyPageNum: "<<dirtyPageNum_<<endl;
#endif
            return;
        }
        else
        {

            if ( delayFlush && commitCondition)
                commit();
            for (size_t i=0; i<directorySize_; i++)
            {
                bucket_chain* sc = entry_[i];
                while (sc)
                {
                    if (sc->isLoaded && !sc->isDirty)
                        sh_cache_.insert(make_pair(sc->level, sc));
                    sc = sc->next;
                }
            }

            for (CacheIter it = sh_cache_.begin(); it != sh_cache_.end(); it++)
            {
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

            if (delayFlush && commitCondition)
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
