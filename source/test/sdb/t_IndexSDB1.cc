#include <sdb/IndexSDB.h>
#include <iostream>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace izenelib::sdb;

static const char* indexFile = "indexsdb124.dat";
static string inputFile = "test2.txt";
static int degree =8;
static size_t cacheSize = 1000000;
static bool trace = 0;

typedef unsigned int FieldID;
typedef unsigned int ColID;
//typedef unsigned int TermID;
typedef string TermID;

typedef unsigned int DocID;

struct KeyType {
	ColID cid;
	FieldID fid;
	TermID tid;

	KeyType() {
		cid = 0;
		fid = 0;
		tid = "";
	}

	KeyType(ColID c, FieldID f, TermID t) :
		cid(c), fid(f), tid(t) {
	}

	template<class Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & fid;
		ar & cid;
		ar & tid;
	}

	int compare(const KeyType& other) const {
		if (cid != other.cid)
			return cid-other.cid;
		else {
			if (fid != other.fid)
				return fid-other.cid;
			else {
				return tid.compare(other.tid);
			}
		}
	}

	bool isPrefix(const KeyType& other) const {
		cout<<"Key typeid "<<typeid(tid).name()<<endl;
		display();
		cout<<endl;
		other.display();
		if (cid != other.cid)
			return false;
		else {
			if (fid != other.fid)
				return false;
			else {
				if (other.tid.substr(0, tid.size() ) == tid) {
					return true;
				} else {
					return false;
				}
			}
		}
	}

	void display(std::ostream& os = std::cout) const {
		//cout<<"KeyType display...\n"<<endl;;
		//cout<<cid<<endl;
		//cout<<fid<<endl;
		os<<tid;
	}

};

/*

 NS_IZENELIB_AM_BEGIN

 namespace util {

 template<> inline void read_image< iKeyType<KeyType> >(iKeyType<KeyType>& key,
 const DbObjPtr& ptr) {
 memcpy(&key, ptr->getData(), ptr->getSize());
 }

 template<> inline void write_image< iKeyType<KeyType> >(
 const iKeyType<KeyType>& key, DbObjPtr& ptr) {
 ptr->setData(&key, sizeof(iKeyType<KeyType>));
 }*/

/*
 template<> inline void read_image<std::vector<DocID> >(std::vector<DocID>& val,
 const DbObjPtr& ptr) {
 val.resize(ptr->getSize()/sizeof(DocID));
 memcpy(&val[0], ptr->getData(), ptr->getSize());
 }

 template<> inline void write_image<std::vector<DocID> >(
 const std::vector<DocID>& val, DbObjPtr& ptr) {	
 ptr->setData(&val[0], sizeof(DocID)*val.size() );
 }

 }

 NS_IZENELIB_AM_END*/

