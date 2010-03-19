#include "Document.h"
#include <sdb/SequentialDB.h>
#include <am/tokyo_cabinet/tc_btree.h>
#include <util/bzip.h>
#include <am/filemapper/persist_stl.h>
#include <boost/filesystem.hpp>

using namespace sf1v5;
using namespace izenelib::am;

bool isDump = false;
int num = 100000;
static std::string sdb_type = "btree";

std::string
		filename =
				"/home/wps/codebase/sf1-r/sf1-revolution/bin/collection/index-data/DocumentPropertyTable.dat";

sdb_storage<docid_t, Document> sdb(filename);

namespace izenelib {
namespace am {
struct ppmm {
	typedef izenelib::am::mapped_vector<izenelib::am::mapped_string> Pmap;
	Pmap pm;
	void test() {
		izenelib::util::ClockTimer timer;
		for (int i=0; i<100; i++) {
			for (int j=0; j<10; j++) {
				int docid = rand()%num+1;
				//cout<<"docid="<<j<<endl;
				izenelib::am::mapped_string docStr = pm[docid];
				izene_deserialization<Document> izd(docStr.c_str(),
						docStr.length() );
				Document doc;
				izd.read_image(doc);
//
//				izene_serialization<Document> izs(doc);
//				char *ptr;
//				size_t size;
//				izs.write_image(ptr, size);
//				cout<<"doc size="<<size<<endl;
			}
		}
		printf(" elapsed : %lf seconds\n", timer.elapsed() );
	}
};
}
}

//sdb_btree<docid_t, Document> sdb(filename);
//sdb_btree<docid_t, Document> sdb0("_bt.dat");
//sdb_bptree<docid_t, Document> sdb1("_bp.dat");
//sdb_storage<docid_t, Document> sdb2("_seq.dat");
//tc_hash<docid_t, Document> sdb3("_tch.dat");
//tc_btree<docid_t, Document> sdb4("_tcb.dat");

template<typename T1, typename T2> void dump1(T1& t1, T2& t2, int num=10) {
	t1.open();

	typename T1::SDBCursor locn = t1.get_first_locn();
	docid_t key;
	Document value;
	int count = 0;
	t2.pm.resize(num+1);
	while (t1.get(locn, key, value)) {
		izene_serialization<Document> izs(value);
		char* ptr;
		size_t sz;
		izs.write_image(ptr, sz);
		//		t2.pm.insert(make_pair(key, izenelib::am::string(ptr,sz)) );
		t2.pm[key] = izenelib::am::mapped_string(ptr, sz);
		//		cout<<"doc size="<<sz<<endl;
		count++;
		if (count%100000 == 0)
			cout<<"idx: "<<count<<endl;
		if (count > num)
			break;
		if ( !t1.seq(locn) )
			break;
	}
	t2.test();
	//	t2.close();
}

//template<typename T1, typename T2> void dump(T1& t1, T2& t2, int num=100000) {
//	//if ( !t1.is_open() )
//	t1.open();
//	//if ( !t2.is_open() )
//	t2.open();
//
//	typename T1::SDBCursor locn = t1.get_first_locn();
//	docid_t key;
//	Document value;
//	int count = 0;
//	while (t1.get(locn, key, value)) {
//		t2.insert(key, value);
//		count++;
//		if (count%10000 == 0)
//			cout<<"idx: "<<count<<endl;
//		if (count > num)
//			break;
//		if ( !t1.seq(locn) )
//			break;
//	}
//	t2.close();
//}

template<typename SDB> void query_test(SDB& sdb) {
	sdb.open();
	Document doc;

	izenelib::util::ClockTimer timer;
	for (int i=0; i<100; i++) {
		for (int j=0; j<10; j++) {
			sdb.get(rand()%num+1, doc);
			izene_serialization<Document> izs(doc);
			char* str;
			size_t sz;
			izs.write_image(str, sz);
			//cout<<"doc sz:"<<sz<<endl;
		}
	}
	printf(" elapsed : %lf seconds\n", timer.elapsed() );
}

