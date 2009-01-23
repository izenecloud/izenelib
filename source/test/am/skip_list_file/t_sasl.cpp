#include <boost/memory.hpp>
#include <string>
#include <ctime>
//#include <time.h>

#include <am/nslf/SkipListFile.h>

using namespace std;
using namespace izenelib::am;

const char* indexFile = "sdb.dat";
static string inputFile = "test.txt";
static int degree = 2;
static size_t cacheSize = 1000000;
static int num = 30;

static bool trace = 1;
static bool rnd = 0;
static bool ins = 1;

typedef string KeyType;
typedef izenelib::am::NullType ValueType;
typedef izenelib::am::DataType<KeyType, NullType> DataType;
typedef izenelib::am::DataType<KeyType, NullType> myDataType;
typedef izenelib::am::SkipListFile<KeyType, izenelib::am::NullType> SLF;

namespace izenelib {
namespace am {
namespace util {

template<> inline void read_image<myDataType>(myDataType& dat, const DbObjPtr& ptr) {
	dat.key = (KeyType)((char*)ptr->getData() );
};

template<> inline void write_image<myDataType>(const myDataType& dat, DbObjPtr& ptr) {
	KeyType key = dat.get_key();
	ptr->setData(key.c_str(), key.size()+1);
};

}
}
}


template<typename T>
void insert_test(T& tb) {
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
			tb.display();
		}
		//cout<<"\nafte insert ...\n";		
	}
	cout<<"mumItem: "<<tb.num_items()<<endl;
	printf("\nIt takes %f seconds before commit()\n", (double)(clock() - start)
			/CLOCKS_PER_SEC);
	if (trace)
		tb.display();
	tb.flush();
	finish = clock();
	printf("\nIt takes %f seconds to insert %d  data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, num);

}

template<typename T>
void random_insert_test(T& tb) {
	clock_t start, finish;
	start = clock();
	for (int i=0; i<num; i++) {
		if (trace) {
			cout<<"insert key="<<i<<endl;
		}
		char p[20];
		sprintf(p, "%08d", rand()%num);
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
	printf("\nIt takes %f seconds before commit()\n", (double)(clock() - start)
			/CLOCKS_PER_SEC);
	if (trace)
		tb.display();
	tb.flush();
	finish = clock();
	printf("\nIt takes %f seconds to insert %d  data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, num);

}

template<typename T>
void random_search_test(T& tb) {

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
			if (trace)
				cout<<str<<" found"<<endl;
			c++;
		} else
			b++;
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

template<typename T>
void  search_test(T& tb) {

	clock_t start, finish;
	ValueType* v;
	start = clock();
	int c, b;
	c=b=0;

	num = 5;
	for (int i=0; i<num; i++) {
		if (trace)
			cout<<"finding "<<i<<endl;
		char p[20];
		sprintf(p, "%08d", i);
		string str = p;
		v = tb.find(str);
		if (v) {
			if (trace)
				cout<<str<<" found"<<endl;
			c++;
		} else
			b++;
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

template<typename T>
void delete_test(T& tb){

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

template<typename T> void run(T& tb) {
	//search_test(tb);
	if (rnd)
		random_insert_test(tb);
	else
		insert_test(tb);
	if (rnd)
		random_search_test(tb);
	else
		search_test(tb);
	//delete_test(tb);
	//search_test(tb);*/
}


void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_slf [-T <trace_option>] [-degree <degree>]  [-n <num>] [-index <index_file>] [-cache <cache_size>.] <input_file>\n\n";

	cout
			<<"Example: /t_slf -T 1 -degree 2  -index sdb.dat -cache 10000 wordlist.txt\n";

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
			} else if (str == "index") {
				indexFile = *argv++;
			} else if (str == "n") {
				num = atoi(*argv++);
			} else if (str == "rnd") {
				rnd = bool(atoi(*argv++));
			}else if (str == "ins") {
				ins = bool(atoi(*argv++));
			}
			else {
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
		SLF tb(indexFile, degree);
		tb.setPageSize(20, 1024);
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
