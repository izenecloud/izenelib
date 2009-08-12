/**
 * w
 * @file    t_DocIdManager.cpp
 * @brief   A Test unit of DocIdManager.
 * @author  Do Hyun Yun
 * @date    2008-08-16
 * @details
 *
 *  ==================================== [ Test Schemes ] ====================================
 *
 *  -# Test Case 1 : Simple DocIdManager check.
 *      - Using number 1 as a collection id, insert one document name by Using
 *        getDocIdByDocName(). Check if the return value is false. It means the given term
 *        name does not exist in the indexer.
 *      - Check if the given document name is correctly inserted into the docIndexer by using
 *        getDocNameByDocId() interface.
 *\n
 *  -# Test Case 2 : Insert 100 collection names by using collectionIdManager and insert 235
 *                   documents by using collectionIdManager & docIdManager. Check if the
 *                   insertion is correct.
 *      - Insert 100 collection names into collectionIdManager. While inserting, get the
 *        collection ids for each name.
 *      - Using collection Ids, insert 235 documents for each collection.
 *      - Check all the terms are correctly inserted into collectionIdManager and docIdManager.
 *\n
 *  -# Test Case 3 : Recovery Test. It checks if DocIdManager can reload the previous index data when it restarts,
 *      - Use the index data from test case 2 (Test case 2 remove index file of previous test).
 *      - Check if the doc id and doc name is equal to the ones of test case 2.\n\n
 */
#include <fstream>
#include <algorithm>

#include <boost/test/unit_test.hpp>

#include <ir/id_manager/DocIdManager.h>
#include <ir/id_manager/SequentialIDFactory.h>
#include <wiselib/ustring/UString.h>

using namespace std;
using namespace wiselib;
using namespace boost::unit_test;
using namespace izenelib::ir::idmanager;

/**
 * @brief DocIdManagerFixture is used to test DocIdManager. It loads collection and document names from text file to build termStringLists.
 */
class DocIdManagerFixture
{
public:

    vector<UString> termUStringList1_; ///< The list of 100 term string. It is used as a collection name.

    vector<unsigned int> termIdList1_; ///< The list of 100 term id. It is used as a collection id.

    vector<UString> termUStringList2_; ///< The list of 2400 term string. It is used as a document name.

    vector<unsigned int> termIdList2_; ///< The list of 2400 term id. It is used as a document id.


    DocIdManagerFixture()
    {
        // generate two term lists
		BOOST_CHECK_EQUAL(generateTermLists(termUStringList1_, termIdList1_, termUStringList2_, termIdList2_), true);

    } // end - DocIdManagerFixture()

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

}; // end - class DocIdManagerFixture

/**********************************************************
 *
 *          Start point of t_DocIdManager suite - 1
 *
 **********************************************************/

BOOST_FIXTURE_TEST_SUITE( t_DocIdManager, DocIdManagerFixture )

/**
 * @brief Test Case 1 : Simple DocIdManager check.
 * @details
 *  -# Using number 1 as a collection id, insert one document name by Using
 *     getDocIdByDocName(). Check if the return value is false. It means the given term
 *     name does not exist in the indexer.
 *\n
 *  -# Check if the given document name is correctly inserted into the docIndexer by using
 *     getDocNameByDocId() interface.
 */

BOOST_AUTO_TEST_CASE( TestCase1 )
{
    // remove previous index file
    // remove(DocIdManager::DOC_ID_MANAGER_INDEX_FILE.c_str());
    // remove(CollectionIdManager<UString, unsigned int>::COLLECTION_ID_MANAGER_INDEX_FILE.c_str());

    cerr << endl;
    cerr << "[ DocIdManager ] Test Case 1 : Simple Document Id Manager check ..............";

    DocIdManager<UString, unsigned int,
        SequentialIDFactory<UString, unsigned int> > docIdManager("docid1");

    string insertString("Test DocIdManager");
    UString insertUString(insertString, UString::CP949);
    UString resultUString;

    unsigned int id = 0;

    // Insert 1 terms into document id manager by using getDocIdByDocName() interface.
    // While inserting, check if the return value is false.
    BOOST_CHECK_EQUAL( docIdManager.getDocIdByDocName(insertUString, id ) , false );
    // Check if the ustring is correctly inserted using getDocNameByDocId() interface.
    BOOST_CHECK_EQUAL( docIdManager.getDocNameByDocId(id, resultUString ) , true );
    // Check again if the id is found in the document indexer using getDocIdByDocName() interface.
    BOOST_CHECK_EQUAL( docIdManager.getDocIdByDocName(insertUString, id ) , true );

    // Check if the resultUString is the same as insertUString.
    BOOST_CHECK ( insertUString == resultUString );

    cerr << "OK" << endl;


} // end - BOOST_AUTO_TEST_CASE( TestCase1 )



BOOST_AUTO_TEST_CASE( TestCase2 )
{
    // remove previous index file
    // remove(DocIdManager::DOC_ID_MANAGER_INDEX_FILE.c_str());
    // remove(CollectionIdManager<UString, unsigned int>::COLLECTION_ID_MANAGER_INDEX_FILE.c_str());

    cerr << endl;
    cerr << "[ DocIdManager ] Test Case 2 : check ID generation continuity ..............";

    unsigned int lastID;

    {
        DocIdManager<UString, unsigned int,
            SequentialIDFactory<UString, unsigned int> > docIdManager("docid2");

        unsigned int id = 0;
        docIdManager.getDocIdByDocName(UString("term1", UString::CP949), id);

        lastID = id;
    }

    {
        DocIdManager<UString, unsigned int,
            SequentialIDFactory<UString, unsigned int> > docIdManager("docid2");

        unsigned int id = 0;
        docIdManager.getDocIdByDocName(UString("term1", UString::CP949), id);
        BOOST_CHECK(id == lastID);

        docIdManager.getDocIdByDocName(UString("term2", UString::CP949), id);
        BOOST_CHECK(id == lastID+1);
    }

    cerr << "OK" << endl;


} // end - BOOST_AUTO_TEST_CASE( TestCase1 )




BOOST_AUTO_TEST_SUITE_END()
