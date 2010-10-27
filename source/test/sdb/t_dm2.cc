#include <boost/test/unit_test.hpp>
#include <am/sdb_storage/sdb_storage_mm1.h>

using namespace izenelib::am;

static string fileName = "1.mmap";
static size_t mapSize = 1024 * 1024 * 1024;
static int seq = 26;

BOOST_AUTO_TEST_CASE(t_memory_map1)
{

	{
		struct stat statbuf;
		bool creating = stat(fileName.c_str(), &statbuf);
		FILE *fp = fopen(fileName.c_str(), creating ? "w+b" : "r+b");
		fseek(fp, seq * mapSize, SEEK_SET);
		putw(0, fp);
		fclose(fp);
	}

	memory_map map1(fileName, 0, seq * mapSize);
	for (int i = 0; i < seq; i++) {
		map1.setOffset(i * mapSize);
		size_t size = rand() % mapSize;
		cout << size << endl;
		map1.write(&size, sizeof(size_t));
	}
}

BOOST_AUTO_TEST_CASE(t_memory_map2)
{
	memory_map map1(fileName, 0, seq * mapSize);
	for (int i = 0; i < seq; i++) {
    size_t	size;
	map1.read(i*mapSize, &size, sizeof(size_t));
	cout<<size<<endl;
	}
}


