#ifndef _IDMANAGER_TEST_UTIL_
#define _IDMANAGER_TEST_UTIL_

#include <fstream>
#include <algorithm>
#include <util/ustring/UString.h>
#include <ir/id_manager/IDManager.h>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

using namespace izenelib::util;
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
        BOOST_CHECK_EQUAL(generateTermLists(termUStringList1_, termUStringList2_), true);

    } // end - TermIdManagerFixture()


    void clean(const string & prefix)
    {
        directory_iterator end_itr; // default construction yields past-the-end
        for ( directory_iterator itr("."); itr != end_itr; ++itr )
        {
            if(path(*itr).filename().string().compare(0, prefix.length(), prefix)  == 0)
                remove_all(itr->path());
        }
    }

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
    bool generateTermLists(vector<UString>& termUStringList1,
        vector<UString>& termUStringList2)
    {
        vector<string> termStringList1;
        vector<string> termStringList2;

        // load term list
        if(!loadTermList("./test-data/100WordList.txt", termStringList1))
            return true;
        if(!loadTermList("./test-data/2400WordList.txt", termStringList2))
            return true;

        // convert term list of strings into term list of UStrings
        termUStringList1.resize(termStringList1.size());
        for(size_t i = 0; i < termStringList1.size(); i++)
            termUStringList1[i].assign(termStringList1[i], UString::CP949);

        termUStringList2.resize(termStringList2.size());
        for(size_t i = 0; i < termStringList2.size(); i++)
            termUStringList2[i].assign(termStringList2[i], UString::CP949);

        return true;

    } // end - generateTermLists()

}; // end - class IDManagerFixture


/**
 * @brief Check function which ckecks if all the ids are corretly generated.
 */

template<typename IDManagerType>
inline bool isIdListCorrect(const vector<unsigned int>& termIdList,const vector<UString>& termStringList, IDManagerType& idManager)
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
