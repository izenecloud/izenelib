/**
 * @file	IDManager.h
 * @brief	Header file of ID Manager Class 
 * @author	Do Hyun Yun
 * @date    2008-06-05
 * @details
 * 
 *================
 * Using SDB/hash
 *  
 * @Peisheng Wang
 * @date 2009-04-16
 */

#ifndef _ID_MANAGER_
#define _ID_MANAGER_

#include "IDManagerTypes.h"
#include "IDFactory.h"
#include "CollectionIdManager.h"
#include "DocIdManager.h"
#include "TermIdManager.h"

/**
 * @brief a class to manage all kinds of operations about ID.
 *
 * @details 
 * IDManager controls many kinds of IDs: Document ID, term ID, 
 * or any other types of ID which are requested from specific manager. 
 * ID data or Key data can be easily taken out from IDManager when the 
 * matched Key or ID is given. If new key is inserted into the IDManager, 
 * IDManager generates new ID which doesn't have duplication in the 
 * vocabulary (storage) of IDManager. Different ID set is managed by its 
 * type information. It means different ID type is stored in different 
 * vocabulary storage. IDManager use UString class for the Key value.
 *
 *  - TODO List
 *      - There's multiple definition linking error while compiling test files. Currently I temporary set the option -Xlinker -zmuldefs to fix them.
 */
