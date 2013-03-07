/**
 * @file HugeDB.h
 * @brief Implementation of HugeDB,
 *        Enhancement to SequentialDB for random accesses on large scale data set.
 *        See my TR docs/pdf/hdb_draft.pdf for my information.
 * @author Wei Cao
 * @date 2009-09-11
 */

#ifndef _HUGEDB_H_
#define _HUGEDB_H_

#include <cstdlib>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <am/concept/DataType.h>
#include <am/sdb_btree/sdb_btree.h>

#include <sdb/SequentialDB.h>

#include "MultiSDBCursor.h"
#include "HDBCursor.h"
#include "HDBHeader.h"

namespace izenelib {

namespace hdb {

/**
 * @brief
 * SequentialDB, is fast for sequential accesses, however not good at random
 * insertions and look up.
 *
 * HugeDB, is based on SequentialDB, but use a keep-modifications-in-memory,
 * flush-small-db-from-memory-to-disk, and merge-small-dbs-to-large-one-on-disk
 * techniques to provide scalable performance for random accesses.
 *
 * If you are to maintain large scale of data on disk and have random insertions,
 * use this one.
 */

template< typename KeyType,
          typename ValueType,
          typename LockType,
          typename ContainerType,
          bool SupportDelta,
          typename Alloc=std::allocator<DataType<KeyType, std::pair<char, ValueType> > > >
class HugeDB {

protected:

    typedef HugeDB<KeyType, ValueType, LockType, ContainerType, SupportDelta, Alloc> ThisType;

    typedef std::pair<char, ValueType> TagType;

    typedef SequentialDB<KeyType, TagType, LockType, ContainerType, Alloc> SdbType;

    typedef MultiSDBCursor_<ThisType> MultiSDBCursor;

    friend class MultiSDBCursor_<ThisType>;

    /**
     * @brief Hdb partition, contain both data and metadata about a partition.
     */
    class SdbInfo {
    public:
        SdbType sdb;
        int level;
        size_t deletions;
        SdbInfo(std::string name) : sdb(name), level(0), deletions(0) {}
    };

public:

    static const int DEFAULT_CACHED_RECORDS_NUM = 2000000;
    static const int DEFAULT_MERGE_FACTOR = 2;
    static const int DEFAULT_BTREE_CACHED_PAGE_NUM = 8*1024;
    static const int DEFAULT_BTREE_PAGE_SIZE = 8*1024;
    static const int DEFAULT_BTREE_DEGREE = 128;

    /**
     * @brief HDBCursor can be used to iterate the whole hdb,
     *        the same as SequentialDB::SDBCursor
     */
    typedef HDBCursor_<ThisType> HDBCursor;

    friend class HDBCursor_<ThisType>;

public:

    /*************************************************
     *           Constructor/Destructor
     *************************************************/

    /**
     * @brief Constructor.
     * @param hdbName - identifier of hdb
     */
    HugeDB(const std::string &hdbName)
        : hdbName_(hdbName), isOpen_(false),
          headerPath_(hdbName_ + ".hdb.header.xml"),
          header_(headerPath_),
          cachedRecordsNumber_(DEFAULT_CACHED_RECORDS_NUM),
          mergeFactor_(DEFAULT_MERGE_FACTOR),
          pageSize_(DEFAULT_BTREE_PAGE_SIZE),
          degree_(DEFAULT_BTREE_DEGREE),
          memorySdb_(hdbName_ + ".memorycache")
    {
#ifdef VERBOSE_HDB
        header_.display();
#endif
        for(size_t i = 0; i<header_.slicesNum; i++)
        {
            SdbInfo* sdbi = new SdbInfo(getSdbName(i,header_.slicesLevel[i]));
            sdbi->sdb.setCacheSize(DEFAULT_BTREE_CACHED_PAGE_NUM);
            sdbi->level = header_.slicesLevel[i];
            sdbi->deletions = header_.deletions[i];
            diskSdbList_.push_back(sdbi);
        }

        memorySdb_.setPageSize(pageSize_);
        memorySdb_.setDegree(degree_);
        memorySdb_.setCacheSize( (size_t)-1 );
        memorySdbDeletion_ = header_.memoryPartitionDeletion;

        lastModificationStamp_ = header_.lastModificationStamp;
        header_.flush();
    }

    ~HugeDB()
    {
        close();

        for(size_t i = 0; i<diskSdbList_.size(); i++)
            delete diskSdbList_[i];
        diskSdbList_.clear();
    }

    /*************************************************
     *               Set Parameters
     *************************************************/

