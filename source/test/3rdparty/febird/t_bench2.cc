#include <stdio.h>

#include <vector>
#include <map>
#include <string>

#include <fstream>

//#include <boost/date_time/posix_time/ptime.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <febird/io/var_int_boost_serialization.h>

#include <febird/io/DataIO.h>
#include <febird/io/StreamBuffer.h>
#include <febird/io/FileStream.h>

using namespace std;
using namespace boost::archive;
using namespace febird;

namespace febird {
struct MyData1
{
	uint32_t a, b, c;
	uint32_t d[5];
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & a & b & c & d;
    }
	DATA_IO_LOAD_SAVE(MyData1, &a&b&c&d)
// 	template<class DataIO> void load(DataIO& dio)	    { dio &a&b&c&d; }
// 	template<class DataIO> void save(DataIO& dio) const { dio &a&b&c&d; }
// 	DATA_IO_OPTIMIZE_ELEMEN_LOAD(DATA_IO_IDENTITY, MyData1, &a&b&c&d)
// 	DATA_IO_OPTIMIZE_ELEMEN_SAVE(DATA_IO_IDENTITY, MyData1, &a&b&c&d)
// 	DATA_IO_OPTIMIZE_VECTOR_LOAD(MyData1, MyData1, &a&b&c&d) 
// 	DATA_IO_OPTIMIZE_VECTOR_SAVE(MyData1, MyData1, &a&b&c&d) 
// 	DATA_IO_OPTIMIZE_ARRAY_LOAD(MyData1, MyData1, &a&b&c&d) 
// 	DATA_IO_OPTIMIZE_ARRAY_SAVE(MyData1, MyData1, &a&b&c&d) 
// 	DATA_IO_REG_LOAD(MyData1)	
// 	DATA_IO_REG_SAVE(MyData1)
};
DataIO_IsDump_TypeTrue1(MyData1)

using febird::MyData1;

struct MyData2
{
	uint32_t a, b, c, d;
	MyData1 e;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & a & b & c & d & e;
    }
    DATA_IO_LOAD_SAVE(MyData2, &a&b&c&d&e)
};
}

struct MyData3
{
	uint32_t a, b, c;
	uint32_t d;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & a & b & c & d;
    }
};
//DATA_IO_LOAD_SAVE_EV(MyData3, 2, &a&b&c&d)
DATA_IO_DUMP_RAW_MEM(MyData3)

struct VarIntD
{
	var_uint32_t a, b, c, d, e, f;

	VarIntD()
	{
		a = 127;
		b = 128;
		c = 128*128;
		d = 128*128*128;
		e = 128*128*128*128;
		f = 1;
	}

	DATA_IO_LOAD_SAVE(VarIntD, &a&b&c&d&e&f)

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & a & b & c & d & e & f;
    }
};

struct TimeCost
{
	int64_t d0, d1, d2, d3, d4, d5, d6;

	TimeCost() { d0 = 0; d1 = 0; d2 = 0; d3 = 0; d4 = 0; d5 = 0; d6 = 0; }
};

typedef pair<MyData2, MyData3> MyData23;
struct Bench
{
	int loop, count;
	vector<pair<int,int> > v0;
	vector<MyData1> v1;
	vector<string> v2;
	map<int, string> v3;
	vector<MyData23> v4;

