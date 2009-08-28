#include <boost/memory.hpp>
#include <string>
#include <ctime>
//#include <time.h>

#include <am/sdb_hash/sdb_hash.h>

using namespace std;
using namespace izenelib::am;

const char* indexFile = "sdb.dat";
static string inputFile = "test.txt";
static size_t bucketSize = 1024;
//static size_t directorySize = 8192;
static size_t degree = 18;
static size_t cacheSize = 500000;
static int num = 1000000;

static bool trace = 1;
static bool rnd = 0;
static bool ins = 1;

typedef string KeyType;
typedef NullType ValueType;
typedef izenelib::am::DataType<KeyType, NullType> DataType;
typedef izenelib::am::DataType<KeyType, NullType> myDataType;
typedef izenelib::am::sdb_hash<KeyType, NullType> SDB_HASH;


template<typename T> void insert_test(T& tb) {
	clock_t start, finish;
	start = clock();
	for (int i=0; i<num; i++) {
		if (trace) {
			cout<<"insert key="<<i<<endl;
		}
		char p[20];
		sprintf(p, "%08d", i);
		string str = p;
		//tb.insert(i, str);
		tb.insert(str);
		if (trace) {
			cout<<"numItem: "<<tb.num_items()<<endl<<endl;
			//tb.display();
		}
		//cout<<"\nafte insert ...\n";		
	}
	cout<<"mumItem: "<<tb.num_items()<<endl;
	printf("\nIt takes %f seconds before flush()\n", (double)(clock() - start)
			/CLOCKS_PER_SEC);
	if (trace)
		tb.display();
	tb.flush();
	if (trace)
		tb.display();
	finish = clock();
	printf("\nIt takes %f seconds to insert %d  data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, num);

}

template<typename T> void random_insert_test(T& tb) {
	clock_t start, finish;
	start = clock();
	for (int i=0; i<num; i++) {
		int k = rand()%num;
		if (trace) {
			cout<<"insert key="<<k<<endl;
		}
		char p[20];
		sprintf(p, "%08d", k);
		string str = p;
		//tb.insert(rand()%num, str);
		tb.insert(str);
		if (trace) {
			cout<<"numItem: "<<tb.num_items()<<endl<<endl;
			tb.display();
		}
		//cout<<"\nafte insert ...\n";		
	}
	cout<<"mumItem: "<<tb.num_items()<<endl;
	printf("\nIt takes %f seconds before flush()\n", (double)(clock() - start)
			/CLOCKS_PER_SEC);
	if (trace)
		tb.display();
	tb.flush();
	finish = clock();
	printf("\nIt takes %f seconds to insert %d  data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, num);

}

template<typename T> void random_search_test(T& tb) {

	clock_t start, finish;
	ValueType * v;
	start = clock();
	int c, b;
	c=b=0;
	for (int i=0; i<num; i++) {
		int k = rand()%num;
		if (trace)
			cout<<"finding "<<k<<endl;

		char p[20];
		sprintf(p, "%08d", k);
		string str = p;
		v = tb.find(p);
		if (v) {
			delete v;
			v = 0;
			if (trace) {
				cout<<str<<" found"<<endl;
				tb.display();
			}
			c++;
		} else
			b++;

	}
	if (trace)
		tb.display();
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to random find %d data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, num, c, b);

	//  tb.display(std::cout)
}

template<typename T> void search_test(T& tb) {

	clock_t start, finish;
	ValueType* v;
	start = clock();
	int c, b;
	c=b=0;

	//num = 5;
	for (int i=0; i<num; i++) {
		if (trace)
			cout<<"finding "<<i<<endl;
		char p[20];
		sprintf(p, "%08d", i);
		string str = p;
		v = tb.find(str);
		if (v) {
			delete v;
			v = 0;
			if (trace) {
				cout<<str<<" found"<<endl;
				tb.display();
			}
			c++;
		} else
			b++;
	}
	if (trace)
		tb.display();
	//tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to find %d data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, num, c, b);

	//  tb.display(std::cout)
}

template<typename T> void delete_test(T& tb) {

	clock_t start, finish;
	int c, b;
	c=b=0;

	start = clock();
	for (int i=0; i<num; i++) {
		if (trace)
			cout<<"del "<<i<<endl;
		char p[20];
		sprintf(p, "%08d", i);
		string str = p;
		if (tb.del(str) != 0)
			c++;
		else
			b++;
		if (trace) {
			cout<<"numItem: "<<tb.num_items()<<endl;
			tb.display();
		}
	}
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to delete %d data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, num/2, c, b);

}

template<typename T> void random_delete_test(T& tb) {

	clock_t start, finish;
	int c, b;
	c=b=0;

	start = clock();
	for (int i=0; i<num; i++) {
		int k = rand()%num;
		if (trace)
			cout<<"del "<<k<<endl;
		char p[20];
		sprintf(p, "%08d", k);
		string str = p;
		if (tb.del(str) != 0)
			c++;
		else
			b++;
		if (trace) {
			cout<<"numItem: "<<tb.num_items()<<endl;
			tb.display();
		}
	}
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to delete %d  data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, num/2, c, b);

}
template<typename T> void seq_test(T& tb) {

	clock_t start, finish;

	start = clock();
	SDB_HASH::SDBCursor locn;
	locn = tb.get_first_locn();
	myDataType dat;
	int a=0;
	while (tb.seq(locn, dat) ) {
		a++;
		if (trace)
			cout<<dat.key<<endl;
	}

	tb.flush();
	finish = clock();
	printf("\nIt takes %f seconds to sequential Access %d  data! \n",
			(double)(finish - start) / CLOCKS_PER_SEC, a);

}

template<typename T> void run(T& tb) {
	//search_test(tb);
	if (rnd) {
		random_insert_test(tb);
		//random_search_test(tb);
		seq_test(tb);
		//random_delete_test(tb);
	} else {
		insert_test(tb);
		//search_test(tb);
		seq_test(tb);
		//delete_test(tb);
	}

	/*delete_test(tb);
	 search_test(tb);
	 insert_test(tb);
	 search_test(tb);*/
}

void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_slf [-T <trace_option>] [-degree <degree>] [-bkt <bucketSize>] [-rnd <0|1>]  [-n <num>] [-index <index_file>] [-cache <cache_size>.] <input_file>\n\n";

	cout<<"\nDescription:\n\n";
	cout
			<<"It will read from {input_file} and take the input words as the input keys to do testing.\n";

	cout<<"\nOption:\n\n";

	cout<<"-T <trace_option>\n";
	cout<<"	If set true, print out progress messages to console.\n";

	cout<<"-index <index_file>\n";
	cout<<"the storage file of the B tree, default is sdb.dat.\n";
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
			} else if (str == "cache") {
				cacheSize = atoi(*argv++);
			} else if (str == "bkt") {
				bucketSize = atoi(*argv++);
			} else if (str == "index") {
				indexFile = *argv++;
			} else if (str == "n") {
				num = atoi(*argv++);
			} else if (str == "rnd") {
				rnd = bool(atoi(*argv++));
			} else if (str == "ins") {
				ins = bool(atoi(*argv++));
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
		SDB_HASH tb(indexFile);
		//tb.setDirectorySize(directorySize);
		tb.setDegree(degree);
		tb.setBucketSize(bucketSize);
		tb.setCacheSize(cacheSize);
		tb.open();
		run(tb);

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
