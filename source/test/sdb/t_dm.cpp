#include "Document.h"
//#include <sdb/SequentialDB.h>
#include <am/tokyo_cabinet/tc_btree.h>
#include <util/bzip.h>
#include <am/filemapper/persist_stl.h>
#include <boost/filesystem.hpp>
#include <3rdparty/am/luxio/array.h>


#include <am/sdb_storage/sdb_storage.h>
#include <am/sdb_storage/sdb_storage_mm.h>
//#include <am/sdb_storage/sdb_storage_mm1.h>

using namespace sf1v5;
using namespace izenelib::am;

bool isDump = false;
int num = 10000000;
static std::string sdb_type = "btree";

std::string
		filename =
				"/home/wps/sf1corpus/default-index-dir/DocumentPropertyTable.dat";

izenelib::am::sdb_storage<docid_t, Document> sdb(filename);
izenelib::am::sdb_storage_mm<docid_t, Document> sdb_mm("/home/wps/sf1corpus/default-index-dir/DocumentPropertyTable_2mm");

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
				
				int nsz=0;
				char *p =(char*)_tc_bzdecompress(docStr.c_str(), docStr.length(), &nsz);	
				izene_deserialization<Document> izd(p, nsz);
				
//				izene_deserialization<Document> izd(docStr.c_str(),
//						docStr.length() );
				
				Document doc;
				izd.read_image(doc);
				cout<<doc.getId()<<endl;
//
				izene_serialization<Document> izs(doc);
				char *ptr;
				size_t size;
				izs.write_image(ptr, size);
				cout<<"doc size="<<size<<endl;
			}
		}
		printf(" elapsed : %lf seconds\n", timer.elapsed() );
	}

	void dump(Lux::IO::Array *ary)
	{
		for(int i = 1;i < num; i++)
		{
			izenelib::am::mapped_string docStr = pm[i];
			ary->put(i, docStr.c_str(), docStr.length(), Lux::IO::NOOVERWRITE);
		}
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
		
		int nsz=0;
		char *p =(char*)_tc_bzcompress(ptr, sz, &nsz);		
		t2.pm[key] = izenelib::am::mapped_string(p, nsz);
		delete p;
		
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

template<typename T1, typename T2> void dump(T1& t1, T2& t2, int num=100000) {
	//if ( !t1.is_open() )
	t1.open();
	//if ( !t2.is_open() )
	t2.open();

	typename T1::SDBCursor locn = t1.get_first_locn();
	docid_t key;
	Document value;
	int count = 0;
	while (t1.get(locn, key, value)) {
		if(count > 1000000-1)
			t2.insert(key, value);
		count++;
		
		if (count%10000 == 0)
			cout<<"idx: "<<count<<endl;
		if (count > num)
			break;
		if ( !t1.seq(locn) )
			break;
	}
	t2.close();
}

template<typename SDB> void query_test(SDB& sdb) {
	sdb.open();
	Document doc;

	izenelib::util::ClockTimer timer;
	for (int i=0; i<100; i++) {
		for (int j=0; j<10; j++) {
			sdb.get(rand()%num+1, doc);
//			izene_serialization<Document> izs(doc);
//			char* str;
//			size_t sz;
//			izs.write_image(str, sz);
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
//			size_t sz;propertyValueTable_
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

void array_test(Lux::IO::Array *ary) {
	izenelib::util::ClockTimer timer;
	for (int i=0; i<100; i++) {
		for (int j=0; j<10; j++) {
			int docid = rand()%num+1;
			//cout<<"docid="<<docid<<endl;
			Lux::IO::data_t val_data;
			Lux::IO::data_t *val_p = &val_data;
			assert(true== ary->get(docid, &val_p, Lux::IO::SYSTEM));
			izene_deserialization<Document> izd((const char*)val_p->data, val_p->size);

			Document doc;
			izd.read_image(doc);
//			cout<<doc.getId()<<endl;
//
//			izene_serialization<Document> izs(doc);
//			char *ptr;
//			size_t size;
//			izs.write_image(ptr, size);
//			cout<<"doc size="<<size<<endl;
			ary->clean_data(val_p);
		}
	}
	printf(" elapsed : %lf seconds\n", timer.elapsed() );
}

void dump(Lux::IO::Array *in, Lux::IO::Array *out) {
	izenelib::util::ClockTimer timer;

	char buf[1024*100];
	for(int i = 1;i < num; i++)
	{
		memset(buf,0,1024*100);
		Lux::IO::data_t val_data = {buf,0,1024*100};
		Lux::IO::data_t *val_p = &val_data;
		assert(true== in->get(i, &val_p, Lux::IO::USER));
		out->put(i, buf, val_p->size, Lux::IO::NOOVERWRITE);
//		izene_deserialization<Document> izd((const char*)val_p->data, val_p->size);

//		Document doc;
//		izd.read_image(doc);
	}
	printf(" elapsed : %lf seconds\n", timer.elapsed() );
}


int main(int argc, char* argv[]) {

//	Lux::IO::Array*	ary = new Lux::IO::Array(Lux::IO::NONCLUSTER);
//			ary->set_noncluster_params(Lux::IO::Padded);
//			std::string db_name = "array";
//			if(! ary->open(db_name.c_str(), Lux::IO::DB_CREAT))
//				ary->open(db_name.c_str(), Lux::IO::DB_RDWR);
//
//	Lux::IO::Array*	ary2 = new Lux::IO::Array(Lux::IO::NONCLUSTER);
//			ary2->set_noncluster_params(Lux::IO::Linked);
//			std::string db_name2 = "array2";
//			if(! ary2->open(db_name2.c_str(), Lux::IO::DB_CREAT))
//				ary2->open(db_name2.c_str(), Lux::IO::DB_RDWR);
//
//	{
////		map_data<izenelib::am::ppmm> root(filename.c_str(), 1);
//		//root->test();
////		root->dump(ary);
//	}
//	array_test(ary2);
//	//dump(ary,ary2);
//	cout<<getMemInfo()<<endl;
//
//	//query_test(sdb);
//	
//	//cout<<getMemInfo()<<endl;
//	delete ary;
//	delete ary2;
//	
	//sdb_mm.setMapSize(5*1024*1024);
	//if( !boost::filesystem::exists("DocumentPropertyTable_1mm_key.dat") )
	//	dump(sdb, sdb_mm, num);	
	query_test(sdb_mm);
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
