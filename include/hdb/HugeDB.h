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
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>

#include <am/concept/DataType.h>
#include <am/sdb_btree/sdb_btree.h>

#include <sdb/SequentialDB.h>

using namespace izenelib::sdb;

namespace izenelib {

namespace hdb {

/**
 * @brief Hdb header
 */
struct HugeDBHeader {

    /**
     * Number of sdbs.
     */
    size_t slicesNum;

    /**
     * Level of each sdb.
     */
    std::vector<int> slicesLevel;

    /**
     * Size of each sdb.
     */
    std::vector<size_t> deletions;

    HugeDBHeader(const std::string& path)
        : path_(path)
	{
	    ifstream ifs(path_.c_str());
        if( !ifs ) {
            slicesNum = 0;
            return;
        }

        ifs.seekg(0, ifstream::end);
        if( 0 == ifs.tellg()) {
            slicesNum = 0;
        } else {
            ifs.seekg(0, fstream::beg);
            boost::archive::xml_iarchive xml(ifs);
            xml >> boost::serialization::make_nvp("PartitionNumber", slicesNum);
            xml >> boost::serialization::make_nvp("PartitionLevel", slicesLevel);
            xml >> boost::serialization::make_nvp("Deletions", deletions);
        }
        ifs.close();
    }

    ~HugeDBHeader()
    {
        flush();
    }

    void flush()
    {
        ofstream ofs(path_.c_str());
        boost::archive::xml_oarchive xml(ofs);
        xml << boost::serialization::make_nvp("PartitionNumber", slicesNum);
        xml << boost::serialization::make_nvp("PartitionLevel", slicesLevel);
        xml << boost::serialization::make_nvp("Deletions", deletions);
        ofs.flush();
    }

	void display(std::ostream& os = std::cout)
	{
		os << "Number of partitions " << slicesNum << std::endl;
		if(slicesNum != 0) {
            os << "Partitions:";
            for(size_t i= 0; i<slicesNum; i++)
                os << " " << "[L" << slicesLevel[i] << "]"
                    << deletions[i] << "deletions";
		}
	}

private:

    std::string path_;

};


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

    enum Tag{
        INSERT = 'I',
        UPDATE = 'U',
        DELETE = 'R', // Remove
        DELTA = 'D',
    };

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

    HugeDB(const std::string &hdbName)
        : hdbName_(hdbName),
          isOpen_(false), isSync_(true),
          headerPath_(hdbName_ + ".hdb.header.xml"),
          header_(headerPath_),
          cachedRecordsNumber_(2000000),
          mergeFactor_(2)
    {
        header_.display();
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
	    std::cout << "optimize ScalableDB, merge " << sdbList_.size() << " small sdbs together" << std::endl;
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
            dst->sdb.setPageSize(8*1024);
            dst->sdb.setDegree(128);
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
        dst->sdb.setPageSize(8*1024);
        dst->sdb.setDegree(128);
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
        ramsdb->sdb.setPageSize(8*1024);
        ramsdb->sdb.setDegree(128);
        ramsdb->sdb.setCacheSize( (size_t)-1 );
        ramsdb->sdb.open();
        sdbList_.push_back(ramsdb);
        std::cout << "create ram sdb " << name << std::endl;
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
    enum Tag{
        INSERT = 'I',
        UPDATE = 'U',
        DELETE = 'R', // Remove
        DELTA = 'D',
    };

    typedef typename SdbType::SDBCursor SdbCursor;
    typedef typename SdbType::SDBKeyType KeyType;
    typedef typename SdbType::SDBValueType TagType;
    typedef typename TagType::second_type ValueType;

    std::cout << "merge btree ";
    for(int i=0; i<n; i++)
        std::cout << src[i]->getName() << "(" << src[i]->numItems() << ") ";
    std::cout << "...\n";

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
#ifdef DEBUG
            std::cout << "detect duplicate key" << std::endl;
#endif

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
    std::cout << " into " << dst->getName() << "(" << dst->numItems() << ")" << std::endl;
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
