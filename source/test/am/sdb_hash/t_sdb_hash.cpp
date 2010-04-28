#include <boost/memory.hpp>
#include <string>
#include <ctime>
//#include <time.h>

#include <am/sdb_hash/sdb_hash.h>
#include <am/sdb_hash/sdb_fixedhash.h>
#include <am/am_test/am_test.h>

using namespace std;
using namespace izenelib::am;
using namespace izenelib::am_test;

const char* indexFile = "sdb.dat";
static string inputFile = "test.txt";
static size_t bucketSize = 512;
//static size_t directorySize = 8192;
static size_t degree = 16;
static size_t cacheSize = 500000;
static int num = 1000000;

static bool trace = 1;
static bool rnd = 0;
static bool ins = 1;

typedef int KeyType;
typedef float ValueType;
typedef izenelib::am::DataType<KeyType, ValueType> DataType;
typedef izenelib::am::DataType<KeyType, ValueType> myDataType;
//typedef izenelib::am::sdb_hash<KeyType, ValueType> SDB_HASH;
typedef izenelib::am::sdb_fixedhash<KeyType, ValueType> SDB_HASH;

template<typename T> void validate_test(T& tb) {

	assert(tb.insert(1, 3) == true);
	assert(tb.insert(2, 7) == true);
	assert(tb.insert(3, 9) == true);

	int val=0.0;
	tb.get(1, val);
	cout<<val<<endl;
	assert(val == 3);
	tb.get(2, val);
	cout<<val<<endl;
	assert(val == 7);
	assert(tb.get(4, val) == false);

	tb.update(1, 5);
	tb.get(1, val);
	cout<<val;
	assert(val == 5);

	tb.del(2);
	assert(tb.get(2, val) == false);

}

template<typename T> void open_test(T& tb) {

  tb.open();
  insert_test(tb);
  tb.close();
  tb.open();
  insert_test(tb);
  tb.close();
  tb.open();
}


template<typename T> void insert_test(T& tb) {
	clock_t start, finish;
	start = clock();
	for (int i=0; i<num; i++) {
		if (trace) {
			cout<<"insert key="<<i<<endl;
		}
		ValueType val = 0.001;
		//tb.insert(i, str);
		tb.insert(i, val);
		if (trace) {
			cout<<"numItem: "<<tb.num_items()<<endl<<endl;
			//tb.display();
		}		
		if(i  > 0 && i % 1000000 == 0){
			displayMemInfo();
			tb.flush();
			displayMemInfo();
		}
			
		//cout<<"\nafte insert ...\n";		
	}
	cout<<"mumItem: "<<tb.num_items()<<endl;
	printf("\nIt takes %f seconds before flush()\n", (double)(clock() - start)
			/CLOCKS_PER_SEC);
	if (trace)
		tb.display();
	tb.flush();
	displayMemInfo();
	if (trace)
		tb.display();
	finish = clock();
	printf("\nIt takes %f seconds to insert %d  data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, num);   
}

/*
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
 */

template<typename T> void search_test(T& tb) {

	clock_t start, finish;
	ValueType v;
	start = clock();
	int c, b;
	c=b=0;

	//num = 5;
	for (int i=0; i<num; i++) {
		if (trace)
			cout<<"finding "<<i<<endl;
		//char p[20];
		//sprintf(p, "%08d", i);
		//string str = p;
		bool ret = tb.get(i, v);
		if (ret) {
			if (trace) {
				cout<<i<<" found"<<endl;
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
	for (int i=0; i<num/2; i++) {
		if (trace)
			cout<<"del "<<i<<endl;
		//char p[20];
		//sprintf(p, "%08d", i);
		//string str = p;
		if (tb.del(i) != 0)
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

/*
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

 }*/

template<typename T> void seq_test(T& tb) {

	//tb.display(cout, false);

	clock_t start, finish;

	start = clock();
	SDB_HASH::SDBCursor locn;
	locn = tb.get_first_locn();
	myDataType dat;
	int a=0;
	while ( tb.get(locn, dat) ) {
		a++;
		tb.seq(locn);
		if (trace)
			cout<<dat.key<<endl;
	}
	cout<<"end at "<<dat.key<<endl;
	tb.flush();
	finish = clock();
	printf("\nIt takes %f seconds to sequential Access %d  data! \n",
			(double)(finish - start) / CLOCKS_PER_SEC, a);

}

template<typename T> void run(T& tb) {
	if (rnd == 0) {
		insert_test(tb);
	search_test(tb);
		seq_test(tb);
		delete_test(tb);
		seq_test(tb);
	} else {
		seq_test(tb);
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
		//{
		//izenelib::am::sdb_fixedhash<int, int>  t1("test.dat");
		//t1.open();
		//validate_test(t1);
		//}

		SDB_HASH tb(indexFile);
		//tb.setDirectorySize(directorySize);
		tb.setDegree(degree);
		tb.setBucketSize(bucketSize);
		tb.setCacheSize(cacheSize);
		tb.open();
		izenelib::util::ClockTimer timer;
		tb.fillCache();
		printf(" elapsed : %lf seconds\n", timer.elapsed() );
		//open_test(tb);
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
