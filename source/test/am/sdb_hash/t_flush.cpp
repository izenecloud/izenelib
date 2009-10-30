#include <boost/memory.hpp>
#include <string>
#include <ctime>

#include <am/sdb_hash/sdb_hash.h>
#include <am/sdb_btree/sdb_btree.h>
#include <am/sdb_hash/sdb_fixedhash.h>
#include <am/am_test/am_test.h>
#include <util/izene_log.h>

typedef sdb_fixedhash<uint32_t, uint32_t> HASH;
unsigned int num = 10000000;
bool trace = false;


struct TY{
     int a;
     char b;
     int compare(const TY& other) const{
          return a - other.a;
     }

     DATA_IO_LOAD_SAVE(TY, &a&b)
};

MAKE_FEBIRD_SERIALIZATION( TY );

void test1(){
    clock_t start, finish;
    start = clock();

    sdb_btree<int, int> tb;

    tb.open();

    for (unsigned int i=0; i<num; i++) {
                int k = rand()%num;              
                tb.insert(k, i);
                if (trace) {
                        cout<<"numItem: "<<tb.num_items()<<endl<<endl;
                        //tb.display();
                }
                DLOG_EVERY_N(ERROR, 1000000) << getMemInfo();
        }
        cout<<"mumItem: "<<tb.num_items()<<endl;
        printf("\nIt takes %f seconds before flush()\n", (double)(clock() - start)
                        /CLOCKS_PER_SEC);

        DLOG(ERROR)<<getMemInfo();
        if (trace)
                tb.display();
        tb.flush();
        DLOG(ERROR)<<"After flush"<<endl;
        DLOG(ERROR)<<getMemInfo();

}


void test(){
    clock_t start, finish;
    start = clock();

    sdb_btree<TY, TY> tb("sss.dat");

    tb.open();

    for (unsigned int i=0; i<num; i++) {
                int k = rand()%num;
                TY t;
                t.a = k;
                t.b = k%128;
                tb.insert(t, t);
                if (trace) {
                        cout<<"numItem: "<<tb.num_items()<<endl<<endl;
                        //tb.display();
                }
                DLOG_EVERY_N(ERROR, 1000000) << getMemInfo();
        }
        cout<<"mumItem: "<<tb.num_items()<<endl;
        printf("\nIt takes %f seconds before flush()\n", (double)(clock() - start)
                        /CLOCKS_PER_SEC);

        DLOG(ERROR)<<getMemInfo();
        if (trace)
                tb.display();
        tb.flush();
        DLOG(ERROR)<<"After flush"<<endl;
        DLOG(ERROR)<<getMemInfo();

}


template<typename T> void random_insert_test(T& tb) {
	clock_t start, finish;
	start = clock();
	for (unsigned int i=0; i<num; i++) {
		int k = rand()%num;		
		tb.insert(k, i);
		if (trace) {
			cout<<"numItem: "<<tb.num_items()<<endl<<endl;
			tb.display();
		}	
		DLOG_EVERY_N(ERROR, 1000000) << getMemInfo();
	}
	cout<<"mumItem: "<<tb.num_items()<<endl;
	printf("\nIt takes %f seconds before flush()\n", (double)(clock() - start)
			/CLOCKS_PER_SEC);
	
	DLOG(ERROR)<<getMemInfo();
	if (trace)
		tb.display();
	tb.flush();
	DLOG(ERROR)<<"After flush"<<endl;
	DLOG(ERROR)<<getMemInfo();
	
	finish = clock();
	printf("\nIt takes %f seconds to insert %d  data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, num);

}


template<typename T> void random_update_test(T& tb) {
	clock_t start, finish;
	start = clock();
	for (unsigned int i=0; i<num; i++) {
		unsigned int k = rand()%num;				
		tb.update(k, k);
		if (trace) {
			cout<<"numItem: "<<tb.num_items()<<endl<<endl;
			tb.display();
		}	
		DLOG_EVERY_N(ERROR, 1000000) << getMemInfo();
	}
	cout<<"mumItem: "<<tb.num_items()<<endl;
	printf("\nIt takes %f seconds before flush()\n", (double)(clock() - start)
			/CLOCKS_PER_SEC);
	
	DLOG(ERROR)<<getMemInfo();
	if (trace)
		tb.display();
	tb.flush();
	DLOG(ERROR)<<"After flush"<<endl;
	DLOG(ERROR)<<getMemInfo();
	
	finish = clock();
	printf("\nIt takes %f seconds to insert %d  data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, num);

}

template<typename T> void random_get_test(T& tb) {
	clock_t start, finish;
	start = clock();
	for (unsigned int i=0; i<num; i++) {
		unsigned int k = rand()%num;		
		unsigned val;
		tb.get(k, val);
		if (trace) {
			cout<<"numItem: "<<tb.num_items()<<endl<<endl;
			tb.display();
		}	
		DLOG_EVERY_N(ERROR, 1000000) << getMemInfo();
	}
	cout<<"mumItem: "<<tb.num_items()<<endl;
	printf("\nIt takes %f seconds before flush()\n", (double)(clock() - start)
			/CLOCKS_PER_SEC);
	
	DLOG(ERROR)<<getMemInfo();
	if (trace)
		tb.display();
	tb.flush();
	DLOG(ERROR)<<"After flush"<<endl;
	DLOG(ERROR)<<getMemInfo();
	
	finish = clock();
	printf("\nIt takes %f seconds to insert %d  data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, num);

}

int main(){
        test1();
        test();
	HASH hash;
	hash.setPageSize(512);
	hash.setCacheSize(1024*600);
	hash.open();
	DLOG(ERROR)<<getMemInfo();		
	random_insert_test(hash);
	DLOG(ERROR)<<getMemInfo();	
	random_get_test(hash);
	random_update_test(hash);
	DLOG(ERROR)<<getMemInfo();	
	
}

