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
#include <boost/test/unit_test.hpp>
#include "IDManagerTestUtil.h"

using namespace boost::unit_test;

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
    cerr << "[ IDManager ] Test Case 1 : Check getTermIdByTermString() ................";

    // remove previous index file
    // remove(TermIdManager::TERM_ID_MANAGER_INDEX_FILE.c_str());
    boost::filesystem::remove_all("idm1");
    boost::filesystem::create_directory("idm1/");
    IDManagerDebug32 idManager("idm1/");
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

    idManager.flush();
    idManager.close();

    clean("idm1/");
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
    cerr << "[ IDManager ] Test Case 2 : Check getTermIdListByTermStringList() ........";

    // remove previous index file
    // remove(TermIdManager::TERM_ID_MANAGER_INDEX_FILE.c_str());
    boost::filesystem::remove_all("idm2");
    boost::filesystem::create_directory("idm2/");
    IDManagerDebug32 idManager("idm2/");

    // termUStringList1_ (100 terms) and termUStringList2_ (2500 terms) are already generated.

    // Get ds for each terms using getTermIdListByTermStringList().
    termIdList1_.resize(termUStringList1_.size());
    termIdList2_.resize(termUStringList2_.size());

    idManager.getTermIdListByTermStringList( termUStringList1_, termIdList1_ );
    idManager.getTermIdListByTermStringList( termUStringList2_, termIdList2_ );

    // Check if the id is correctly generated using getTermStringByTermId().
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList1_, termUStringList1_, idManager ) , true );
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList2_, termUStringList2_, idManager ) , true );

    idManager.flush();
    idManager.close();

    clean("idm2/");
    cerr << "OK"<< endl;

} // end - BOOST_AUTO_TEST_CASE( TestCase2 )

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
    cerr << "[ IDManager ] Test Case 4 : Simple Document Id Manager check .............";

    // remove previous index file
    // remove(DocIdManager::DOC_ID_MANAGER_INDEX_FILE.c_str());
    // remove(CollectionIdManager<UString, unsigned int><UString, unsigned int>::COLLECTION_ID_MANAGER_INDEX_FILE.c_str());
    boost::filesystem::remove_all("idm4");
    boost::filesystem::create_directory("idm4/");
    IDManagerDebug32 idManager("idm4/");

    string insertString("Test DocIdManager");
    UString insertUString(insertString, UString::CP949);
    UString resultUString;

    unsigned int id = 0;

    // Insert 1 terms into document id manager by using getDocIdByDocName() interface.
    // While inserting, check if the return value is false.
    BOOST_CHECK_EQUAL( idManager.getDocIdByDocName(insertUString, id ) , false );
    // Check if the ustring is correctly inserted using getDocNameByDocId() interface.
    BOOST_CHECK_EQUAL( idManager.getDocNameByDocId(id, resultUString ) , true );

    BOOST_CHECK_EQUAL( idManager.getDocIdByDocName(insertUString, id ) , true );

    // Check if the resultUString is the same as insertUString.
    BOOST_CHECK ( insertUString == resultUString );

    idManager.flush();
    idManager.close();

    // Clear data of this test case
    clean("idm4/");
    cerr << "OK" << endl;
} // end - BOOST_AUTO_TEST_CASE( TestCase4 )

BOOST_AUTO_TEST_CASE( TestCase5 )
{
    cerr << "[ IDManager ] Test Case 5 : Bloom filter check for large amount of insertions .............";
    boost::filesystem::remove_all("idm5");
    boost::filesystem::create_directory("idm5/");
    IDManager idManager("idm5/");

    izenelib::util::ClockTimer t;
    uint32_t id = 0, count = 0;
    for (uint64_t i = 0; i < 100; i++)
    {
        BOOST_CHECK_EQUAL( idManager.getDocIdByDocName(MurmurHash3_x64_128((const void *)&i, sizeof(i), 0), id) , false );
        if (++count % 100000 == 0)
            cerr << "DOCIDs inserted " << count << endl;
    }
    cout << "time elapsed for inserting " << t.elapsed() << endl;

    count = 0;
    t.restart();
    for (uint64_t i = 0; i < 100; i++)
    {
        BOOST_CHECK_EQUAL( idManager.getDocIdByDocName(MurmurHash3_x64_128((const void *)&i, sizeof(i), 0), id , false) , true );
        if (++count % 100000 == 0)
            cerr << "DOCIDs queried " << count << endl;
    }
    cout << "time elapsed for querying " << t.elapsed() << endl;

    count = 50000000;
    t.restart();
    for (uint64_t i = 100; i < 200; i++)
    {
        BOOST_CHECK_EQUAL( idManager.getDocIdByDocName(MurmurHash3_x64_128((const void *)&i, sizeof(i), 0), id) , false );
        if (++count % 100000 == 0)
            cerr << "DOCIDs inserted " << count << endl;
    }
    cout << "time elapsed for inserting " << t.elapsed() << endl;

    idManager.flush();
    idManager.close();

    // Clear data of this test case
    clean("idm5/");
    cerr << "OK" << endl;
} // end - BOOST_AUTO_TEST_CASE( TestCase5 )

///**
// * @brief Test Case 8 : test read/write interfaces of IDManager
// * @details
// *      - Insert 2350 collection names, document names, and term strings into IDManager
// *      - Store data of IDManager in test.data.
// *      - Load data from test.dat, make sure that the IDs are not changed
// * /
//
//BOOST_AUTO_TEST_CASE( TestCase8 )
//{
//	// Clear data of previous test case
//	remove("test.dat");
//	// File IDManagerData.dat is defalt file name of IDManager.
//	// IDManager automatically stores its data in IDManagerData.dat
//	// Clear data of previous test
//	remove("IDManagerData.dat");
//
//    cerr << "[ IDManager ] Test Case 8 : test read/write from/to file ......";
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
//
//    for(i = 0; i < termUStringList2_.size(); i++)
//	{
//        idManager_Store.getTermIdByTermString(termUStringList2_[i], termIdList[i]);
//        idManager_Store.getDocIdByDocName(collectionId, termUStringList2_[i], docIdList[i]);
//        idManager_Store.getCollectionIdByCollectionName( termUStringList2_[i], collectionIdList[i] );
//	}
//
//	// store data in test.dat
//	idManager_Store.write("test.dat");
//	remove("IDManagerData.dat");
//
//    cerr << "==================================================================== 1" << endl;
//
//	// load data from test.dat
//	idManager_Load = new IDManager();
//	idManager_Load->read("test.dat");
//
//    cerr << "==================================================================== 3" << endl;
//
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
//    cerr << "==================================================================== 2" << endl;
//	delete idManager_Load;
//
//	remove("test.dat");
//	// Clear data of this test case
//	// remove("IDManagerData.dat");
//
//    cerr << "OK" << endl;
//
//} // end - BOOST_AUTO_TEST_CASE( TestCase8 )
////*/

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
