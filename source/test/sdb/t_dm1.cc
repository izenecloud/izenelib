#include "Document.h"
#include "DocContainer.h"
#include <boost/test/unit_test.hpp>
//#include <sdb/SequentialDB.h>
#include <am/tokyo_cabinet/tc_btree.h>
#include <util/bzip.h>
#include <am/filemapper/persist_stl.h>
#include <boost/filesystem.hpp>
#include <3rdparty/am/luxio/array.h>

#include <am/sdb_storage/sdb_storage1.h>
#include <am/sdb_storage/sdb_storage_mm.h>
#include <3rdparty/compression/minilzo/minilzo.h>
#include <boost/shared_array.hpp>

using namespace sf1v5;
using namespace izenelib::am;

static izenelib::am::sdb_storage<docid_t, Document> sdb("DocumentPropertyTable_4mm");
//izenelib::am::sdb_storage_mm<docid_t, Document> sdb_mm("/home/wps/sf1corpus/default-index-dir/DocumentPropertyTable_2mm");
static izenelib::am::sdb_storage_mm<docid_t, Document> sdb_mm(
		"DocumentPropertyTable_3mm");


//static sdb_btree<docid_t, Document> btree("DocumentPropertyTable.dat");

//static DocContainer dc("./");
static izenelib::am::sdb_storage<docid_t, Document> dc("DocumentPropertyTable_2mm");

docid_t start = 1;

template<typename T1, typename T2> void dump(T1& t1, T2& t2, int num = 2000000) {
	izenelib::util::ClockTimer timer;
	cout << "dump" << endl;
	//if ( !t1.is_open() )
	t1.open();
	//if ( !t2.is_open() )
	t2.open();
	cout << "num: " << t1.num_items() << endl;

	docid_t key = start;
	Document value;
	int count = 0;
	while (t1.get(key, value)) {
		//cout << "dump doc=" << key << endl;
		t2.insert(key, value);
		++key;
		++count;	
		if (key % 10000 == 0)
			cout << "idx: " << key << endl;
		if (count > num)
			break;

	}
	printf(" elapsed : %lf seconds\n", timer.elapsed());
	cout<<"t2.numItems(): "<<t2.num_items()<<endl;
	//t2.close();
}

template<typename SDB> void query_test(SDB& sdb) {
	cout << "query test" << endl;
	sdb.open();

	int count = sdb.num_items();
	Document doc;
	cout << "!!! " << count << endl;
	izenelib::util::ClockTimer timer;
#if 1
	for (int i = 0; i < 1000; i++) {
		for (int j = 0; j < 10; j++) {
			docid_t did = rand() % count + start;			
			sdb.get(did, doc);		
		}

	}
	printf(" elapsed : %lf seconds\n", timer.elapsed());
#endif
#if 0
	izenelib::util::ClockTimer timer1;
	for (int i = 1; i < count; i++) {
		//cout<<"doc: "<<start+i<<endl;
		sdb.get(start + i, doc);
		//cout<<"doc: "<<start+i<<endl;
	}
	printf(" elapsed : %lf seconds\n", timer1.elapsed());
#endif
}

BOOST_AUTO_TEST_CASE(t_dm1)
{
	//dump( sdb_mm, dc);
	//query_test(dc);
    //dump(dc, sdb_mm);
	//dump(dc, sdb);
    //query_test( dc );
    //cout<<getMemInfo()<<endl;
	
    //query_test( dc );
	cout<<getMemInfo()<<endl;

	query_test( sdb_mm );
    cout<<getMemInfo()<<endl;

	query_test( sdb);
    cout<<getMemInfo()<<endl;
	//return 1;
}

BOOST_AUTO_TEST_CASE(t_dm2)
{
	dc.open();
	docid_t did = 4996990;
	//docid_t did = 4996991;
	Document doc;
	cout << "1" << endl;
	dc.get(did, doc);
	{
		izene_serialization<Document> izs(doc);
		char* str;
		size_t sz;
		izs.write_image(str, sz);
		cout << "doc sz:" << sz << endl;
	}
	sdb_mm.open();
	sdb_mm.del(did);
	sdb_mm.insert(did, doc);
	cout << "2" << endl;
	sdb_mm.get(did, doc);
	cout << "3" << endl;
	{
		izene_serialization<Document> izs(doc);
		char* str;
		size_t sz;
		izs.write_image(str, sz);
		cout << "doc sz:" << sz << endl;
	}

}
