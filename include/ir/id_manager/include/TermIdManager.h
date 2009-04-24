/**
 * @file	TermIdManager.h
 * @brief	Header file of Term ID Manager Class 
 * @author	Do Hyun Yun
 * @date    2008-11-18
 * @details
 * ============== * 
 * 
 * Using SDB/hash
 *  
 * @Peisheng Wang
 * @date 2009-04-16
 * 
 * log
 */

#ifndef _TERM_ID_MANAGER_ 
#define _TERM_ID_MANAGER_

#include "IDFactory.h"
#include "LexicalTrie.h"

/**
 * @brief a class to generate, serve, and manage all about of the term id. 
 */

namespace idmanager {


template<typename NameString, typename NameID,
		typename TRIE = LexicalTrie<NameString> > class TermIdManager :
	protected IDFactory<NameString, NameID> {

public:

	/**
	 * @brief Constructor of TermIdManager having term ids within a given range
	 * @param initialTermIdValue the initial value of term id
	 * @param maxTermIdValue the maximum value of term id
	 */
	TermIdManager(const string& sdbname = "termid_manager",
			NameID initialTermIdValue = 1, NameID maxTermIdValue = -2);

	/**
	 * @brief A Destructor.
	 *
	 * @details
	 */
	~TermIdManager();

public:

	/**
	 * @brief a member function to offer a termID which exists in the dictionary.
	 * If it is not in the dictionary, termIdManager will generate new term id and automatically inserts into dictionary.
	 *
	 * @param termString	a term string which is used to find the term ID.
	 * @param termId        a term identifier which is the matched id value of termString.
	 * @return true     : The term ID is in dictionary.
	 * @return false    : There is no matched term ID in dictionary. New term Id generation and Insertion processes are done.
	 */
	bool getTermIdByTermString(const NameString& termString, NameID& termId);

	/**
	 * @brief a member function to offer a term string according to the ID.
	 *
	 * @param termId	    a term identifier which is used to get term string.
	 * @param termString	a term string for the output.
	 * @return true  :  Given id exists in the dictionary.
	 * @return false :	Given id does not exist in the dictionary.
	 */
	bool getTermStringByTermId(NameID termId, NameString& termString);

	/**
	 * TODO : NYI (not yet implemented)
	 * @brief a member function to offer a result set of wildcard search with WildcardSearchManager.
	 *
	 * @param wildcardPattern   a string of wildcard pattern which contains '*';
	 * @param termIdList        a list of term IDs which is the result of WildcardSearchManager.
	 * @return true  :          Given wildcard pattern is matched at least once in the dictionary.
	 * @return false :          Given wildcard pattern is not matched in the dictionary. 
	 */
	bool getTermIdListByWildcardPattern(const NameString& wildcardPattern,
			std::vector<NameID>& termIdList);

	/**
	 * @brief a member function to offer a set of search result of term id list. If one or more term strings are not matched in the dictionary, 0 will be contained for each unmatched termIdList.
	 *
	 * @param termStringList    a string list 
	 * @param termIdList        a list of term IDs which is the result of searching
	 * @return true  :          all the term strings in given list are matched in the dictionary.
	 * @return false :          one or more term strings are not matched in the dictionary. 
	 */
	bool getTermIdListByTermStringList(
			const std::vector<NameString>& termStringList,
			std::vector<NameID>& termIdList);

	/**
	 * @brief a memeber function to offer a list of term string by term id list. If one or more ids are not matched in the dictionary, 0 will be contained for each unmatched termIdList.
	 *
	 * @param termIdList        a list of term Ids which indicates the identifier.
	 * @param termStringList    a list of term string which is the result of searching
	 *
	 * @return true     :       all the term ids are matched in the dictionary.
	 * @return false    :       one or more term ids are not matched in the dictionary.
	 */
	bool getTermStringListByTermIdList(const std::vector<NameID>& termIdList,
			std::vector<NameString>& termStringList);

	/**
	 * @brief a member function to display all the contents of the sequential db. this function is used for debugging. 
	 */
	void displaySDBList();

	//private:

	//friend class IDManager<NameString, NameID>;

private:

	/**
	 * @brief This thread is used only for updating star search index
	 */
	boost::thread* starSearchUpdateThread_;

	///**
	// * @brief This variable signale a event when there is a new term
	// */
	//boost::condition_variable newTermEvent_;


	/**
	 * @brief This lock allows exclusive access to termsQueueForStarSearchIndex_
	 */
	boost::mutex termIndexerLock_;

	/**
	 * @brief star search indexer
	 */

	TRIE starSearchIndexer_;

}; // end - class TermIdManager 

template<typename NameString, typename NameID, typename TRIE> TermIdManager<
		NameString, NameID, TRIE>::TermIdManager(const string& sdbname,
		NameID initialValue, NameID maxValue) :
	IDFactory<NameString, NameID>(sdbname, initialValue, maxValue) {

} // end - TermIdManager()

template<typename NameString, typename NameID, typename TRIE> TermIdManager<
		NameString, NameID, TRIE>::~TermIdManager() {
} // end - ~TermIdManager()


template<typename NameString, typename NameID, typename TRIE>bool TermIdManager<
		NameString, NameID, TRIE>::getTermIdByTermString(
		const NameString& termString, NameID& termId) {
	// If given term string is not in the dictionary, insertion into File and starSearchInsertion is needed.
	// If the return value of getNameIdByNameString is false, termId contains new id of the dictionary.
	if (false == getNameIDByNameString(termString, termId) ) {

		// Write into startSearchIndexer
		boost::mutex::scoped_lock lock(termIndexerLock_);
		starSearchIndexer_.addWord(termString, termId);
		return false;
	}

	return true;
} // end - getTermIdByTermString()


template<typename NameString, typename NameID, typename TRIE> bool TermIdManager<
		NameString, NameID, TRIE>::getTermStringByTermId(NameID termId,
		NameString& termString) {
	return getNameStringByNameID(termId, termString);
} // end - getTermStringByTermId()


template<typename NameString, typename NameID, typename TRIE>bool TermIdManager<
		NameString, NameID, TRIE>::getTermIdListByWildcardPattern(
		const NameString& wildcardPattern, std::vector<NameID>& termIdList) {
	boost::mutex::scoped_lock indexLock(termIndexerLock_);
	return starSearchIndexer_.matchRegExp(wildcardPattern, termIdList);

}

template<typename NameString, typename NameID, typename TRIE>bool TermIdManager<
		NameString, NameID, TRIE>::getTermIdListByTermStringList(
		const std::vector<NameString>& termStringList,
		std::vector<NameID>& termIdList) {
	bool newTermAppear = true; // false if new term is added to the vocabulary

	termIdList.resize(termStringList.size());
	// process each string in the list
	for (size_t i = 0; i < termStringList.size(); i++) {
		if (false == getTermIdByTermString(termStringList[i], termIdList[i]))
			newTermAppear = false;
	}
	return newTermAppear;
} // end - getTermIdListByTermStringList()

template<typename NameString, typename NameID, typename TRIE>bool TermIdManager<
		NameString, NameID, TRIE>::getTermStringListByTermIdList(
		const std::vector<NameID>& termIdList,
		std::vector<NameString>& termStringList) {
	return getNameStringListByNameIDList(termIdList, termStringList);
} // end - getTermStringListByTermIdList()


template<typename NameString, typename NameID, typename TRIE> void TermIdManager<
		NameString, NameID, TRIE>::displaySDBList() {
} // end - displaySDBList()


}
// end - namespace sf1v5

#endif // _TERM_ID_MANAGER_

