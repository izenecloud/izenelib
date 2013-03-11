/**
 * @file HDBCursor.h
 * @brief Implementation of HDBCursor.
 * @author Wei Cao
 * @date 2009-09-24
 */

#ifndef _HDBCURSOR_H_
#define _HDBCURSOR_H_

#include <vector>

#include <hdb/HugeDB.h>
#include <hdb/MultiSDBCursor.h>

using namespace izenelib::sdb;

namespace izenelib {

namespace hdb {

template<typename HdbType>
class HDBCursor_ {

    typedef typename HdbType::SdbType SdbType;
    typedef typename SdbType::SDBCursor SDBCursor;
    typedef typename SdbType::SDBKeyType KeyType;
    typedef typename SdbType::SDBValueType TagType;
    typedef typename TagType::second_type ValueType;

    typedef MultiSDBCursor_<HdbType> MultiSDBCursor;
    typedef HDBCursor_<HdbType> ThisType;

public:

    /**
     * @brief Constructor that iterate the whole hdb.
     */
    HDBCursor_(HdbType& hdb)
    : hdb_(hdb)
    {
        hdb_.diskSdbLock_.lock_shared();
        hdb_.memorySdbLock_.lock_shared();

        creationStamp_ = hdb_.lastModificationStamp_;
        std::vector<SdbType*> sdbList;
        // all disk partitions plus the memory partition
        sdbList.resize(hdb.diskSdbList_.size()+1);
        for(size_t i = 0; i< hdb.diskSdbList_.size(); i++ ) {
            sdbList[i] = &(hdb.diskSdbList_[i]->sdb);
        }
        sdbList[hdb.diskSdbList_.size()] = &(hdb.memorySdb_);
        cursor_.init(sdbList);

        hdb_.memorySdbLock_.unlock_shared();
        hdb_.diskSdbLock_.unlock_shared();
    }

    /**
     * @brief Low cost copy constructor
     */
    HDBCursor_(const ThisType& hc)
        : hdb_(hc.hdb_),creationStamp_(hc.creationStamp_),
          cursor_(hc.cursor_) { }

    /**
     * @brief Low cost assignment operator
     */
    const HDBCursor_& operator = (const ThisType& hc)
    {
        hdb_ = hc.hdb_;
        creationStamp_ = hc.creationStamp_;
        cursor_ = hc.cursor_;
        return *this;
    }

    /**
     * @brief Move to previous element.
     */
    bool prev() {
        hdb_.diskSdbLock_.lock_shared();
        hdb_.memorySdbLock_.lock_shared();
        if(hdb_.lastModificationStamp_ != creationStamp_) {
            hdb_.diskSdbLock_.unlock_shared();
            throw std::runtime_error("concurrent modification");
        }
        bool ret = cursor_.prev();
        hdb_.memorySdbLock_.unlock_shared();
        hdb_.diskSdbLock_.unlock_shared();
        return ret;
    }

    /**
     * @brief Move to next element.
     */
    bool next() {
        hdb_.diskSdbLock_.lock_shared();
        hdb_.memorySdbLock_.lock_shared();
        if(hdb_.lastModificationStamp_ != creationStamp_) {
            hdb_.diskSdbLock_.unlock_shared();
            throw std::runtime_error("concurrent modification");
        }
        bool ret = cursor_.next();
        hdb_.memorySdbLock_.unlock_shared();
        hdb_.diskSdbLock_.unlock_shared();
        return ret;
    }

    /**
     * @brief Skip to a given element.
     */
    bool seek(const KeyType& target)
    {
        hdb_.diskSdbLock_.lock_shared();
        hdb_.memorySdbLock_.lock_shared();
        if(hdb_.lastModificationStamp_ != creationStamp_) {
            hdb_.diskSdbLock_.unlock_shared();
            throw std::runtime_error("concurrent modification");
        }
        bool ret = cursor_.seek(target);
        hdb_.memorySdbLock_.unlock_shared();
        hdb_.diskSdbLock_.unlock_shared();
        return ret;
    }

    /**
     * @brief Get key of current element
     */
    inline const KeyType& getKey() const { return cursor_.getKey(); }

    /**
     * @brief Get Tag type of current element
     */
    inline const TagType& getTag() const { return cursor_.getTag(); }

private:

    HdbType& hdb_;

    size_t creationStamp_;

    MultiSDBCursor cursor_;
};

}

}

#endif