//template<typename SDB> void query_test(SDB& sdb) {
//	sdb.open();
//	Document doc;
//
//	izenelib::util::ClockTimer timer;
//	double worst = 0.0;
//	int times = 1000;
//	for (int i=0; i<times; i++) {
//		izenelib::util::ClockTimer timer1;
//
//		set<int> input;
//		set<int>::iterator it;
//		int num = sdb.num_items()+1;
//		for (int j=0; j<20; j++)
//			input.insert(rand()%num);
//		for (it=input.begin(); it != input.end(); it++) {
//			//			cout<<*it<<endl;
//			sdb.get(*it, doc);
//		}
//		double elapsed = timer1.elapsed();
//		printf("one query elapsed: ( actually ): %lf seconds\n", elapsed);
//		if (elapsed > worst)
//			worst = elapsed;
//
//		if (i % 100 == 0) {
//			izene_serialization<Document> izs(doc);
//			char* str;
//			size_t sz;
//			izs.write_image(str, sz);
//
//			cout<<"doc sz:"<<sz<<endl;
//			char* str1;
//			int sz1;
//
//			str1 = _tc_bzcompress(str, sz, &sz1);
//			cout<<"after bzip compresss"<<endl;
//			cout<<"sz: "<<sz1<<endl;
//			
//		}
//	}
//	printf("average elapsed 1 ( actually ): %lf seconds\n, worse: %lf seconds",
//			timer.elapsed()/times, worst);
//}
//
//template<typename SDB> void query_test1() {
//	sdb.open();
//	sdb2.open();
//	Document doc;
//
//	izenelib::util::ClockTimer timer;
//	double worst = 0.0;
//	int times = 1000;
//	for (int i=0; i<times; i++) {
//		izenelib::util::ClockTimer timer1;
//
//		set<int> input;
//		set<int>::iterator it;
//		int num = sdb.num_items()ppmm+1;
//		for (int j=0; j<20; j++)
//			input.insert(rand()%num);
//		for (it=input.begin(); it != input.end(); it++) {
//			//			cout<<*it<<endl;
//			sdb.get(*it, doc);
//			sdb2.get(*it, doc);
//		}
//		double elapsed = timer1.elapsed();
//		printf("one query elapsed: ( actually ): %lf seconds\n", elapsed);
//		if (elapsed > worst)
//			worst = elapsed;
//
//		if (i % 100 == 0) {
//			izene_serialization<Document> izs(doc);
//			char* str;
//			size_t sz;
//			izs.write_image(str, sz);
//			cout<<"doc sz:"<<sz<<endl;
//		}
//	}
//	printf("average elapsed 1 ( actually ): %lf seconds\n, worse: %lf seconds",
//			timer.elapsed()/times, worst);
//}
//
//template<typename SDB> void process(SDB& db) {
//	if (isDump)
//		dump(sdb, db, num);
//	else
//		query_test(db);
//}
//
//void initialize() {
//	sdb0.setDegree(12);
//	//sdb0.setPageSize(8192*8);
//}

int main(int argc, char* argv[]) {

	if ( !boost::filesystem::exists("ppmm.map") ) {
		map_data<izenelib::am::ppmm> root("ppmm.map", 1, create_new |auto_grow,
				1024*1024*1024);
		//		
		//		map_data<izenelib::am::ppmm> root("ppmm.map", 1);
		dump1(sdb, *root, num);
	} else {
		map_data<izenelib::am::ppmm> root("ppmm.map", 1);
		root->test();
	}
	cout<<getMemInfo()<<endl;

	query_test(sdb);
	cout<<getMemInfo()<<endl;
	//	initialize();
	//	//query_test1();
	//	if (argv[1]) {
	//		isDump = atoi(argv[1]);
	//	}
	//	if (argv[2])
	//		sdb_type = argv[2];
	//	if (argv[3])
	//		num = atoi(argv[3]);
	//
	//	if (sdb_type == "ori") {
	//		isDump = false;
	//		process(sdb);
	//	}
	//	if (sdb_type == "btree")
	//		process(sdb0);
	//	else if (sdb_type == "bptree")
	//		process(sdb1);
	//	else if (sdb_type == "tc")
	//		process(sdb3);
	//	else if (sdb_type == "tcb")
	//		;//process(sdb4);
	//	else if (sdb_type == "seq") {
	//		process(sdb2);
	//	} else {
	//		process(sdb0);
	//		process(sdb1);
	//		process(sdb2);
	//		process(sdb);
	//	}

	return 1;
}
