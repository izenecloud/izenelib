#include <am_test/am_test.h>
#include <am_test/ylib_types.h>

using namespace std;

using namespace izenelib::am_test;


static string inputFile = "test.txt";
static string indexFile = "index.dat";
static int num = 1000000;
static bool rnd = 0;
static int loop = 1;
static bool trace = 0;

void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_am_ylib [-T <trace_option>] [-loop <num>][-n <num>] [-rnd <true|false>] <input_file>\n\n";
}

template<typename T> void run1(T& cm) {
	cm.run_insert_ylib();
//#ifdef ONLY_INSERT
	cm.run_find();
	cm.run_del();
//#endif	
}

void test_ylib() {
	{
		cout<<"\nylib LinearHashTable<YString, YString>"<<endl;
		typedef ylib::hashing_methods::LinearHashTable<YString, YString>
				LHT_STRING_STRING;
		AmTest<YString, YString, LHT_STRING_STRING> am;
		run1(am);
	}

	{
		cout<<"\nylib ExtendibleHashTable<YString, YString>"<<endl;
		typedef ylib::hashing_methods::ExtendibleHash<YString, YString>
				EHT_STRING_STRING;
		AmTest<YString, YString, EHT_STRING_STRING> am;
		run1(am);
	}

	{
		cout<<"\nylib EfficientLHT<YString, YString>"<<endl;
		typedef ylib::EfficientLHT<YString, YString> ELHT_STRING_STRING;
		AmTest<YString, YString, ELHT_STRING_STRING> am;
		run1(am);
	}
	{
		cout<<"\nylib BTree<YString, YString>"<<endl;
		typedef ylib::BTree<YString, YString> BTREE_STRING_STRING;
		AmTest<YString, YString, BTREE_STRING_STRING> am;
		run1(am);
	}
	{
		cout<<"\nylib BPTree<YString, YString>"<<endl;
		typedef ylib::BPTree<YString, YString> BPTREE_STRING_STRING;
		AmTest<YString, YString, BPTREE_STRING_STRING> am;
		run1(am);
	}
	/*{
		cout<<"\nylib ExtendibleHashLSB<YString, YString>"<<endl;
		typedef ylib::ExtendibleHashLSB<YString, YString> EHLSB_STRING_STRING;
		AmTest<YString, YString, EHLSB_STRING_STRING> am;
		run1(am);
	}*/
	/*{
		cout<<"\nylib PtrTree<YString, YString>"<<endl;
		typedef ylib::PtrTree<YString, YString> PTREE_STRING_STRING;
		AmTest<YString, YString, PTREE_STRING_STRING> am;
		run1(am);
	}*/
	{
		cout<<"\nylib RBTrie<YString>"<<endl;
		typedef ylib::RBTrie<YString> RBTRIE_STRING_STRING;
		AmTest<string, YString, RBTRIE_STRING_STRING> am;
		run1(am);
	}
	/*{
		cout<<"\nylib SkipList<YString, YString>"<<endl;
		typedef ylib::SkipList<YString, YString> SKIPLIST_STRING_STRING;
		AmTest<YString, YString, SKIPLIST_STRING_STRING> am;
		run1(am);
	}*/

	{
		cout<<"\nylib SplayTrie<YString>"<<endl;
		typedef ylib::SplayTrie<YString> SPRAY_STRING_STRING;
		AmTest<string, YString, SPRAY_STRING_STRING> am;
		run1(am);
	}

	{
		cout<<"\nylib TernarySearchTree<YString, YString>"<<endl;
		typedef ylib::TernarySearchTree<YString> TST_STRING_STRING;
		AmTest<string, YString, TST_STRING_STRING> am;
		run1(am);
	}

	{
		cout<<"\nylib Trie<YString, YString>"<<endl;
		typedef ylib::Trie<YString> TRIE_STRING_STRING;
		AmTest<string, YString, TRIE_STRING_STRING> am;
		run1(am);
	}

	{
		cout<<"\nylib Tree<YString, YString>"<<endl;
		typedef ylib::Tree<YString, YString> TREE_STRING_STRING;
		AmTest<YString, YString, TREE_STRING_STRING> am;
		run1(am);
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
		test_ylib();
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
