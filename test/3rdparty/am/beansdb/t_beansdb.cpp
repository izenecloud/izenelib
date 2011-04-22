

#include <string>
#include <stdlib.h>
#include <sys/time.h>

#include <fstream>
#include <iostream>
#include <iomanip>

#include <map>
#include <3rdparty/am/beansdb/hstore.h>

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

/// Speed test them!
int main()
{
    test_beansdb(100);
}
