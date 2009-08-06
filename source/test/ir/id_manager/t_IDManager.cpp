/**
 * @file    t_IDManager.cpp
 * @brief   A Test unit of IDManager
 * @author  Do Hyun Yun
 * @date    2008-08-18
 * @details
 *
 *  ==================================== [ Test Schemes ] ====================================
 *
 *  -# Test Case 1 : Get ids of 100 & 2400 terms using getTermIdByTermString() Interfaces. After checking the ids of them, get strings of 100 & 2400 ids using getTermStringByTermId() Interfaces.\n\n
 *  -# Test Case 2 : Get ids of 100 & 2400 terms using getTermIdListByTermStringList() Interfaces. After checking the ids of them, get strings of 100 & 2400 ids using getTermStringByTermId() Interfaces. \n\n
 *  -# Test Case 3 : Get idLists of specific term using getTermIdListByWildcardPattern() Interfaces. After getting the ids, check if the getting term id list is correct.\n\n
 *  -# Test Case 4 : Simple DocIdManager check.
 *      - Using number 1 as a collection id, insert one document name by Using
 *        getDocIdByDocName(). Check if the return value is false. It means the given term
 *        name does not exist in the indexer.
 *      - Check if the given document name is correctly inserted into the docIndexer by using
 *        getDocNameByDocId() interface.
 *\n
 *  -# Test Case 5 : Insert 100 collection names by using collectionIdManager and insert 235
 *                   documents by using collectionIdManager & docIdManager. Check if the
 *                   insertion is correct.
 *      - Insert 100 collection names into collectionIdManager. While inserting, get the
 *        collection ids for each name.
 *      - Using collection Ids, insert 235 documents for each collection.
 *      - Check all the terms are correctly inserted into collectionIdManager and docIdManager.
 *\n
 *  -# Test Case 6 : Simple CollectionIdManager<UString, unsigned int><UString, unsigned int> check.
 *      - Inset one collection name to the empty collectionIndexer using
 *        getCollectionIdByCollectionName() interface. And check if the return value is false
 *        which means the collection name is not in the collectionIndexer. Also check if the
 *        return value of getCollectionNameByCollectionId() is false/true before and after
 *        using getCollectionIdByCollectionName() interface.
 *      - Process getCollectionIdByCollectionName() using the same name as one at the previous
 *        test. Check if the return value is true which means the collection name is in the
 *        termIndexer.
 *\n
 *  -# Test Case 7 : Insert 2450 collection names and check if the collectionIndexer contains
 *                   correct data.
 *      - Insert 2400 collection names using getCollectionIdByCollectionName(). Store
 *        CollectionIdList
 *      - Check if the collection name (which is gotten by using
 *        getCollectionNameByCollectionId() with its id from collectionIndexer) is the same as
 *        the original one.
 */
#include <fstream>
#include <algorithm>

#include <boost/test/unit_test.hpp>
#include <wiselib/ustring/UString.h>

#include <ir/id_manager/IDManager.h>

using namespace std;
using namespace wiselib;
using namespace boost::unit_test;
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

/**
 * @brief Check function which checks if all the doc ids are corretly generated.
 */

inline bool isDocIdListCorrect(unsigned int collectionIdNum, unsigned int docIdNum, const vector<UString>& docNameList, IDManager& IDManager)
{
    UString compare;

    for(unsigned int i = 1; i <= collectionIdNum; i++)
    {
        for(unsigned int j = 1; j <= docIdNum; j++)
        {
            // Check if the id is in the DocIdManager with getting document name according to the id.
            if ( IDManager.getDocNameByDocId(i, j, compare) == false )
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
 *          Start point of t_IDManager suite - 1
 *
 **********************************************************/

BOOST_FIXTURE_TEST_SUITE( t_IDManager, IDManagerFixture )

/**
 * @brief Test Case 1 : Get ids of 100 & 2400 terms using getTermIdByTermString() Interfaces.
 *                      After checking the ids of them, get strings of 100 & 2400 ids using getTermStringByTermId() Interfaces.\n
 *
 * @details
 *  - Get 100 & 2400 terms. termUStringList1_ and termUStringList2_ contains terms.
 *  - Get ids of each terms using getTermIdByTermString() interface which can get one id by one ter string.
 *  - Check if the id is correctly generated using getTermStringByTermId().
 */
BOOST_AUTO_TEST_CASE( TestCase1 )
{
	// Clear data of previous test case
	remove("IDManagerData.dat");

    cerr << endl;
    cerr << "[ IDManager ] Test Case 1 : Check getTermIdByTermString() ................";

    // remove previous index file
    // remove(TermIdManager::TERM_ID_MANAGER_INDEX_FILE.c_str());


    IDManager idManager("idm1");
    unsigned int i;
    UString compare;

    // termUStringList1_ (100 terms) and termUStringList2_ (2500 terms) are already generated.


    // Get ids for each terms using getTermIdByTermString().
    termIdList1_.resize(termUStringList1_.size());

    termIdList2_.resize(termUStringList2_.size());

    for(i = 0; i < termUStringList1_.size(); i++)
        idManager.getTermIdByTermString( termUStringList1_[i], termIdList1_[i] );

    for(i = 0; i < termUStringList2_.size(); i++)
        idManager.getTermIdByTermString( termUStringList2_[i], termIdList2_[i] );

    // Check if the id is correctly generated using getTermStringByTermId().
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList1_, termUStringList1_, idManager ) , true );
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList2_, termUStringList2_, idManager ) , true );

	// Clear data of this test case
	remove("IDManagerData.dat");
    cerr << "OK" << endl;

} // end - BOOST_AUTO_TEST_CASE( TestCase1 )


