#include <sdb/TrieIndexSDB.h>
#include <iostream>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace izenelib::sdb;
using namespace izenelib::util;

static const char* indexFile = "indexsdb.dat";
static string inputFile = "test.txt";
static int degree = 8;
static size_t cacheSize = 1000000;
static bool trace = 0;

static void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_IndexSDB [-T <trace_option>] [-degree <degree>] [-index <index_file>] [-dataSize <maxDataSize> ] [-cache <cache_size>.] <input_file>\n\n";

	cout
			<<"Example: /t_IndexSDB -T 1 -degree 2  -index sdb.dat -dataSize 100 -cache 10000 wordlist.txt\n";

	cout<<"\nDescription:\n\n";
	cout
			<<"It will read from <input_file> and take the input words as the input keys to do testing.\n";

	cout<<"\nOption:\n\n";

	cout<<"-T <trace_option>\n";
	cout<<"	If set true, print out progress messages to console.\n";

	cout<<"-degree <degree>\n";
	cout<<"	set the minDegree of the B tree\n";

	cout<<"-dataSize <MaxDataSize>\n";
	cout<<"	set the pageSize.\n";

	cout<<"-index <index_file>\n";
	cout<<"the storage file of the B tree, default is sdb.dat.\n";
}

template< typename V> void run(V& dm) {
	run_insert(dm);
	run_get(dm);
	//run_getValue(dm);
	//run_del(dm);
}

template< typename V> void run_insert(V& dm) {

	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;

	int a=0;
	while (inf>>ystr) {
		a++;
		if (trace)
			cout<<" After insert: key="<<ystr<<endl;
		if (a%10000 == 0)
			cout<<"idx="<<a<<endl;
		dm.add_suffix(izenelib::util::UString(ystr, UString::CP949), izenelib::util::UString(
				ystr, UString::CP949) );

		/*size_t pos = 0;
		 for (; pos<ystr.size(); pos++) {
		 string suf = ystr.substr(pos);
		 cout<<"add: "<<suf<<"->"<<ystr<<endl;
		 dm.add(suf, ystr);
		 }*/

		if (trace) {
			//cout<<" After insert: key="<<ystr<<endl;
			dm.display();
		}
	}

	//dm.display();
	dm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	if (trace) {
		cout<<"After run_insert, display..."<<endl;
		//dm.display();
	}
}

template< typename V> void run_get(V& dm) {

	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;

	while (1) {
		cout<<"input perfix"<<endl;
		cin>>ystr;
		vector<izenelib::util::UString> result;
		dm.getValuePrefix(izenelib::util::UString(ystr, UString::CP949), result);
		{
			cout<<"\n prefix="<<ystr<<endl;
			cout<<"result size: "<<result.size()<<endl;
			for (size_t i=0; i<result.size(); i++) {
				cout<<result[i]<<" "<<endl;
				cout<<endl;
				//cout<<typeid(result[i]).name();
			}
		}
		cout<<"input suffix"<<endl;
		cin>>ystr;
		vector<izenelib::util::UString> result1;
		dm.getValueSuffix(izenelib::util::UString(ystr, UString::CP949), result1);
		{
			cout<<"\n suffix="<<ystr<<endl;
			cout<<"result size: "<<result1.size()<<endl;
			for (size_t i=0; i<result1.size(); i++) {
				cout<<result1[i]<<" "<<endl;
				cout<<endl;
			}
		}

	}

	/*while (inf>>ystr) {
	 size_t pos = 0;
	 for (; pos<ystr.size(); pos++) {
	 vector<string> result;
	 string suf = ystr.substr(pos);			
	 dm.getValuePrefix(suf, result);			
	 if (trace) {
	 cout<<"\n pre="<<suf<<endl;
	 for (size_t i=0; i<result.size(); i++) {
	 cout<<result[i]<<" "<<endl;
	 cout<<endl;
	 }
	 }
	 }
	 
	 }*/

	//dm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	if (trace) {
		cout<<"After run_insert, display..."<<endl;
		//dm.display();
	}
}

static void test() {

	TrieIndexSDB2<izenelib::util::UString, unsigned int> dm;
	dm.open();
	dm.add_suffix(izenelib::util::UString("abc", UString::CP949), 1);
	dm.add_suffix(izenelib::util::UString("ab3d", UString::CP949), 2);
	dm.add_suffix(izenelib::util::UString("hello word", UString::CP949), 3);
	dm.add_suffix(izenelib::util::UString("word hello", UString::CP949), 4);
	dm.add_suffix(izenelib::util::UString("aabbc4do", UString::CP949), 5);

	vector<unsigned int> result;
	dm.getValuePrefix(izenelib::util::UString("a", UString::CP949), result);
	{
		cout<<"\n prefix="<<"a"<<endl;
		cout<<"result size: "<<result.size()<<endl;
		for (size_t i=0; i<result.size(); i++) {
			cout<<result[i]<<" "<<endl;
			cout<<endl;
			//cout<<typeid(result[i]).name();
		}
	}
	cout<<"input suffix"<<endl;
	vector<unsigned int> result1;
	
	dm.getValueSuffix(izenelib::util::UString("hello", UString::CP949), result1);
	{
		cout<<"\n suffix="<<"hello"<<endl;
		cout<<"result size: "<<result1.size()<<endl;
		for (size_t i=0; i<result1.size(); i++) {
			cout<<result1[i]<<" "<<endl;
			cout<<endl;
		}
	}

}

//int main(int argc, char *argv[]) 
BOOST_AUTO_TEST_CASE(t_trieIndexSDB1)
{

	/*if (argc < 2) {
		ReportUsage();
		return 0;
	}
	argv++;
	while (*argv != NULL) {
		string str;
		str = *argv++;

		if (str[0] == '-') {
			str = str.substr(1, str.length());
			if (str == "T") {
				trace = bool(atoi(*argv++));
			} else if (str == "degree") {
				degree = atoi(*argv++);
			} else if (str == "cache") {
				cacheSize = atoi(*argv++);
			} else if (str == "index") {
				indexFile = *argv++;
			} else {
				cout<<"Input parameters error\n";
				return 0;
			}
		} else {
			inputFile = str;
			break;
		}
	}*/

	try
	{
		//TrieIndexSDB<string, string> sufSDB;
		//sufSDB.open();		
		//run_insert(sufSDB);


		//TrieIndexSDB2<izenelib::util::UString, izenelib::util::UString> sufSDB2;
		//sufSDB2.open();
		//sufSDB.initialize(20, degree, 1024*2, cacheSize);
		//run(sufSDB2);
		test();
	}
	catch(bad_alloc)
	{
		cout<<"Memory allocation error!"<<endl;
	}
	catch(ios_base::failure)
	{
		cout<<"Reading or writing file error!"<<endl;
	}
	catch(...)
	{
		cout<<"OTHER ERROR HAPPENED!"<<endl;
	}

}
