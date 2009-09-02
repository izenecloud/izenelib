

#include <string>
#include <stdlib.h>
#include <sys/time.h>

#include <fstream>
#include <iostream>
#include <iomanip>

#include <map>
#include <3rdparty/am/stx/btree_map>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <3rdparty/am/judyhash/judy_map.h>
#include <am/cccr_hash/cccr_hash.h>
#include <am/sdb_btree/sdb_btree.h>
#include <am/sdb_hash/sdb_hash.h>

#include <util/ClockTimer.h>
#include <boost/timer.hpp>

#include <assert.h>

using namespace izenelib::util;
using namespace izenelib::am;

static int data_size = 5000000;
static unsigned *int_data;
static char **str_data;

void init_data()
{
	int i;
	char buf[256];
	std::cout<<"generating data... "<<std::endl;
	srand48(11);
	int_data = (unsigned*)calloc(data_size, sizeof(unsigned));
	str_data = (char**)calloc(data_size, sizeof(char*));
	for (i = 0; i < data_size; ++i) {
		int_data[i] = (unsigned)(data_size * drand48() / 4) * 271828183u;
		sprintf(buf, "%x", int_data[i]);
		str_data[i] = strdup(buf);
	}
	std::cout<<"done!\n";
}

void destroy_data()
{
	int i;
	for (i = 0; i < data_size; ++i) free(str_data[i]);
	free(str_data); free(int_data);
}

typedef stx::btree_map<int, int> btreemap;
void test_btreemap(int N, const unsigned *data)
{
  btreemap m1;

  for (int i = 0; i < N; ++i) 
  {
	std::pair<btreemap::iterator, bool> p = m1.insert(std::pair<unsigned, int>(data[i], i));
  }
  for(btreemap::iterator iter = m1.begin(); iter != m1.end(); ++iter)
  {
    int v = iter->second;
  }
}

typedef rde::hash_map<unsigned, int> inthash;

int test_int(int N, const unsigned *data)
{
	int i, ret;
	
	inthash *h = new inthash();
	for (i = 0; i < N; ++i) {
		rde::pair<inthash::iterator, bool> p = h->insert(rde::pair<unsigned, int>(data[i], i));
		if (p.second == false) h->erase(p.first);
	}
    for(inthash::iterator iter = h->begin(); iter != h->end(); ++iter)	
    {
      int v = iter->second;
    }
	ret = h->size();
	delete h;
	return ret;
}

typedef rde::hash_map<const char*, int> strhash;

int test_str(int N, char * const *data)
{
	int i, ret;
	strhash *h = new strhash();
	for (i = 0; i < N; ++i) {
		rde::pair<strhash::iterator, bool> p = h->insert(rde::pair<const char*, int>(data[i], i));
		if (p.second == false) h->erase(p.first);
	}
	ret = h->size();
	delete h;
	return ret;
}

typedef cccr_hash<unsigned, int, 17> CCCR_INT;
void test_ccc(int N, const unsigned *data)
{
	int i;
	
	CCCR_INT h;
	for (i = 0; i < N; ++i) 
	{
	    h.insert(data[i], i);
	}
}

typedef sdb_btree<unsigned, int> SDBTREE_INT;
void test_sdbbtree(int N, const unsigned *data)
{
	int i;
	ClockTimer t;	
	SDBTREE_INT h;
	h.setCacheSize(10000000);
	h.open();
	for (i = 0; i < N; ++i) 
	{
	    h.insert(data[i], i);
	}
	std::cout<<"time elapsed for sdb btree "<<t.elapsed()<<std::endl;		
}

typedef sdb_hash<unsigned, int> SDBHASH_INT;
void test_sdbhash(int N, const unsigned *data)
{
	int i;
	
	SDBHASH_INT h;
	h.setCacheSize(10000000);
	h.open();
	for (i = 0; i < N; ++i) 
	{
	    h.insert(data[i], i);
	}
}

typedef std::map<int, int> stdmap;
void test_stdmap(int N, const unsigned *data)
{
  stdmap m1;

  for (int i = 0; i < N; ++i) 
  {
	std::pair<stdmap::iterator, bool> p = m1.insert(std::pair<unsigned, int>(data[i], i));
  }
  for(stdmap::iterator iter = m1.begin(); iter != m1.end(); ++iter)
  {
    int v = iter->second;
  }
}
template <class T>
struct hash_ident {
	size_t operator () (int a) const {
		return (size_t) a;
	}
};

typedef judy_map_l<unsigned, int,hash_ident<unsigned>, std::equal_to <int> > judymap;

void test_judy(int N, const unsigned *data)
{
  judymap m1;

  for (int i = 0; i < N; ++i) 
  {
	std::pair<judymap::iterator, bool> p = m1.insert(std::pair<unsigned, int>(data[i], i));
  }

  for(judymap::iterator iter = m1.begin(); iter != m1.end(); ++iter)
  {
    int v = iter->second;
  }
}

/// Speed test them!
int main()
{
	init_data();
	ClockTimer t;
	test_judy(5000000, int_data);
	std::cout<<"time elapsed for judy "<<t.elapsed()<<std::endl;	
	test_int(5000000, int_data);
	std::cout<<"time elapsed for rde "<<t.elapsed()<<std::endl;
    //test_str(5000000, str_data);	
    //std::cout<<"time elapsed for rde string"<<t.elapsed()<<std::endl;
	test_btreemap(5000000, int_data);
	std::cout<<"time elapsed for btree "<<t.elapsed()<<std::endl;	
	//test_ccc(5000000, int_data);
	//std::cout<<"time elapsed for ccc "<<t.elapsed()<<std::endl;	
	test_stdmap(5000000, int_data);
	std::cout<<"time elapsed for std "<<t.elapsed()<<std::endl;	
	//test_sdbbtree(5000000, int_data);
	//std::cout<<"time elapsed for sdb btree "<<t.elapsed()<<std::endl;	
	//test_sdbhash(5000000, int_data);
	//std::cout<<"time elapsed for sdb hash "<<t.elapsed()<<std::endl;	
	destroy_data();	

}