//*/

/**
 * @brief Test Case 2 : Get ids of 100 & 2400 terms using getTermIdListByTermStringList() Interfaces.
 *                      After checking the ids of them, get strings of 100 & 2400 ids. \n
 * @details
 *  - Get 100 & 2400 terms. termUStringList1_ and termUStringList2_ contains terms.
 *  - Get ids of terms using getTermIdListByTermStringList() interface which can get all the id lists of term strings.
 *  - Check if the id is correctly generated using getTermStringByTermId() interface.
 */
BOOST_AUTO_TEST_CASE( TestCase2 )
{
	// Clear data of previous test case
	remove("IDManagerData.dat");
    cerr << "[ IDManager ] Test Case 2 : Check getTermIdListByTermStringList() ........";

    // remove previous index file
    // remove(TermIdManager::TERM_ID_MANAGER_INDEX_FILE.c_str());


    IDManager idManager("idm2");

    // termUStringList1_ (100 terms) and termUStringList2_ (2500 terms) are already generated.

    // Get ds for each terms using getTermIdListByTermStringList().
    termIdList1_.resize(termUStringList1_.size());
    termIdList2_.resize(termUStringList2_.size());

    idManager.getTermIdListByTermStringList( termUStringList1_, termIdList1_ );
    idManager.getTermIdListByTermStringList( termUStringList2_, termIdList2_ );

    // Check if the id is correctly generated using getTermStringByTermId().
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList1_, termUStringList1_, idManager ) , true );
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList2_, termUStringList2_, idManager ) , true );

	// Clear data of this test case
	remove("IDManagerData.dat");
    cerr << "OK"<< endl;

} // end - BOOST_AUTO_TEST_CASE( TestCase2 )
//*/

/**
 * @brief Test Case 3 : Get idLists of specific term using getTermIdListByWildcardPattern() Interfaces. After getting the ids, check if the getting term id list is correct.\n
 *
 * @details
 *  - Build term index dictionary using getTermIdListByTermStringList() Interface.
 *  - Get term id list using getTermIdListByWildcardPattern() Interface with pattern string "ate*", "*ate" and "*ate*".
 *  - Figure out if all the terms which is the result of getTermIdListByWildcardPattern() have the pattern string in it.
 *
 */

BOOST_AUTO_TEST_CASE( TestCase3 )
{
    cerr << "[ IDManager ] Test Case 3 : Check getTermIdListByWildcardPattern() .......";

    // remove previous index file
    // remove(TermIdManager::TERM_ID_MANAGER_INDEX_FILE.c_str());


    IDManager idManager("idm3");
    UString compare;

    // Build term index dictionary using getTermIdListByTermStringList() Interface.
    termIdList1_.resize(termUStringList1_.size());
    termIdList2_.resize(termUStringList2_.size());
    idManager.getTermIdListByTermStringList( termUStringList1_, termIdList1_ );
    idManager.getTermIdListByTermStringList( termUStringList2_, termIdList2_ );

    // Get term id list using getTermIdListByWildcardPattern() Interface with pattern string "ate".
    termIdList1_.clear();

    std::string patternSource("ad");
    UString patternCheck(patternSource, UString::CP949);
    patternSource = "*";
    UString starChar(patternSource, UString::CP949);
    UString pattern;

    // ---------------------------------------------- pattern = "ad*"
    pattern.clear();
    pattern = patternCheck;
    pattern += starChar;
    idManager.getTermIdListByWildcardPattern(pattern, termIdList1_);
    BOOST_CHECK_EQUAL( termIdList1_.size() , static_cast<unsigned int>(35) );

    // ---------------------------------------------- pattern = "*ad"
    pattern.clear();
    termIdList1_.clear();
    pattern = starChar;
    pattern += patternCheck;

    idManager.getTermIdListByWildcardPattern(pattern, termIdList1_);
    BOOST_CHECK_EQUAL( termIdList1_.size() , static_cast<unsigned int>(4) );

    // ---------------------------------------------- pattern = "*ad*"
    pattern.clear();
    termIdList1_.clear();
    pattern += starChar;
    pattern += patternCheck;
    pattern += starChar;
    idManager.getTermIdListByWildcardPattern(pattern, termIdList1_);
    BOOST_CHECK_EQUAL( termIdList1_.size() , static_cast<unsigned int>(81));

    idManager.display();
    cerr << "OK" << endl;

} // end - BOOST_AUTO_TEST_CASE( TestCase3 )