    /**
     * @brief Cache at most cachedRecordsNumber numbers of records in memory.
     */
    void setCachedRecordsNumber(int cachedRecordsNumber)
    {
        if(isOpen_)
            throw std::runtime_error("cannot set cache size after opened");
        if(cachedRecordsNumber <= 0)
            throw std::runtime_error("cachedRecordsNumber should be positive");
        cachedRecordsNumber_ = cachedRecordsNumber;
    }

    /**
     * @brief Mergefactor controls when pieces of hdb partition will merge.
     *        All hdb partition starts from level 0.
     *        A merge happens when there are M numbers of level L hdb partitions exist on disk,
     *        they will be merged to a single level L+1 hdb partition.
     *        Default mergefactor is set to 2, a value of 4 is recommened in practice.
     */
    void setMergeFactor(size_t mergeFactor)
    {
        if(isOpen_)
            throw std::runtime_error("cannot set merge factor after opened");
        if(mergeFactor < 2)
        {
            std::cerr << "invalid mergeFactor setting, should >= 2 "
                <<  mergeFactor << std::endl;
            return;
        }
        mergeFactor_ = mergeFactor;
    }

    /**
     * @brief Set Btree's page size, the same as SequentialDB.
     *        Default page size is 8K.
     */
	void setPageSize(size_t pageSize) {
        if(isOpen_)
            throw std::runtime_error("cannot set cache size after opened");
        if(pageSize <= 0 || pageSize % 512)
            throw std::runtime_error("pageSize should be positive and aligned to 512bytes");
		pageSize_ = pageSize;
	}

    /**
     * @brief Set Btree's degree, the same as SequentialDB.
     *        Default degree is 128, which means the number of children
     *        of each Btree node ranges from 129~256
     */
	void setDegree(int degree) {
        if(isOpen_)
            throw std::runtime_error("cannot set merge factor after opened");
        if(degree <= 0)
            throw std::runtime_error("degree should be positive");
		degree_ = degree;
	}

    /*************************************************
     *               Open/Clear/Flush/Close
     *************************************************/

    /**
     * Open a hdb.
     */
    void open()
    {
        if(!isOpen_) {

            diskSdbLock_.lock();
            for(size_t i = 0; i<diskSdbList_.size(); i++)
                diskSdbList_[i]->sdb.open();
            diskSdbLock_.unlock();

            memorySdbLock_.lock();
            memorySdb_.open();
            memorySdbLock_.unlock();

            isOpen_ = true;
        }
    }

    /**
     * Clear all records in a hdb.
     */
    void clear()
    {
        if(isOpen_) {

            diskSdbLock_.lock();
            for(size_t i = 0; i<diskSdbList_.size(); i++) {
                std::string n = diskSdbList_[i]->sdb.getName();
                diskSdbList_[i]->sdb.close();
                delete diskSdbList_[i];
                std::remove(n.c_str());
            }
            diskSdbList_.clear();
            lastModificationStamp_ ++;
            diskSdbLock_.unlock();

            memorySdbLock_.lock();
            memorySdb_.clear();
            memorySdbDeletion_ = 0;
            lastModificationStamp_ ++;
            memorySdbLock_.unlock();

            // update header
            flush();
            isOpen_ = false;
        }
    }

    /**
     * Close a hdb.
     */
    void close()
    {
        if(isOpen_) {

            flush();

            diskSdbLock_.lock();
            for(size_t i = 0; i<diskSdbList_.size(); i++)
                diskSdbList_[i]->sdb.close();
            diskSdbLock_.unlock();

            memorySdbLock_.lock();
            memorySdb_.close();
            memorySdbDeletion_ = 0;
            memorySdbLock_.unlock();

            isOpen_ = false;
        }
    }

    /**
     * Release all memory.
     */
    void release()
    {
        if(isOpen_) {

            flush();

            for(size_t i = 0; i<diskSdbList_.size(); i++) {
                std::string n = diskSdbList_[i]->sdb.getName();
                int l = diskSdbList_[i]->level;
                size_t d = diskSdbList_[i]->deletions;

                diskSdbList_[i]->sdb.close();
                delete diskSdbList_[i];

                SdbInfo* sdbi = new SdbInfo(n);
                sdbi->level = l;
                sdbi->deletions = d;
                sdbi->sdb.setCacheSize(DEFAULT_BTREE_CACHED_PAGE_NUM);
                sdbi->sdb.open();
                diskSdbList_[i] = sdbi;
            }
        }
    }

