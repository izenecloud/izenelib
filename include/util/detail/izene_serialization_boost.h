#ifndef IZENE_SERIALIZATION_BOOST_H_
#define IZENE_SERIALIZATION_BOOST_H_

#include "izene_type_traits.h"
#include <types.h>

// for serialization
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <am/util/Wrapper.h>


NS_IZENELIB_UTIL_BEGIN

/*
const int archive_flags = archive::no_header | archive::no_codecvt;


template<typename T> class izene_serialization_boost1 {
	std::stringstream ostr;
	boost::archive::text_oarchive oa;
public:
	izene_serialization_boost1(const T& dat) :
		oa(ostr, archive_flags) {
		oa & dat;
	}
	void write_image(char* &ptr, size_t& size) {
		ptr = (char*)ostr.str().c_str();
		size = ostr.str().size();
	}

};

template<typename T> class izene_deserialization_boost1 {
	std::stringstream istr;
	boost::archive::text_iarchive ia;
public:
	izene_deserialization_boost1(char* &ptr, size_t& size) :
		istr(ptr), ia(istr, archive_flags) {
	}
	void read_image(T& dat) {
		ia & dat;
	}
};

*/


template<typename T> class izene_serialization_boost {		
	const T &dat_;
	izenelib::am::util::DbObjPtr dptr;
public:
	izene_serialization_boost(const T& dat) :dat_(dat){
		dptr.reset(new izenelib::am::util::DbObj);		
	}
	void write_image(char* &ptr, size_t& size) {		
		izenelib::am::util::write_image(dat_, dptr);
		ptr = (char*)dptr->getData();
		size = dptr->getSize();
	}
};

template<typename T> class izene_deserialization_boost {
	izenelib::am::util::DbObjPtr dptr;
public:
	izene_deserialization_boost(const char* ptr, const size_t size)
	{
		dptr.reset(new izenelib::am::util::DbObj(ptr, size));
	}
	void read_image(T& dat) {
		izenelib::am::util::read_image(dat, dptr);
	}
};


/*
 class izene_streambuf:public std::streambuf
 {
 public:
 char* str() const
 {
 return std::streambuf::eback();
 }
 void setbuf(char* ptr, size_t sz)
 {
 std::streambuf::setbuf(ptr, sz);
 }
 
 };


 template<typename T>
 class izene_serialization_boost1
 {	
 boost::asio::streambuf ostr;	
 boost::archive::binary_oarchive oa;	
 public:
 izene_serialization_boost(const T& dat):oa(ostr, archive_flags)
 {
 oa & dat;
 }
 void write_image(void* &ptr, size_t& size){
 ptr = (void*)ostr.c_str();
 size = ostr.size();		
 }	
 };

 template<typename T>
 class izene_deserialization_boost1
 {
 std::stringstream::istr;
 boost::archive::binary_iarchive ia;	
 public:		
 izene_deserialization_boost(void* &ptr, size_t& size):istr(ptr), ia(istr, archive_flags)
 {
 }
 void read_image(T& dat)
 {
 ia & dat;
 }		
 };*/



NS_IZENELIB_UTIL_END



#endif /*IZENE_SERIALIAZATION_BOOST_H_*/