struct suffix {
	string suf;

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

static void ReportUsage(void) {
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

template<typename T, typename V> void run(T& cm, V& dm) {
	run_insert(cm, dm);
	run_getValue_perf(cm, dm);
	//run_getValue(cm, dm);
	//run_del(cm);
}

template<typename T, typename V> void run_insert(T& cm, V& dm) {

	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;

	while (inf>>ystr)
	//for(int i=0; i<100; i++)
	{
		if (trace)
			cout<<"input= "<<ystr<<endl;
		//int p = rand()%100000;
		KeyType key(11, 22, ystr);
		vector<string> vIdx;

		int size = rand()%10;
		for (int i=1; i<=size; i++) {
			if (trace)
				cout<<i<<":"<<ystr<<endl;
			cm.add_nodup(key, ystr);
			//vIdx.push_back(ystr);
		}

		/*	size_t pos = 0;
		 for (; pos<ystr.size(); pos++) {
		 string suf = ystr.substr(pos);
		 suffix skey(suf);
		 dm.add(skey, ystr);
		 //dm.display();

		 }*/

		//cout<<"\nupdate "<<size<<"\n\n";
		//		if (cm.update(key, vIdx) ) {
		//			if (trace) {
		//				cm.display();
		//			}
		//		}

		if (trace) {
			cout<<" After insert: key="<<key.tid<<endl;
			cm.display();
			//dm.display();
		}
	}
	//sfh.display();
	cm.flush();
	dm.flush();
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	if (trace) {
		cout<<"After run_insert, display..."<<endl;
		dm.display();
		cm.display();
	}
}

template<typename T, typename V> void run_getValue_perf(T& cm, V&dm) {

	cout<<"testing indexSDB performance\n";

	clock_t t1 = clock();

	vector<string> result;

	//cout<<"pls input one key:\n";
	//string str = "haha";
	//string str1= "show";
	TermID low = "howers";
	TermID high= "wps";
	//cin>>str;	
	KeyType key(11, 22, low);
	cout<<"\ngetValueGreat\n\n";
	cm.getValueGreat(key, result);

	for (unsigned int i=0; i<result.size(); i++) {
		if (trace) {
			cout<<i<<" : "<<result[i]<<endl;
		}
	}
	result.clear();
	cout<<"\ngetValueGreatEqual\n\n";
	cm.getValueGreatEqual(key, result);

	for (unsigned int i=0; i<result.size(); i++) {
		if (trace) {
			cout<<i<<" : "<<result[i]<<endl;
		}
	}

	result.clear();
	cout<<"\ngetValue\n\n";
	cm.getValue(key, result);

	for (unsigned int i=0; i<result.size(); i++) {
		if (trace) {
			cout<<i<<": "<<result[i]<<endl;
		}
	}
	result.clear();
	//cout<<"pls input another key:\n";
	//cin>>str1;

	KeyType key1(11, 22, high);

	cout<<"\ngetValueLess\n\n";
	cm.getValueLess(key1, result);
	for (unsigned int i=0; i<result.size(); i++) {
		if (trace) {
			cout<<i<<" : "<<result[i]<<endl;
		}
	}

	result.clear();
	cout<<"\ngetValueLessEqual\n\n";
	cm.getValueLessEqual(key1, result);
	for (unsigned int i=0; i<result.size(); i++) {
		if (trace)
			cout<<i<<" : "<<result[i]<<endl;
	}

	result.clear();
	cout<<"\ngetValueBetween\n\n";
	cm.getValueBetween(key, key1, result);
	for (unsigned int i=0; i<result.size(); i++) {
		if (trace)
			cout<<i<<" : "<<result[i]<<endl;
	}
	result.clear();

	KeyType key2(11, 22, "a");

	cout<<"\ngetValuePrefix\n\n";
	cm.getValuePrefix(key2, result);
	for (unsigned int i=0; i<result.size(); i++) {
		if (trace) {
			cout<<i<<" : "<<result[i]<<endl;
		}
	}

	//cout<<"pls input substring\n";
	//string suf = "c";
	//cin>>suf;
	//vector<string> vkey;
	//cout<<"getvalue prefix"<<endl;
	//dm.getValuePrefix(suf, vkey);
	//vector<KeyType> keys;
	//for (size_t i=0; i<vkey.size(); i++) {
	//	if (trace)
	//		cout<<vkey[i]<<endl;
	//keys.push_back(KeyType(11, 22, vkey[i]) );
	//}

	result.clear();
	/*cm.getValueIn(keys, result);
	 for (unsigned int i=0; i<result.size(); i++) {
	 if (trace) {
	 cout<<result[i]<<endl;
	 }
	 }*/
	cout<<"delete key:"<<endl;

	cm.del(key);
	result.clear();
	cout<<"\ngetValue\n\n";
	cm.getValue(key, result);

	for (unsigned int i=0; i<result.size(); i++) {
		if (trace) {
			cout<<i<<": "<<result[i]<<endl;
		}
	}
	result.clear();

	cout<<"\ngetValueBetween\n\n";
	cm.getValueBetween(key, key1, result);
	for (unsigned int i=0; i<result.size(); i++) {
		if (trace)
			cout<<i<<" : "<<result[i]<<endl;
	}
	result.clear();

	KeyType key3(11, 22, "the");
	cm.remove(key3, "the");

	cout<<"\ngetValueBetween\n\n";
	cm.getValueBetween(key, key1, result);
	for (unsigned int i=0; i<result.size(); i++) {
		if (trace)
			cout<<i<<" : "<<result[i]<<endl;
	}
	result.clear();

	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

}

/*
 template<typename T, typename V> void run_getValue(T& cm, V&dm) {

 vector<DocID> result;

 cout<<"pls input one key:\n";
 string str;
 string str1;
 cin>>str;
 KeyType key(11, 22, str);
 cout<<"\ngetValueGreat\n\n";
 cm.getValueGreat(key, result);

 for (unsigned int i=0; i<result.size(); i++) {
 if (trace) {
 if (result[i] == 1)
 cout<<"idx "<<endl;
 cout<<result[i]<<endl;
 }
 }
 result.clear();
 cout<<"\ngetValueGreatEqual\n\n";
 cm.getValueGreatEqual(key, result);

 for (unsigned int i=0; i<result.size(); i++) {
 if (trace) {
 if (result[i] == 1)
 cout<<"idx "<<endl;
 cout<<result[i]<<endl;
 }
 }
 result.clear();
 cout<<"pls input another key:\n";
 cin>>str1;

 KeyType key1(11, 22, str1);

 cout<<"\ngetValueLess\n\n";
 cm.getValueLess(key1, result);
 for (unsigned int i=0; i<result.size(); i++) {
 if (trace) {
 cout<<result[i]<<endl;
 }
 }

 result.clear();
 cout<<"\ngetValueLessEqual\n\n";
 cm.getValueLessEqual(key1, result);
 for (unsigned int i=0; i<result.size(); i++) {
 if (trace)
 cout<<result[i]<<endl;
 }

 result.clear();
 cout<<"\ngetValueBetween\n\n";
 cm.getValueBetween(key, key1, result);
 for (unsigned int i=0; i<result.size(); i++) {
 if (trace)
 cout<<result[i]<<endl;
 }
 result.clear();
 cout<<"pls input substring\n";
 string suf;
 cin>>suf;
 vector<string> vkey;
 dm.getValuePrefix(suf, vkey);
 vector<KeyType> keys;
 for (size_t i=0; i<vkey.size(); i++) {
 //if(trace)
 cout<<vkey[i]<<endl;
 keys.push_back(KeyType(11, 22, vkey[i]) );
 }

 result.clear();
 cm.getValueIn(keys, result);
 for (unsigned int i=0; i<result.size(); i++) {
 if (trace) {
 cout<<result[i]<<endl;
 }
 }
 }
 */

/*
 template<typename T> void run_del(T& cm) {

 ifstream inf2(inputFile.c_str());
 string ystr;	
 while (inf2>>ystr) {
 //cout<<"input ="<<ystr<<" "<<endl;	
 KeyType key(11, 22, ystr);

 if (cm.del(key) ) {
 }
 if (trace) {
 cout<<" After del: key="<<key.tid<<endl;
 //cm.display();
 }
 }
 }
 */

//int main(int argc, char *argv[]) 
BOOST_AUTO_TEST_CASE(t_IndexSBB1)
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
	}*/

	try
	{
		string file(indexFile);

		string sufFile("suffix1.dat");
		IndexSDB<suffix, string> sufSDB(sufFile);
		IndexSDB<KeyType, string> isdb(file);

		isdb.initialize();
		sufSDB.initialize(20, degree, 1024*2, cacheSize);

		run(isdb, sufSDB);

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