    /**
     * Flush all records in hdb to disk.
     */
    void flush()
    {
        if(isOpen_) {

            // Flush header
            header_.slicesNum = diskSdbList_.size();
            header_.deletions.resize(header_.slicesNum);
            header_.slicesLevel.resize(header_.slicesNum);
            header_.lastModificationStamp = lastModificationStamp_;
            for(size_t i = 0; i<diskSdbList_.size(); i++) {
                header_.deletions[i] = diskSdbList_[i]->deletions;
                header_.slicesLevel[i] = diskSdbList_[i]->level;
            }
            header_.flush();

            // Flush all on-disk partitions
            for(size_t i = 0; i<diskSdbList_.size(); i++)
                diskSdbList_[i]->sdb.flush();

            header_.memoryPartitionDeletion = memorySdbDeletion_;
            memorySdb_.flush();
        }
    }

    /*************************************************
     *               HDB Access Methods
     *************************************************/

	/**
	 * @brief Insert value for a given key into sdb, do not modify anything if key exists.
	 */
	void insertValue(const KeyType& key, const ValueType& value)
	{
	    tryToMerge();

        TagType tag = TagType();

        memorySdbLock_.lock();
	    bool keyExist = memorySdb_.getValue(key, tag);
	    if(!keyExist || tag.first == DELETE) {
            memorySdb_.update(key, TagType(INSERT, value));
            lastModificationStamp_ ++;
        }
        memorySdbLock_.unlock();
	}

	/**
	 * @brief Update value for a given key in sdb, overwrite if key exists.
	 */
	void update(const KeyType& key, const ValueType& value)
	{
	    tryToMerge();

        memorySdbLock_.lock();
	    memorySdb_.update(key, TagType(UPDATE, value));
        lastModificationStamp_ ++;
        memorySdbLock_.unlock();
	}

	/**
	 * @brief Delete record .
	 */
	void del(const KeyType& key)
	{
	    tryToMerge();

        TagType tag = TagType();

        memorySdbLock_.lock();
	    bool keyExist = memorySdb_.getValue(key, tag);
	    if(!keyExist || (keyExist && tag.first != DELETE) )
            memorySdbDeletion_++;
	    memorySdb_.update(key, TagType(DELETE, ValueType()));
        lastModificationStamp_ ++;
        memorySdbLock_.unlock();
	}

    /**
     * @brief Add a delta value to the given key in sdb. newvalue := oldvalue + delta.
     *
     * This operation
     * {
     *   sdb.delta(key,delta);
     * }
     * equals to following operations:
     * {
     *   ValueType oldValue = ValueType();
     *   sdb.get(key, oldValue);
     *   ValueType newValue = oldValue + delta;
     *   sdb.update(key, newValue);
     * }
     * delta() is far more faster.
     */
	void delta(const KeyType& key, const ValueType& delta)
	{
	    tryToMerge();

	    TagType tag = TagType();

        memorySdbLock_.lock();
	    bool keyExist = memorySdb_.getValue(key, tag);
	    if(!keyExist) {
            memorySdb_.update(key, TagType(DELTA, delta));
	    } else if (tag.first == DELTA) {
	        memorySdb_.update(key, TagType(DELTA, ADD(tag.second, delta) ));
	    } else if (tag.first == UPDATE) {
	        memorySdb_.update(key, TagType(UPDATE, ADD(tag.second, delta) ));
	    } else if (tag.first == DELETE) {
            memorySdb_.update(key, TagType(UPDATE, delta));
	    } else if (tag.first == INSERT) {
            memorySdbLock_.unlock();
	        throw std::runtime_error( "Warning: In hdb, use insert() together\
                with delta() is not recommended, which causes bad performance,\
                try update() instead." );
	    } else {
            memorySdbLock_.unlock();
	        throw std::runtime_error("unrecognized tag format in hdb");
	    }
        lastModificationStamp_ ++;
        memorySdbLock_.unlock();
	}

