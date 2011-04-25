

#include <string>
#include <stdlib.h>
#include <sys/time.h>

#include <fstream>
#include <iostream>
#include <iomanip>

#include <map>
#include <3rdparty/am/beansdb/hstore.h>

#include <util/Int2String.h>
#include <am/beansdb/Hash.h>

#include <boost/timer.hpp>

#include <assert.h>
using namespace std;
void test_beansdb(int N)
{
    time_t before_time = 0;

    HStore* store = hs_open("testbeansdb", 1, before_time);

    int i;
    char key[256];

    srand48(11);

    unsigned flag = 0;
    int rlen = 0;
	
    for (i = 0; i < N; ++i)
    {
        unsigned intdata = i;//(N * drand48() / 4) * 271828183u;
        sprintf(key, "%x", intdata);    
        //hs_append(store, key, (char*)&i, sizeof(int));
        char *body = hs_get(store, key, &rlen, &flag);
        int v = i;
        if(body != NULL)
        {
            v = *((int*)body);
            flag-=1;
            v = (v*2);
            free(body);
        }
        hs_set(store, key, (char*)&v, sizeof(int), flag, 0);
    }

    int flush_limit = 1024 * 2;
    hs_flush(store, flush_limit);

    for (i = 0; i < N; ++i)
    {
        unsigned intdata = i;//(N * drand48() / 4) * 271828183u;
        sprintf(key, "%x", intdata);    
        char *body = hs_get(store, key, &rlen, &flag);
        if(body != NULL)
        {
            unsigned int v = *((unsigned int*)body);
            cout<<"value "<<v<<" vlen "<<rlen<<" flag "<<flag<<endl;
            free(body);
        }
        //hs_set(store, key, (char*)&i, sizeof(int), 0, 1);
    }	
    hs_close(store);
}

static int data_size = 1000000;
static unsigned *int_data;

void init_data()
{
    int i;
    std::cout<<"generating data... "<<std::endl;
    srand48(11);
    int_data = (unsigned*)calloc(data_size, sizeof(unsigned));
    for (i = 0; i < data_size; ++i) {
        int_data[i] = (unsigned)(data_size * drand48() / 4) * 271828183u;
    }
    std::cout<<"done!\n";
}

void destroy_data()
{
    free(int_data);
}

/// Speed test them!
int main()
{
    //test_beansdb(100);
    init_data();
    izenelib::am::beansdb::Hash<Int2String, int> table("beansdb");
    int size = 100;
    int i;
    for (i = 1; i < size; ++i) {
        Int2String key(i);
        //table.insert(key,int_data[i]);
        table.insert(key,i*100);
    }
    cout<<"insert finished"<<endl;
    for (i = 1; i < size; ++i) {
       Int2String key(i);
       int value;
	table.get(key, value);
	//if(value != int_data[i])
           //cout<<"i "<<i<<" value "<<value<<" data "<<int_data[i]<<endl;
        cout<<"i "<<i<<" value "<<value<<endl;           
    }
    table.flush();
    destroy_data();
}