namespace idmanager {

#define MAJOR_VERSION "1"
#define MINOR_VERSION "0"
#define PATCH_VERSION "20081203"


template<typename NameString=wiselib::UString, typename NameID=unsigned int> class _IDManager {
public:
	_IDManager(const string& sdbname = "idm") :
		termIdManager_(sdbname + "_tid"), docIdManager_(sdbname + "_did"),
				collectionIdManager_(sdbname + "_cid") {
		version_ = "ID Manager - ver. alpha ";
		version_ += MAJOR_VERSION;
		version_ += ".";
		version_ += MINOR_VERSION;
		version_ += ".";
		version_ += PATCH_VERSION;
	}

	~_IDManager();

	/**
	 * @brief a member function to get term ID from vocabulary which matches to the given term string.
	 *
	 * @param termString	a UString object which contains term string.
	 * @param termId	    a term identifier which matches to the term string.
	 * @return true  : 	Term exists in the dictionary.
	 * @return false : 	Term does not exist in the dictionary.
	 */
	bool getTermIdByTermString(const NameString& termString, NameID& termId);

	/**
	 * @brief a member function to get a set of term ID list which matches to the given term strings respectively. 
	 *
	 * @param termStringList	a list of term strings.
	 * @param termIdList	    a list of term IDs.
	 * @return true  :		One or more terms exist in the dictionary.
	 * @return false :		No term exists in the dictionary.
	 */
	bool getTermIdListByTermStringList(
			const std::vector<NameString>& termStringList,
			std::vector<NameID>& termIdList);

	/**
	 * @brief a member function to offer a result set of wildcard search with WildcardSearchManager.
	 *
	 * @param wildcardPattern   a UString of wildcard pattern which contains '*';
	 * @param termIdList        a list of term IDs which is the result of WildcardSearchManager.
	 * @return true  :          Given wildcard pattern is matched at least once in the dictionary.
	 * @return false :          Given wildcard pattern is not matched in the dictionary. 
	 */
	bool getTermIdListByWildcardPattern(const NameString& wildcardPattern,
			std::vector<NameID>& termIdList);

	/**
	 * @brief a member function to get term string by its ID. 
	 *
	 * @param termId	    a term identifier for the input.
	 * @param termString	a UString object which contains term string.
	 * @return true  :  Given term string exists in the dictionary.
	 * @return false :  Given term string does not exist in the dictionary.
	 */
	bool getTermStringByTermId(NameID termId, NameString& termString);

	/**
	 * @brief a member function to term string list by a set of term IDs.
	 *
	 * @param termIdList	    a list of term IDs.
	 * @param termStringList	a list of term strings.
	 * @return true  :      At least one term in the given list is matched in the dictionary.
	 * @return false :      No term is matched in the dictionary. 
	 */
	bool getTermStringListByTermIdList(const std::vector<NameID>& termIdList,
			std::vector<NameString>& termStringList);

	/**
	 * @brief a member function to get document ID from the vocabulary which matches to the given document name. 
	 *
	 * @param collectionId	a collection identifier which includes the document inside.
	 * @param docName		a unique string of the document which is used to distinguish between documents.
	 * @param docId    	    a document identifier which matches to the document name.
	 * @return true  : 	    Document name exists in the dictionary.
	 * @return false : 	    Document name does not exist in the dictionary.
	 */
	bool getDocIdByDocName(NameID collectionId, const NameString& docName,
			NameID& docId);

	/**
	 * @brief a member function to get a name of the document by its ID. 
	 *
	 * @param collectionId	a collection identifier which includes the document inside.
	 * @param docId		    a document identifier.
	 * @param docName	   	a unique string of the document which matches to the document ID.
	 * @return true  : 	    Given document name exists in the dictionary.
	 * @return false : 	    Given document name does not exist in the dictionary.
	 */
	bool getDocNameByDocId(NameID collectionId, NameID docId,
			NameString& docName);

	/**
	 * @brief a member function to get a collection ID which is matched to the given collection name.
	 *
	 * @param collectionName	a unique string of collection which is used to distinguish between collections.
	 * @param collectionId		a collection identifier for the output.
	 * @return true  : 	        Collection name exists in the dictionary.
	 * @return false : 	        Collection name does not exist in the dictionary.
	 */
	bool getCollectionIdByCollectionName(const NameString& collectionName,
			NameID& collectionId);

	/**
	 * @brief a member function to get collection name by its ID 
	 *
	 * @param collectionId		a collection identifier for the input.
	 * @param collectionName	a unique string of collection which matches to the collection ID.
	 * @return true  : 	        Given collection name exists in the dictionary.
	 * @return false : 	        Given collection name does not exist in the dictionary.
	 */
	bool getCollectionNameByCollectionId(NameID collectionId,
			NameString& collectionName);

	/** 
	 * @brief retrieve version string of id-manager
	 * @return version string of id-manager
	 */
	const std::string& getVersionString() const {
		return version_;
	}

private:
	TermIdManager<NameString, NameID> termIdManager_; ///< Term Id Manager Class
	DocIdManager<NameString, NameID> docIdManager_; ///< Document Id Manager Class
	CollectionIdManager<NameString, NameID> collectionIdManager_; ///< Collection Id Manager Class
	std::string version_; ///< version of id-manager

}; // end - class _IDManager


/*****************************************************************************
 *                                                     Term Related Interfaces
 *****************************************************************************/


template<typename NameString, typename NameID> _IDManager<NameString, NameID>::~_IDManager() {

} // end - ~_IDManager()

/*****************************************************************************
 *                                                     Term Related Interfaces
 *****************************************************************************/

template<typename NameString, typename NameID> bool _IDManager<NameString,
	NameID>::getTermIdByTermString(const NameString& termString, NameID& termId) {
return termIdManager_.getTermIdByTermString(termString, termId);
} // end - getTermIdByTermString() 

template<typename NameString, typename NameID> bool _IDManager<NameString,
	NameID>::getTermIdListByTermStringList(
	const std::vector<NameString>& termStringList, std::vector<NameID>& termIdList) {
return termIdManager_.getTermIdListByTermStringList(termStringList, termIdList);
} // end - getTermIdListByTermStringList()

template<typename NameString, typename NameID> bool _IDManager<NameString,
	NameID>::getTermIdListByWildcardPattern(const NameString& wildcardString,
	std::vector<NameID>& termIdList) {
return termIdManager_.getTermIdListByWildcardPattern(wildcardString, termIdList);
} // end - getTermIdListByWildcardString()

template<typename NameString, typename NameID> bool _IDManager<NameString,
	NameID>::getTermStringByTermId(NameID termId, NameString& termString) {
return termIdManager_.getTermStringByTermId(termId, termString);
} // end - getTermStringByTermId()

template<typename NameString, typename NameID> bool _IDManager<NameString,
	NameID>::getTermStringListByTermIdList(
	const std::vector<NameID>& termIdList, std::vector<NameString>& termStringList) {
return termIdManager_.getTermStringListByTermIdList(termIdList, termStringList);
} // end - getTermStringListByTermIdList()


/*****************************************************************************
 *                                                 Document Related Interfaces
 *****************************************************************************/
template<typename NameString, typename NameID> bool _IDManager<NameString,
	NameID>::getDocIdByDocName(NameID collectionId, const NameString& docName,
	NameID& docId) {
return docIdManager_.getDocIdByDocName(collectionId, docName, docId);
} // end - getDocIdByDocName()

template<typename NameString, typename NameID> bool _IDManager<NameString,
	NameID>::getDocNameByDocId(NameID collectionId, NameID docId,
	NameString& docName) {
return docIdManager_.getDocNameByDocId(collectionId, docId, docName);
} // end - getDocNameByDocId()


/*****************************************************************************
 *                                               Collection Related Interfaces
 *****************************************************************************/

template<typename NameString, typename NameID> bool _IDManager<NameString,
	NameID>::getCollectionIdByCollectionName(const NameString& collectionName,
	NameID& collectionId) {
return collectionIdManager_.getCollectionIdByCollectionName(collectionName,
		collectionId);
} // end - getCollectionIdByCollectionName()

template<typename NameString, typename NameID> bool _IDManager<NameString,
	NameID>::getCollectionNameByCollectionId(NameID collectionId,
	NameString& collectionName) {
return collectionIdManager_.getCollectionNameByCollectionId(collectionId,
		collectionName);
} // end - getCollectionNameByCollectionId()

typedef _IDManager<> IDManager;

} // end - namespace sf1v5

#endif // _ID_MANAGER_