    /**
     * @brief Retrieve value for a given key in sdb, fail if key doesn't exist.
     * @return true sucess
     *         false if key doesn't exist
     */
	bool getValue(const KeyType& key, ValueType& value)
	{
        std::vector<TagType> tagList;

        // Collect all tags from both on-disk partitions and the in-memory partition.
        diskSdbLock_.lock_shared();
        TagType tmp = TagType();
        for(size_t i = 0; i < diskSdbList_.size() ; i++)
        {
            if(diskSdbList_[i]->sdb.getValue(key, tmp) )
                tagList.push_back(tmp);
        }
        memorySdbLock_.lock_shared();
        if( memorySdb_.getValue(key,tmp) )
            tagList.push_back(tmp);
        memorySdbLock_.unlock_shared();
        diskSdbLock_.unlock_shared();

        // Iterate all tags to caculate the correct answer.
	    bool found = false;
        ValueType accumulator = ValueType();
        for(size_t i = 0; i < tagList.size() ; i++)
        {
            switch(tagList[i].first)
            {
                case INSERT:
                    if(found == false) {
                        accumulator = tagList[i].second;
                        found = true;
                    }
                    break;
                case UPDATE:
                    accumulator = tagList[i].second;
                    found = true;
                    break;
                case DELTA:
                    accumulator = ADD( accumulator, tagList[i].second);
                    found = true;
                    break;
                case DELETE:
                    accumulator = ValueType();
                    found = false;
                    break;
                default:
                    throw std::runtime_error("unrecognized tag format in hdb");
            }
        }

        if(found)
            value = accumulator;
        return found;
	}


    /*************************************************
     *               Misc utilities
     *************************************************/
    /**
     * @brief Merge all pieces of small sdbs in this ScalableDB into a larger one.
     * This is usually called after all writings and before reading to improve searching
     * efficiency. Of course, you can search without calling this function, after all,
     * it is just a optimization.
     */
	void optimize()
	{
	    // optimize() is kind of merge process, hence single-threaded.
        mergeLock_.lock();

        // Step 1, First flush memory partition to disk.
        if( memorySdb_.numItems() ) {
            flushRamSdb();
        }
        // Sanity check.
        if(diskSdbList_.size() < 2) {
            mergeLock_.unlock();
            return;
        }

#ifdef VERBOSE_HDB
	    std::cout << "optimize ScalableDB, merge " << diskSdbList_.size() << " small sdbs together" << std::endl;
#endif

        // Step 2, prepare the final on-disk partition.
        std::string dstName = diskSdbList_[0]->sdb.getName() + "+";
        int dstLevel = diskSdbList_[0]->level + 1;

        SdbInfo* dst = new SdbInfo(dstName);
        dst->level = dstLevel;
        dst->deletions = 0;
        dst->sdb.setPageSize(pageSize_);
        dst->sdb.setDegree(degree_);
        dst->sdb.setCacheSize(DEFAULT_BTREE_CACHED_PAGE_NUM);
        dst->sdb.open();

        // Step 3, Dump all records to the final partition.
        //          lock the memory parition is enough, because disk partition
        //          cannot be modified except during merging.
        memorySdbLock_.lock_shared();
        HDBCursor cursor(*this);
        while( cursor.next() ) {
            if(cursor.getTag().first != DELETE)
                dst->sdb.insertValue(cursor.getKey(), cursor.getTag());
        }
        memorySdbLock_.unlock_shared();
        dst->sdb.flush();

        // Step 4, Store all to-be-deleted partitions.
        std::vector<SdbInfo*> tobeDeleted;
        for(size_t i = 0; i< diskSdbList_.size(); i++ ) {
            tobeDeleted.push_back(diskSdbList_[i]);
        }

        // Step 5, Maintain list.
        diskSdbLock_.lock();
        diskSdbList_.clear();
        diskSdbList_.push_back(dst);
        lastModificationStamp_ ++;
        diskSdbLock_.unlock();

        // Step 5, Close and delete old partitions.
        for(size_t i = 0; i< tobeDeleted.size(); i++ ) {
            std::string n = tobeDeleted[i]->sdb.getName();
            tobeDeleted[i]->sdb.close();
            delete tobeDeleted[i];
            std::remove(n.c_str());
        }

        mergeLock_.unlock();
	}

    /**
     * @brief Print out debug messages.
     */
    void display(std::ostream& os = std::cout)
    {
        header_.display(os);
        for(size_t i = 0; i<diskSdbList_.size(); i++)
            diskSdbList_[i]->sdb.display(os);
    }

    /**
     * @return number of records kept in hdb. However, restrict by hdb's design,
     *      the interface is avaiable to call only when only one partition exist.
     *      you can call it after HugeDB::optimize() finishes.
     */
    size_t numItems()
    {
        if(diskSdbList_.size() == 0)
            return memorySdb_.numItems() - memorySdbDeletion_;
        if(diskSdbList_.size() == 1)
            return (diskSdbList_[0]->sdb.numItems() - diskSdbList_[0]->deletions);

        std::cerr << "Warning: There are unmerged sdb files, \
                Call ScalableDB::optimize() before numItems(), \
                Or It would be extreamly inefficient." << std::endl;

        size_t count = 0;
        memorySdbLock_.lock_shared();
        HDBCursor cursor(*this);
        while( cursor.next() ) {
            if(cursor.getTag().first != DELETE)
                count ++;
        }
        memorySdbLock_.unlock_shared();
        return count;
    }

protected:

    std::string getMemorySdbName()
    {
        std::string ret = hdbName_ + ".hdb.memorycache";
        return ret;
    }

    std::string getSdbName(int slice, int level = 0)
    {
        std::string ret = hdbName_ + ".hdb.partition" +
            boost::lexical_cast<std::string>(slice);
        for(int i = 0; i<level; i++ )
            ret += "+";
        return ret;
    }

    static ValueType ADD(const ValueType& op1, const ValueType& op2 )
    {
        return ADDImpl<SupportDelta>::op(op1, op2);
    }

    template <bool Enable, typename T = void> class ADDImpl;

    template <typename T> class ADDImpl<true, T> {
    public:
        static ValueType op(const ValueType& op1, const ValueType& op2) {
            return op1+op2;
        }
    };

    template <typename T> class ADDImpl<false, T> {
    public:
        static ValueType op(const ValueType& op1, const ValueType& op2) {
            throw std::runtime_error("delta not supported in the weak hdb version");
        }
    };

    void tryToMerge()
    {
        if(cachedRecordsNumber_ <= (size_t)memorySdb_.numItems() )
	    {
	        // Step 1, flush memory partition to disk.
            flushRamSdb();

            // Step 2, merge disk partitions when condition satisfied.
            while (true)
            {
                // check
                if(!mergable() ) break;

                // Ensure only one thread enter merging.
                mergeLock_.lock();
                // recheck

                if(mergable()) merge();
                mergeLock_.unlock();
            }
	    }
    }

    bool mergable()
    {
        diskSdbLock_.lock_shared();
        bool test = false;
        if(diskSdbList_.size() >= mergeFactor_) {
            test = true;
            for(size_t i = 1; i < mergeFactor_; i++) {
                if(diskSdbList_[diskSdbList_.size()-1-i]->level !=
                    diskSdbList_.back()->level) {
                    test = false;
                    break;
                }
            }
        }
        diskSdbLock_.unlock_shared();
        return test;
    }

    /// Merge will be exectued by only one thread at a given time,
    /// protected by the mutex mergeLock_.
    void merge()
    {
#ifdef VERBOSE_HDB
    std::cout << "merge btree ";
    for(size_t i=0; i<mergeFactor_; i++)
        std::cout << diskSdbList_[diskSdbList_.size()-mergeFactor_+i]->sdb.getName()
            << "(" << diskSdbList_[diskSdbList_.size()-mergeFactor_+i]->sdb.numItems() << ") ";
    std::cout << "...\n";
#endif

        // Step 1. Prepare for the destination partition.
        std::string dstName = diskSdbList_[diskSdbList_.size()-mergeFactor_]->sdb.getName() + "+";
        int dstLevel = diskSdbList_[diskSdbList_.size()-mergeFactor_]->level + 1;

        SdbInfo* dst = new SdbInfo(dstName);
        dst->level = dstLevel;
        dst->sdb.setPageSize(pageSize_);
        dst->sdb.setDegree(degree_);
        dst->sdb.setCacheSize(DEFAULT_BTREE_CACHED_PAGE_NUM);
        dst->sdb.open();

        // Step 2. Prepare for an iterator from all to-be-merged partitions.
        std::vector<SdbType*> source;
        source.resize(mergeFactor_);
        for(size_t i = 0; i < mergeFactor_; i++ ) {
            source[i] = &(diskSdbList_[diskSdbList_.size()-mergeFactor_+i]->sdb);
        }
        MultiSDBCursor cursor(source);

        // Step 3. Merge.
        //         Since modification to on-disk partitions happens during merging process only,
        //          and merging process is single-threaded, so no proctection needed here.
        size_t deletions = 0;
        while( cursor.next() ) {
            if(cursor.getTag().first == DELETE)
                deletions ++;
            dst->sdb.insertValue(cursor.getKey(), cursor.getTag());
        }
        dst->deletions = deletions;
        dst->sdb.flush();

        // Step 4. Prepare for to-be-deleted partitions.
        std::vector<SdbInfo*> tobeDeleted;
        tobeDeleted.resize(mergeFactor_);
        for(size_t i=0; i<mergeFactor_; i++) {
            tobeDeleted[i] = diskSdbList_[diskSdbList_.size()-mergeFactor_+i];
        }

        // Step 5. delete old partitions from list and insert the new partition
        //          into list in a write lock.
        diskSdbLock_.lock();
        for(size_t i=0; i<mergeFactor_; i++) {
            diskSdbList_.pop_back();
        }
        diskSdbList_.push_back(dst);
        lastModificationStamp_ ++;
        diskSdbLock_.unlock();

        // Step 6. close and delete old partitions
        for(size_t i=0; i<mergeFactor_; i++) {
            std::string fn = tobeDeleted[i]->sdb.getName();
            tobeDeleted[i]->sdb.close();
            delete tobeDeleted[i];
            std::remove(fn.c_str());
        }

#ifdef VERBOSE_HDB
    std::cout << " into " << dst->sdb.getName() << "(" << dst->sdb.numItems() << ")" << std::endl;
#endif
    }

