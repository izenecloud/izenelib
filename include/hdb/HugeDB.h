/**
 * @file HugeDB.h
 * @brief Implementation of HugeDB,
 *        Enhancement of SequentialDB for random accesses on large scale data set.
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

template<typename SdbType, typename Comparator>
static void merge_btree( SdbType* src1, SdbType* src2, SdbType* dst, size_t& dels, const Comparator& comp);

template<typename SdbType, typename Comparator>
static void merge_btree( SdbType** src, int n , SdbType* dst, size_t& dels, const Comparator& comp);


/**
 * @brief
 * SequentialDB, is fast for sequential accesses, however not good at random
 * insertions and look up.
 *
 * ScalableDB, is based on SequentialDB, but use a keep-modifications-in-memory,
 * flush-small-db-from-memory-to-disk, and merge-small-dbs-to-large-one-on-disk
 * techniques to provide scalable performance for random accesses.
 *
 * If you are to maintain large scale of data on disk, use this one.
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

    class SdbInfo {
    public:
        SdbType sdb;
        int level;
        size_t deletions;
        SdbInfo(std::string name) : sdb(name), level(0), deletions(0) {}
    };

public:

    typedef HDBCursor_<ThisType> HDBCursor;

    friend class HDBCursor_<ThisType>;

    HugeDB(const std::string &hdbName)
        : hdbName_(hdbName),
          isOpen_(false), isSync_(true),
          headerPath_(hdbName_ + ".hdb.header.xml"),
          header_(headerPath_),
          cachedRecordsNumber_(2000000),
          mergeFactor_(2),
          pageSize_(8*1024),
          degree_(128)
    {
#ifdef VERBOSE_HDB
        header_.display();
#endif
        for(size_t i = 0; i<header_.slicesNum; i++)
        {
            SdbInfo* sdbi = new SdbInfo(getSdbName(i,header_.slicesLevel[i]));
            sdbi->sdb.setCacheSize(8*1024);
            sdbi->level = header_.slicesLevel[i];
            sdbi->deletions = header_.deletions[i];
            sdbList_.push_back(sdbi);
        }
    }

    ~HugeDB()
    {
        close();
        for(size_t i = 0; i<sdbList_.size(); i++)
        {
            delete sdbList_[i];
            sdbList_[i] = NULL;
        }
    }

    /**
     * @brief Set CacheRecordsNumber to N.
     * It means you will cache at most N records in memory.
     */
    void setCachedRecordsNumber(int cachedRecordsNumber)
    {
        if(isOpen_)
            throw std::runtime_error("cannot set cache size after opened");
        cachedRecordsNumber_ = cachedRecordsNumber;
    }

    /**
     * @brief Set MergeFactor to M.
     * This factor controls sdb merging process. When there are M sdbs of equal size,
     * a merge process will be triggered, which merges M small sdbs into a larger one.
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

	void setPageSize(size_t pageSize) {
		pageSize_ = pageSize;
	}

	void setDegree(int degree) {
		degree_ = degree;
	}

    void open()
    {
        if(sdbList_.size() == 0)
            createRamSdb();
        for(size_t i = 0; i<sdbList_.size(); i++)
            sdbList_[i]->sdb.open();
        isOpen_ = true;
    }

    void close()
    {
        flush();
        for(size_t i = 0; i<sdbList_.size(); i++)
            sdbList_[i]->sdb.close();
        isOpen_ = false;
    }

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

        int finalLevel = sdbList_[0]->level + 1;

        SdbInfo* right = sdbList_.back();
        std::string rightName = right->sdb.getName();
        right->sdb.setCacheSize(8*1024);
        sdbList_.pop_back();

        while(sdbList_.size() > 0)
        {
            // 2nd db
            SdbInfo* left = sdbList_.back();
            std::string leftName = left->sdb.getName();
            left->sdb.setCacheSize(8*1024);
            sdbList_.pop_back();

            std::string dstname = leftName + "+";
            SdbInfo* dst = new SdbInfo(dstname);
            dst->sdb.setPageSize(pageSize_);
            dst->sdb.setDegree(degree_);
            dst->sdb.setCacheSize(8*1024);
            dst->sdb.open();

            merge_btree<SdbType>(&(left->sdb), &(right->sdb), &(dst->sdb), dst->deletions, comp_);

            left->sdb.close();
            delete left;
            std::remove(leftName.c_str());

            right->sdb.close();
            delete right;
            std::remove(rightName.c_str());

            right = dst;
            rightName = dstname;
        }

        right->level = finalLevel;
        sdbList_.push_back(right);

        isSync_ = true;
	}

    void display(std::ostream& os = std::cout)
    {
        header_.display(os);
        for(size_t i = 0; i<sdbList_.size(); i++)
            sdbList_[i]->sdb.display(os);
    }

    size_t numItems()
    {
        if(sdbList_.size() == 1)
            return (sdbList_[0]->sdb.numItems() - sdbList_[0]->deletions);
        flush();
        throw std::runtime_error("\nThere are unmerged sdb files, \
            Call ScalableDB::optimize() before numItems()\n");
    }

	HDBCursor get_first_Locn() {
	    HDBCursor cursor(*this);
	    /*
	     * to keep the same semantics with sdb
	     * that is, get() after get_first_Locn is legal
	     */
	    seq(cursor, ESD_FORWARD);
	    return cursor;
	}

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

	bool seq(HDBCursor& cursor, KeyType& key, ValueType& value,
			ESeqDirection sdir=ESD_FORWARD) {
        if( seq(cursor, sdir) ) {
            get(cursor, key, value);
            return true;
        }
        return false;
	}

	bool seq(HDBCursor& cursor, DataType<KeyType, ValueType>& dat,
			ESeqDirection sdir=ESD_FORWARD) {
        return seq(cursor, dat.key, dat.value, sdir);
	}

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

	HDBCursor search(const KeyType& key, ESeqDirection sdir = ESD_FORWARD) {
	    HDBCursor cursor(*this);
	    search(key, cursor, sdir);
	    return cursor;
	}

	bool get(const HDBCursor& cursor, KeyType& key, ValueType& value) {
	    if(cursor.getTag().first != DELETE) {
            key = cursor.getKey();
            value = cursor.getTag().second;
            return true;
	    }
	    return false;
	}

	bool get(const HDBCursor& cursor, DataType<KeyType,ValueType> & dat) {
	    return get(cursor, dat.key, dat.value);
	}

	bool getNext(const KeyType& key, KeyType& nxtKey) {
	    KeyType tmpk;
	    TagType tmpt;
	    HDBCursor cursor;
	    if( search(key, cursor, ESD_FORWARD) ) {
	        if( seq(cursor, ESD_FORWARD) ) {
                get(cursor, tmpk, tmpt);
                nxtKey = tmpk;
                return true;
	        }
	        return false;
	    } else {
	        get(cursor, tmpk, tmpt);
	        nxtKey = tmpk;
	        return true;
	    }
	}

	bool getPrev(const KeyType& key, KeyType& prevKey) {
	    KeyType tmpk;
	    TagType tmpt;
	    HDBCursor cursor;
	    if( search(key, cursor, ESD_BACKWARD) ) {
	        if( seq(cursor, ESD_BACKWARD) ) {
                get(cursor, tmpk, tmpt);
                prevKey = tmpk;
                return true;
	        }
	        return false;
	    } else {
	        get(cursor, tmpk, tmpt);
	        prevKey = tmpk;
	        return true;
	    }
	}

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

	bool getValueForward(int count, vector<DataType<KeyType,ValueType> >& result) {
        KeyType key;
        return getValueForward(count, result, key);
	}

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

	bool getValueBackward(int count, vector<DataType<KeyType,ValueType> >& result) {
        KeyType key;
	    return getValueBackward(count, result, key);
	}

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

