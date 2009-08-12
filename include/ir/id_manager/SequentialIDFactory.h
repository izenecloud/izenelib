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
 * Refactor to SequentialIDFactory.h
 * @author Wei Cao
 * @date 2009-08-07
 *
 * ==============
 */

#ifndef _SEQENTIAL_ID_FACTORY_H_
#define _SEQENTIAL_ID_FACTORY_H_

#include "IDManagerTypes.h"
#include "IDFactoryException.h"
#include "IDFactoryErrorString.h"

NS_IZENELIB_IR_BEGIN

namespace idmanager {

template <
          typename  NameString  = std::string,
          typename  NameID      = uint32_t,
          NameID    MinIDValue  = 1,
          NameID    MaxIDValue  = -2,
          typename  LockType    = izenelib::util::NullLock>
class SequentialIDFactory
{
	typedef izenelib::sdb::unordered_sdb<NameString, NameID, LockType> IdFinder;
	typedef izenelib::sdb::unordered_sdb<NameID, NameString, LockType> NameFinder;
public:

	/**
	 * @brief Constructor which initialize initialValue and max value of id.
	 *
	 * @param sdbName       name of sdb storage.
	 * @param initialValue  start value of id.
	 * @param maxValue      maximum value of id.
	 */
	SequentialIDFactory(const string& sdbName);

	virtual ~SequentialIDFactory();

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
		idFinder_.display();
		nameFinder_.display();
	}

protected:

	NameID minID_; ///< An minimum ID.
	NameID maxID_; ///< An maximum ID.
	NameID newID_; ///< An ID for new name.
	string sdbName_;

	IdFinder idFinder_; ///< an indexer which gives ids according to the name.
	NameFinder nameFinder_; ///< an inverted indexer which gives name according to the id.
}; // end - template SequentialIDFactory

template <typename NameString, typename NameID,
    NameID MinValueID, NameID MaxValueID, typename LockType>
SequentialIDFactory<NameString, NameID,
    MinValueID, MaxValueID, LockType>::SequentialIDFactory(
        const string& sdbName)
:
	minID_(MinValueID),
    maxID_(MaxValueID),
    newID_(MinValueID),
    sdbName_(sdbName),
    idFinder_(sdbName_ + "_name.sdb"),
    nameFinder_(sdbName_ + "_id.sdb")
{
	idFinder_.open();
	nameFinder_.open();
} // end - SequentialIDFactory()

template <typename NameString, typename NameID,
    NameID MinValueID, NameID MaxValueID, typename LockType>
SequentialIDFactory<NameString, NameID,
    MinValueID, MaxValueID, LockType>::~SequentialIDFactory()
{
} // end - ~SequentialIDFactory()

template <typename NameString, typename NameID,
    NameID MinValueID, NameID MaxValueID, typename LockType>
bool SequentialIDFactory<NameString, NameID,
    MinValueID, MaxValueID, LockType>::getNameIDByNameString(
        const NameString& nameString,
        NameID& nameID)
{
	// If name string is found, return the id.
	if (idFinder_.getValue(nameString, nameID) ) {
		return true;
	} // end - if

	// Because there's no name string in idFinder, create new id according to the string.
	nameID = newID_;
	newID_++;

	// check correctness of input nameID
	if (newID_> maxID_)
		throw IDFactoryException(SF1_ID_FACTORY_OUT_OF_BOUND, __LINE__, __FILE__);

	idFinder_.insertValue(nameString, nameID);
	nameFinder_.insertValue(nameID, nameString);

	return false;
} // end - getNameIDByNameString()

template <typename NameString, typename NameID,
    NameID MinValueID, NameID MaxValueID, typename LockType>
bool SequentialIDFactory< NameString, NameID,
    MinValueID, MaxValueID, LockType>::getNameStringByNameID(
        const NameID& nameID, NameString& nameString)
{
	return nameFinder_.getValue(nameID, nameString);
} // end - getNameStringByNameID()

}
// end - namespace idmanager

NS_IZENELIB_IR_END

#endif // #ifndef _ID_FACTORY_H_