    void flushRamSdb()
    {
        // Step 1. Initialize a new disk partition
        SdbInfo* newDiskSdb = new SdbInfo( getSdbName(diskSdbList_.size()) );
        newDiskSdb->sdb.setPageSize(pageSize_);
        newDiskSdb->sdb.setDegree(degree_);
        newDiskSdb->sdb.setCacheSize( DEFAULT_BTREE_CACHED_PAGE_NUM );
        newDiskSdb->sdb.open();

        // Step 2. Dump all records in memory partition to the new disk partition.
        //          Protected by a read lock.
        memorySdbLock_.lock_shared();
        KeyType tmpk = KeyType();
        TagType tmpv = TagType();
        typename SdbType::SDBCursor cursor = memorySdb_.get_first_locn();
        while(memorySdb_.get(cursor, tmpk, tmpv)) {
            newDiskSdb->sdb.insertValue(tmpk, tmpv);
            memorySdb_.seq(cursor);
        }
        newDiskSdb->deletions = memorySdbDeletion_;
        memorySdbLock_.unlock_shared();

        // Step 3. Flush new disk partition, need not lock.
        newDiskSdb->sdb.flush();

        // Step 4. Insert new disk partition to list, protected by write lock.
        diskSdbLock_.lock();
        diskSdbList_.push_back(newDiskSdb);
        lastModificationStamp_ ++;
        diskSdbLock_.unlock();

        // Dont worry the memory partition and the new disk partition coexist
        // between Step 4 and Step 5. Because hdb allows redundancy.

        // Step 5. Clear memory partition, protected by write lock.
        memorySdbLock_.lock();
        memorySdb_.clear();
        memorySdbDeletion_ = 0;
        lastModificationStamp_ ++;
        memorySdbLock_.unlock();
    }

public:

    /*************************************************
     *                                                *
     *           SDB compatible interfaces            *
     *                                                *
     *************************************************/


    /*************************************************
     *               Cursor/Iterator
     *************************************************/

    /**
     * @brief Get the cursor of the first element in hdb.
     *        You can get element's content by call get().
     */
	HDBCursor get_first_locn() {
	    HDBCursor cursor(*this);
	    /*
	     * to keep the same semantics with sdb
	     * that is, get() after get_first_locn is legal
	     */
	    seq(cursor, ESD_FORWARD);
	    return cursor;
	}

    /**
     * @brief Update the cursor to move to the previous or next element.
     * @return true - if success
     *         false - to the beginning or the end of hdb, no available element.
     */
	bool seq(HDBCursor& cursor, ESeqDirection sdir = ESD_FORWARD) {
	    if(sdir == ESD_FORWARD ) {
            while( cursor.next() ) {
                if(cursor.getTag().first != DELETE)
                    return true;
            }
            return false;
	    } else {
            while( cursor.prev() ) {
                if(cursor.getTag().first != DELETE)
                    return true;
            }
            return false;
	    }
	}

    /**
     * @brief Retrieve element's content , see above
     */
	bool seq(HDBCursor& cursor, KeyType& key, ValueType& value,
			ESeqDirection sdir=ESD_FORWARD) {
        if( seq(cursor, sdir) ) {
            get(cursor, key, value);
            return true;
        }
        return false;
	}

    /**
     * @brief See above
     */
	bool seq(HDBCursor& cursor, DataType<KeyType, ValueType>& dat,
			ESeqDirection sdir=ESD_FORWARD) {
        return seq(cursor, dat.key, dat.value, sdir);
	}