	TimeCost bdo, bdi;
#if defined(_WIN32) || defined(_WIN64)
	LARGE_INTEGER freq;
#endif

Bench(int loop, int count): loop(loop), count(count)
{
#if defined(_WIN32) || defined(_WIN64)
	QueryPerformanceFrequency(&freq);
#endif
	for (int i = 0; i < count; ++i)
	{
		char szbuf[32];
		int x = rand();
	//	itoa(x, szbuf, 10);
		sprintf(szbuf, "%016d", x);
		v0.push_back(make_pair(x, x));
		v2.push_back(szbuf);
		v3.insert(make_pair(x, szbuf));

		MyData1 md1;
		md1.a = md1.b = md1.c = md1.d[0] = x;
		v1.push_back(md1);

		MyData2 md2;
		MyData3 md3;
		md2.a = md2.b = md2.c = md2.d = x;
		md2.e = md1;
		md3.a = md3.b = md3.c = md3.d = x;
		v4.push_back(make_pair(md2, md3));
	}
}

void print_report(const char* prefix, const TimeCost& d, const TimeCost& bd)
{
	printf("%s: loop=%d, time[febird, boost, ratio=b/f] in us\n", prefix, loop);
	printf("    vector<pair<int,int> >.size=%5d, time[%7ld, %7ld, %9.4f]\n", count, us(d.d0), us(bd.d0), (double)bd.d0/d.d0);
	printf("    vector<MyData1>       .size=%5d, time[%7ld, %7ld, %9.4f]\n", count, us(d.d1), us(bd.d1), (double)bd.d1/d.d1);
	printf("    vector<string>        .size=%5d, time[%7ld, %7ld, %9.4f]\n", count, us(d.d2), us(bd.d2), (double)bd.d2/d.d2);
	printf("    map<int,string>       .size=%5d, time[%7ld, %7ld, %9.4f]\n",(int)v3.size(), us(d.d3), us(bd.d3), (double)bd.d3/d.d3);
	printf("    loop{MyData1 },  loop count=%5d, time[%7ld, %7ld, %9.4f]\n", count, us(d.d4), us(bd.d4), (double)bd.d4/d.d4);
	printf("    loop{VarIntD },  loop count=%5d, time[%7ld, %7ld, %9.4f]\n", count, us(d.d5), us(bd.d5), (double)bd.d5/d.d5);
	printf("    vector<MyData23>,     .size=%5d, time[%7ld, %7ld, %9.4f]\n", count, us(d.d6), us(bd.d6), (double)bd.d6/d.d6);
	printf("\n");
}

#if defined(_WIN32) || defined(_WIN64)
	void QueryPerformanceCounter(int64_t* x)
	{
		LARGE_INTEGER y;
		::QueryPerformanceCounter(&y);
		*x = y.QuadPart;
	}
	int64_t us(int64_t x) { return x*1000000/freq.QuadPart;	}
#else
	void QueryPerformanceCounter(int64_t* x)
	{
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		*x = (int64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
	}
	int64_t us(int64_t x) { return x/1000; }
#endif

	template<class ArchiveT>
	void test_serialize(ArchiveT& ar, TimeCost& d)
	{
		int64_t c0, c1, c2, c3, c4, c5, c6, c7;

		QueryPerformanceCounter(&c0);
		ar & v0;
		QueryPerformanceCounter(&c1);
		ar & v1;
		QueryPerformanceCounter(&c2);
		ar & v2;
		QueryPerformanceCounter(&c3);
		ar & v3;
		QueryPerformanceCounter(&c4);

		for (int i=0, n=v0.size(); i < n; ++i)
		{
			MyData1 md1;
			ar & md1;
		}
		QueryPerformanceCounter(&c5);

		for (int i=0, n=v0.size(); i < n; ++i)
		{
			VarIntD vid;
			ar & vid;
		}
		QueryPerformanceCounter(&c6);

		ar & v4;
		QueryPerformanceCounter(&c7);

		d.d0 += c1 - c0;
		d.d1 += c2 - c1;
		d.d2 += c3 - c2;
		d.d3 += c4 - c3;
		d.d4 += c5 - c4;
		d.d5 += c6 - c5;
		d.d6 += c7 - c6;
	}

	void run_boost()
	{
		{
			std::ofstream file("boost.bin", ios::binary);
			for (int i = 0; i < loop; ++i)
			{
				file.seekp(0);
				binary_oarchive ar(file);
			//	test_serialize(ar, bdo);
				TimeCost& d = bdo;
				test_serialize(ar, d);
			}
			print_report("boost_bin_save", bdo, bdo);
		}
		{
			std::ifstream file("boost.bin", ios::binary);
			for (int i = 0; i < loop; ++i)
			{
				file.seekg(0);
				binary_iarchive ar(file);
			//	test_serialize(ar, bdi);
				TimeCost& d = bdi;
				test_serialize(ar, d);
			}
			print_report("boost_bin_load", bdi, bdi);
		}
	}


template<class Archive>
void file_ar_run(const char* prefix, const char* mode, Archive*)
{
	FileStream file0("febird.bin", mode);
	setvbuf(file0.fp(), 0, _IONBF, 0);

	TimeCost d;
	for (int i = 0; i < loop; ++i)
	{
		file0.seek(0);
		Archive ar;
		ar.attach(&file0);
		test_serialize(ar, d);
	}
	print_report(prefix, d, Archive::is_loading::value ? bdi : bdo);
}

template<class Archive>
Archive new_mem_ar(unsigned char* buffer, int length, Archive*, MinMemIO*)
{
	Archive ar; ar.set(buffer);
	return ar;
}
template<class Archive>
Archive new_mem_ar(unsigned char* buffer, int length, Archive*, MemIO*)
{
	Archive ar; ar.set(buffer, length);
	return ar;
}

template<class Archive>
void mem_ar_run(const char* prefix, unsigned char* buffer, int length, Archive*)
{
	TimeCost d;
//	unsigned char* tail = 0;
	for (int i = 0; i < loop; ++i)
	{
		Archive ar = new_mem_ar(buffer, length, (Archive*)0, (typename Archive::stream_t*)0);
		test_serialize(ar, d);
	//	tail = ar.current();

		if (ar.diff(buffer) > length)
		{
			fprintf(stderr, "buffer overrun\n");
			abort();
		}
	}
	print_report(prefix, d, Archive::is_loading::value ? bdi : bdo);
}

template<class Archive>
void mem_ar_run(const char* prefix, Archive*)
{
	TimeCost d;
	for (int i = 0; i < loop; ++i)
	{
		Archive ar;
		test_serialize(ar, d);
	}
	print_report(prefix, d, Archive::is_loading::value ? bdi : bdo);
}

void run_febird()
{
	file_ar_run("File Save Native", "wb+", (NativeDataOutput<OutputBuffer>*)0);
 	file_ar_run("File Load Native", "rb" , (NativeDataInput<InputBuffer>*)0);
	file_ar_run("File Save Portable", "wb+", (PortableDataOutput<OutputBuffer>*)0);
	file_ar_run("File Load Portable", "rb" , (PortableDataInput<InputBuffer>*)0);

	file_ar_run("File Save NoVarInt Native", "wb+", (NativeNoVarIntOutput<OutputBuffer>*)0);
	file_ar_run("File Load NoVarInt Native", "rb" , (NativeNoVarIntInput<InputBuffer>*)0);
	file_ar_run("File Save NoVarInt Portable", "wb+", (PortableNoVarIntOutput<OutputBuffer>*)0);
	file_ar_run("File Load NoVarInt Portable", "rb" , (PortableNoVarIntInput<InputBuffer>*)0);

	int length = count*256;
	vector<unsigned char> vbuffer(length);
	unsigned char *buffer = &*vbuffer.begin();
	mem_ar_run("Memory Save Native", buffer, length, (NativeDataOutput<MemIO>*)(0));
	mem_ar_run("Memory Load Native", buffer, length, (NativeDataInput<MemIO>*)(0));
	mem_ar_run("Memory Save Portable", buffer, length, (PortableDataOutput<MemIO>*)(0));
	mem_ar_run("Memory Load Portable", buffer, length, (PortableDataInput<MemIO>*)(0));

	mem_ar_run("Memory Save NoVarInt Native", buffer, length, (NativeNoVarIntOutput<MemIO>*)(0));
	mem_ar_run("Memory Load NoVarInt Native", buffer, length, (NativeNoVarIntInput<MemIO>*)(0));
	mem_ar_run("Memory Save NoVarInt Portable", buffer, length, (PortableNoVarIntOutput<MemIO>*)(0));
	mem_ar_run("Memory Load NoVarInt Portable", buffer, length, (PortableNoVarIntInput<MemIO>*)(0));

	mem_ar_run("Uncheck Save Native", buffer, length, (NativeDataOutput<MinMemIO>*)(0));
	mem_ar_run("Uncheck Load Native", buffer, length, (NativeDataInput<MinMemIO>*)(0));
	mem_ar_run("Uncheck Save Portable", buffer, length, (PortableDataOutput<MinMemIO>*)(0));
	mem_ar_run("Uncheck Load Portable", buffer, length, (PortableDataInput<MinMemIO>*)(0));

	mem_ar_run("Uncheck Save NoVarInt Native", buffer, length, (NativeNoVarIntOutput<MinMemIO>*)(0));
	mem_ar_run("Uncheck Load NoVarInt Native", buffer, length, (NativeNoVarIntInput<MinMemIO>*)(0));
	mem_ar_run("Uncheck Save NoVarInt Portable", buffer, length, (PortableNoVarIntOutput<MinMemIO>*)(0));
	mem_ar_run("Uncheck Load NoVarInt Portable", buffer, length, (PortableNoVarIntInput<MinMemIO>*)(0));

	mem_ar_run("Memory Save AutoGrown", (NativeDataOutput<AutoGrownMemIO>*)(0));
//	mem_ar_run("Memory Load AutoGrown", (NativeDataInput<AutoGrownMemIO>*)(0));
	
}
};


int main(int argc, char* argv[])
{
	int loop = argc >= 2 ? atoi(argv[1]) : 1000;
	int count = argc >= 3 ? atoi(argv[2]) : 1000;

	printf("sizeof MyData1=%02d\n", (int)sizeof(MyData1));
	printf("sizeof MyData2=%02d\n", (int)sizeof(MyData2));
	printf("sizeof MyData3=%02d\n", (int)sizeof(MyData3));
	printf("sizeof MyData23=%02d\n", (int)sizeof(MyData23));
	printf("sizeof pair<int,int>=%02d\n", (int)sizeof(pair<int,int>));

	Bench ben(loop, count);
	try {
		ben.run_boost();
		ben.run_febird();
	}
	catch (const std::exception& exp)
	{
		fprintf(stderr, "exception: %s\n", exp.what());
	}
	return 0;
}


