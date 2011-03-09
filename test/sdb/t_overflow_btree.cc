#include <sdb/SequentialDB.h>
#include <util/ProcMemInfo.h>
#include <boost/test/unit_test.hpp>

//#include  "YString.h"

using namespace std;
//using namespace ylib;
using namespace izenelib::sdb;

typedef string YString;
typedef int Key;
typedef YString Value;

static const char* indexFile = "osdb34.dat";
static string inputFile = "test.txt";
static int degree = 12;
static size_t cacheSize = 100000;
//static size_t dataSize = 50;
static size_t pageSize = 1024;
typedef SequentialDB<Key, Value, NullLock> SDB;

void displayMemInfo(std::ostream& os = std::cout) {
	unsigned long vm = 0, rss = 0;
	ProcMemInfo::getProcMemInfo(vm, rss);
	os << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
}

static bool trace = 0;

static void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_sdb [-T <trace_option>] [-degree <degree>] [-page <pageSize>] [-index <index_file>]  [-dsize <data_size>] [-cache <cache_size>] <input_file>\n\n";

	cout
			<<"Example: /t_Cachedb -T 1 -degree 2  -index sdb.dat -cache 10000 wordlist.txt\n";

	cout<<"\nDescription:\n\n";
	cout
			<<"It will read from {input_file} and take the input words as the input keys to do testing.\n";

	cout<<"\nOption:\n\n";

	cout<<"-T <trace_option>\n";
	cout<<"	If set true, print out progress messages to console.\n";

	cout<<"-degree <degree>\n";
	cout<<"	set the minDegree of the B tree\n";

	cout<<"-index <index_file>\n";
	cout<<"the storage file of the B tree, default is sdb.dat.\n";
}

template<typename T> void run(T& cm) {
	run_insert(cm);
	//	run_getValue(cm);
	//	run_del(cm);
}

//ofstream  outf("unique.out");
template<typename T> void run_insert(T& cm) {
	int sum =0;
	int hit =0;
	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	YString ystr;

	int cnt = 0;
	while (inf>>ystr) {
		//cout<<"input ="<<ystr<<endl;	
		
		for (int i=0; i<12; i++) {
			if (cnt++ % 10000 == 0) {
				std::cout<<"idx="<<cnt<<std::endl;
				displayMemInfo();
				std::cout<<std::endl;
			}

			ystr = ystr + ystr;

			sum++;
			if (cm.insertValue(cnt, ystr) ) {
				hit++;

			} else {
				//cout<<"input ="<<ystr<<" "<<ystr.get_key()<<endl;	
				//cout<<"\nnot hit\n";
				//outf<<ystr<<endl;
			}
		}
		if (trace) {
			cout<<" After insert: key="<<cnt<<endl;
			//cm.display();
			//cm.display();
			cout<<"\nnumItem: "<<cm.numItems()<<endl;
		}
	}
	cm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	//printf("eclipse: %ld seconds\n", time(0)- start);
	cout<<"\nnumItem: "<<cm.numItems()<<endl;

}
/*
 template<typename T> void run_getValue(T& cm) {

 clock_t t1 = clock();
 int count = cm.numItems();

 ifstream inf(inputFile.c_str());
 vector< DataType<YString> > result;
 cm.getValueForward(count, result);
 cout<<"\ngetvalue forward testing...."<<endl;;
 for (int i=0; i<count; i++) {
 if (trace) 
 {
 cout<<result[i].get_key()<<endl;
 }
 }

 cout<<"-------------"<<endl;
 result.clear();
 cm.getValueBackward(count, result);
 cout<<"\n\nget value backward testing...."<<endl;;
 for (int i=0; i<count; i++) {
 //if (trace)
 {
 cout<<result[i].get_key()<<endl;
 }
 }

 cout<<"-------------"<<endl;
 result.clear();
 cm.getValueBackward(3, result, "kkkk");
 cout<<"\n\nget value backward testing...."<<endl;;
 for (unsigned int i=0; i<result.size(); i++) {
 //if (trace)
 {
 cout<<result[i].get_key()<<endl;
 }
 }

 cout<<"-------------"<<endl;
 result.clear();
 cout<<"\n\nget value between testing...."<<endl;;
 cm.getValueBetween(result, "a", "b");
 for (unsigned int i=0; i<result.size(); i++) {
 //if (trace)
 {
 cout<<result[i].get_key()<<endl;
 }
 }

 printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
 cout<< "finish getValue "<<endl;
 cm.flush();

 }

 template<typename T> void run_del(T& cm) {

 clock_t t1 = clock();
 cout<<"del testing...\n";

 ifstream inf(inputFile.c_str());
 YString ystr;
 cout<<" here "<<endl;
 int count = 100;
 while (inf>>ystr && count--) {
 cout<<"input ystr="<<ystr<<endl;
 for (int i=0; i<7; i++) {
 ystr = ystr + ystr;
 }
 cout<<ystr<<endl;
 cm.del(ystr.get_key() ) ;
 if (trace) {
 cout<< "after delete: key="<<ystr.get_key()<<endl;
 cout<<"\nnumItem: "<<cm.numItems()<<endl;
 cm.display();
 }

 }
 cm.flush();
 printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
 cout<<"\nnumItem: "<<cm.numItems()<<endl;
 //cm.display1();

 }*/

//int main(int argc, char *argv[])
BOOST_AUTO_TEST_CASE( t_overflow_btree )
{

	/*if (argc < 2) {
		ReportUsage();
		return 0;

	}
	argv++;
	while (*argv != NULL) {
		string str;
		str = *argv++;
		//cout<< str<<endl;

		if (str[0] == '-') {
			str = str.substr(1, str.length());
			if (str == "T") {
				trace = bool(atoi(*argv++));
			} else if (str == "degree") {
				degree = atoi(*argv++);
			} else if (str == "cache") {
				cacheSize = atoi(*argv++);
			} else if (str == "page") {
				pageSize = atoi(*argv++);
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
	//	try
	{
		SDB sdb(indexFile);

		sdb.setDegree(degree);
		sdb.setPageSize(pageSize);
		sdb.setCacheSize(cacheSize);
		sdb.open();

		run(sdb);

	}
	/*	catch(SDBException& e)
	 {
	 cout<<e.what()<<endl;
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
	 }*/

}