    /**
     * @brief Search the given key in either up or down direction from current position.
     * @param key - the given key
     *        cursor - current position
     *        sdir - search direction, forward or backward
     * @return true - if key is found, update cursor to point to new position.
     *                when no hit found,
     *                   if we are searching forward, return the next element that larger than key,
     *                   if we are searching backward, return the previous element that smaller than the key.
     *         false - only in two cases:
     *                       the given key is larger than the largest key inside hdb when searching forward.
     *                       or key is smaller than the smallest key inside hdb when searching backward.
     */
	bool search(const KeyType& key, HDBCursor& cursor,
            ESeqDirection sdir = ESD_FORWARD) {
	    bool suc = cursor.seek(key);
	    if(suc && cursor.getKey() == key) {
            if(cursor.getTag().first != DELETE)
                return true;
            seq(cursor, sdir);
            return false;
	    }
        if(sdir == ESD_BACKWARD)
            seq(cursor, ESD_BACKWARD);
        return false;
	}

    /**
     * @brief See above
     */
	HDBCursor search(const KeyType& key, ESeqDirection sdir = ESD_FORWARD) {
	    HDBCursor cursor(*this);
	    search(key, cursor, sdir);
	    return cursor;
	}

    /**
     * @brief Get the content of element at a position.
     * @return false - if the element doesn't exist, e.g. before the begining of hdb
     *          or after the end of hdb.
     *         true - otherwise.
     */
	bool get(const HDBCursor& cursor, KeyType& key, ValueType& value) {
	    if(cursor.getTag().first != DELETE) {
            key = cursor.getKey();
            value = cursor.getTag().second;
            return true;
	    }
	    return false;
	}

    /**
     * @brief See above
     */
	bool get(const HDBCursor& cursor, DataType<KeyType,ValueType> & dat) {
	    return get(cursor, dat.key, dat.value);
	}

    /*************************************************
     *                Range Search
     *************************************************/

    /**
     * Find the next bigger key than the given key.
     */
	bool getNext(const KeyType& key, KeyType& nxtKey) {
        bool ret = false;
	    KeyType tmpk = KeyType();
	    ValueType tmpv = ValueType();

	    diskSdbLock_.lock_shared();
	    memorySdbLock_.lock_shared();
	    HDBCursor cursor(*this);
	    if( search(key, cursor, ESD_FORWARD) ) {
	        if( seq(cursor, ESD_FORWARD) ) {
                get(cursor, tmpk, tmpv);
                nxtKey = tmpk;
                ret = true;
	        }
	    } else if( get(cursor, tmpk, tmpv) ) {
	        nxtKey = tmpk;
	        ret = true;
	    }
	    memorySdbLock_.unlock_shared();
	    diskSdbLock_.unlock_shared();
	    return ret;
	}

    /**
     * Find the next smaller key than the given key.
     */
	bool getPrev(const KeyType& key, KeyType& prevKey) {
        bool ret = false;
	    KeyType tmpk = KeyType();
	    ValueType tmpv = ValueType();

	    diskSdbLock_.lock_shared();
	    memorySdbLock_.lock_shared();
	    HDBCursor cursor(*this);
	    if( search(key, cursor, ESD_BACKWARD) ) {
	        if( seq(cursor, ESD_BACKWARD) ) {
                get(cursor, tmpk, tmpv);
                prevKey = tmpk;
                ret = true;
	        }
	    } else if( get(cursor, tmpk, tmpv) ) {
	        prevKey = tmpk;
	        ret = true;
	    }
	    memorySdbLock_.unlock_shared();
	    diskSdbLock_.unlock_shared();
	    return ret;
	}

    /**
     * Find N keys next to the given key.
     */
	bool getValueForward(const int count,
            vector<DataType<KeyType,ValueType> >& result, const KeyType& key) {
        result.clear();

	    diskSdbLock_.lock_shared();
	    memorySdbLock_.lock_shared();
        HDBCursor cursor(*this);
        search(key, cursor, ESD_FORWARD);
        KeyType tmpk;
        ValueType tmpv;
        for( int i=0; i<count; i++ ) {
            if(!get(cursor, tmpk, tmpv))
                break;
            result.push_back(DataType<KeyType, ValueType>(tmpk, tmpv));
            seq(cursor, ESD_FORWARD);
        }
	    memorySdbLock_.unlock_shared();
	    diskSdbLock_.unlock_shared();
	    return result.size() > 0 ? true:false;
    }

    /**
     * @brief See above
     */
	bool getValueForward(int count, vector<DataType<KeyType,ValueType> >& result) {
        KeyType key;
        return getValueForward(count, result, key);
	}

