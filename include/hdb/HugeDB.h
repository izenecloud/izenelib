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
          typename Alloc=std::allocator<DataType<KeyType, std::pair<char, ValueType> > > >
class HugeDB {

protected:

    typedef HugeDB<KeyType, ValueType, LockType, ContainerType, Alloc> ThisType;

    typedef std::pair<char, ValueType> TagType;

    typedef SequentialDB<KeyType, TagType, LockType, ContainerType, Alloc> SdbType;

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

    static const int DEFAULT_CACHED_RECORDS_NUM = 2000000;
    static const int DEFAULT_MERGE_FACTOR = 2;
    static const int DEFAULT_BTREE_CACHED_PAGE_NUM = 8*1024;
    static const int DEFAULT_BTREE_PAGE_SIZE = 8*1024;
    static const int DEFAULT_BTREE_DEGREE = 128;

public:

    /**
     * @brief HDBCursor can be used to iterate the whole hdb,
     *        the same as SequentialDB::SDBCursor
     */
    typedef HDBCursor_<ThisType> HDBCursor;

    friend class HDBCursor_<ThisType>;

public:

    /**
     * @brief Constructor.
     * @param hdbName - identifier of hdb
     */
    HugeDB(const std::string &hdbName)
        : hdbName_(hdbName),
          isOpen_(false), isSync_(true),
          headerPath_(hdbName_ + ".hdb.header.xml"),
          header_(headerPath_),
          cachedRecordsNumber_(DEFAULT_CACHED_RECORDS_NUM),
          mergeFactor_(DEFAULT_MERGE_FACTOR),
          pageSize_(DEFAULT_BTREE_PAGE_SIZE),
          degree_(DEFAULT_BTREE_DEGREE)
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
            sdbList_.push_back(sdbi);
        }

        header_.flush();
    }

    ~HugeDB()
    {
        close();
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
     *               Open/Close/Flush
     *************************************************/

    /**
     * Open a hdb.
     */
    void open()
    {
        for(size_t i = 0; i<sdbList_.size(); i++)
            sdbList_[i]->sdb.open();

        // create a empty partition
        createRamSdb();

        isSync_ = true;
        isOpen_ = true;
    }

    void clear()
    {
        // delete each partitions
        for(size_t i = 0; i<sdbList_.size(); i++) {
            std::string n = sdbList_[i]->sdb.getName();
            sdbList_[i]->sdb.close();
            delete sdbList_[i];
            std::remove(n.c_str());
        }
        sdbList_.clear();

        // create a empty partition
        createRamSdb();

        // update header
        flush();
    }

    /**
     * Close a hdb.
     */
    void close()
    {
        flush();
        for(size_t i = 0; i<sdbList_.size(); i++) {
            sdbList_[i]->sdb.close();
            delete sdbList_[i];
        }
        sdbList_.clear();
        isSync_ = false;
        isOpen_ = false;
    }

    /**
     * Flush hdb content to disk.
     */
    void flush()
    {
        header_.slicesNum = sdbList_.size();
        header_.deletions.resize(sdbList_.size());
        header_.slicesLevel.resize(sdbList_.size());
        for(size_t i = 0; i<sdbList_.size(); i++) {
            header_.deletions[i] = sdbList_[i]->deletions;
            header_.slicesLevel[i] = sdbList_[i]->level;
        }
        header_.flush();

        for(size_t i = 0; i<sdbList_.size(); i++)
            sdbList_[i]->sdb.flush();
        isSync_ = true;
    }

    /*************************************************
     *               HDB Access Methods
     *************************************************/

	/**
	 * @brief Insert value for a given key into sdb, do not modify anything if key exists.
	 */
	void insertValue(const KeyType& key, const ValueType& value)
	{
	    tryMerge();
	    SdbInfo* ramSdb = sdbList_.back();

	    TagType tag = TagType();
	    bool keyExist = ramSdb->sdb.getValue(key, tag);
	    if(!keyExist || tag.first == DELETE) {
            ramSdb->sdb.update(key, TagType(INSERT, value));
            isSync_ = false;
        }
	}

	/**
	 * @brief Update value for a given key in sdb, overwrite if key exists.
	 */
	void update(const KeyType& key, const ValueType& value)
	{
	    tryMerge();
	    SdbInfo* ramSdb = sdbList_.back();

	    ramSdb->sdb.update(key, TagType(UPDATE, value));
        isSync_ = false;
	}

	/**
	 * @brief Delete record .
	 */
	void del(const KeyType& key)
	{
	    tryMerge();
	    SdbInfo* ramSdb = sdbList_.back();

	    TagType tag = TagType();
	    bool keyExist = ramSdb->sdb.getValue(key, tag);
	    if(!keyExist || (keyExist && tag.first != DELETE) )
            ramSdb->deletions++;

	    ramSdb->sdb.update(key, TagType(DELETE, ValueType()));
        isSync_ = false;
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
	    tryMerge();
	    SdbInfo* ramSdb = sdbList_.back();

	    TagType tag = TagType();
	    bool keyExist = ramSdb->sdb.getValue(key, tag);
	    if(!keyExist) {
            ramSdb->sdb.update(key, TagType(DELTA, delta));
            isSync_ = false;
	    } else if (tag.first == DELTA) {
	        ramSdb->sdb.update(key, TagType(DELTA, tag.second + delta));
	        isSync_ = false;
	    } else if (tag.first == UPDATE) {
	        ramSdb->sdb.update(key, TagType(UPDATE, tag.second + delta));
	        isSync_ = false;
	    } else if (tag.first == DELETE) {
            ramSdb->sdb.update(key, TagType(UPDATE, delta));
            isSync_ = false;
	    } else if (tag.first == INSERT) {
	        throw std::runtime_error( "Warning: In hdb, use insert() together with delta() is not recommended, \
                which causes bad performance, try update() instead." );
	    } else {
	        throw std::runtime_error("unrecognized tag format in hdb");
	    }
	}

    /**
     * @brief Retrieve value for a given key in sdb, fail if key doesn't exist.
     * @return true sucess
     *         false if key doesn't exist
     */
	bool getValue(const KeyType& key, ValueType& value)
	{
	    bool found = false;
	    ValueType accumulator = ValueType();

	    TagType tag = TagType();
        for(size_t i = 0; i < sdbList_.size() ; i++)
        {
            if(sdbList_[i]->sdb.getValue(key, tag) )
            {
                switch(tag.first)
                {
                    case INSERT:
                        if(found == false) {
                            accumulator = tag.second;
                            found = true;
                        }
                        break;
                    case UPDATE:
                        accumulator = tag.second;
                        found = true;
                        break;
                    case DELTA:
                        accumulator += tag.second;
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
        }

        if(found)
            value = accumulator;
        return found;
	}

    /*************************************************
     *               Cursor/Iterator
     *************************************************/

    /**
     * @brief Get the cursor of the first element in hdb.
     *        You can get element's content by call get().
     */
	HDBCursor get_first_Locn() {
	    HDBCursor cursor(*this);
	    /*
	     * to keep the same semantics with sdb
	     * that is, get() after get_first_Locn is legal
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
	    KeyType tmpk = KeyType();
	    ValueType tmpv = ValueType();
	    HDBCursor cursor(*this);
	    if( search(key, cursor, ESD_FORWARD) ) {
	        if( seq(cursor, ESD_FORWARD) ) {
                get(cursor, tmpk, tmpv);
                nxtKey = tmpk;
                return true;
	        }
	    } else if( get(cursor, tmpk, tmpv) ) {
	        nxtKey = tmpk;
	        return true;
	    }
	    return false;
	}

    /**
     * Find the next smaller key than the given key.
     */
	bool getPrev(const KeyType& key, KeyType& prevKey) {
	    KeyType tmpk = KeyType();
	    ValueType tmpv = ValueType();
	    HDBCursor cursor(*this);
	    if( search(key, cursor, ESD_BACKWARD) ) {
	        if( seq(cursor, ESD_BACKWARD) ) {
                get(cursor, tmpk, tmpv);
                prevKey = tmpk;
                return true;
	        }
	    } else if( get(cursor, tmpk, tmpv) ) {
	        prevKey = tmpk;
	        return true;
	    }
        return false;
	}

    /**
     * Find N keys next to the given key.
     */
	bool getValueForward(const int count,
            vector<DataType<KeyType,ValueType> >& result, const KeyType& key) {
        result.clear();

        HDBCursor cursor(*this);
        search(key, cursor, ESD_FORWARD);
        KeyType tmpk;
        ValueType tmpv;
        for( int i=0; i<count; i++ ) {
            if(!get(cursor, tmpk, tmpv))
                return false;
            result.push_back(DataType<KeyType, ValueType>(tmpk, tmpv));
            seq(cursor, ESD_FORWARD);
        }
        return true;
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

        HDBCursor cursor(*this);
        search(key, cursor, ESD_BACKWARD);
        KeyType tmpk;
        ValueType tmpv;
        for( int i=0; i<count; i++ ) {
            if(!get(cursor, tmpk, tmpv))
                return false;
            result.push_back(DataType<KeyType, ValueType>(tmpk, tmpv));
            seq(cursor, ESD_BACKWARD);
        }
        return true;
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
        if(result.size() > 0) return true;
        return false;
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
#ifdef VERBOSE_HDB
	    std::cout << "optimize ScalableDB, merge " << sdbList_.size() << " small sdbs together" << std::endl;
#endif
        if(sdbList_.size() < 2) return;

        std::string dstName = sdbList_[0]->sdb.getName() + "+";
        int dstLevel = sdbList_[0]->level + 1;

        SdbInfo* dst = new SdbInfo(dstName);
        dst->sdb.setPageSize(pageSize_);
        dst->sdb.setDegree(degree_);
        dst->sdb.setCacheSize(DEFAULT_BTREE_CACHED_PAGE_NUM);
        dst->sdb.open();

        HDBCursor cursor(*this);
        while( cursor.next() ) {
            if(cursor.getTag().first != DELETE)
                dst->sdb.insertValue(cursor.getKey(), cursor.getTag());
        }
        dst->sdb.flush();

        for(size_t i = 0; i< sdbList_.size(); i++ ) {
            std::string n = sdbList_[i]->sdb.getName();
            sdbList_[i]->sdb.close();
            delete sdbList_[i];
            std::remove(n.c_str());
        }
        sdbList_.clear();

        dst->level = dstLevel;
        dst->deletions = 0;
        sdbList_.push_back(dst);

        isSync_ = true;
	}

    /**
     * @brief Print out debug messages.
     */
    void display(std::ostream& os = std::cout)
    {
        header_.display(os);
        for(size_t i = 0; i<sdbList_.size(); i++)
            sdbList_[i]->sdb.display(os);
    }

    /**
     * @return number of records kept in hdb. However, restrict by hdb's design,
     *      the interface is avaiable to call only when only one partition exist.
     *      you can call it after HugeDB::optimize() finishes.
     */
    size_t numItems()
    {
        if(sdbList_.size() == 1)
            return (sdbList_[0]->sdb.numItems() - sdbList_[0]->deletions);
        flush();
        throw std::runtime_error("\nThere are unmerged sdb files, \
            Call ScalableDB::optimize() before numItems()\n");
    }

protected:

    inline void tryMerge()
    {
        if(cachedRecordsNumber_ <= (size_t)sdbList_.back()->sdb.numItems() )
	    {
            if( !isSync_ )
                flushRamSdb();

            tryMergeDiskSdb();
            createRamSdb();
	    }
    }

    inline void tryMergeDiskSdb()
    {
        while (sdbList_.size() >= mergeFactor_)
        {
            for(size_t i = 1; i < mergeFactor_; i++)
                if(sdbList_[sdbList_.size()-1-i]->level !=
                    sdbList_.back()->level) return;
            mergeDiskSdb();
        }
    }

    inline void mergeDiskSdb()
    {
#ifdef VERBOSE_HDB
    std::cout << "merge btree ";
    for(size_t i=0; i<mergeFactor_; i++)
        std::cout << sdbList_[sdbList_.size()-mergeFactor_+i]->sdb.getName()
            << "(" << sdbList_[sdbList_.size()-mergeFactor_+i]->sdb.numItems() << ") ";
    std::cout << "...\n";
#endif
        std::string dstName = sdbList_[sdbList_.size()-mergeFactor_]->sdb.getName() + "+";
        int dstLevel = sdbList_[sdbList_.size()-mergeFactor_]->level + 1;

        SdbInfo* dst = new SdbInfo(dstName);
        dst->sdb.setPageSize(pageSize_);
        dst->sdb.setDegree(degree_);
        dst->sdb.setCacheSize(DEFAULT_BTREE_CACHED_PAGE_NUM);
        dst->sdb.open();

        size_t deletions = 0;
        HDBCursor cursor(*this, sdbList_.size()-mergeFactor_, mergeFactor_);
        while( cursor.next() ) {
            if(cursor.getTag().first == DELETE)
                deletions ++;
            dst->sdb.insertValue(cursor.getKey(), cursor.getTag());
        }
        dst->sdb.flush();

        for(size_t i=0; i<mergeFactor_; i++) {
            SdbInfo* last = sdbList_.back();
            sdbList_.pop_back();

            std::string fn = last->sdb.getName();
            last->sdb.close();
            delete last;
            std::remove(fn.c_str());
        }
        dst->level = dstLevel;
        dst->deletions = deletions;
        sdbList_.push_back(dst);
#ifdef VERBOSE_HDB
    std::cout << " into " << dst->sdb.getName() << "(" << dst->sdb.numItems() << ")" << std::endl;
#endif
    }

    inline void createRamSdb()
    {
        std::string name = getSdbName(sdbList_.size());
        SdbInfo* ramsdb = new SdbInfo(name);
        // upper limit
        ramsdb->sdb.setPageSize(pageSize_);
        ramsdb->sdb.setDegree(degree_);
        ramsdb->sdb.setCacheSize( (size_t)-1 );
        ramsdb->sdb.open();
        sdbList_.push_back(ramsdb);
#ifdef VERBOSE_HDB
        std::cout << "create ram sdb " << name << std::endl;
#endif
    }

    inline void flushRamSdb()
    {
        sdbList_.back()->sdb.setCacheSize(DEFAULT_BTREE_CACHED_PAGE_NUM);
        sdbList_.back()->sdb.flush();
    }

    inline std::string getSdbName(int slice, int level = 0)
    {
        std::string ret = hdbName_ + ".hdb.partition" +
            boost::lexical_cast<std::string>(slice);
        for(int i = 0; i<level; i++ )
            ret += "+";
        return ret;
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

    std::vector<SdbInfo*> sdbList_;

	CompareFunctor<KeyType> comp_;
};

template< typename KeyType, typename ValueType,
		typename LockType =NullLock > class ordered_hdb :
	public HugeDB<KeyType, ValueType, LockType,
        sdb_btree<KeyType, std::pair<char, ValueType>, LockType> >
{
public:
	ordered_hdb(const string& sdbname) :
		HugeDB<KeyType, ValueType, LockType,
				sdb_btree<KeyType, std::pair<char, ValueType>, LockType> >(sdbname) {

	}
};

template< typename KeyType, typename ValueType,
		typename LockType =NullLock > class ordered_hdb_fixed :
	public HugeDB<KeyType, ValueType, LockType,
        sdb_btree<KeyType, std::pair<char, ValueType>, LockType, true> >
{
public:
	ordered_hdb_fixed(const string& sdbname) :
		HugeDB<KeyType, ValueType, LockType,
				sdb_btree<KeyType, std::pair<char, ValueType>, LockType, true> >(sdbname) {

	}
};

}

}

#endif
