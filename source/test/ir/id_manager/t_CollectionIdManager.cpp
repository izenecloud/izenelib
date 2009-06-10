/**
 * @file    t_CollectionIdManager.cpp
 * @brief   A Test unit of CollectionIdManager. 
 * @author  Do Hyun Yun
 * @date    2008-08-14
 * @details
 *
 *  ==================================== [ Test Schemes ] ====================================
 *
 *  -# Test Case 1 : Simple CollectionIdManager check.
 *      - Inset one collection name to the empty collectionIndexer using 
 *        getCollectionIdByCollectionName() interface. And check if the return value is false 
 *        which means the collection name is not in the collectionIndexer. Also check if the
 *        return value of getCollectionNameByCollectionId() is false/true before and after
 *        using getCollectionIdByCollectionName() interface.
 *      - Process getCollectionIdByCollectionName() using the same name as one at the previous
 *        test. Check if the return value is true which means the collection name is in the
 *        termIndexer.
 *  -# Test Case 2 : Insert 2450 collection names and check if the collectionIndexer contains
 *                   correct data.
 *      - Insert 2400 collection names using getCollectionIdByCollectionName(). Store
 *        CollectionIdList
 *      - Check if the collection name (which is gotten by using 
 *        getCollectionNameByCollectionId() with its id from collectionIndexer) is the same as 
 *        the original one.
 *\n
 *  -# Test Case 3 : Recovery Test. It checks if CollectionIdManager can reload the previous index data when it restarts, 
 *      - Use the index data from test case 2 (Test case 2 remove index file of previous test). 
 *      - Check if the collection id and collection name is equal to the ones of test case 2.\n\n
 */
#include <fstream>
#include <boost/test/unit_test.hpp>
#include <wiselib/ustring/UString.h>

#include <ir/id_manager/CollectionIdManager.h>
#include <algorithm>

using namespace std;
using namespace wiselib;
using namespace boost::unit_test;
using namespace izenelib::ir::idmanager;

/**
 * @brief CollectionIdManagerFixture is used to test CollectionIdManager. It loads collection names from text file to build termStringLists.
 */
class CollectionIdManagerFixture
{
public:

    vector<UString> termUStringList1_; ///< The list of 100 term string.

    vector<unsigned int> termIdList1_; ///< The list of 100 term id.

    vector<UString> termUStringList2_; ///< The list of 2400 term string.

    vector<unsigned int> termIdList2_; ///< The list of 2400 term id.


    CollectionIdManagerFixture()
    {
        // generate two term lists
		BOOST_CHECK_EQUAL(generateTermLists(termUStringList1_, termIdList1_, termUStringList2_, termIdList2_), true);

    } // end - CollectionIdManagerFixture()

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

}; // end - class CollectionIdManagerFixture


/**
 * @brief Check function which checks if all the ids are corretly generated.
 */

inline bool isIdListCorrect(const vector<unsigned int>& collectionIdList,const vector<UString>& collectionNameList, CollectionIdManager<UString, unsigned int>& collectionIdManager)
{
    UString compare;
    
    // Check if the id is correctly generated using getCollectionNameByCollectionId().
    for(unsigned int i = 0; i < collectionIdList.size(); i++) 
    {

        // Check if the id is in the CollectionIdManager with getting collection name according to the id.
        if ( collectionIdManager.getCollectionNameByCollectionId(collectionIdList[i], compare) == false ){
        	cout<<"i="<<i<<" : " <<collectionIdList[i]<<endl;
            return false;
        }

        // Check if the collection name from collectionIdManager is the same as the original one.
        if ( collectionNameList[i] != compare ) {
        	cout<<"i="<<i<<" : " <<collectionIdList[i]<<endl;
        	cout<<"failed"<<endl;       	
            return false;
        }

    } // end - for

    return true;

} // end - isIdListCorrect()




/**********************************************************
 *
 *          Start point of t_CollectionIdManager suite - 1
 *
 **********************************************************/

BOOST_FIXTURE_TEST_SUITE( t_CollectionIdManager, CollectionIdManagerFixture )

//*/

BOOST_AUTO_TEST_CASE( TestCase1 )
{
    // remove previous index file
    // remove(CollectionIdManager::COLLECTION_ID_MANAGER_INDEX_FILE.c_str());

    cerr << endl;
    cerr << "[ CollectionIdManager ] Test Case 1 : Simple Collection Id Manager check ...........";

    
    CollectionIdManager<UString, unsigned int> collectionIdManager("colid1");

    string insertString("Test CollectionIdManager");
    UString insertUString(insertString, UString::CP949);
    UString resultUString;

    unsigned int id = 0;

    // Insert 1 terms into collection id manager using getCollectionIdByCollectionName() interface.
    // While inserting, check if the return value is false.
    BOOST_CHECK_EQUAL( collectionIdManager.getCollectionIdByCollectionName( insertUString, id ) , false );
 
    // Check if the ustring is correctly inserted using getCollectionNameByCollectionId() interface.
    BOOST_CHECK_EQUAL( collectionIdManager.getCollectionNameByCollectionId( id , resultUString ) , true );

    // Check again if the id is found in the collection indexer using getCollectionIdByCollectionName() interface.
    BOOST_CHECK_EQUAL( collectionIdManager.getCollectionIdByCollectionName( insertUString, id ) , true );
    
    // Check if the resultUString is the same as insertUString.
    BOOST_CHECK ( insertUString == resultUString );

    cerr << "OK" << endl;

} // end - BOOST_AUTO_TEST_CASE( TestCase1 )


