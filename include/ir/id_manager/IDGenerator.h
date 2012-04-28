/**-
 * @file	IDGenerator.h
 * @brief	Contain two types of IDGenerator, HashID and UniqueID.
 *          HashID use hash to genreate ID,
 *          UniqueID generate ID using a sequential number.
 * @author Wei Cao
 * @date 2009-08-07
 */

#ifndef _ID_GENERATOR_H_
#define _ID_GENERATOR_H_
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <types.h>

#include <util/hashFunction.h>
#include <util/DynamicBloomFilter.h>
#include <util/ThreadModel.h>
#include <sdb/SequentialDB.h>
#include <am/leveldb/Table.h>

#include "IDFactoryException.h"
#include "IDFactoryErrorString.h"

#include "NameIDTraits.h"


NS_IZENELIB_IR_BEGIN

namespace idmanager {

template <typename  NameString,
          typename  NameID>
class EmptyIDGenerator
{
public:

    /**
     * @brief Constructor.
     */
    EmptyIDGenerator(const string&){}

    /**
     * @brief Always return false, which means failure to generate ID
     */
    inline bool conv(const NameString& nameString, NameID& nameID, bool insert = false)
    {
        return false;
    }

    /**
     * @brief Always return 0, which means failure to generate ID.
     */
    NameID maxConvID() const
    {
        return 0;
    }

    void flush(){}

    void close(){}

    void display(){}

}; // end - template EmptyIDGenerator



template <typename  NameString,
          typename  NameID>
class HashIDGenerator
{
public:

    /**
     * @brief Constructor.
     */
    HashIDGenerator(const string&){}

    /**
     * @brief Convert String to ID, ID may be not unique
     * @param nameString the name string
     * @param nameID the NameID that may be not unique
     * @return always false
     */
    inline bool conv(const NameString& nameString, NameID& nameID, bool insert = false)
    {
        nameID = NameIDTraits<NameID>::hash(nameString);
        return false;
    }

    /**
     * @brief Always return 0, which means this function is not supported
     */
    NameID maxConvID() const
    {
        return 0;
    }

    void flush(){}

    void close(){}

    void display(){}

}; // end - template HashIDGenerator


template <
          typename  NameString,
          typename  NameID,
          typename  LockType    = izenelib::util::NullLock,
          NameID    MinIDValue  = NameIDTraits<NameID>::MinValue,
          NameID    MaxIDValue  = NameIDTraits<NameID>::MaxValue>
class UniqueIDGenerator
{
    typedef izenelib::am::leveldb::Table<NameString, NameID> IdFinder;

public:

    /**
     * @brief Constructor.
     *
     * @param sdbName       name of sdb storage.
     */
    UniqueIDGenerator(const string& sdbName);

    virtual ~UniqueIDGenerator();

    /**
     * @brief This function returns a unique name id given a name string.
     * @param nameString the name string
     * @param nameID the unique NameID
     * @param insert whether insert nameString if it does not exist
     * @return true if DocID already in dictionary
     * @return false otherwise
     */
    inline bool conv(const NameString& nameString, NameID& nameID, bool insert = true);

    /**
     * @brief This function returns a unique name id given a name string, update old id to
     * satisfy the incremental semantic
     * @param nameString the name string
     * @param oldID the old unique NameID
     * @param updatedID the updated unique NameID
     * @return true if DocID already in dictionary
     * @return false otherwise
     */
    inline bool conv(const NameString& nameString, NameID& oldID, NameID& updatedID);

    /**
     * @brief Get the maximum converted id.
     * @return max converted id, 0 for no id converted before.
     */
    NameID maxConvID() const
    {
        if (newID_ != MinIDValue)
            return newID_ - 1;

        return 0;
    }

    void flush()
    {
        saveBloomFilter_();
        saveNewId_();
        idFinder_.flush();
    }

    void close()
    {
        flush();
        idFinder_.close();
    }

    void display()
    {
//      idFinder_.display();
    }

protected:

    bool saveBloomFilter_() const
    {
        try
        {
            std::ofstream ofs(bloomFilterFile_.c_str());
            if (ofs)
            {
                bloomFilter_.save(ofs);
            }
            return ofs;
        }
        catch (boost::archive::archive_exception& e)
        {
            return false;
        }
    }

    bool loadBloomFilter_()
    {
        try
        {
            std::ifstream ifs(bloomFilterFile_.c_str(), std::ios_base::binary);
            if (ifs)
            {
                bloomFilter_.load(ifs);
            }
            return ifs;
        }
        catch (boost::archive::archive_exception& e)
        {
            newID_ = minID_;
            return false;
        }
    }

