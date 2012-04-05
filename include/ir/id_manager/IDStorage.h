/**
 * @file IDStorage.h
 * @brief store <ID, String> pairs, and return string for given ID.
 * @author Wei Cao
 * @date 2009-08-07
 */

#ifndef _ID_STORAGE_H_
#define _ID_STORAGE_H_

#include <string>

#include <types.h>

#include <sdb/SequentialDB.h>
#include <am/tokyo_cabinet/tc_btree.h>
#include <am/tokyo_cabinet/tc_hash.h>
NS_IZENELIB_IR_BEGIN

namespace idmanager {

/**
 * Store <ID, String> pair in SDB.
 */
template <typename  NameString,
          typename  NameID,
          typename  LockType    = izenelib::util::NullLock>
class SDBIDStorage
{
    typedef izenelib::sdb::unordered_sdb_tc<NameID, NameString, LockType> NameFinder;
public:

    /**
     * @brief Constructor.
     *
     * @param sdbName       name of sdb storage.
     */
    SDBIDStorage(const std::string& sdbName);

    virtual ~SDBIDStorage();

    /**
     * @brief This function inserts a <ID, String> pair into storage.
     * @param nameID the Name ID
     * @param nameString the name string
     * @return true if successfully inserted
     * @return false otherwise
     */
    void put(const NameID& nameID, const NameString& nameString);

    /**
     * @brief This function returns the String for a given ID.
     * @param nameID the Name ID
     * @param nameString the name string
     * @return true if the name string is successfully returned
     * @return false if name id is not available
     */
    bool get(const NameID& nameID, NameString& nameString);

    void flush()
    {
        nameFinder_.flush();
    }

    void close()
    {
        nameFinder_.close();
    }

    void display()
    {
        nameFinder_.display();
    }

protected:

    std::string sdbName_;

    NameFinder nameFinder_; ///< an inverted indexer which gives name according to the id.
}; // end - template SDBIDStorage

template <typename NameString, typename NameID, typename LockType>
SDBIDStorage<NameString, NameID, LockType>::SDBIDStorage(
        const std::string& sdbName)
:
    sdbName_(sdbName),
    nameFinder_(sdbName_ + "_id.sdb")
{
  	nameFinder_.open();
    //nameFinder_.setCacheSize(1000);
} // end - SDBIDStorage()

template <typename NameString, typename NameID, typename LockType>
SDBIDStorage<NameString, NameID, LockType>::~SDBIDStorage()
{
} // end - ~SDBIDStorage()

template <typename NameString, typename NameID, typename LockType>
void SDBIDStorage<NameString, NameID, LockType>::put( const NameID& nameID,
    const NameString& nameString)
{
    nameFinder_.insertValue(nameID, nameString);
} // end - put()

template <typename NameString, typename NameID, typename LockType>
bool SDBIDStorage<NameString, NameID, LockType>::get( const NameID& nameID,
    NameString& nameString)
{
    return nameFinder_.getValue(nameID, nameString);
} // end - get()



/**
 * Store <ID, String> pair in HDB.
 */
template <typename  NameString,
          typename  NameID,
          typename  LockType    = izenelib::util::NullLock>
class HDBIDStorage
{
    typedef izenelib::hdb::ordered_hdb_no_delta<NameID, NameString, LockType> NameFinder;
public:

    /**
     * @brief Constructor.
     *
     * @param sdbName       name of sdb storage.
     */
    HDBIDStorage(const std::string& sdbName);

    virtual ~HDBIDStorage();

    /**
     * @brief This function inserts a <ID, String> pair into storage.
     * @param nameID the Name ID
     * @param nameString the name string
     * @return true if successfully inserted
     * @return false otherwise
     */
    void put(const NameID& nameID, const NameString& nameString);

    /**
     * @brief This function returns the String for a given ID.
     * @param nameID the Name ID
     * @param nameString the name string
     * @return true if the name string is successfully returned
     * @return false if name id is not available
     */
    bool get(const NameID& nameID, NameString& nameString);

    void flush()
    {
        nameFinder_->flush();
    }

    void release()
    {
        nameFinder_->optimize();
        nameFinder_->release();
        if(nameFinder_)
        {
            delete(nameFinder_);
            nameFinder_=new NameFinder(sdbName_ + "_id.sdb");
        }

    }

    void close()
    {
        nameFinder_->close();
    }

    void display()
    {
        nameFinder_->display();
    }

protected:

    std::string sdbName_;

