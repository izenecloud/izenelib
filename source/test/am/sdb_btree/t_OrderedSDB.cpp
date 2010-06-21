#include <fstream>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <util/ustring/UString.h>

#include <sdb/SequentialDB.h>
#include <util/hashFunction.h>
//#include <util/ustring/UString.h>


using namespace std;
using namespace izenelib::util;
using namespace boost::unit_test;
//typedef std::string UString;
namespace TestData
{

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
	 * @brief This function calls loadTermList to load a list of terms. The function also
	 * generates id for each term in the list. This function is implemented by TuanQuang Nguyen.
	 * @param
	 * 	termUStringList1 - list of the term string
	 */
	bool generateTermLists(vector<UString>& termUStringList1)
    {
        vector<string> termStringList1;

        // load term list
        if(!loadTermList("./test-data/100WordList.txt", termStringList1) )
            return false;

        // convert term list of strings into term list of UStrings
        termUStringList1.resize(termStringList1.size());
        for(size_t i = 0; i < termStringList1.size(); i++)
            termUStringList1[i].assign(termStringList1[i], UString::CP949);

        return true;

    } // end - generateTermLists()

}

//BOOST_AUTO_TEST_SUITE( t_SDB)

//
//BOOST_AUTO_TEST_CASE( TestCase1 )
int main()
{
    vector<unsigned int> keys;
    vector<UString> values;
    vector<string> values1;
  //  izenelib::sdb::ordered_sdb<unsigned int, UString, izenelib::util::NullLock> sdb("SDB1.sdb");
    sdb_btree<unsigned int, string>  btree("1.sdb#");  
    btree.open();
  //  sdb.open();
    TestData::loadTermList("./test-data/100WordList.txt", values1);
    TestData::generateTermLists(values);
    keys.resize(values.size());
    for(unsigned int i=0; i<values.size(); i++)
    {
        keys[i] = HashFunction<UString>::generateHash32(values[i]);
        //sdb.insertValue(keys[i], values[i]);
        btree.insert(keys[i], values1[i]); 

    }
    btree.display(cout, false);
    for(unsigned int i=0; i<keys.size(); i++)
    {
        string v;    
        if( false == btree.get(keys[i], v) )
            cerr << "find sdb err " << i << " th term: " << values[i] << " , ID" << keys[i] << std::endl;
    }
    return 1;
}

//BOOST_AUTO_TEST_SUITE_END()