    bool saveNewId_() const
    {
        try
        {
            std::ofstream ofs(newIdFile_.c_str(), std::ios_base::binary);
            if (ofs)
            {
                boost::archive::xml_oarchive oa(ofs);
                oa << boost::serialization::make_nvp(
                    "NewID", newID_
                );
            }

            return ofs;
        }
        catch (boost::archive::archive_exception& e)
        {
            return false;
        }
    }

    bool restoreNewId_()
    {
        try
        {
            std::ifstream ifs(newIdFile_.c_str());
            if (ifs)
            {
                boost::archive::xml_iarchive ia(ifs);
                ia >> boost::serialization::make_nvp(
                    "NewID", newID_
                );
            }
            return ifs;
        }
        catch (boost::archive::archive_exception& e)
        {
            newID_ = minID_;
            return false;
        }
    }

protected:

    NameID minID_; ///< An minimum ID.
    NameID maxID_; ///< An maximum ID.
    NameID newID_; ///< An ID for new name.
    string sdbName_;
    string bloomFilterFile_;
    string newIdFile_;

    LockType mutex_;

    DynamicBloomFilter<NameString> bloomFilter_;
    IdFinder idFinder_; ///< an indexer which gives ids according to the name.
}; // end - template UniqueIDGenerator

template <typename NameString, typename NameID,
    typename LockType, NameID MinValueID, NameID MaxValueID>
UniqueIDGenerator<NameString, NameID,
    LockType, MinValueID, MaxValueID>::UniqueIDGenerator(
        const string& sdbName)
:
    minID_(MinValueID),
    maxID_(MaxValueID),
    newID_(MinValueID),
    sdbName_(sdbName),
    bloomFilterFile_(sdbName_ + "_bloom_filter"),
    newIdFile_(sdbName_ + "_newid.xml"),
    bloomFilter_(10000000, 0.001, 10000000),
    idFinder_(sdbName_ + "_name_storage")
{
    loadBloomFilter_();
    idFinder_.open();
    restoreNewId_();
} // end - UniqueIDGenerator()

template <typename NameString, typename NameID,
    typename LockType, NameID MinValueID, NameID MaxValueID>
UniqueIDGenerator<NameString, NameID,
    LockType, MinValueID, MaxValueID>::~UniqueIDGenerator()
{
    close();
} // end - ~UniqueIDGenerator()

template <typename NameString, typename NameID,
    typename LockType, NameID MinValueID, NameID MaxValueID>
inline bool UniqueIDGenerator<NameString, NameID,
    LockType, MinValueID, MaxValueID>::conv(
        const NameString& nameString,
        NameID& nameID,
        bool insert)
{
    mutex_.acquire_write_lock();

    // If name string is found, return the id.
    if (bloomFilter_.Get(nameString))
    {
        if (idFinder_.get(nameString, nameID))
        {
            mutex_.release_write_lock();
            return true;
        }
    }// end - if

    if (!insert)
    {
        mutex_.release_write_lock();
        return false;
    }

    bloomFilter_.Insert(nameString);
    // Because there's no name string in idFinder, create new id according to the string.
    nameID = newID_;
    newID_++;

    // check correctness of input nameID
    if (newID_> maxID_)
    {
        mutex_.release_write_lock();
        throw IDFactoryException(SF1_ID_FACTORY_OUT_OF_BOUND, __LINE__, __FILE__);
    }

    idFinder_.insert(nameString, nameID);
    mutex_.release_write_lock();
    return false;
} // end - conv()

template <typename NameString, typename NameID,
    typename LockType, NameID MinValueID, NameID MaxValueID>
inline bool UniqueIDGenerator<NameString, NameID,
    LockType, MinValueID, MaxValueID>::conv(
        const NameString& nameString,
        NameID& oldID,
        NameID& updatedID)
{
    mutex_.acquire_write_lock();

    // If name string is found, return the id.
    bool ret = bloomFilter_.Get(nameString) && idFinder_.get(nameString, oldID);

    if (!ret)
    {
        oldID = 0;
        ///will be removed until MIA can support index unexist documents from Update SCDs
        mutex_.release_write_lock();
        return ret;
    }

    // Because there's no name string in idFinder, create new id according to the string.
    updatedID = newID_;
    newID_++;

    // check correctness of input nameID
    if (newID_ > maxID_)
    {
        mutex_.release_write_lock();
        throw IDFactoryException(SF1_ID_FACTORY_OUT_OF_BOUND, __LINE__, __FILE__);
    }

    idFinder_.update(nameString, updatedID);
    mutex_.release_write_lock();
    return ret;
} // end - conv()

}
// end - namespace idmanager

NS_IZENELIB_IR_END

#endif // #ifndef _HASH_ID_H_
