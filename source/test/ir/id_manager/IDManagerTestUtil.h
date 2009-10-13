#ifndef _IDMANAGER_TEST_UTIL_
#define _IDMANAGER_TEST_UTIL_

#include <fstream>
#include <algorithm>
#include <wiselib/ustring/UString.h>
#include <ir/id_manager/IDManager.h>

using namespace std;
using namespace wiselib;
using namespace izenelib::ir::idmanager;

/**
 * @brief IDManagerFixture is used to test IDManager. It loads terms from text file to build termStringLists.
 */
class IDManagerFixture
{
public:

    vector<UString> termUStringList1_; ///< The list of 100 term string.

    vector<unsigned int> termIdList1_; ///< The list of 100 term id.

    vector<UString> termUStringList2_; ///< The list of 2400 term string.

    vector<unsigned int> termIdList2_; ///< The list of 2400 term id.


    IDManagerFixture()
    {
        // generate two term lists
		BOOST_CHECK_EQUAL(generateTermLists(termUStringList1_, termIdList1_, termUStringList2_, termIdList2_), true);

    } // end - TermIdManagerFixture()

    /**
 	 * @brief This function loads a file that is a list of terms into a memory. This function is implementeded by TuanQuang Nguyen.
 	 * @param
 	 *     filePath - the file path that includes the file location and file name
 	 * @param
 	 *     termList - the output list of terms
 	 * @return true if the file is successfully loaded
 	 */
    bool loadTermList(const string& filePath, vector<string>& termList)

	{
    	char line[1024];
    	ifstream dictionaryFile(filePath.data());

    	if(!dictionaryFile.good() || !dictionaryFile.is_open())
        	return false;

    	termList.clear();
    	memset(line, 0, 1024);
    	dictionaryFile.getline(line, 1024);
    	while( !dictionaryFile.eof())
    	{
        	for(size_t i = 0; i < strlen(line); i++)
            	if(line[i] == ' ')
                	line[i] = 0;
        	// add word to the list
        	termList.push_back(line);

        	// read next line
        	memset(line, 0, 1024);
        	dictionaryFile.getline(line, 1024);
    	}
    	dictionaryFile.close();

    	return true;
	} // end - loadTermList

    /**
	 * @brief This function calls loadTermList to load two lists of terms. The function also
	 * generates id for each term in the lists. This function is implemented by TuanQuang Nguyen.
	 * @param
	 * 	termUStringList1 - list 1 of the term string
	 * @param
	 * 	termIdList1 - list 1 of term ids
	 * @param
	 * 	termUStringList2 - list 2 of the term string
	 * @param
	 * 	termIdList2 - list 2 of term ids
	 */
	bool generateTermLists(vector<UString>& termUStringList1, vector<unsigned int>& termIdList1,
            vector<UString>& termUStringList2, vector<unsigned int>& termIdList2)
    {
        vector<string> termStringList1;
        vector<string> termStringList2;
        unsigned int termId, i;

        // load term list
        if(!loadTermList("./test-data/100WordList.txt", termStringList1) ||
                !loadTermList("./test-data/2400WordList.txt", termStringList2))
            return false;


        // convert term list of strings into term list of UStrings
        termUStringList1.resize(termStringList1.size());
        for(i = 0; i < termStringList1.size(); i++)
            termUStringList1[i].assign(termStringList1[i], UString::CP949);

        termUStringList2.resize(termStringList2.size());
        for(i = 0; i < termStringList2.size(); i++)
            termUStringList2[i].assign(termStringList2[i], UString::CP949);



        // generate termId  for each term in the lists
        // generate termId  for each term in the list1
        termIdList1.clear();
        termIdList1.resize(termStringList1.size());
        termId = 0;
        for(i = 0; i < termUStringList1.size(); i++)
        {
            termIdList1[i] = termId++;
        }

        // generate termId  for each term in the list2
        termIdList2.clear();
        termIdList2.resize(termStringList2.size());
        for(i = 0; i < termUStringList2.size(); i++)
        {
            termIdList2[i] = termId++;
        }

        return true;

    } // end - generateTermLists()

}; // end - class IDManagerFixture


/**
 * @brief Check function which ckecks if all the ids are corretly generated.
 */

inline bool isIdListCorrect(const vector<unsigned int>& termIdList,const vector<UString>& termStringList, IDManager& idManager)
{
    UString compare;

    // Check if the id is correctly generated using getTermStringByTermId().
    for(unsigned int i = 0; i < termIdList.size(); i++)
    {

        // Check if the id is in the termIdManager with getting termstring according to the id.
        if ( idManager.getTermStringByTermId(termIdList[i], compare) == false )
            return false;

        // Check if the term string from termIdManager is the same as the original one.
        if ( termStringList[i] != compare )
            return false;

    } // end - for

    return true;

} // end - isIdListCorrect()

#endif