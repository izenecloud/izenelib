/**
 * @file    t_TermIdManager.cpp
 * @brief   A Test unit of TermIdManager
 * @author  Do Hyun Yun
 * @date    2008-07-20
 * @details
 *
 *  ==================================== [ Test Schemes ] ====================================
 *
 *  -# Test Case 1 : Get ids of 100 & 2400 terms using getTermIdByTermString() Interfaces. After checking the ids of them, get strings of 100 & 2400 ids using getTermStringByTermId() Interfaces.\n\n
 *  -# Test Case 2 : Get ids of 100 & 2400 terms using getTermIdListByTermStringList() Interfaces. After checking the ids of them, get strings of 100 & 2400 ids using getTermStringByTermId() Interfaces. \n\n
 *  -# Test Case 3 : Get idLists of specific term using getTermIdListByWildcardPattern() Interfaces. After getting the ids, check if the getting term id list is correct.\n\n
 *  -# Test Case 4 : Merging Test. Generate and get ids of MAX_BUFFER_TERM_NUMBER  terms. MAX_BUFFER_TERM_NUMBER is the amount to occur merge operation between bufferTermIndexer and mainTermIndexer.\n\n
 *  -# Test Case 5 : Random term test. it contains duplicate terms. We'll check if it works correctly.
 *  -# Test Case 6 : Recovery Test. Just use the index data from test case 4 (Test case 2~4 remove index file of previous test) and check if the term id and term string is equal to the ones of test case 4.\n\n
 *  -# Test Case 6 : Test with Korean characters. There are 14 different list of Korean words. Add thoses
 *  lists into the IdManager
 */
#include <algorithm>
#include <fstream>
#include <vector>
#include <set>

#include <boost/test/unit_test.hpp>
#include <wiselib/ustring/UString.h>

#include <ir/id_manager/TermIdManager.h>
#include <ir/id_manager/IDGenerator.h>
#include <ir/id_manager/IDStorage.h>
#include <ir/id_manager/IDFactory.h>

using namespace std;
using namespace wiselib;
using namespace boost::unit_test;
using namespace izenelib::ir::idmanager;


typedef TermIdManager<UString, unsigned int,
                    HashIDGenerator<UString, unsigned int>,
                    SDBIDStorage<UString, unsigned int>,
                    izenelib::am::BTrie_CJK> TestTermIdManager;

/**
 * @brief TermIdManager<UString, unsigned int>Fixture is used to test TermIdManager<UString, unsigned int>. It loads terms from text file to build termStringLists.
 */
class TermIdManagerFixture
{
public:

    vector<UString> termUStringList1_; ///< The list of 100 term string.

    vector<unsigned int> termIdList1_; ///< The list of 100 term id.

    vector<UString> termUStringList2_; ///< The list of 2400 term string.

    vector<unsigned int> termIdList2_; ///< The list of 2400 term id.


    TermIdManagerFixture()
    {
        // generate two term lists
		BOOST_CHECK_EQUAL(generateTermLists(termUStringList1_, termIdList1_, termUStringList2_, termIdList2_), true);

    } // end - TermIdManager<UString, unsigned int>Fixture()

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
                           vector<unsigned int>& termIdList1,
                           vector<UString>& termUStringList2,
                           vector<unsigned int>& termIdList2)
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
            termUStringList1[i].assign(termStringList1[i], UString::UTF_8);

        termUStringList2.resize(termStringList2.size());
        for(i = 0; i < termStringList2.size(); i++)
            termUStringList2[i].assign(termStringList2[i], UString::UTF_8);

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

}; // end - class TermIdManager<UString, unsigned int>Fixture


/**
 * @brief Check function which ckecks if all the ids are corretly generated.
 */

inline bool isIdListCorrect(const vector<unsigned int>& termIdList,
                            const vector<UString>& termStringList,
                            TestTermIdManager& termIdManager)
{
    UString compare;

    // Check if the id is correctly generated using getTermStringByTermId().
    for(unsigned int i = 0; i < termIdList.size(); i++)
    {

        // Check if the id is in the termIdManager with getting termstring according to the id.
        if ( termIdManager.getTermStringByTermId(termIdList[i], compare) == false )
            return false;

        // Check if the term string from termIdManager is the same as the original one.
        else if ( termStringList[i] != compare )
            return false;

    } // end - for

    return true;

} // end - isIdListCorrect()

/**********************************************************
 *
 *          Start point of t_TermIdManager<UString, unsigned int> suite - 1
 *
 **********************************************************/