/**
 * @brief Test Case 2 : Insert 2450 collection names and check if the collectionIndexer_ contains
 *                      correct data.
 * @details
 *      - Insert 2400 collection names using getCollectionIdByCollectionName(). Store
 *        CollectionIdList
 *      - Check if the collection name (which is gotten by using 
 *        getCollectionNameByCollectionId() with its id from collectionIndexer) is the same as 
 *        the original one.
 */

BOOST_AUTO_TEST_CASE( TestCase2 )
{
    // remove previous index file
    // remove(CollectionIdManager::COLLECTION_ID_MANAGER_INDEX_FILE.c_str());

    cerr << "[ CollectionIdManager ] Test Case 2 : 2450 collection name insertion checking ......";
    
    unsigned int i;
    CollectionIdManager<UString, unsigned int> collectionIdManager("colid2");

    // termUStringList1_ (100 terms) and termUStringList2_ (2500 terms) are already generated.

    // Get ids for each collection names using getCollectionIdByCollectionName().
    termIdList1_.resize(termUStringList1_.size());
    termIdList2_.resize(termUStringList2_.size());

    for(i = 0; i < termUStringList1_.size(); i++) 
        collectionIdManager.getCollectionIdByCollectionName( termUStringList1_[i], termIdList1_[i] );

    for(i = 0; i < termUStringList2_.size(); i++) 
        collectionIdManager.getCollectionIdByCollectionName( termUStringList2_[i], termIdList2_[i] );
    
    // Check if the id is correctly generated using getCollectionNameByCollectionId().
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList1_, termUStringList1_, collectionIdManager ) , true );
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList2_, termUStringList2_, collectionIdManager ) , true );
    cerr << "OK" << endl;

} // end - BOOST_AUTO_TEST_CASE( TestCase2 ) 
//*/

/**
 * @brief Test Case 3 : Recovery Test. It checks if CollectionIdManager can reload the previous index data when it restarts, 
 *
 * @details
 *  -# Use the index data from test case 2 (Test case 2 remove index file of previous test). 
 *  -# Check if the collection id and collection name is equal to the ones of test case 2.\n\n
 * /
 
BOOST_AUTO_TEST_CASE( TestCase3 )
{
    cerr << "[ CollectionIdManager ] Test Case 3 : recovery checking ............................";
    
    unsigned int i;
    CollectionIdManager collectionIdManager;

    // termUStringList1_ (100 terms) and termUStringList2_ (2500 terms) are already generated.

    // Get ids for each collection names using getCollectionIdByCollectionName().
    termIdList1_.resize(termUStringList1_.size());
    termIdList2_.resize(termUStringList2_.size());

    for(i = 0; i < termUStringList1_.size(); i++) 
        BOOST_CHECK_EQUAL( collectionIdManager.getCollectionIdByCollectionName( termUStringList1_[i], termIdList1_[i] ) , true );

    for(i = 0; i < termUStringList2_.size(); i++) 
        BOOST_CHECK_EQUAL( collectionIdManager.getCollectionIdByCollectionName( termUStringList2_[i], termIdList2_[i] ) , true );
    
    // Check if the id is correctly generated using getCollectionNameByCollectionId().
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList1_, termUStringList1_, collectionIdManager ) , true );
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList2_, termUStringList2_, collectionIdManager ) , true );
    cerr << "OK" << endl;

} // end - BOOST_AUTO_TEST_CASE( TestCase3 ) 
//*/

/**
 * @brief  Test Case 1 : Simple CollectionIdManager check.
 * @details
 *      - Inset one collection name to the empty collectionIndexer_ using 
 *        getCollectionIdByCollectionName() interface. And check if the return value is false 
 *        which means the collection name is not in the collectionIndexer. Also check if the
 *        return value of getCollectionNameByCollectionId() is true after using
 *        getCollectionIdByCollectionName() interface.
 *      - Process getCollectionIdByCollectionName() using the same name as one at the previous
 *        test. Check if the return value is true which means the collection name is in the
 *        termIndexer
 */
 

BOOST_AUTO_TEST_SUITE_END()