    /**
     * Find N keys next to the given key in the reverse order.
     */
	bool getValueBackward(const int count,
			vector<DataType<KeyType,ValueType> >& result, const KeyType& key) {
        result.clear();

	    diskSdbLock_.lock_shared();
	    memorySdbLock_.lock_shared();
        HDBCursor cursor(*this);
        search(key, cursor, ESD_BACKWARD);
        KeyType tmpk;
        ValueType tmpv;
        for( int i=0; i<count; i++ ) {
            if(!get(cursor, tmpk, tmpv))
                break;
            result.push_back(DataType<KeyType, ValueType>(tmpk, tmpv));
            seq(cursor, ESD_BACKWARD);
        }
	    memorySdbLock_.unlock_shared();
	    diskSdbLock_.unlock_shared();
	    return result.size() > 0 ? true:false;
    }

    /**
     * @brief See above
     */
	bool getValueBackward(int count, vector<DataType<KeyType,ValueType> >& result) {
        KeyType key;
	    return getValueBackward(count, result, key);
	}

    /**
     * Find all keys between the given range.
     */
	bool getValueBetween(vector<DataType<KeyType,ValueType> >& result,
			const KeyType& lowKey, const KeyType& highKey) {
        result.clear();

	    diskSdbLock_.lock_shared();
	    memorySdbLock_.lock_shared();
        HDBCursor cursor(*this);
        search(lowKey, cursor, ESD_FORWARD);
        KeyType tmpk;
        ValueType tmpv;
        while(true) {
            if(!get(cursor, tmpk, tmpv)) break;
            if(comp_(tmpk, highKey) > 0) break;
            result.push_back(DataType<KeyType, ValueType>(tmpk, tmpv));
            seq(cursor, ESD_FORWARD);
        }
	    memorySdbLock_.unlock_shared();
	    diskSdbLock_.unlock_shared();
	    return result.size() > 0 ? true:false;
    }


private:

    std::string hdbName_;

    bool isOpen_;

    bool isSync_;

    std::string headerPath_;

    HugeDBHeader header_;

    size_t cachedRecordsNumber_;

    size_t mergeFactor_;

    size_t pageSize_;

    size_t degree_;

	CompareFunctor<KeyType> comp_;

    /// A list of on-disk partitions. Implemented by sdb_btree.
    std::vector<SdbInfo*> diskSdbList_;

    /// The in-memory partition. Implemented by sdb_btree.
    SdbType memorySdb_;

    size_t memorySdbDeletion_;

    /// Stamp of last modification in either on-disk or in-memory partitions,
    /// incremented by 1 after each modify operation.
    size_t lastModificationStamp_;

    /// Lock for managing on-disk partitions,
    /// They are modified only during merging process.
    LockType diskSdbLock_;

    /// Lock for managing in-memory partition.
    LockType memorySdbLock_;

    /// A mutex for allowing only one thread entering the merging process.
    LockType mergeLock_;
};


template< typename KeyType, typename ValueType,
		typename LockType =NullLock > class ordered_hdb :
	public HugeDB<KeyType, ValueType, LockType,
        sdb_btree<KeyType, std::pair<char, ValueType>, LockType>, true >
{
public:
	ordered_hdb(const string& sdbname) :
		HugeDB<KeyType, ValueType, LockType,
				sdb_btree<KeyType, std::pair<char, ValueType>, LockType>, true >(sdbname) {

	}
};

template< typename KeyType, typename ValueType,
		typename LockType =NullLock > class ordered_hdb_no_delta :
	public HugeDB<KeyType, ValueType, LockType,
        sdb_btree<KeyType, std::pair<char, ValueType>, LockType>, false >
{
public:
	ordered_hdb_no_delta(const string& sdbname) :
		HugeDB<KeyType, ValueType, LockType,
				sdb_btree<KeyType, std::pair<char, ValueType>, LockType>, false >(sdbname) {

	}
};

template< typename KeyType, typename ValueType,
		typename LockType =NullLock > class ordered_hdb_fixed :
	public HugeDB<KeyType, ValueType, LockType,
        sdb_btree<KeyType, std::pair<char, ValueType>, LockType, true>, true >
{
public:
	ordered_hdb_fixed(const string& sdbname) :
		HugeDB<KeyType, ValueType, LockType,
				sdb_btree<KeyType, std::pair<char, ValueType>, LockType, true>, true >(sdbname) {

	}
};


template< typename KeyType, typename ValueType,
		typename LockType =NullLock > class ordered_hdb_fixed_no_delta :
	public HugeDB<KeyType, ValueType, LockType,
        sdb_btree<KeyType, std::pair<char, ValueType>, LockType, true>, false >
{
public:
	ordered_hdb_fixed_no_delta(const string& sdbname) :
		HugeDB<KeyType, ValueType, LockType,
				sdb_btree<KeyType, std::pair<char, ValueType>, LockType, true>, false >(sdbname) {

	}
};

}

}

#endif