/**
 * @brief Test Case 5 : Simple DocIdManager check.
 * @details
 *  -# Using number 1 as a collection id, insert one document name by Using
 *     getDocIdByDocName(). Check if the return value is false. It means the given term
 *     name does not exist in the indexer.
 *\n
 *  -# Check if the given document name is correctly inserted into the docIndexer by using
 *     getDocNameByDocId() interface.
 */
BOOST_AUTO_TEST_CASE( TestCase4 )
{
	// Clear data of previous test case
	remove("IDManagerData.dat");
    cerr << "[ IDManager ] Test Case 4 : Simple Document Id Manager check .............";

    // remove previous index file
    // remove(DocIdManager::DOC_ID_MANAGER_INDEX_FILE.c_str());
    // remove(CollectionIdManager<UString, unsigned int><UString, unsigned int>::COLLECTION_ID_MANAGER_INDEX_FILE.c_str());


    IDManager idManager("idm4");

    string insertString("Test DocIdManager");
    UString insertUString(insertString, UString::CP949);
    UString resultUString;

    unsigned int collectionId = 1; // Collection id 1 is used for inserting one document name.

    unsigned int id = 0;

    // Insert 1 terms into document id manager by using getDocIdByDocName() interface.
    // While inserting, check if the return value is false.
    BOOST_CHECK_EQUAL( idManager.getDocIdByDocName(collectionId, insertUString, id ) , false );
    // Check if the ustring is correctly inserted using getDocNameByDocId() interface.
    BOOST_CHECK_EQUAL( idManager.getDocNameByDocId(collectionId, id, resultUString ) , true );

    // Check again if the id is found in the document indexer using getDocIdByDocName() interface.
    BOOST_CHECK_EQUAL( idManager.getDocIdByDocName(collectionId, insertUString, id ) , true );


    // Check if the resultUString is the same as insertUString.
    BOOST_CHECK ( insertUString == resultUString );

	// Clear data of this test case
	remove("IDManagerData.dat");
    cerr << "OK" << endl;

} // end - BOOST_AUTO_TEST_CASE( TestCase4 )


