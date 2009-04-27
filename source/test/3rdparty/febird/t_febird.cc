#include <stdio.h>
#include <febird/io/DataIO.h>
#include <febird/io/StreamBuffer.h>
#include <febird/io/FileStream.h>

using namespace boost;
using namespace febird;
using namespace std;

using febird::ulong;
using febird::ushort;

typedef unsigned char uchar;
typedef signed char schar;

typedef long long longlong;
typedef unsigned long long ulonglong;

#include "data_febird.h"

int main(int argc, char* argv[])
{
	febird_pod_0 bp0;
	febird_pod_1 bp1;
	febird_pod_2 bp2;
	febird_complex_0 bc0;
	febird_complex_1 bc1;
	febird_complex_2 bc2;

	{
		FileStream file("boost.bin", "w+");
		NativeDataOutput<OutputBuffer> ar;
		ar.attach(&file);
		ar & bp0;
		ar & bp1;
		ar & bp2;
		ar & bc0;
		ar & bc1;
		ar & bc2;
		febird_foo(ar);
	}

	{
		FileStream file("boost.bin", "r");
		NativeDataInput<InputBuffer> ar;
		ar.attach(&file);
		ar & bp0;
		ar & bp1;
		ar & bp2;
		ar & bc0;
		ar & bc1;
		ar & bc2;
		febird_foo(ar);
	}
	return 0;
}


