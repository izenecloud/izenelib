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

#include <CollectionIdManager.h>
#include <DocIdManager.h>

using namespace std;
using namespace wiselib;
using namespace boost::unit_test;
using namespace idmanager;

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


/**
 * @brief Check function which checks if all the collection ids are corretly generated.
 */

inline bool isCollectionIdListCorrect(const vector<unsigned int>& collectionIdList,const vector<UString>& collectionNameList, CollectionIdManager& collectionIdManager)
{
    UString compare;
    
    // Check if the id is correctly generated using getCollectionNameByCollectionId().
    for(unsigned int i = 0; i < collectionIdList.size(); i++) 
    {

        // Check if the id is in the CollectionIdManager with getting collection name according to the id.
        if ( collectionIdManager.getCollectionNameByCollectionId(collectionIdList[i], compare) == false )
            return false;

        // Check if the collection name from collectionIdManager is the same as the original one.
        if ( collectionNameList[i] != compare ) 
            return false;

    } // end - for

    return true;

} // end - isIdListCorrect()

/**
 * @brief Check function which checks if all the doc ids are corretly generated.
 */

inline bool isDocIdListCorrect(unsigned int collectionIdNum, unsigned int docIdNum, const vector<UString>& docNameList, DocIdManager& docIdManager)
{
    UString compare;

    for(unsigned int i = 1; i <= collectionIdNum; i++)
    {
        for(unsigned int j = 1; j <= docIdNum; j++)
        {
            // Check if the id is in the DocIdManager with getting document name according to the id.
            if ( docIdManager.getDocNameByDocId(i, j, compare) == false )
                return false;

            // Check if the document name from docIdManager is the same as the original one.
            if ( docNameList[ (i - 1)*235 + (j - 1)] != compare ) 
                return false;
        } // end - for
        
    } // end - for

    return true;

} // end - isIdListCorrect()




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
    // remove(CollectionIdManager::COLLECTION_ID_MANAGER_INDEX_FILE.c_str());

    cerr << endl;
    cerr << "[ DocIdManager ] Test Case 1 : Simple Document Id Manager check .............."; 
    
    DocIdManager docIdManager("docid1");

    string insertString("Test DocIdManager");
    UString insertUString(insertString, UString::CP949);
    UString resultUString;

    unsigned int collectionId = 1; // Collection id 1 is used for inserting one document name.

    unsigned int id = 0;

    // Insert 1 terms into document id manager by using getDocIdByDocName() interface.
    // While inserting, check if the return value is false.
    BOOST_CHECK_EQUAL( docIdManager.getDocIdByDocName(collectionId, insertUString, id ) , false );   
    // Check if the ustring is correctly inserted using getDocNameByDocId() interface.
    BOOST_CHECK_EQUAL( docIdManager.getDocNameByDocId(collectionId, id, resultUString ) , true );
    // Check again if the id is found in the document indexer using getDocIdByDocName() interface.
    BOOST_CHECK_EQUAL( docIdManager.getDocIdByDocName(collectionId, insertUString, id ) , true );  

    // Check if the resultUString is the same as insertUString.
    BOOST_CHECK ( insertUString == resultUString );

    cerr << "OK" << endl;


} // end - BOOST_AUTO_TEST_CASE( TestCase1 )

/**
 * @brief Test Case 2 : Insert 100 collection names by using collectionIdManager and insert 235 
 *                      documents by using collectionIdManager & docIdManager. Check if the 
 *                      insertion is correct.
 * @details
 *
 *  -# Insert 100 collection names into collectionIdManager. While inserting, get the 
 *     collection ids for each name.
 *  -# Using collection Ids, insert 235 documents for each 10 collections.
 *  -# Check all the terms are correctly inserted into collectionIdManager and docIdManager.
 */

BOOST_AUTO_TEST_CASE( TestCase2 )
{
    // remove previous index file
    // remove(DocIdManager::DOC_ID_MANAGER_INDEX_FILE.c_str());
    // remove( CollectionIdManager::COLLECTION_ID_MANAGER_INDEX_FILE.c_str() );

    cerr << "[ DocIdManager ] Test Case 2 : collectionIdManager and docIdManager check ....";

    unsigned int i;
    unsigned int collectionId;
    CollectionIdManager collectionIdManager("colid_d");   
    DocIdManager docIdManager("docid2");


    // Get ids for each collection names using getCollectionIdByCollectionName().
    termIdList1_.resize(termUStringList1_.size());
    termIdList2_.resize(termUStringList2_.size());

    // Insert 100 collection names.
    for(i = 0; i < termUStringList1_.size(); i++) 
        collectionIdManager.getCollectionIdByCollectionName( termUStringList1_[i], termIdList1_[i] );

    // Insert 235 documents for each 10 collections.
    collectionId = 0;
    for(i = 0; i < termUStringList2_.size(); i++) 
    {
        if ( (i % 235) == 0 )
            collectionId++;
        BOOST_CHECK_EQUAL( docIdManager.getDocIdByDocName( collectionId, termUStringList2_[i], termIdList2_[i] ) , false );
    } // end - for

    // Check all the collection names are correctly inserted into collectionIdManager. 
    BOOST_CHECK_EQUAL( isCollectionIdListCorrect( termIdList1_, termUStringList1_, collectionIdManager ) , true );

    // Check all the document names are correctly inserted into docIdManager. 
    BOOST_CHECK_EQUAL( isDocIdListCorrect( 1, 235, termUStringList2_, docIdManager ) , true ); 

    cerr << "OK" << endl;


} // end - BOOST_AUTO_TEST_CASE( TestCase2 ) 
//*/


/**
 * @brief Test Case 3 : Recovery Test. It checks if DocIdManager can reload the previous index data when it restarts, 
 *
 * @details
 *  -# Use the index data from test case 2 (Test case 2 remove index file of previous test).
 *  -# Check if the doc id and doc name is equal to the ones of test case 2.
 * /

BOOST_AUTO_TEST_CASE( TestCase3 )
{

    cerr << "[ DocIdManager ] Test Case 3 : recovery check ................................";

    unsigned int i;
    unsigned int collectionId;
    CollectionIdManager collectionIdManager;
    DocIdManager        docIdManager;

    // Get ids for each collection names using getCollectionIdByCollectionName().
    termIdList1_.resize(termUStringList1_.size());
    termIdList2_.resize(termUStringList2_.size());


    // Check if the collection id is already in the CollectionIdManager.
    for(i = 0; i < termUStringList1_.size(); i++) 
        BOOST_CHECK_EQUAL( collectionIdManager.getCollectionIdByCollectionName( termUStringList1_[i], termIdList1_[i] ) , true );

    // Check if the document id is already in the DocIdManager.
    collectionId = 0;
    for(i = 0; i < termUStringList2_.size(); i++) 
    {
        if ( (i % 235) == 0 )
            collectionId++;
        BOOST_CHECK_EQUAL( docIdManager.getDocIdByDocName( collectionId, termUStringList2_[i], termIdList2_[i] ) , true );
    } // end - for

    // Check all the collection names are correctly inserted into collectionIdManager. 
    BOOST_CHECK_EQUAL( isCollectionIdListCorrect( termIdList1_, termUStringList1_, collectionIdManager ) , true );

    // Check all the document names are correctly inserted into docIdManager. 
    BOOST_CHECK_EQUAL( isDocIdListCorrect( 10 , 235, termUStringList2_, docIdManager ) , true ); 

    cerr << "OK" << endl;


} // end - BOOST_AUTO_TEST_CASE( TestCase3 ) 
//*/


BOOST_AUTO_TEST_SUITE_END()
