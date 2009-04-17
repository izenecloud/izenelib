/**
 * @file	TermIdManager.cpp
 * @brief	Source file of Term ID Manager Class 
 * @author	Do Hyun Yun
 * @date    2008-11-18
 * @details
 *  - Log
 * =======================================
 * 
 * Using SDB/hash
 *  
 * @author Peisheng Wang
 * @date 2009-04-16
 *  - Log 
 */

#include <TermIdManager.h>

namespace idmanager {

TermIdManager::TermIdManager(const string& sdbname, unsigned int initialValue, unsigned int maxValue):
	IDFactory<wiselib::UString, unsigned int>(sdbname, initialValue, maxValue)
{
	
} // end - TermIdManager()

TermIdManager::~TermIdManager()
{
} // end - ~TermIdManager()


bool TermIdManager::getTermIdByTermString(const wiselib::UString& termString, unsigned int& termId)
{
    // If given term string is not in the dictionary, insertion into File and starSearchInsertion is needed.
    // If the return value of getNameIdByNameString is false, termId contains new id of the dictionary.
    if(false == getNameIDByNameString(termString, termId) )
	{
        
    	// Write into startSearchIndexer
		boost::mutex::scoped_lock lock(termIndexerLock_);
		starSearchIndexer_.addWord(termString, termId);
		return false;
	}

	return true;
} // end - getTermIdByTermString()


	
bool TermIdManager::getTermStringByTermId(unsigned int termId, wiselib::UString& termString)
{
    return getNameStringByNameID( termId, termString );
} // end - getTermStringByTermId()

bool TermIdManager::getTermIdListByWildcardPattern(const wiselib::UString& wildcardPattern, 
				std::vector<unsigned int>& termIdList)
{
	boost::mutex::scoped_lock indexLock(termIndexerLock_);
	return starSearchIndexer_.matchRegExp(wildcardPattern, termIdList);
}

bool TermIdManager::getTermIdListByTermStringList(
        const std::vector<wiselib::UString>& termStringList, 
        std::vector<unsigned int>& termIdList)
{
	bool newTermAppear = true; // false if new term is added to the vocabulary

	termIdList.resize(termStringList.size());
	// process each string in the list
	for(size_t i = 0; i < termStringList.size(); i++)
	{
		if(false == getTermIdByTermString(termStringList[i], termIdList[i]))
			newTermAppear = false;
	}
	return newTermAppear;
} // end - getTermIdListByTermStringList()

bool TermIdManager::getTermStringListByTermIdList( const std::vector<unsigned int>& termIdList, 
        std::vector<wiselib::UString>& termStringList)
{
    return getNameStringListByNameIDList( termIdList, termStringList );
} // end - getTermStringListByTermIdList()


void TermIdManager::displaySDBList()
{
} // end - displaySDBList()

/*
void TermIdManager::addData(const wiselib::UString& termName, const unsigned int & termId)
{
    insertNameString(termName, termId);
    boost::mutex::scoped_lock lock(termIndexerLock_);
    starSearchIndexer_.addWord(termName, termId);
}*/


} // end - namespace sf1v5


