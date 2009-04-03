#include <sdb/SequentialDB.h>

using namespace std;
using namespace izenelib::sdb;

/* pseudo random number generator */
inline int myrand(void) {
	static int cnt = 0;
	return (lrand48() + cnt++) & 0x7FFFFFFF;
}

template<class T> struct MyKeyType {
	friend class boost::serialization::access;
	T key; //not use ystring here, for ystring has no serialize member.
	int data;

	MyKeyType() {
	}

	MyKeyType(T& key1, int& data1) :
		key(key1), data(data1) {

	}

	//must be const now
	const string get_key() const {
		return key;
	}

	inline int compare(const MyKeyType& other) const {
		return key.compare(other.key);
	}

	template<class Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & key;
		ar & data;
	}

	void display() const {
		cout<<key<<" "<<data<<" ";
	}

};

template<> struct MyKeyType<int> {
	friend class boost::serialization::access;
	int key; //not use ystring here, for ystring has no serialize member.
	int data;

	MyKeyType() {
	}

	MyKeyType(int& key1, int& data1) :
		key(key1), data(data1) {

	}
	//must be const now
	const int get_key() const {
		return key;
	}

	inline int compare(const MyKeyType& other) const {
		return key -other.key;
	}

	template<class Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & key;
		ar & data;
	}

	void display() const {
		cout<<key<<" "<<data<<" ";
	}

};

typedef MyKeyType<string> Key;

typedef NullType Value;

const char* indexFile = "tsdb.dat";
static string inputFile = "test.txt";
static int degree = 12;
static size_t cacheSize = 1000000;
static size_t pageSize = 1024;
static bool rnd = 0;
static string dbType = "btree";

static int num = 1000000;

typedef SequentialDB<Key, Value, NullLock> SDB;

typedef sdb_btree<Key, Value, NullLock> SBTREE;
typedef sdb_hash<Key, Value, NullLock> SHASH;

typedef SequentialDB<Key, Value, NullLock, SHASH> SDB_HASH;
typedef SequentialDB<Key, Value, NullLock, SBTREE> SDB_BTREE;

typedef DataType<Key,Value> myDataType;

bool trace = 0;

void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_sdb [-T <trace_option>] [-degree <degree | directorySize>] [-page <pageSize | bucketSize>] [-db <btree|hash>]  [-n <num>] [-index <index_file>] [-cache <cache_size>.] <input_file>\n\n";

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
	//run_test2(cm);
	//run_test4(cm);
	//run_test3(cm);
	//run_insert(cm);
	//run_get(cm);
}

template<typename T> void run_test1(T& cm) {

	//time_t start = time(0);

	clock_t t1 = clock();
	int ret;
	string ystr;

	for (int i=0; i<num; i++) {
		char p[20];
		if (!rnd) {
			sprintf(p, "%08d", i);
		} else {
			sprintf(p, "%08d", myrand()% num+1);
		}	
		ystr = p;
		MyKeyType<string> key(ystr, i);
		NullType value;

		//DataType dat(key, value);
		/* Store a key/data pair. */
		if ((ret = cm.insertValue(key, value)) == 0)
			;
		//printf("db: %s: key stored.\n", (char *)key.data);
		else {
			// goto err;
		}
	}
	//cm.flush();
	cm.close();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	//printf("eclipse: %ld seconds\n", time(0)- start);
	cout<<"\nnumItem: "<<cm.numItems()<<endl;
}

int main(int argc, char *argv[]) {

	if (argc < 2) {
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
			} else if (str == "rnd") {
				rnd = atoi(*argv++);
			} else if (str == "cache") {
				cacheSize = atoi(*argv++);
			} else if (str == "page") {
				pageSize = atoi(*argv++);
			} else if (str == "index") {
				indexFile = *argv++;
			} else if (str == "db") {
				dbType = *argv++;
			} else if (str == "n") {
				num = atoi(*argv++);
			} else {
				cout<<"Input parameters error\n";
				return 0;
			}
		} else {
			inputFile = str;
			break;
		}
	}
	try
	{
		if( dbType.compare("btree") == 0) {
			SDB sdb(indexFile);
			sdb.setCacheSize(cacheSize);
			sdb.setDegree(degree);
			sdb.setPageSize(pageSize);
			sdb.open();
			run(sdb);
		} else
		{
			SDB_HASH sdb(indexFile);
			sdb.setCacheSize(cacheSize);
			sdb.setDirectorySize(degree);
			sdb.setBucketSize(pageSize);
			sdb.open();
			run(sdb);
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
