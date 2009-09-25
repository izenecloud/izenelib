/**
 * @file HugeDB.h
 * @brief Implementation of HugeDB,
 *        Enhancement of SequentialDB for random accesses on large scale data set.
 * @author Wei Cao
 * @date 2009-09-24
 */

#ifndef _HDBCURSOR_H_
#define _HDBCURSOR_H_

#include <cstdlib>
#include <string>
#include <vector>

#include <am/concept/DataType.h>

#include <sdb/SequentialDB.h>
#include <hdb/HugeDB.h>

using namespace izenelib::sdb;

namespace izenelib {

namespace hdb {

/**
 * @brief Modification tags
 */
enum Tag{
    INSERT = 'I',
    UPDATE = 'U',
    DELETE = 'R', // Remove
    DELTA = 'D',
};

template<typename HdbType>
class HDBCursor_ {

    typedef typename HdbType::SdbType SdbType;
    typedef typename SdbType::SDBCursor SDBCursor;
    typedef typename SdbType::SDBKeyType KeyType;
    typedef typename SdbType::SDBValueType TagType;
    typedef typename TagType::second_type ValueType;

    typedef HDBCursor_<HdbType> ThisType;

public:

    HDBCursor_(HdbType& hdb, size_t start, size_t len)
    : hdb_(hdb),start_(start), len_(len)
    {
        cursorList_.reserve(sdbNum());
        for(size_t i = 0; i<sdbNum(); i++ )
            cursorList_.push_back( sdb(i).get_first_Locn() );
        key_ = KeyType();
        tag_.first = DELETE;
        lastDirection_ = ESD_FORWARD;
        toEnd_ = false;
    }

    HDBCursor_(HdbType& hdb)
    : hdb_(hdb),start_(0), len_(hdb.sdbList_.size())
    {
        cursorList_.reserve(sdbNum());
        for(size_t i = 0; i<sdbNum(); i++ )
            cursorList_.push_back( sdb(i).get_first_Locn() );
        key_ = KeyType();
        tag_ = TagType(DELETE, ValueType());
        lastDirection_ = ESD_FORWARD;
        toEnd_ = false;
    }

    HDBCursor_(const ThisType& hc)
        : hdb_(hc.hdb_), start_(hc.start_), len_(hc.len_),
          key_(hc.key_), tag_(hc.tag_),
          cursorList_(hc.cursorList_),
          lastDirection_(hc.lastDirection_),
          toEnd_(hc.toEnd_) { }

    const HDBCursor_& operator = (const ThisType& hc)
    {
        hdb_ = hc.hdb_;
        start_ = hc.start_;
        len_ = hc.len_;
        key_ = hc.key_;
        tag_ = hc.tag_;
        cursorList_ = hc.cursorList_;
        lastDirection_ = hc.lastDirection_;
        toEnd_ = hc.toEnd_;
        return *this;
    }

    bool prev() {
        if(lastDirection_ == ESD_FORWARD) {
            lastDirection_ = ESD_BACKWARD;
            for(size_t i=0; i<sdbNum(); i++)
                sdb(i).seq(cursorList_[i], ESD_BACKWARD);
            if(!toEnd_) seq<ESD_BACKWARD>();
        }
        return seq<ESD_BACKWARD>();
    }

    bool next() {
        if(lastDirection_ == ESD_BACKWARD) {
            lastDirection_ = ESD_FORWARD;
            for(size_t i=0; i<sdbNum(); i++)
                sdb(i).seq(cursorList_[i], ESD_FORWARD);
            if(!toEnd_) seq<ESD_FORWARD>();
        }
        return seq<ESD_FORWARD>();
    }

    bool seek(const KeyType& target)
    {
        lastDirection_ = ESD_FORWARD;
        for(size_t i=0; i<sdbNum(); i++) {
            sdb(i).search(target, cursorList_[i]);
        }
        return seq<ESD_FORWARD>();
    }

    inline const KeyType& getKey() const { return key_; }

