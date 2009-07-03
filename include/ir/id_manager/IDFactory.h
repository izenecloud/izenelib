/**-
 * @file	IDFactory.h
 * @brief	Header file of ID Factor Class 
 * @author	Quang & Do Hyun Yun
 * @date    2008-11-18
 * @details
 * ============== * 
 * 
 * Using SDB/hash
 *  
 * @author Peisheng Wang
 * @date 2009-04-16
 *  - Log
 */

#ifndef _ID_FACTORY_H_
#define _ID_FACTORY_H_

#include "IDManagerTypes.h"
#include "IDFactoryException.h"
#include "IDFactoryErrorString.h"

NS_IZENELIB_IR_BEGIN

namespace idmanager {

template <typename NameString, typename NameID,
		typename LockType =izenelib::util::NullLock > class IDFactory {
	typedef izenelib::sdb::unordered_sdb_1<NameString, NameID, LockType> IdFinder;
	typedef izenelib::sdb::unordered_sdb_1<NameID, NameString, LockType>	NameFinder;
public:

	/**
	 * @brief Constructor which initialize initialValue and max value of id.
	 *
	 * @param initialValue  start value of id.
	 * @param maxValue      maximum value of id.
	 */
	IDFactory(const string& sdbName, NameID& initialValue, NameID& maxValue);
	//IDFactory(NameID& initialValue, NameID& maxValue, char* sdbName);

	IDFactory(const IDFactory<NameString, NameID, LockType>& idFactory);

	/**
	 * @brief set the max and min value of name ID
	 * @param minIDValue the min value
	 * @param maxIDValue the max value
	 */
	void setIDRange(const NameID& minIDValue, const NameID& maxIDValue);

	virtual ~IDFactory();

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

	/**
	 * @brief This function inserts a paired value of (name string, name id) to
	 * the data. The function checks if input nameId is in range [minID_, maxID_].
	 * Then it calls _insertNameString to really insert data. The function also
	 * updates newID_ value to make sure that newID_ is always the biggest ever
	 * id value.
	 * @param nameString the value of name string
	 * @param nameID the id value
	 * 
	 * @return true if insert success, otherwise false
	 */
	bool insertNameString(const NameString& nameString, const NameID& nameID);

protected:

	NameID minID_; ///< An minimum ID. 
	NameID maxID_; ///< An maximum ID. 
	NameID newID_; ///< An ID for new name.
	string sdbName_;

	IdFinder idFinder_; ///< an indexer which gives ids according to the name.
	NameFinder nameFinder_; ///< an inverted indexer which gives name according to the id.	
}; // end - typename IDFactory


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////    Source Part
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

template <typename NameString, typename NameID, typename LockType> IDFactory<
		NameString, NameID, LockType>::IDFactory(const string& sdbName,
		NameID& initialValue, NameID& maxValue) :
	minID_(initialValue), maxID_(maxValue), newID_(initialValue),
			sdbName_(sdbName), idFinder_(sdbName_ + "_name.sdb"),
			nameFinder_(sdbName_ + "_id.sdb") {
	idFinder_.open();
	nameFinder_.open();
} // end - IDFactory()


template <typename NameString, typename NameID, typename LockType> IDFactory<
		NameString, NameID, LockType>::IDFactory(
		const IDFactory<NameString, NameID, LockType>& idFactory) :
	minID_(idFactory.minID_), maxID_(idFactory.maxID_),
			newID_(idFactory.newID_) {
}
template <typename NameString, typename NameID, typename LockType> IDFactory<
		NameString, NameID, LockType>::~IDFactory() {
} // end - IDFactory()


template <typename NameString, typename NameID, typename LockType> void IDFactory<
		NameString, NameID, LockType>::setIDRange(const NameID& minIDValue,
		const NameID& maxIDValue) {
	minID_ = minIDValue;
	maxID_ = maxIDValue;
}

template <typename NameString, typename NameID, typename LockType> bool IDFactory<
		NameString, NameID, LockType>::getNameIDByNameString(
		const NameString& nameString, NameID& nameID) {
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


template <typename NameString, typename NameID, typename LockType> bool IDFactory<
		NameString, NameID, LockType>::getNameIDListByNameStringList(
		const std::vector<NameString>& nameStringList,
		std::vector< NameID>& nameIDList) {

	bool ret;
	bool isAllIDFound = true;
	size_t sizeOfNameStringList = nameStringList.size();

	// clear nameIDList
	nameIDList.clear();
	nameIDList.resize(sizeOfNameStringList);

	for (size_t i = 0; i < sizeOfNameStringList; i++) {
		ret = getNameIDByNameString(nameStringList[i], nameIDList[i]);
		if (ret == false)
			isAllIDFound = false;
	} // end - for

	return isAllIDFound;

} // end - getNameIDListByNameStringList()


template <typename NameString, typename NameID, typename LockType> bool IDFactory<
		NameString, NameID, LockType>::getNameStringByNameID(
		const NameID& nameID, NameString& nameString) {
	return nameFinder_.getValue(nameID, nameString);

} // end - getNameStringByNameID()


template <typename NameString, typename NameID, typename LockType> bool IDFactory<
		NameString, NameID, LockType>::getNameStringListByNameIDList(
		const std::vector<NameID> &nameIDList,
		std::vector< NameString> &nameStringList) {

	bool ret;
	bool isAllNameFound = true;
	size_t sizeOfNameIDList = nameIDList.size();

	// clear nameStringList
	nameStringList.clear();
	nameStringList.resize(sizeOfNameIDList);

	for (size_t i = 0; i < sizeOfNameIDList; i++) {
		ret = getNameStringByNameID(nameIDList[i], nameStringList[i]);
		if (ret == false)
			isAllNameFound = false;
	} // end - for

	return isAllNameFound;

} // end - getNameStringListByNameIDList()


template <typename NameString, typename NameID, typename LockType> bool IDFactory<
		NameString, NameID, LockType>::insertNameString(
		const NameString& nameString, const NameID& nameID) {
	// check correctness of input nameID
	if (nameID> maxID_ || nameID < minID_)
		throw IDFactoryException(SF1_ID_FACTORY_OUT_OF_BOUND, __LINE__, __FILE__);
	return (idFinder_.insertValue(nameString, nameID)
			&& nameFinder_.insertValue(nameID, nameString) );

} // end - insertNameString()


}
// end - namespace idmanager

NS_IZENELIB_IR_END

#endif // #ifndef _ID_FACTORY_H_
