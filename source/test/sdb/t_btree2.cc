#include <sdb/SequentialDB.h>

using namespace std;
using namespace izenelib::sdb;

typedef int Key;
typedef int Value;
//typedef map<int, int> Value;
typedef DataType<Key,Value> MyDataType;

const char* indexFile = "test_sdb.dat";
static string inputFile = "test.txt";
static int degree = 2;
static size_t cacheSize = 1000000;
static size_t pageSize = 1024;

typedef SequentialDB<Key, Value, ReadWriteLock> SDB;

typedef ordered_sdb<Key, Value, ReadWriteLock> SDB1;

typedef ordered_sdb_fixed<Key, Value, ReadWriteLock> SDB2;

bool trace = 0;

/*
 namespace izenelib {
 namespace am {
 namespace util {

 template<> inline void read_image<MyDataType>(MyDataType& dat, const DbObjPtr& ptr) {
 dat.key = (YString)((char*)ptr->getData() );
 }

 template<> inline void write_image<MyDataType>(const MyDataType& dat, DbObjPtr& ptr) {
 Key key = dat.get_key();
 ptr->setData(key.c_str(), key.size()+1);
 }

 }
 }
 }*/

void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_sdb [-T <trace_option>] [-degree <degree>] [-page <pageSize>][-index <index_file>] [-cache <cache_size>.] <input_file>\n\n";

	cout
			<<"Example: /t_sdb -T 1 -degree 2  -index sdb.dat -cache 10000 wordlist.txt\n";

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
	//run_getValue(cm);
	//run_del(cm);
}

//ofstream  outf("unique.out");
/*
template<typename T> void run_insert1(T& cm) {
	int sum =0;
	int hit =0;
	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;

	while (inf>>ystr) {
		//cout<<"input ="<<ystr<<" "<<ystr.get_key()<<endl;		
		sum++;
		Value val;
		int b=random()%100;
		for (int a=0; a<b; a++) {
			val.insert(make_pair(a, b));
		}
		if (cm.insertValue(b, val) ) {
			hit++;

		} else {
			//cout<<"input ="<<ystr<<" "<<ystr.get_key()<<endl;	
			//cout<<"\nnot hit\n";
			//outf<<ystr<<endl;
		}
		if (trace) {
			cout<<" After insert: key="<<ystr<<endl;
			cm.display();
			//cm.display();
			cout<<"\nnumItem: "<<cm.numItems()<<endl;
		}
	}
	cm.commit();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	//printf("eclipse: %ld seconds\n", time(0)- start);
	cout<<"\nnumItem: "<<cm.numItems()<<endl;

}*/

template<typename T> void run_insert(T& cm) {
	int sum =0;
	int hit =0;
	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;

	int num = 1000000;
	int i=0;
	while (i++ < num) {
		//cout<<"input ="<<ystr<<" "<<ystr.get_key()<<endl;		
		sum++;
		Value val;

		if (cm.insertValue(i, i) ) {
			hit++;

		} else {
			//cout<<"input ="<<ystr<<" "<<ystr.get_key()<<endl;	
			//cout<<"\nnot hit\n";
			//outf<<ystr<<endl;
		}
		if (trace) {
			cout<<" After insert: key="<<ystr<<endl;
			cm.display();
			//cm.display();
			cout<<"\nnumItem: "<<cm.numItems()<<endl;
		}
	}
	cm.commit();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	//printf("eclipse: %ld seconds\n", time(0)- start);
	cout<<"\nnumItem: "<<cm.numItems()<<endl;

}

/*

 template<typename T> void run_getValue(T& cm) {

 clock_t t1 = clock();
 int count = cm.numItems();

 ifstream inf(inputFile.c_str());
 vector<MyDataType> result;
 cm.getValueForward(count, result, "");
 cout<<"\ngetvalue forward testing...."<<endl;;
 for (int i=0; i<count; i++) {
 if (trace) {
 cout<<result[i].get_key()<<endl;
 }
 }

 /*	cout<<"-------------"<<endl;
 result.clear();
 cm.getValueBackward(count, result, "");
 cout<<"\n\nget value backward testing...."<<endl;;
 for (int i=0; i<count; i++) {
 if (trace) {
 cout<<result[i]<<endl;
 }
 }

 cout<<"-------------"<<endl;
 result.clear();
 cm.getValueBackward(3, result, "kkkk");
 cout<<"\n\nget value backward testing...."<<endl;;
 for (int i=0; i<3; i++) {
 if (trace) {
 cout<<result[i]<<endl;
 }
 }*

 printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
 cout<< "finish getValue "<<endl;

 }

 template<typename T> void run_del(T& cm) {

 clock_t t1 = clock();

 ifstream inf(inputFile.c_str());
 YString ystr;	
 while (inf>>ystr) {
 cm.del( ystr  ) ;
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
	}
	try
	{
		{
			SDB sdb(indexFile);
			sdb.setDegree(degree);
			sdb.setPageSize(pageSize);
			sdb.setCacheSize(cacheSize);
			sdb.open();
			run(sdb);
		}

		{
			SDB1 sdb(indexFile+string("1"));
			sdb.setDegree(degree);
			sdb.setPageSize(pageSize);
			sdb.setCacheSize(cacheSize);
			sdb.open();
			run(sdb);
		}

		{
			SDB2 sdb(indexFile+string("2"));
			sdb.setDegree(degree);
			sdb.setPageSize(pageSize);
			sdb.setCacheSize(cacheSize);
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
