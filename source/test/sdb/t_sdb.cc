#include <sdb/SequentialDB.h>
//#include <am/skip_list_file/SkipListFile.h>
//#include <am/btree/BTreeFile.h>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace izenelib::sdb;

typedef string Key;
typedef DataType<Key,NullType> MyDataType;

static const char* indexFile = "test_sdb.dat";
static string inputFile = "test.txt";
static int degree = 2;
static size_t cacheSize = 1000000;

enum {BTREE=0, TCHASH, HASH, OBTREE, SKIPLIST};
static int container = 5;

//typedef BTreeFile<Key, NullType, NullLock> BTF;
typedef sdb_btree<Key, NullType,NullLock> SBTREE;
//typedef SkipListFile<Key, NullType, NullLock> SLF;
typedef sdb_hash<Key, NullType, NullLock> SHASH;
typedef tc_hash<Key, NullType, NullLock> THASH;

//typedef SequentialDB<Key, NullType, NullLock, BTF> SDB_BT;
//typedef SequentialDB<Key, NullType, NullLock,SLF> SDB_SL;
typedef SequentialDB<Key, NullType, NullLock,SHASH> SDB_HASH;
typedef SequentialDB<Key, NullType, NullLock,SBTREE> SDB_BTREE;
typedef SequentialDB<Key, NullType, NullLock,THASH> SDB_TCHASH;

typedef unordered_sdb<Key, NullType> UNORDERED_SDB;
typedef ordered_sdb<Key, NullType> ORDERED_SDB;


static bool trace = 0;
static int op = 31;

static void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_psdb [-T <trace_option>] [-degree <degree>] [-op <1~15> ][-index <index_file>] [-cache <cache_size>.] [-db <tc|btree|hash|skiplist||obtree>.] <input_file>\n\n";

	cout
			<<"Example: /t_psdb -T 1 -degree 2  -index sdb.dat -cache 10000 wordlist.txt\n";

	cout<<"\nDescription:\n\n";
	cout
			<<"It will read from {input_file} and take the input words as the input keys to do testing.\n";

	cout<<"\nOption:\n\n";

	cout<<"-T <trace_option>\n";
	cout<<"	If set true, print out progress messages to console.\n";

	cout<<"-degree <degree>\n";
	cout<<"	set the minDegree of the B tree or directorySize of sdb_hash\n";

	cout<<"-index <index_file>\n";
	cout<<"the storage file of the B tree, default is sdb.dat.\n";
}

namespace izenelib {
namespace am {
namespace util {

template<> inline void read_image<MyDataType>(MyDataType& dat,
		const DbObjPtr& ptr) {
	dat.key = (string)((char*)ptr->getData() );
}

template<> inline void write_image<MyDataType>(const MyDataType& dat,
		DbObjPtr& ptr) {
	Key key = dat.get_key();
	ptr->setData(key.c_str(), key.size()+1);
}

}
}
}

template<typename T> void run(T& cm) {
	cout<<"op="<<op<<endl;
	if (op & 1)
		run_insert(cm);
	if (op & 2)
		run_get(cm);
	if (op & 4)
		run_seq(cm);
	if (op & 8)
		run_del(cm);
	cm.flush();
}

//ofstream  outf("unique.out");
template<typename T> void run_insert(T& cm) {
	int sum =0;
	int hit =0;
	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;

	while (inf>>ystr) {
		//cout<<"input ="<<ystr<<" "<<ystr.get_key()<<endl;		
		sum++;
		MyDataType dat(ystr);
		if (cm.insertValue(dat) ) {
			hit++;
		}
		if (trace) {
			cout<<" After insert: key="<<ystr<<endl;
			cm.display();
			cout<<"\nnumItem: "<<cm.numItems()<<endl;
		}
	}
	cm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	cm.display();
	cout<<"\nnumItem: "<<cm.numItems()<<endl;
}

template<typename T> void run_get(T& cm) {
	int sum =0;
	int hit =0;
	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;

	while (inf>>ystr) {
		sum++;
		MyDataType dat(ystr);
		if (cm.getValue(ystr, dat) ) {
			hit++;
		}
		if (trace) {
			cout<<" After insert: key="<<ystr<<endl;
			cm.display();
			cout<<"\nnumItem: "<<cm.numItems()<<endl;
		}
	}
	cm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	cm.display();
	cout<<"\nnumItem: "<<cm.numItems()<<endl;
}

template<typename T> void run_seq(T& cm) {

	clock_t t1 = clock();

	vector<MyDataType> result;
	typename T::SDBCursor locn;
	locn = cm.get_first_locn();
	MyDataType dat;
	int a=0;
	while (cm.get(locn, dat) ) {
		a++;
		result.push_back(dat);
		if( cm.seq(locn) )
			break;
		if (trace)
			cout<<dat.key<<endl;
	}
	cout<<"\nseq testing...."<<endl;
	for (unsigned int i=0; i<result.size(); i++) {
		if (trace) {
			cout<<result[i].get_key()<<endl;
		}
	}
	cm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	cm.display();
	cout<< "finish getValue "<<endl;

}

template<typename T> void run_del(T& cm) {

	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;
	//int a = 100000;
	while (inf>>ystr) {
		cm.del(ystr) ;
		if (trace) {
			cout<< "after delete: key="<<ystr<<endl;
			cout<<"\nnumItem: "<<cm.numItems()<<endl;
			cm.display();
		}
	}
	cm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	cm.display();
	cout<<"\nnumItem: "<<cm.numItems()<<endl;

}

//int main(int argc, char *argv[])
BOOST_AUTO_TEST_CASE(t_sdb)
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
			} else if (str == "op") {
				op = atoi(*argv++);
			} else if (str == "cache") {
				cacheSize = atoi(*argv++);
			} else if (str == "index") {
				indexFile = *argv++;
			} else if (str == "db") {
				str = *argv++;
				if (str == "skiplist") {
					container = SKIPLIST;
				} else if (str == "btree") {
					container = BTREE;
				} else if (str == "obtree") {
					container = OBTREE;
				} else if (str == "tc") {
					container = TCHASH;
				} else {
					container = HASH;
				}

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

		//if(container == BTREE)
		{
			SDB_BTREE sdb1(indexFile);
			sdb1.setDegree(degree);		
			sdb1.setCacheSize(cacheSize);
			sdb1.open();
			run(sdb1);
		}
		/*else if(container == SKIPLIST) {
			SDB_SL sdb2(indexFile);
			sdb2.setPageSize(50);
			sdb2.setCacheSize(cacheSize);
			sdb2.open();
			run(sdb2);
		} else if(container == OBTREE) {
			SDB_BT sdb3(indexFile);
			sdb3.setPageSize(20);
			sdb3.setDegree(degree);
			sdb3.open();
			run(sdb3);
		}*/
		//else if(container == TCHASH)
		{
			SDB_TCHASH sdb(indexFile);
			sdb.open();
			run(sdb);
		}
		//else //if(container == HASH) 
		{
			UNORDERED_SDB sdb4(string("unordered_")+indexFile);			
			sdb4.open();
			run(sdb4);
			
			ORDERED_SDB sdb5(string("ordered_")+indexFile);			
			sdb5.open();
			run(sdb5);
		}

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