/**
 * @brief Test Case 8 : test read/write interfaces of IDManager
 * @details
 *      - Insert 2350 collection names, document names, and term strings into IDManager
 *      - Store data of IDManager in test.data.
 *      - Load data from test.dat, make sure that the IDs are not changed
 * /

BOOST_AUTO_TEST_CASE( TestCase8 )
{
	// Clear data of previous test case
	remove("test.dat");
	// File IDManagerData.dat is defalt file name of IDManager.
	// IDManager automatically stores its data in IDManagerData.dat
	// Clear data of previous test
	remove("IDManagerData.dat");

    cerr << "[ IDManager ] Test Case 8 : test read/write from/to file ......";

    // remove previous index file
    // remove(CollectionIdManager<UString, unsigned int>::COLLECTION_ID_MANAGER_INDEX_FILE.c_str());


    unsigned int i;
	unsigned int collectionId = 1;
    IDManager idManager_Store;
    IDManager* idManager_Load;
    std::vector<unsigned int> termIdList;
    std::vector<unsigned int> docIdList;
    std::vector<unsigned int> collectionIdList;
    unsigned int testId;

    // termUStringList1_ (100 terms) and termUStringList2_ (2500 terms) are already generated.

    // Get ids for each collection names using getCollectionIdByCollectionName().
    termIdList.resize(termUStringList2_.size());
    docIdList.resize(termUStringList2_.size());
    collectionIdList.resize(termUStringList2_.size());

    for(i = 0; i < termUStringList2_.size(); i++)
	{
        idManager_Store.getTermIdByTermString(termUStringList2_[i], termIdList[i]);
        idManager_Store.getDocIdByDocName(collectionId, termUStringList2_[i], docIdList[i]);
        idManager_Store.getCollectionIdByCollectionName( termUStringList2_[i], collectionIdList[i] );
	}

	// store data in test.dat
	idManager_Store.write("test.dat");
	remove("IDManagerData.dat");

    cerr << "==================================================================== 1" << endl;

	// load data from test.dat
	idManager_Load = new IDManager();
	idManager_Load->read("test.dat");

    cerr << "==================================================================== 3" << endl;

    for(i = 0; i < termUStringList2_.size(); i++)
	{
		// check if the IDs are changed
        idManager_Load->getTermIdByTermString(termUStringList2_[i], testId);
		BOOST_CHECK_EQUAL(termIdList[i], testId);
        idManager_Load->getDocIdByDocName(collectionId, termUStringList2_[i], testId);
		BOOST_CHECK_EQUAL(docIdList[i], testId);
        idManager_Load->getCollectionIdByCollectionName( termUStringList2_[i], testId);
		BOOST_CHECK_EQUAL(collectionIdList[i], testId);
	}
    cerr << "==================================================================== 2" << endl;
	delete idManager_Load;

	remove("test.dat");
	// Clear data of this test case
	// remove("IDManagerData.dat");

    cerr << "OK" << endl;

} // end - BOOST_AUTO_TEST_CASE( TestCase8 )
//*/

//
///**
// * @brief Test Case 9 : Automatically load data of IDManager from a file
// * @details
// *      - Insert 2350 collection names, document names, and term strings into IDManager
// *      - Load data from defalt file IDManagerData.dat, make sure that the IDs are not changed
// */
//BOOST_AUTO_TEST_CASE( TestCase9 )
//{
//	// File IDManagerData.dat is defalt file name of IDManager.
//	// IDManager automatically stores its data in IDManagerData.dat
//	// Clear data of previous test case
//	// remove("IDManagerData.dat");
//
//    cerr << "[ IDManager ] Test Case 9 : test read/write from/to file ......";
//
//    // remove previous index file
//    // remove(CollectionIdManager<UString, unsigned int>::COLLECTION_ID_MANAGER_INDEX_FILE.c_str());
//
//
//    unsigned int i;
//	unsigned int collectionId = 1;
//    IDManager idManager_Store;
//    IDManager* idManager_Load;
//    std::vector<unsigned int> termIdList;
//    std::vector<unsigned int> docIdList;
//    std::vector<unsigned int> collectionIdList;
//    unsigned int testId;
//
//    // termUStringList1_ (100 terms) and termUStringList2_ (2500 terms) are already generated.
//
//    // Get ids for each collection names using getCollectionIdByCollectionName().
//    termIdList.resize(termUStringList2_.size());
//    docIdList.resize(termUStringList2_.size());
//    collectionIdList.resize(termUStringList2_.size());
//    for(i = 0; i < termUStringList2_.size(); i++)
//	{
//        idManager_Store.getTermIdByTermString(termUStringList2_[i], termIdList[i]);
//        idManager_Store.getDocIdByDocName(collectionId, termUStringList2_[i], docIdList[i]);
//        idManager_Store.getCollectionIdByCollectionName( termUStringList2_[i], collectionIdList[i] );
//	}
//
//	// load data from IDManagerData.dat
//	idManager_Load = new IDManager();
//    for(i = 0; i < termUStringList2_.size(); i++)
//	{
//		// check if the IDs are changed
//        idManager_Load->getTermIdByTermString(termUStringList2_[i], testId);
//		BOOST_CHECK_EQUAL(termIdList[i], testId);
//        idManager_Load->getDocIdByDocName(collectionId, termUStringList2_[i], testId);
//		BOOST_CHECK_EQUAL(docIdList[i], testId);
//        idManager_Load->getCollectionIdByCollectionName( termUStringList2_[i], testId);
//		BOOST_CHECK_EQUAL(collectionIdList[i], testId);
//	}
//	delete idManager_Load;
//
//	// Clear data of this test case
//	remove("IDManagerData.dat");
//
//
//
//
//    cerr << "OK" << endl;
//
//} // end - BOOST_AUTO_TEST_CASE( TestCase9 )
////*/

BOOST_AUTO_TEST_SUITE_END()
