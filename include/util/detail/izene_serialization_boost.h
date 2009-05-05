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


NS_IZENELIB_UTIL_BEGIN


const int archive_flags = archive::no_header | archive::no_codecvt;
//const int archive_flags = archive::no_codecvt;


template<typename T> class izene_serialization_boost {
	std::stringstream ostr;
	boost::archive::text_oarchive oa;
public:
	izene_serialization_boost(const T& dat) :
		oa(ostr, archive_flags) {
		oa & dat;
	}
	void write_image(void* &ptr, size_t& size) {
		ptr = (void*)ostr.str().c_str();
		size = ostr.str().size();
	}

};

template<typename T> class izene_deserialization_boost {
	std::stringstream istr;
	boost::archive::text_iarchive ia;
public:
	izene_deserialization_boost(void* &ptr, size_t& size) :
		istr((char*)ptr), ia(istr, archive_flags) {
	}
	void read_image(T& dat) {
		ia & dat;
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