protected:

    inline void tryMerge()
    {
        if(cachedRecordsNumber_ == (size_t)sdbList_.back()->sdb.numItems() )
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
            doMergeDiskSdb();
        }
    }

    inline void doMergeDiskSdb()
    {
        SdbType** input = new SdbType*[mergeFactor_];
        for(size_t i=0; i<mergeFactor_; i++) {
            SdbInfo* info = sdbList_[sdbList_.size() - mergeFactor_ + i];
            info->sdb.setCacheSize(8*1024);
            input[i] = &(info->sdb);
        }

        std::string dstname = input[0]->getName() + "+";
        SdbInfo* dst = new SdbInfo(dstname);
        dst->sdb.setPageSize(pageSize_);
        dst->sdb.setDegree(degree_);
        dst->sdb.setCacheSize(8*1024);
        dst->sdb.open();

        dst->level = sdbList_.back()->level + 1;
        merge_btree(input, mergeFactor_, &(dst->sdb), dst->deletions, comp_);

        for(size_t i=0; i<mergeFactor_; i++) {
            SdbInfo* last = sdbList_.back();
            sdbList_.pop_back();

            std::string fn = last->sdb.getName();
            last->sdb.close();
            delete last;
            std::remove(fn.c_str());
        }
        sdbList_.push_back(dst);
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
        sdbList_.back()->sdb.setCacheSize(8*1024);
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

template<typename SdbType, typename Comparator>
static void merge_btree( SdbType* src1, SdbType* src2, SdbType* dst, size_t& dels, const Comparator& comp)
{
    SdbType* input[2];
    input[0] = src1;
    input[1] = src2;

    merge_btree(input, 2, dst, dels, comp);
}

template<typename SdbType, typename Comparator>
static void merge_btree( SdbType** src, int n , SdbType* dst, size_t& dels, const Comparator& comp)
{
    typedef typename SdbType::SDBCursor SdbCursor;
    typedef typename SdbType::SDBKeyType KeyType;
    typedef typename SdbType::SDBValueType TagType;
    typedef typename TagType::second_type ValueType;

#ifdef VERBOSE_HDB
    std::cout << "merge btree ";
    for(int i=0; i<n; i++)
        std::cout << src[i]->getName() << "(" << src[i]->numItems() << ") ";
    std::cout << "...\n";
#endif

    dels = 0;

    // intialize all cursors
    SdbCursor* locn = new SdbCursor[n];
    KeyType* key = new KeyType[n];
    TagType* tag = new TagType[n];
    bool* nonEmpty = new bool[n];
    int nonEmptySdbNumber = 0;
    for(int i=0; i<n; i++)
    {
        locn[i] = src[i]->get_first_Locn();
        key[i] = KeyType();
        tag[i] = TagType();
        if( (nonEmpty[i] = src[i]->get(locn[i], key[i], tag[i])) )
            nonEmptySdbNumber++;
    }

    while(nonEmptySdbNumber > 1)
    {
        int idx = -1;
        KeyType leastKey = KeyType();
        bool hasDuplicatedKey = false;
        // pass 1: find the least key
        for(int i=0; i<n; i++) {
            if(!nonEmpty[i]) continue;
            if(idx == -1) {
                idx = i;
                leastKey = key[i];
                hasDuplicatedKey = false;
            } else {
                int lt = comp(key[i], leastKey);
                if( lt < 0 ) {
                    idx = i;
                    leastKey = key[i];
                    hasDuplicatedKey = false;
                } else if( lt == 0 ) {
                    hasDuplicatedKey = true;
                }
            }
        }

        // simple case
        if(!hasDuplicatedKey) {
            dst->insertValue(leastKey, tag[idx]);
            if(tag[idx].first == DELETE)
                dels ++;
            src[idx]->seq(locn[idx]);
            if( !(nonEmpty[idx] = src[idx]->get(locn[idx], key[idx], tag[idx])) )
                nonEmptySdbNumber --;
        } else {

            ValueType accumulator = ValueType();
            bool hasInsert = false;
            bool hasUpdate = false;
            bool hasDelta = false;
            // pass 2: process all duplicated keys, safe starting from idx
            for(int i=idx; i<n; i++) {
                if(!nonEmpty[i]) continue;
                if(comp(key[i], leastKey) == 0) {
                    switch(tag[i].first)
                    {
                        case INSERT:
                            if(!hasInsert && !hasUpdate && !hasDelta) {
                                accumulator = tag[i].second;
                                hasInsert = true;
                            }
                            break;
                        case UPDATE:
                            accumulator = tag[i].second;
                            hasUpdate = true;
                            break;
                        case DELTA:
                            // insert() is forbidden used together with delta
                            if(hasInsert)
                                throw std::runtime_error("Warning: In hdb, use insert() together with delta() is not recommended, \
                                    which causes bad performance, try update() instead.");

                            accumulator += tag[i].second;
                            hasDelta = true;
                            break;
                        case DELETE:
                            accumulator = ValueType();
                            hasInsert = false;
                            hasUpdate = false;
                            hasDelta = false;
                            break;
                        default:
                            throw std::runtime_error("unrecognized tag format in hdb");
                    }
                    src[i]->seq(locn[i]);
                    if( !(nonEmpty[i] = src[i]->get(locn[i], key[i], tag[i]) ) )
                        nonEmptySdbNumber --;
                }
            }
            if(hasUpdate)
                dst->insertValue(leastKey, TagType(UPDATE, accumulator));
            else if(hasDelta)
                dst->insertValue(leastKey, TagType(DELTA, accumulator));
            else if(hasInsert)
                dst->insertValue(leastKey, TagType(INSERT, accumulator));
            else {
                dst->insertValue(leastKey, TagType(DELETE, ValueType()));
                dels++;
            }
        }
    }

    for(int i=0; i<n; i++) {
        if(nonEmpty[i]) {
            while(src[i]->get(locn[i], key[i], tag[i])) {
                dst->insertValue(key[i], tag[i]);
                if(tag[i].first == DELETE)
                    dels ++;
                src[i]->seq(locn[i]);
            }
            break;
        }
    }

    delete[] locn;
    delete[] key;
    delete[] tag;
    delete[] nonEmpty;

    dst->flush();

#ifdef VERBOSE_HDB
    std::cout << " into " << dst->getName() << "(" << dst->numItems() << ")" << std::endl;
#endif
}

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
