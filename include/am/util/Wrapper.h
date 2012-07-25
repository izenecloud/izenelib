#ifndef WRAPPER_H_
#define WRAPPER_H_

#include <types.h>
#include <am/util/DbObj.h>
#include <am/concept/DataType.h>
#include <sstream>

// for serialization
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
//#include <util/ustring/UString.h>

BEGIN_SERIALIZATION

const int archive_flags_u = boost::archive::no_header | boost::archive::no_codecvt;

template<class T> inline void read_image(T& dat, const DbObjPtr& ptr)
{
    std::stringstream istr((char*)ptr->getData());
	{
		boost::archive::text_iarchive ia(istr, archive_flags_u);
		ia & dat;
	}
	//make sure oa is deconstructed to avoid stream_error exception.

}

template<class T> inline void write_image(const T& dat, DbObjPtr& ptr)
{
    std::stringstream ostr;
	{
		boost::archive::text_oarchive oa(ostr, archive_flags_u);
		oa & dat;
	}
	//make sure oa is deconstructed to avoid stream_error exception.
	ptr->setData(ostr.str().c_str(), ostr.str().size() );
}

/*template<> inline void read_image<YString>(YString& dat, const DbObjPtr& ptr)
 * {
 dat = (YString)((char*)ptr->getData() );
 }

 template<> inline void write_image<YString>(const YString& dat, DbObjPtr& ptr)
 {
 ptr->setData(dat.c_str(), dat.size()+1);
 }*/

/*
template<> inline void read_image<string>(string& dat, const DbObjPtr& ptr)
{
	dat = (string)((char*)ptr->getData() );
}

template<> inline void write_image<string>(const string& dat, DbObjPtr& ptr)
{
	ptr->setData(dat.c_str(), dat.size()+1);
}*/

/*
 template<> inline void read_image<char* >(char* dat, const DbObjPtr& ptr)
 {
 dat = ( (char*)ptr->getData() );
 }

 template<> inline void write_image<char* >(const char* dat, DbObjPtr& ptr)
 {
 ptr->setData(dat, strlen(dat)+1);
 }*/

template<> inline void read_image<int>(int& dat, const DbObjPtr& ptr)
{
	memcpy(&dat, ptr->getData(), ptr->getSize());
}

template<> inline void write_image<int>(const int& dat, DbObjPtr& ptr)
{
	ptr->setData(&dat, sizeof(int));
}

template<> inline void read_image<unsigned int>(unsigned int& dat, const DbObjPtr& ptr)
{
	memcpy(&dat, ptr->getData(), ptr->getSize());
}

template<> inline void write_image<unsigned int>(const unsigned int& dat, DbObjPtr& ptr)
{
	ptr->setData(&dat, sizeof(unsigned int));
}

template<> inline void read_image<vector<int> >(vector<int>& dat, const DbObjPtr& ptr)
{
	dat.resize( ptr->getSize()/sizeof(int) );
	memcpy(&dat[0], ptr->getData(), ptr->getSize() );
}

template<> inline void write_image<vector<int> >(const vector<int>& dat, DbObjPtr& ptr)
{
	ptr->setData( &dat[0], sizeof(int)*dat.size() );
}

template<> inline void read_image<vector<unsigned int> >(vector<unsigned int>& dat, const DbObjPtr& ptr)
{
	dat.resize( ptr->getSize()/sizeof(unsigned int) );
	memcpy(&dat[0], ptr->getData(), ptr->getSize());
}

template<> inline void write_image<vector<unsigned int> >(const vector<unsigned int>& dat, DbObjPtr& ptr)
{
	ptr->setData( &dat[0], sizeof(int)*dat.size() );
}


template<> inline void read_image<long>(long& dat, const DbObjPtr& ptr)
{
	memcpy(&dat, ptr->getData(), ptr->getSize());
}

template<> inline void write_image<long>(const long& dat, DbObjPtr& ptr)
{
	ptr->setData(&dat, sizeof(long));
}

template<> inline void read_image<unsigned long>(unsigned long& dat, const DbObjPtr& ptr)
{
	memcpy(&dat, ptr->getData(), ptr->getSize());
}

template<> inline void write_image<unsigned long>(const unsigned long& dat, DbObjPtr& ptr)
{
	ptr->setData(&dat, sizeof(unsigned long));
}

template<> inline void read_image<short>(short& dat, const DbObjPtr& ptr)
{
	memcpy(&dat, ptr->getData(), ptr->getSize());
}

template<> inline void write_image<short>(const short& dat, DbObjPtr& ptr)
{
	ptr->setData(&dat, sizeof(short));
}

template<> inline void read_image<unsigned short>(unsigned short& dat, const DbObjPtr& ptr)
{
	memcpy(&dat, ptr->getData(), ptr->getSize());
}

template<> inline void write_image<unsigned short>(const unsigned short& dat, DbObjPtr& ptr)
{
	ptr->setData(&dat, sizeof(unsigned short));
}

template<> inline void read_image<float>(float& dat, const DbObjPtr& ptr)
{
	memcpy(&dat, ptr->getData(), ptr->getSize());
}

template<> inline void write_image<float>(const float& dat, DbObjPtr& ptr)
{
	ptr->setData(&dat, sizeof(float));
}

template<> inline void read_image<double>(double& dat, const DbObjPtr& ptr)
{
	memcpy(&dat, ptr->getData(), ptr->getSize());
}

template<> inline void write_image<double>(const double& dat, DbObjPtr& ptr)
{
	ptr->setData(&dat, sizeof(double));
}

template<> inline void read_image<NullType>(NullType& dat, const DbObjPtr& ptr)
{
}

template<> inline void write_image<NullType>(const NullType& dat, DbObjPtr& ptr)
{

}

/* example for struct
 *
 *
 template<> inline void read_image<Value>(Value& dat, const DbObjPtr& ptr)
 {
 memcpy(&dat, ptr->getData(), sizeof(myKey));
 }

 template<> inline void write_image<Value>(const Value& dat, DbObjPtr& ptr)
 {
 ptr->setData(&dat, sizeof(dat));
 }*/

/*
 inline static int _defaultCompare(const DbObjPtr& obj1, const DbObjPtr& obj2)
 {
 if (obj1 == obj2)
 {
 return 0;
 }
 return memcmp(obj1->getData(), obj2->getData(), min(obj1->getSize(),
 obj2->getSize()) );
 }*/


END_SERIALIZATION

#endif /*WRAPPER_H_*/
