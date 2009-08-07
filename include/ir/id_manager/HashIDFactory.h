/**-
 * @file	IDFactory.h
 * @brief	Header file of ID Factor Class
 * @author	Quang & Do Hyun Yun
 * @date    2008-11-18
 * @details
 * ==============
 *
 * Using SDB/hash
 * @author Peisheng Wang
 * @date 2009-04-16
 *
 * ==============
 *
 * Refactor to HashIDFactory
 * Using hash to genreate ID
 * @author Wei Cao
 * @date 2009-08-07
 *
 * ==============
 */

#ifndef _HASH_ID_FACTORY_H_
#define _HASH_ID_FACTORY_H_

#include "IDManagerTypes.h"
#include "IDFactoryException.h"
#include "IDFactoryErrorString.h"

NS_IZENELIB_IR_BEGIN

namespace idmanager {

template <typename  NameString  = std::string,
          typename  NameID      = uint32_t,
          NameID    (*HashFunc)(const NameString&) = HashFunction<NameString>::generateHash32,
          typename  LockType    = izenelib::util::NullLock>
class HashIDFactory
{
	typedef izenelib::sdb::ordered_sdb<NameID, NameString, LockType> NameFinder;

public:

	/**
	 * @brief Constructor which initialize initialValue and max value of id.
	 *
	 * @param sdbName       name of sdb storage.
	 * @param initialValue  start value of id.
	 * @param maxValue      maximum value of id.
	 */
	HashIDFactory(const string& sdbName);

	HashIDFactory(const HashIDFactory<NameString, NameID, HashFunc, LockType>& idFactory);

	virtual ~HashIDFactory();

	/**
	 * @brief This function returns a unique name id given a name string
	 * @param nameString the name string
	 * @param nameID the unique NameID
	 * @return true if the name id is successfully returned
	 * @return false if no more name id is available
	 */
	bool getNameIDByNameString(const NameString& nameString, NameID& nameID);

	/**
	 * @brief This function returns a list of unique name ids given a list of
	 * name string
	 * @param nameStringList the list of name strings
	 * @param nameIDList the list of unique name ids
	 * @return true if the list of name ids is successfully returned
	 * @return false if no more name id is available
	 */
	bool getNameIDListByNameStringList(
			const std::vector<NameString>& nameStringList,
			std::vector< NameID>& nameIDList);
	/**
	 * @brief This function returns a name string given name ID
	 * @param nameID the unique Name ID
	 * @param nameString the name string
	 * @return true if the name string is successfully returned
	 * @return false if name id is not available
	 */
	bool getNameStringByNameID(const NameID& nameID, NameString& nameString);

	/**
	 * @brief This function returns a list of name strings given
	 * a list of name ids
	 * @param namelDList the list of name ids
	 * @param nameStringList the list of name strings
	 * @return true if the list of name strings is successfully returned
	 * @return false if a name id in the input list is not available
	 */
	bool getNameStringListByNameIDList(const std::vector<NameID> &nameIDList,
			std::vector< NameString> &nameStringList);

	void display()
	{
		nameFinder_.display();
	}

protected:

	string sdbName_;

	NameFinder nameFinder_; ///< an inverted indexer which gives name according to the id.
}; // end - template SequentialIDFactory

template <typename NameString, typename NameID,
    NameID (*HashFunc)(const NameString&), typename LockType>
HashIDFactory<NameString, NameID, HashFunc, LockType>::HashIDFactory(
    const string& sdbName)
:
    sdbName_(sdbName),
    nameFinder_(sdbName_ + "_id.sdb")
{
	nameFinder_.open();
} // end - SequentialIDFactory()


template <typename NameString, typename NameID,
    NameID (*HashFunc)(const NameString&), typename LockType>
HashIDFactory<NameString, NameID, HashFunc, LockType>::HashIDFactory(
        const HashIDFactory<NameString, NameID,
            HashFunc, LockType>& idFactory)
{
} // end - SequentialIDFactory()

template <typename NameString, typename NameID,
    NameID (*HashFunc)(const NameString&), typename LockType>
HashIDFactory<NameString, NameID, HashFunc, LockType>::~HashIDFactory()
{
} // end - ~SequentialIDFactory()

template <typename NameString, typename NameID,
    NameID (*HashFunc)(const NameString&), typename LockType>
bool HashIDFactory<NameString, NameID, HashFunc, LockType>::getNameIDByNameString(
        const NameString& nameString,
        NameID& nameID)
{
    nameID = HashFunc(nameString);
	nameFinder_.insertValue(nameID, nameString);

	return false;
} // end - getNameIDByNameString()

template <typename NameString, typename NameID,
    NameID (*HashFunc)(const NameString&), typename LockType>
bool HashIDFactory<NameString, NameID, HashFunc, LockType>::getNameStringByNameID(
        const NameID& nameID, NameString& nameString)
{
	return nameFinder_.getValue(nameID, nameString);
} // end - getNameStringByNameID()

}
// end - namespace idmanager

NS_IZENELIB_IR_END

#endif // #ifndef _ID_FACTORY_H_
