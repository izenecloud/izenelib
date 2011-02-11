#include <iostream>

#include <rde_hashmap/hash_map.h>

using namespace rde;

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


struct eqstr {
	inline bool operator()(const char *s1, const char *s2) const {
		return strcmp(s1, s2) == 0;
    }
};

struct hashf_int {
	inline unsigned operator()(unsigned key) const {
		return key;
	}
};
struct hasheq_int {
	inline unsigned operator()(unsigned key1, unsigned key2) const {
		return key1 == key2;
	}
};

typedef rde::hash_map<unsigned, int> inthash;
typedef rde::hash_map<const char*, int> strhash;

int test_int(int N, const unsigned *data)
{
	int i, ret;
	
	inthash *h = new inthash();
	for (i = 0; i < N; ++i) {
		pair<inthash::iterator, bool> p = h->insert(pair<unsigned, int>(data[i], i));
		if (p.second == false) h->erase(p.first);
	}
	ret = h->size();
	delete h;
	return ret;
}


int test_str(int N, char * const *data)
{
	int i, ret;
	strhash *h = new strhash();
	for (i = 0; i < N; ++i) {
		pair<strhash::iterator, bool> p = h->insert(pair<const char*, int>(data[i], i));
		if (p.second == false) h->erase(p.first);
	}
	ret = h->size();
	delete h;
	return ret;
}


int main(int argc, char *argv[])
{
	init_data();
	test_int(5000000, int_data);
	test_str(5000000, str_data);
	destroy_data();	
	return 0;
}
