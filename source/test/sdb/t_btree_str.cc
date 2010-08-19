#include <boost/test/unit_test.hpp>
#include <sdb/SequentialDB.h>

using namespace izenelib::sdb;

static const char* indexFile = "test_sdb_str.dat";
static string inputFile = "test.txt";

static bool trace = 0;

static void ReportUsage(void) {
	cout <<"\nUSAGE:./t_btree_str [-T <trace_option>]  <input_file>\n\n";

	cout <<"Example: /t_btree_str  wordlist.txt\n";

	cout<<"-T <trace_option>\n";
	cout<<"	If set true, print out progress messages to console.\n";

	cout<<"-degree <degree>\n";
	cout<<"	set the minDegree of the B tree\n";

	cout<<"-index <index_file>\n";
	cout<<"the storage file of the B tree, default is sdb.dat.\n";
}

template<typename T> void run(T& cm) {
	run_insert(cm);
	run_getPrefix(cm);
}

//ofstream  outf("unique.out");
template<typename T> void run_insert(T& cm) {
	int sum =0;
	int hit =0;
	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string str;

	while (inf>>str) {
		sum++;
		if (cm.insertValue(str) ) {
			hit++;

		} else {
		}
		if (trace) {
			cout<<" After insert: key="<<str<<endl;
			cm.display();
		}
	}
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	cout<<"\nnumItem: "<<cm.numItems()<<endl;

}

bool isPrefix(string a, string b) {
	return (b.substr(0, a.size()) == a );
}

template<typename T> void run_getPrefix(T& cm) {
	string str;
	while (1) {
		cin>>str;
		string key;
		key = cm.getNearest(str);
		bool yes = true;
		SequentialDB<string>::SDBCursor cur;
		cm.search(key, cur);
		DataType<string> dat;
		while ( !isPrefix(key, str) ) {
			//key = cm.getPrev(key);	
			cm.seq(cur, ESD_BACKWARD);
			if (cm.get(cur, dat)) {
				key = dat.get_key();
				if (key[0] != str[0]) {
					yes = false;
					break;
				}
			}
		}
		if (yes)
			cout<<key<<endl;
		if (str.compare("exit") == 0)
			break;
	}
}

//int main(int argc, char *argv[])
BOOST_AUTO_TEST_CASE(t_btree_str)
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
		SequentialDB<string> sdb(indexFile);
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
