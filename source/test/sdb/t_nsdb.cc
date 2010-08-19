#include <sdb/SequentialDB.h>
#include <boost/test/unit_test.hpp>
//#include <am/btree/BTreeFile.h>

using namespace std;

using namespace izenelib::am;

using namespace izenelib::sdb;

typedef string Key;
typedef DataType<string> Value;

namespace izenelib {
namespace am {
namespace util {

template<> inline void read_image<Value>(Value& dat, const DbObjPtr& ptr) {
	dat.key = (Key)((char*)ptr->getData() );
}
;

template<> inline void write_image<Value>(const Value& dat, DbObjPtr& ptr) {
	Key key = dat.get_key();
	ptr->setData(key.c_str(), key.size()+1);
}
;

}
}
}

static const char* indexFile = "1s13db.dat";
static string inputFile = "test.txt";
static int degree = 2;
static size_t pageSize = 1024;
static size_t cacheSize = 1000000;

static int num = 1000000;
static bool rnd = 0;

typedef SequentialDB<Key, NullType, NullLock> SDB;

//typedef BTreeFile<Key, NullType, ReadWriteLock> BTF;
//typedef SequentialDB<Key, NullType, ReadWriteLock, BTF> SDB;

static bool trace = 0;

/* pseudo random number generator */
inline int myrand(void) {
	static int cnt = 0;
	return (lrand48() + cnt++) & 0x7FFFFFFF;
}

static void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_sdb [-T <trace_option>] [-degree <degree>] [-index <index_file>] [-n number] [-rnd <true|false>] [-cache <cache_size>.] <input_file>\n\n";

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
	run_test1(cm);
	run_test2(cm);
	run_test4(cm);
	run_test3(cm);
	//run_insert(cm);
	//run_getValue(cm);
	//run_del(cm);
}

template<typename T> void run_test1(T& cm) {

	//time_t start = time(0);
	clock_t t1 = clock();
	int ret;
	for (int i =0; i<num; i++) {
		char p[20];
		if (!rnd) {
			sprintf(p, "%08d", i);
		} else {
			sprintf(p, "%08d", myrand()% num+1);
		}
		string str = p;
		Value val(p);
		/* Store a key/data pair. */
		if ((ret = cm.insertValue(val)) == 0)
			;
		//printf("db: %s: key stored.\n", (char *)key.data);
		else {
			// goto err;
		}
	}
	cout<<"before commit"<<endl;
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	//cm.commit();
	cm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	//printf("eclipse: %ld seconds\n", time(0)- start);
	cout<<"\nnumItem: "<<cm.numItems()<<endl;
}

template<typename T> void run_test2(T& cm) {
	clock_t t1 = clock();
	//time_t start = time(0);

	for (int i =0; i<num; i++) {
		char p[20];
		if (!rnd) {
			sprintf(p, "%08d", i);
		} else {
			sprintf(p, "%08d", myrand()% num+1);
		}
		string str = p;
		Value res;
		//YString res;
		/* Store a key/data pair. */
		if (cm.getValue(str, res) == 1) {
			if (trace)
				cout<<str<<endl;
		}
		//printf("db: %s: key stored.\n", (char *)key.data);
		else {
			// goto err;
		}
	}
	cm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	//printf("eclipse: %ld seconds\n", time(0)- start);
	cout<<"\nnumItem: "<<cm.numItems()<<endl;
}

template<typename T> void run_test3(T& cm) {
	clock_t t1 = clock();
	//time_t start = time(0);
	int ret;
	for (int i =0; i<num/2; i++) {
		char p[20];
		if (!rnd) {
			sprintf(p, "%08d", i);
		} else {
			sprintf(p, "%08d", myrand()% num+1);
		}
		string str = p;
		//YString str = p;
		//YString res;
		/* Store a key/data pair. */
		if ((ret = cm.del(str)) == 0)
			;
		//printf("db: %s: key stored.\n", (char *)key.data);
		else {
			// goto err;
		}
	}
	cm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	//printf("eclipse: %ld seconds\n", time(0)- start);
	cout<<"\nnumItem: "<<cm.numItems()<<endl;
}

template<typename T> void run_test4(T& cm) {
	clock_t t1 = clock();
	//time_t start = time(0);	

	vector<Value> result;
	int count = cm.numItems()+10;

	result.clear();
	cm.getValueForward(count, result);
	cout<<"\ngetvalue forward testing...."<<endl;
	cout<<result.size();
	for (unsigned int i=0; i<result.size(); i++) {
		if (trace) {
			cout<<result[i].key<<endl;
		}
	}
	result.clear();
	cm.getValueBackward(count, result);
	cout<<"\ngetvalue Backward testing...."<<endl;;
	for (unsigned int i=0; i<result.size(); i++) {
		if (trace) {
			cout<<result[i].key<<endl;
		}
	}

	while (1) {
		result.clear();
		
		/*Key key;
		cout<<"input key:" <<endl;
		cin>>key;
		cout<<"getNext: "<<cm.getNext(key)<<endl;
		cout<<"getPrev: "<<cm.getPrev(key)<<endl;
		cout<<"getNeareast: "<<cm.getNearest(key)<<endl;	*/	
		
		//cm.display();
		
		Key lowKey;
		Key highKey;
		cout<<"input lowKey:" <<endl;
		cin>>lowKey;
	
		if(lowKey == "exit")break;
		
		cout<<"input highKey:" <<endl;
		cin>>highKey;		
	

		result.clear();
		cm.getValueForward(count, result, lowKey);
		cout<<"\ngetvalue forward testing...."<<endl;;
		for (unsigned int i=0; i<result.size(); i++) {
			if (trace) {
				cout<<result[i].key<<endl;
			}
		}

		result.clear();
		cm.getValueBackward(count, result, highKey);
		cout<<"\ngetvalue Backward testing...."<<endl;;
		for (unsigned int i=0; i<result.size(); i++) {
			if (trace) {
				cout<<result[i].key<<endl;
			}
		}

		result.clear();
		cm.getValueBetween(result, lowKey, highKey);
		cout<<"\ngetvalue between testing...."<<endl;;
		for (unsigned int i=0; i<result.size(); i++) {
			if (trace) {
				cout<<result[i].key<<endl;
			}
		}

	}

	cm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	//printf("eclipse: %ld seconds\n", time(0)- start);
	cout<<"\nnumItem: "<<cm.numItems()<<endl;
}

//int main(int argc, char *argv[]) 
BOOST_AUTO_TEST_CASE(t_nsdb)
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
			} else if (str == "n") {
				num = atoi(*argv++);
			} else if (str == "rnd") {
				rnd = atoi(*argv++);
			} else if (str == "degree") {
				degree = atoi(*argv++);
			} else if (str == "page") {
				pageSize = atoi(*argv++);
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
	}
*/
	try {
		SDB sdb(indexFile);
		sdb.setDegree(degree);
		sdb.setPageSize(pageSize);
		sdb.setCacheSize(cacheSize);
		sdb.open();
		run(sdb);
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