    inline const TagType& getTag() const { return tag_; }

protected:

    template <ESeqDirection direction>
    inline bool seq()
    {
        size_t idx = -1U;
        KeyType hitKey = KeyType();
        bool hasDuplicatedKey = false;

        // pass 1: find the least key
        KeyType tmpk = KeyType();
        TagType tmpt = TagType();
        for(size_t i=0; i<sdbNum(); i++) {
            if( !sdb(i).get(cursorList_[i], tmpk, tmpt) ) continue;

            if(idx == -1U) {
                idx = i;
                hitKey = tmpk;
                hasDuplicatedKey = false;
            } else {
                int lt = hdb_.comp_(tmpk, hitKey);
                if(direction == ESD_FORWARD) {
                    if( lt < 0 ) {
                        idx = i;
                        hitKey = tmpk;
                        hasDuplicatedKey = false;
                    } else if( lt == 0 ) {
                        hasDuplicatedKey = true;
                    }
                } else {
                    if( lt > 0 ) {
                        idx = i;
                        hitKey = tmpk;
                        hasDuplicatedKey = false;
                    } else if( lt == 0 ) {
                        hasDuplicatedKey = true;
                    }
                }
            }
        }

        // simple case
        if(!hasDuplicatedKey) {
            if(idx == -1U)  {
                key_ = KeyType();
                tag_ = TagType(DELETE, ValueType());
                toEnd_ = true;
                return false;
            }
            sdb(idx).get(cursorList_[idx], key_, tag_);
            if(direction == ESD_FORWARD)
                sdb(idx).seq(cursorList_[idx], ESD_FORWARD);
            else
                sdb(idx).seq(cursorList_[idx], ESD_BACKWARD);
            toEnd_ = false;
            return true;
        } else {
            ValueType accumulator = ValueType();
            bool hasInsert = false;
            bool hasUpdate = false;
            bool hasDelta = false;
            // pass 2: process all duplicated keys, safe starting from idx
            for(size_t i=idx; i<sdbNum(); i++) {
                if( !sdb(i).get(cursorList_[i], tmpk, tmpt) ) continue;
                if( hdb_.comp_(tmpk, hitKey) == 0) {
                    switch(tmpt.first)
                    {
                        case INSERT:
                            if(!hasInsert && !hasUpdate && !hasDelta) {
                                accumulator = tmpt.second;
                                hasInsert = true;
                            }
                            break;
                        case UPDATE:
                            accumulator = tmpt.second;
                            hasUpdate = true;
                            break;
                        case DELTA:
                            // insert() is forbidden used together with delta
                            if(hasInsert)
                                throw std::runtime_error("Warning: In hdb, use insert() together with delta() is not recommended, \
                                    which causes bad performance, try update() instead.");

                            accumulator += tmpt.second;
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

                    if(direction == ESD_FORWARD)
                        sdb(i).seq(cursorList_[i], ESD_FORWARD);
                    else
                        sdb(i).seq(cursorList_[i], ESD_BACKWARD);
                }
            }
            key_ = hitKey;
            if(hasUpdate)
                tag_ = TagType(UPDATE, accumulator);
            else if(hasDelta)
                tag_ = TagType(DELTA, accumulator);
            else if(hasInsert)
                tag_ = TagType(INSERT, accumulator);
            else
                tag_ = TagType(DELETE, ValueType());
            toEnd_ = false;
            return true;
        }
    }

    inline size_t sdbNum() { return len_; }

    inline SdbType& sdb(size_t idx) {
        return hdb_.sdbList_[idx]->sdb;
    }

    inline SDBCursor& cursor(size_t idx) {
        return cursorList_[idx];
    }

private:

    HdbType& hdb_;

    size_t start_;

    size_t len_;

    KeyType key_;

    TagType tag_;

    std::vector<SDBCursor> cursorList_;

    ESeqDirection lastDirection_;

    bool toEnd_;
};

}

}

#endif
