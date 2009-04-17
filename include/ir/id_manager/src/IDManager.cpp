/**
 * @file	IDManager.cpp
 * @brief	Source file of ID Manager Class 
 * @author	Do Hyun Yun
 * @date 	2008-06-05 : Created and add simple contents
 *          2008-07-04 : Changed return value of interfaces
 *          2008-08-07 : Changed string -> UString
 * =======================================
 * 
 * Using SDB/hash
 *  
 * @author Peisheng Wang
 * @date 2009-04-16
 *  - Log
 */

#include <IDManager.h>
#include <IDManagerErrorString.h>
#include <IDManagerException.h>

namespace idmanager {

/*****************************************************************************
 *                                                     Term Related Interfaces
 *****************************************************************************/

#define MAJOR_VERSION "1"
#define MINOR_VERSION "0"
#define PATCH_VERSION "20081203"

IDManager::IDManager(const string& sdbname):
	termIdManager_(sdbname + "_tid"),
	docIdManager_(sdbname + "_did"),
	collectionIdManager_(sdbname + "_cid")	
{
	version_ = "ID Manager - ver. alpha ";
	version_ += MAJOR_VERSION;
	version_ += ".";
	version_ += MINOR_VERSION;
	version_ += ".";
	version_ += PATCH_VERSION;

	// load from default file
	// check if file exists
	/*
	 if(boost::filesystem::exists(ID_MANAGER_DEFAULT_DATA_FILE))
	 {
	 std::cout << "Load Default file : " << ID_MANAGER_DEFAULT_DATA_FILE << std::endl;
	 boost::filesystem::remove(ID_MANAGER_DEFAULT_DATA_BACKUP_FILE);
	 boost::filesystem::rename(ID_MANAGER_DEFAULT_DATA_FILE,ID_MANAGER_DEFAULT_DATA_BACKUP_FILE);
	 read(ID_MANAGER_DEFAULT_DATA_BACKUP_FILE);
	 boost::filesystem::remove(ID_MANAGER_DEFAULT_DATA_BACKUP_FILE);
	 }
	 */
} // end - IDManager()

IDManager::~IDManager() {

} // end - ~IDManager()

/*****************************************************************************
 *                                                     Term Related Interfaces
 *****************************************************************************/

bool IDManager::getTermIdByTermString(const wiselib::UString& termString,
		unsigned int& termId) {
	return termIdManager_.getTermIdByTermString(termString, termId);
} // end - getTermIdByTermString() 

bool IDManager::getTermIdListByTermStringList(
		const std::vector<wiselib::UString>& termStringList,
		std::vector<unsigned int>& termIdList) {
	return termIdManager_.getTermIdListByTermStringList(termStringList,
			termIdList);
} // end - getTermIdListByTermStringList()

bool IDManager::getTermIdListByWildcardPattern(
		const wiselib::UString& wildcardString,
		std::vector<unsigned int>& termIdList) {
	return termIdManager_.getTermIdListByWildcardPattern(wildcardString,
			termIdList);
} // end - getTermIdListByWildcardString()

bool IDManager::getTermStringByTermId(unsigned int termId,
		wiselib::UString& termString) {
	return termIdManager_.getTermStringByTermId(termId, termString);
} // end - getTermStringByTermId()

bool IDManager::getTermStringListByTermIdList(
		const std::vector<unsigned int>& termIdList,
		std::vector<wiselib::UString>& termStringList) {
	return termIdManager_.getTermStringListByTermIdList(termIdList,
			termStringList);
} // end - getTermStringListByTermIdList()


/*****************************************************************************
 *                                                 Document Related Interfaces
 *****************************************************************************/

bool IDManager::getDocIdByDocName(unsigned int collectionId,
		const wiselib::UString& docName, unsigned int& docId) {
	return docIdManager_.getDocIdByDocName(collectionId, docName, docId);
} // end - getDocIdByDocName()

bool IDManager::getDocNameByDocId(unsigned int collectionId,
		unsigned int docId, wiselib::UString& docName) {
	return docIdManager_.getDocNameByDocId(collectionId, docId, docName);
} // end - getDocNameByDocId()


/*****************************************************************************
 *                                               Collection Related Interfaces
 *****************************************************************************/

bool IDManager::getCollectionIdByCollectionName(
		const wiselib::UString& collectionName, unsigned int& collectionId) {
	return collectionIdManager_.getCollectionIdByCollectionName(collectionName,
			collectionId);
} // end - getCollectionIdByCollectionName()

bool IDManager::getCollectionNameByCollectionId(unsigned int collectionId,
		wiselib::UString& collectionName) {
	return collectionIdManager_.getCollectionNameByCollectionId(collectionId,
			collectionName);
} // end - getCollectionNameByCollectionId()


} // end - namespace sf1v5


