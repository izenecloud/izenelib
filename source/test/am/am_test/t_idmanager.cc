#include <am/am_test/am_test.h>
#include <am/am_test/am_types.h>
#include <wiselib/ustring/UString.h>

using namespace std;
using namespace izenelib::am;
using namespace izenelib::am_test;
using namespace wiselib;

static string inputFile = "test.txt";
static string indexFile = "index.dat";
static int num = 1000000;
static bool rnd = 0;
static int loop = 1;
static bool trace = 0;

namespace izenelib {
namespace am_test {

template<> inline UString generateData<UString>(const int a, int num, bool rand) {
	string::value_type p[10];
	int b;
	if (rand)
		b = myrand()%(num+1);
	else
		b = a;
	sprintf(p, "%08d", b);
	return UString(p, UString::UTF_8);
}
}
}

void ReportUsage(void) {
	cout
			<<"\nUSAGE:./test [-T <trace_option>] [-loop <num>][-n <num>] [-rnd <1|0>] <input_file>\n\n";
}

void run() {
	cout<<"\n============tc_hash===============\n"<<endl;

	{
		cout<<"\ntc_hash<ustring, int>"<<endl;
		typedef tc_hash<UString, int> SBTREE_STRING_INT;
		AmTest<UString, int, SBTREE_STRING_INT, true> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am_nod(am);
	}

	{
		cout<<"\ntc_hash<int, UString>"<<endl;
		typedef tc_hash<int, UString> SBTREE_STRING_INT;
		AmTest<int, UString, SBTREE_STRING_INT, true> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}

	cout<<"\n============sdb_btree===============\n"<<endl;

	{
		cout<<"\nsdb_btree<int, UString>"<<endl;
		typedef sdb_btree<int, UString> SBTREE_STRING_INT;
		AmTest<int, UString, SBTREE_STRING_INT, true> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am_nod(am);
	}

	{
		cout<<"\nsdb_btree<ustring, int>"<<endl;
		typedef sdb_btree<UString, int> SBTREE_STRING_INT;
		AmTest<UString, int, SBTREE_STRING_INT, true> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}

	cout<<"\n============sdb_hash===============\n"<<endl;

	{
		cout<<"\nsdb_hash<ustring, int>"<<endl;
		typedef sdb_hash<UString, int> SBTREE_STRING_INT;
		AmTest<UString, int, SBTREE_STRING_INT, true> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}

	{
		cout<<"\nsdb_hash<int, UString>"<<endl;
		typedef sdb_hash<int, UString> SBTREE_STRING_INT;
		AmTest<int, UString, SBTREE_STRING_INT, true> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}
	cout<<"===============cccr_hash=========="<<endl;
	{
		cout<<"\ncccr_hash<ustring, int>"<<endl;
		typedef cccr_hash<UString, int> SBTREE_STRING_INT;
		AmTest<UString, int, SBTREE_STRING_INT> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}

	{
		cout<<"\ncccr_hash<int, UString>"<<endl;
		typedef cccr_hash<int, UString> SBTREE_STRING_INT;
		AmTest<int, UString, SBTREE_STRING_INT> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}
	cout<<"================linearHashTable============"<<endl;
	{
		cout<<"\nlinear_hash<ustring, int>"<<endl;
		typedef LinearHashTable<UString, int> SBTREE_STRING_INT;
		AmTest<UString, int, SBTREE_STRING_INT> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}

	{
		cout<<"\nLinearHashTable<int, UString>"<<endl;
		typedef LinearHashTable<int, UString> SBTREE_STRING_INT;
		AmTest<int, UString, SBTREE_STRING_INT> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}

	cout<<"================wrapped_hash_map============"<<endl;
	{
		cout<<"\nwrapped_hash_map<ustring, int>"<<endl;
		typedef wrapped_hash_map<UString, int> SBTREE_STRING_INT;
		AmTest<UString, int, SBTREE_STRING_INT> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}

	{
		cout<<"\nwrapped_hash_map<int, UString>"<<endl;
		typedef wrapped_hash_map<int, UString> SBTREE_STRING_INT;
		AmTest<int, UString, SBTREE_STRING_INT> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}

	cout<<"================wrapped_map============"<<endl;
	{
		cout<<"\nwrapped_map<ustring, int>"<<endl;
		typedef wrapped_map<UString, int> SBTREE_STRING_INT;
		AmTest<UString, int, SBTREE_STRING_INT> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
	}

	{
		cout<<"\nwrapped_map<int, UString>"<<endl;
		typedef wrapped_map<int, UString> SBTREE_STRING_INT;
		AmTest<int, UString, SBTREE_STRING_INT> am;
		am.setNum(num);
		am.setRandom(rnd);
		run_am(am);
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
			} else if (str == "n") {
				num = atoi(*argv++);
			} else if (str == "rnd") {
				rnd = (bool)atoi(*argv++);
			} else if (str == "loop") {
				loop = atoi(*argv++);
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
	try {
		run();
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