    NameFinder* nameFinder_; ///< an inverted indexer which gives name according to the id.
}; // end - template SDBIDStorage

template <typename NameString, typename NameID, typename LockType>
HDBIDStorage<NameString, NameID, LockType>::HDBIDStorage(
        const std::string& sdbName)
:
    sdbName_(sdbName)
    /*nameFinder_(sdbName_ + "_id.sdb")*/
{
    nameFinder_=new NameFinder(sdbName_ + "_id.sdb");
    nameFinder_->setCachedRecordsNumber(2000000);
  	nameFinder_->open();
} // end - SDBIDStorage()

template <typename NameString, typename NameID, typename LockType>
HDBIDStorage<NameString, NameID, LockType>::~HDBIDStorage()
{
    if(nameFinder_)
        delete(nameFinder_);
} // end - ~SDBIDStorage()

template <typename NameString, typename NameID, typename LockType>
void HDBIDStorage<NameString, NameID, LockType>::put( const NameID& nameID,
    const NameString& nameString)
{
    nameFinder_->insertValue(nameID, nameString);
} // end - put()

template <typename NameString, typename NameID, typename LockType>
bool HDBIDStorage<NameString, NameID, LockType>::get( const NameID& nameID,
    NameString& nameString)
{
    return nameFinder_->getValue(nameID, nameString);
} // end - get()



/**
* Store <ID, String> pair in TC.
*/
template <typename  NameString,
typename  NameID,
typename  LockType    = izenelib::util::NullLock>
class TCIDStorage
{

    typedef izenelib::am::tc_hash<NameID, NameString, LockType> NameFinder;
    public:

    /**
    * @brief Constructor.
    *
    * @param sdbName       name of sdb storage.
    */
    TCIDStorage(const std::string& sdbName);

    virtual ~TCIDStorage();

    /**
    * @brief This function inserts a <ID, String> pair into storage.
    * @param nameID the Name ID
    * @param nameString the name string
    * @return true if successfully inserted
    * @return false otherwise
    */
    void put(const NameID& nameID, const NameString& nameString);

    /**
    * @brief This function returns the String for a given ID.
    * @param nameID the Name ID
    * @param nameString the name string
    * @return true if the name string is successfully returned
    * @return false if name id is not available
    */
    bool get(const NameID& nameID, NameString& nameString);

    void flush()
    {
        nameFinder_.release();
    }

    void close()
    {
        nameFinder_.close();
    }

    void display()
    {
        nameFinder_.display();
    }

    protected:

    std::string sdbName_;

    NameFinder nameFinder_; ///< an inverted indexer which gives name according to the id.
}; // end - template SDBIDStorage

template <typename NameString, typename NameID, typename LockType>
TCIDStorage<NameString, NameID, LockType>::TCIDStorage(
const std::string& sdbName)
:
sdbName_(sdbName),
nameFinder_(sdbName_ + "_id.tc")
{
    nameFinder_.open();
} // end - SDBIDStorage()

template <typename NameString, typename NameID, typename LockType>
TCIDStorage<NameString, NameID, LockType>::~TCIDStorage()
{
} // end - ~SDBIDStorage()

template <typename NameString, typename NameID, typename LockType>
void TCIDStorage<NameString, NameID, LockType>::put( const NameID& nameID,const NameString& nameString)
{
//     std::cout<<"IDADD "<<nameID<<","<<nameString<<std::endl;
    nameFinder_.insert(nameID, nameString);
} // end - put()

template <typename NameString, typename NameID, typename LockType>
bool TCIDStorage<NameString, NameID, LockType>::get( const NameID& nameID, NameString& nameString)
{
//     std::cout<<"IDGET "<<nameID<<std::endl;
    return nameFinder_.get(nameID, nameString);
} // end - get()



/**
 * This class does nothing and never save anything to disk.
 * It's written for those guys who want String->ID only
 * and don't need the reverse conversion.
 *
 * Use this class to instantiate template IDFactory, like:
 *
 * typedef IDFactory<UString, uint32_t, HashID, EmptyIDStorage>
 *         IDFactoryWithoutFile;
 */

template <typename  NameString,
          typename  NameID>
class EmptyIDStorage
{
public:

    EmptyIDStorage(const std::string& ){}

    void put(const NameID& nameID, const NameString& nameString)
    {
    }

    bool get(const NameID& nameID, NameString& nameString)
    {
        return false;
    }

    void flush(){}

    void close(){}

    void display(){}
}; // end - template EmptyIDStorage

}
// end - namespace idmanager

NS_IZENELIB_IR_END

#endif // #ifndef _ID_FACTORY_H_
