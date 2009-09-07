#include <boost/memory.hpp>
#include <string>
#include <ctime>
//#include <time.h>

#include <am/sdb_btree/sdb_btree.h>
#include <am/sdb_btree/sdb_fixedbtree.h>
#include <util/izene_log.h>

using namespace std;
using namespace izenelib::am;

const char* indexFile = "sdb.dat";
static string inputFile = "test.txt";
static size_t maxKeys = 32;
static size_t pageSize = 1024;
static size_t cacheSize = 100000;
static int num = 1000000;

static bool trace = 1;
static bool rnd = 0;
static bool ins = 1;

typedef string KeyType;
typedef NullType ValueType;
typedef izenelib::am::DataType<KeyType, NullType> DataType;
typedef izenelib::am::DataType<KeyType, NullType> myDataType;
typedef izenelib::am::sdb_btree<KeyType, NullType> SDB_BTREE;
//typedef izenelib::am::sdb_fixedbtree<KeyType, NullType> SDB_BTREE;

template<typename T> void insert_test(T& tb) {
	izenelib::util::ClockTimer timer;
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
			cout<<"numItem: "<<tb.num_items()<<"a"<<endl;
			tb.display();
		}
		DLOG_EVERY_N(INFO, 1000000) << getMemInfo();
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
	printf("commit elapsed 1 ( actually ): %lf seconds\n",
					timer.elapsed() );

}

template<typename T> void random_insert_test(T& tb) {
	izenelib::util::ClockTimer timer;
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
		DLOG_EVERY_N(INFO, 1000000) << getMemInfo();
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
	printf("commit elapsed 1 ( actually ): %lf seconds\n",
					timer.elapsed() );

}

template<typename T> void random_search_test(T& tb) {

	clock_t start, finish;
	ValueType v;
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
		bool ret = tb.get(p , v);
		if (ret) {
			if (trace) {
				cout<<str<<" found"<<endl;
				tb.display();
			}
			c++;
		} else
			b++;
		DLOG_EVERY_N(INFO, 1000000) << getMemInfo();
	}
	if (trace)
		tb.display();
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to random find %d random data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, num, c, b);

	//  tb.display(std::cout)
}

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
		char p[20];
		sprintf(p, "%08d", i);
		string str = p;
		bool ret = tb.get(str, v);
		if (ret) {
			if (trace) {
				cout<<str<<" found"<<endl;
				tb.display();
			}
			c++;
		} else
			b++;
		DLOG_EVERY_N(INFO, 100000) << getMemInfo();
	}
	if (trace)
		tb.display();
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to find %d random data! %d data found, %d data lost!\n",
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
			"\nIt takes %f seconds to delete %d random data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, num/2, c, b);

}

template<typename T> void random_delete_test(T& tb) {

	clock_t start, finish;
	int c, b;
	c=b=0;

	start = clock();
	for (int i=0; i<num; i++) {
		int k = rand()%num;
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
			"\nIt takes %f seconds to delete %d random data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, num/2, c, b);

}
template<typename T> void seq_test(T& tb) {

	clock_t start, finish;

	start = clock();
	SDB_BTREE::SDBCursor locn;
	locn = tb.get_first_locn();
	myDataType dat;
	int a=0;
	while (tb.get(locn, dat) ) {
		//cout<<dat.key<<endl;
		a++;
		if( !tb.seq(locn) );
			//break;
		if (trace)
			cout<<dat.key<<endl;
		DLOG_EVERY_N(INFO, 1000000) << getMemInfo();
		//LOG_IF(INFO, (a > 5000000 ) )<<"!!!!!!!! " <<dat.key<<endl;
	}
	cout<<"end at "<<dat.key<<endl;
	//cout<<dat.key<<endl;
	tb.flush();
	finish = clock();
	printf("\nIt takes %f seconds to sequential Access %d random data! \n",
			(double)(finish - start) / CLOCKS_PER_SEC, a);

}

template<typename T> void dump_test(T& tb) {

	clock_t start, finish;
	start = clock();
    tb.dump2f("sdb.dat.bak");    
	finish = clock();
	printf("\nIt takes %f seconds for dump! \n",
			(double)(finish - start) / CLOCKS_PER_SEC);

}


template<typename T> void run(T& tb) {
	//search_test(tb);
	if (rnd) {
		random_insert_test(tb);
		//random_search_test(tb);
		//seq_test(tb);
		//dump_test(tb);
	   //random_delete_test(tb);
		//search_test(tb);
	} else {
		 insert_test(tb);		 
		 search_test(tb);
		 seq_test(tb);	
		 dump_test(tb);
		// delete_test(tb);
		// search_test(tb);
	}

}

void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_slf [-T <trace_option>] [-mkey <maxKeys>] [-page <pageSize>] [-rnd <0|1>]  [-n <num>] [-index <index_file>] [-cache <cache_size>.] <input_file>\n\n";

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
			} else if (str == "mkey") {
				maxKeys = atoi(*argv++);
			} else if (str == "page") {
				pageSize = atoi(*argv++);
			} else if (str == "cache") {
				cacheSize = atoi(*argv++);
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
		SDB_BTREE tb(indexFile);
		tb.setMaxKeys(maxKeys);
		tb.setPageSize(pageSize);
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