BOOST_FIXTURE_TEST_SUITE( t_TermIdManager, TermIdManagerFixture )


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
    cerr << endl;
    cerr << "[ TermIdManager<UString, unsigned int> ] Test Case 1 : Check getTermIdByTermString() ................";

    // remove previous index file
    //remove(TermIdManager<UString, unsigned int>::TERM_ID_MANAGER_INDEX_FILE.c_str());


    TestTermIdManager termIdManager("tid1");
    unsigned int i;
    UString compare;

    // termUStringList1_ (100 terms) and termUStringList2_ (2500 terms) are already generated.

    // Get ids for each terms using getTermIdByTermString().
    termIdList1_.resize(termUStringList1_.size());

    termIdList2_.resize(termUStringList2_.size());


    std::cerr << std::endl;
    for(i = 0; i < termUStringList1_.size(); i++)
      termIdManager.getTermIdByTermString( termUStringList1_[i], termIdList1_[i] );


    for(i = 0; i < termUStringList2_.size(); i++)
        termIdManager.getTermIdByTermString( termUStringList2_[i], termIdList2_[i] );

    // Check if the id is correctly generated using getTermStringByTermId().
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList1_, termUStringList1_, termIdManager ) , true );
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList2_, termUStringList2_, termIdManager ) , true );

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
    cerr << "[ TermIdManager<UString, unsigned int> ] Test Case 2 : Check getTermIdListByTermStringList() ........";

    // remove previous index file
    //remove(TermIdManager<UString, unsigned int>::TERM_ID_MANAGER_INDEX_FILE.c_str());

    TestTermIdManager termIdManager("tid2");

    // termUStringList1_ (100 terms) and termUStringList2_ (2500 terms) are already generated.
    // Get ds for each terms using getTermIdListByTermStringList().
    termIdManager.getTermIdListByTermStringList( termUStringList1_, termIdList1_ );
    termIdManager.getTermIdListByTermStringList( termUStringList2_, termIdList2_ );

    // Check if the id is correctly generated using getTermStringByTermId().
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList1_, termUStringList1_, termIdManager ) , true );
    BOOST_CHECK_EQUAL( isIdListCorrect( termIdList2_, termUStringList2_, termIdManager ) , true );

    cerr << "OK" << endl;

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
    cerr << "[ TermIdManager<UString, unsigned int> ] Test Case 3 : Check getTermIdListByWildcardPattern() .......";

    // remove previous index file
    // remove(TermIdManager<UString, unsigned int>::TERM_ID_MANAGER_INDEX_FILE.c_str());

    TestTermIdManager idManager("tid3");
    UString compare;

    // Build term index dictionary using getTermIdListByTermStringList() Interface.
    termIdList1_.resize(termUStringList1_.size());
    termIdList2_.resize(termUStringList2_.size());
    idManager.getTermIdListByTermStringList( termUStringList1_, termIdList1_ );
    idManager.getTermIdListByTermStringList( termUStringList2_, termIdList2_ );

    // Get term id list using getTermIdListByWildcardPattern() Interface with pattern string "ate".
    termIdList1_.clear();

    std::string patternSource("ad");
    UString patternCheck(patternSource, UString::UTF_8);
    patternSource = "*";
    UString starChar(patternSource, UString::UTF_8);
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
//*/

/**
 * @brief Test Case 6 : Variable lenght & duplicated term Test
 *
 * @details
 * Read random.txt file which contains random & duplicated terms.
 * Insert termList into TermIdManager<UString, unsigned int> using getTermIdListbyTermStringList. Store idlist in termIdList1_.
 * get termIdList twice and insert result into termIdList2_
 * Compare termIdList1_ and termIdList2_
 */

BOOST_AUTO_TEST_CASE( TestCase4 )
{

    cerr << "[ TermIdManager<UString, unsigned int> ] Test Case 4 : Check variable length and duplicate terms ....";

    TestTermIdManager termIdManager("tid4");
    vector<UString> termUStringList;
    vector<unsigned int> termIdList1;
    vector<unsigned int> termIdList2;
    UString termUString;
    string term;
    vector<string>termList;

    // Load term list from file
    ifstream fpin;
    fpin.open("test-data/random.txt");

    termList.clear();

    while(fpin.good())
    {
        term.clear();
        fpin >> term;
        if (term.empty()) break;
        termList.push_back(term);
    }

    for(unsigned int i = 0; i < termList.size(); i++)
    {
        termUString.clear();
        termUString.assign(termList[i], UString::CP949);
        termUStringList.push_back(termUString);
    } // end - for


    termIdManager.getTermIdListByTermStringList( termUStringList, termIdList1 );

    termIdList2_.clear();

    termIdManager.getTermIdListByTermStringList( termUStringList, termIdList2 );

    for(unsigned int i = 0; i < termIdList1.size(); i++)
    {
        BOOST_CHECK_EQUAL( termIdList1[i] , termIdList2[i] );
    } // end - for

    cerr << "OK" << endl;

} // end - BOOST_AUTO_TEST_CASE( TestCase4 )
//*/


/**
 * @brief Test Case 5: Test with korean characters
 *
 * @details
 * Test getTermIdListByTermStringList function with Korea Characters.
 */
BOOST_AUTO_TEST_CASE(TermIndexer_With_Korean_Characters )
{
    cerr << "[ TermIdManager<UString, unsigned int> ] Test Case 5 : Check TermIdManager<UString, unsigned int> with Korean Terms ....";

    TestTermIdManager termIdManager("tid5");
    vector<unsigned int> termIdList;
	set<string> uniqueStringList;
	set<string> uniqueStringList2;

    ifstream fpin;
    fpin.open("test-data/output.txt");

    string inputString;
    vector<UString> inputStringList;

    while( fpin.good() )
    {

        fpin >> inputString;

        if (inputString == "")
            continue;

        else if ( inputString == "--" )
        {
            termIdManager.getTermIdListByTermStringList(inputStringList, termIdList);
            inputStringList.clear();
        } // end - else if
        else
        {
            UString inputUString(inputString, UString::CP949);
            inputStringList.push_back( inputUString );
        }

    } // end - while
    fpin.close();

    cerr << "OK" << endl;
} // TESTCASE 5

BOOST_AUTO_TEST_SUITE_END()

