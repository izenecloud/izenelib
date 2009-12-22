/**
 * @file MultiSDBCursor.h
 * @brief Provide an integrated view on multiple SDBCursors.
 * @author Wei Cao
 * @date 2009-12-10
 */


#ifndef _MULTISDBCURSOR_H_
#define _MULTISDBCURSOR_H_

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
class MultiSDBCursor_ {

    typedef typename HdbType::SdbType SdbType;
    typedef typename SdbType::SDBCursor SDBCursor;
    typedef typename SdbType::SDBKeyType KeyType;
    typedef typename SdbType::SDBValueType TagType;
    typedef typename TagType::second_type ValueType;

    typedef MultiSDBCursor_<HdbType> ThisType;

public:

    /**
     * @brief Empty constructor
     */
    MultiSDBCursor_()
        : key_(), tag_(DELETE,ValueType()),
          lastDirection_(ESD_FORWARD),
          toEnd_(false) { }

    /**
     * @brief Constructor
     */
    MultiSDBCursor_(const std::vector<SdbType*>& sdbList)
        : sdbList_(sdbList),
          key_(), tag_(DELETE,ValueType()),
          lastDirection_(ESD_FORWARD),
          toEnd_(false)
    {
        cursorList_.resize(sdbList_.size());
        for(size_t i =0; i<sdbList_.size(); i++) {
            cursorList_[i] = sdbList_[i]->get_first_locn();
        }
    }

    /**
     * @brief Low cost copy constructor
     */
    MultiSDBCursor_(const ThisType& hc)
        : sdbList_(hc.sdbList_),
          cursorList_(hc.cursorList_),
          key_(hc.key_), tag_(hc.tag_),
          lastDirection_(hc.lastDirection_),
          toEnd_(hc.toEnd_) { }

    /**
     * @brief Low cost assignment operator
     */
    const MultiSDBCursor_& operator = (const ThisType& hc)
    {
        sdbList_ = hc.sdbList_;
        cursorList_ = hc.cursorList_;
        key_ = hc.key_;
        tag_ = hc.tag_;
        lastDirection_ = hc.lastDirection_;
        toEnd_ = hc.toEnd_;
        return *this;
    }

    void init(const std::vector<SdbType*>& sdbList)
    {
        sdbList_.resize(sdbList.size());
        cursorList_.resize(sdbList_.size());
        for(size_t i =0; i<sdbList.size(); i++) {
            sdbList_[i] = sdbList[i];
            cursorList_[i] = sdbList[i]->get_first_locn();
        }
        key_ = KeyType();
        tag_ = TagType(DELETE,ValueType());
        lastDirection_ = ESD_FORWARD;
        toEnd_ = false;
    }

    /**
     * @brief Move to previous element.
     */
    bool prev() {
        if(lastDirection_ == ESD_FORWARD) {
            lastDirection_ = ESD_BACKWARD;
            for(size_t i= 0; i< cursorList_.size(); i++)
                sdb(i).seq(cursorList_[i], ESD_BACKWARD);
            if(!toEnd_) seq<ESD_BACKWARD>();
        }
        return seq<ESD_BACKWARD>();
    }

    /**
     * @brief Move to next element.
     */
    bool next() {
        if(lastDirection_ == ESD_BACKWARD) {
            lastDirection_ = ESD_FORWARD;
            for(size_t i= 0; i< cursorList_.size(); i++)
                sdb(i).seq(cursorList_[i], ESD_FORWARD);
            if(!toEnd_) seq<ESD_FORWARD>();
        }
        return seq<ESD_FORWARD>();
    }

    /**
     * @brief Skip to a given element.
     */
    bool seek(const KeyType& target)
    {
        lastDirection_ = ESD_FORWARD;
        for(size_t i= 0; i< cursorList_.size(); i++)
            sdb(i).search(target, cursorList_[i]);
        return seq<ESD_FORWARD>();
    }

    /**
     * @brief Get key of current element
     */
    inline const KeyType& getKey() const { return key_; }

    /**
     * @brief Get Tag type of current element
     */
    inline const TagType& getTag() const { return tag_; }

protected:

    /**
     * @brief This works really like to part of a merge-sort algorithm.
     *        There are multiple partitions inside a hdb, each partition
     *        can be regarded as a sorted list of keys.
     *        so it needs to find the smallest key from these partitions
     *        if we are moving forward, or select the biggest key if we're
     *        moving backward.
     *        The challenging thing is duplicated keys may exist between
     *        partitions, so we need to combine them together based on tag.
     */
    template <ESeqDirection direction>
    inline bool seq()
    {
        size_t idx = -1U;
        KeyType hitKey = KeyType();
        bool hasDuplicatedKey = false;

        // Select the smallest/biggest key among serveral partitions
        KeyType tmpk = KeyType();
        TagType tmpt = TagType();
        for(size_t i= 0; i< cursorList_.size(); i++) {
            if( !sdb(i).get(cursorList_[i], tmpk, tmpt) ) continue;

            if(idx == -1U) {
                idx = i;
                hitKey = tmpk;
                hasDuplicatedKey = false;
            } else {
                int lt = comp_(tmpk, hitKey);
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

        // only one partition contains this key
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
            // serveral partitions contain the key,
            // so we need to combine tags and values into a single record.
            ValueType accumulator = ValueType();
            bool hasInsert = false;
            bool hasUpdate = false;
            bool hasDelta = false;
            // process all duplicated keys, safe starting from idx
            for(size_t i=idx; i<cursorList_.size(); i++) {
                if( !sdb(i).get(cursorList_[i], tmpk, tmpt) ) continue;
                if( comp_(tmpk, hitKey) == 0) {
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

                            accumulator = HdbType::ADD(accumulator, tmpt.second);
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

    inline SdbType& sdb(size_t idx) {
        return *sdbList_[idx];
    }

    inline SDBCursor& cursor(size_t idx) {
        return cursorList_[idx];
    }

private:

    std::vector<SdbType*> sdbList_;

    std::vector<SDBCursor> cursorList_;

    KeyType key_;

    TagType tag_;

    ESeqDirection lastDirection_;

    bool toEnd_;

    CompareFunctor<KeyType> comp_;
};

}

}

#endif
