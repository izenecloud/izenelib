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
	typedef izenelib::sdb::ordered_sdb<NameID, NameString, LockType> NameFinder;
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

	void display(){}
}; // end - template EmptyIDStorage

}
// end - namespace idmanager

NS_IZENELIB_IR_END

#endif // #ifndef _ID_FACTORY_H_