#include <sdb/TrieIndexSDB.h>
#include <iostream>

using namespace std;
using namespace izenelib::sdb;

const char* indexFile = "indexsdb.dat";
static string inputFile = "test.txt";
static int degree = 8;
static size_t cacheSize = 1000000;
bool trace = 0;


 struct suffix {
 string suf;
 typedef string::value_type value_type;

 suffix() {
 }
 suffix(string& other) :
 suf(other) {
 }
 template<class Archive> void serialize(Archive & ar,
 const unsigned int version) {
 ar & suf;
 }
 int compare(const suffix& other) const {
 return suf.compare(other.suf);

 }
 bool isPrefix(const suffix& other) const {
 //cout<<"Key typeid "<<typeid(other.suf).name()<<endl;
 //cout<<suf<<" vs "<<other.suf<<endl;
 if (other.suf.substr(0, suf.size() ) == suf) {
 return true;
 } else {
 return false;
 }
 }
 void display() const {
 cout<<suf;
 }

 };
 
 TrieIndexSDB<suffix, string> tddd;

void ReportUsage(void) {
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
		if (a%10000 == 0)
			cout<<"idx="<<a<<endl;
		dm.add_suffix(ystr, ystr);
		
		/*size_t pos = 0;
		for (; pos<ystr.size(); pos++) {
			string suf = ystr.substr(pos);
			cout<<"add: "<<suf<<"->"<<ystr<<endl;
			dm.add(suf, ystr);
		}*/

		if (trace) {
			cout<<" After insert: key="<<ystr<<endl;
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
		vector<string> result;
		dm.getValuePrefix(ystr, result);
		{
			cout<<"\n prefix="<<ystr<<endl;
			cout<<"result size: "<<result.size()<<endl;
			for (size_t i=0; i<result.size(); i++) {
				cout<<result[i]<<" "<<endl;
				cout<<endl;
			}
		}
		cout<<"input suffix"<<endl;
		cin>>ystr;
		vector<string> result1;
		dm.getValueSuffix(ystr, result1);
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

int main(int argc, char *argv[]) {

	if (argc < 2) {
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
	}

	try
	{
		//TrieIndexSDB<string, string> sufSDB;
		//sufSDB.open();		
		//run_insert(sufSDB);
		

		TrieIndexSDB2<string, string> sufSDB2;
		sufSDB2.open();
		//sufSDB.initialize(20, degree, 1024*2, cacheSize);
		run(sufSDB2);
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
