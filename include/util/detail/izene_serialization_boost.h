#ifndef IZENE_SERIALIZATION_BOOST_H_
#define IZENE_SERIALIZATION_BOOST_H_

#include "izene_type_traits.h"
#include <types.h>
#include <am/util/Wrapper.h>

// for serialization
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/shared_ptr.hpp> 

#include <boost/shared_ptr.hpp>
#include <util/streambuf.h>
#include <sstream>

using namespace std;

NS_IZENELIB_UTIL_BEGIN

const int archive_flags = archive::no_header | archive::no_codecvt;


template<typename T> class izene_serialization_boost {
izene_streambuf buf;
public:
izene_serialization_boost(const T& dat) {

boost::archive::binary_oarchive oa(buf, archive_flags);
oa & dat;
}

void write_image(char * &ptr, size_t& size) {
ptr = buf.data();
size = buf.size();
}
};

template<typename T> class izene_deserialization_boost {
izene_streambuf buf;
public:
izene_deserialization_boost(const char* ptr, const size_t size) {
buf.sputn(ptr, size);
}
void read_image(T& dat) {
boost::archive::binary_iarchive ia(buf, archive_flags);
ia & dat;
}
};


/*
template<typename T> class izene_serialization_boost {
	izenelib::am::util::DbObjPtr ptr_;
public:
	izene_serialization_boost(const T& dat):ptr_(new izenelib::am::util::DbObj)
	{		
		izenelib::am::util::write_image<T>(dat, ptr_);		
	}

	void write_image(char * &ptr, size_t& size) {
		ptr = (char*)ptr_->getData();
		size = ptr_->getSize();
	}
};

template<typename T> class izene_deserialization_boost {
	izenelib::am::util::DbObjPtr ptr_;
public:
	izene_deserialization_boost(const char* ptr, const size_t size):ptr_(new izenelib::am::util::DbObj(ptr, size)) {
		
	}
	void read_image(T& dat) {
		izenelib::am::util::read_image<T>(dat, ptr_);
	}
};*/


/*
 template<typename T> class izene_serialization_boost {
 size_t size_;
 izene_streambuf b;
 ostream ostr;
 public:
 izene_serialization_boost(const T& dat) :
 ostr(&b) {
 {
 boost::archive::binary_oarchive oa(ostr, archive_flags);
 oa & dat;
 }
 size_ = ((izene_streambuf *)ostr.rdbuf() )->size();
 }
 ~izene_serialization_boost() {
 }
 void write_image(char * &ptr, size_t& size) {
 ptr = ((izene_streambuf *)ostr.rdbuf() )->data();
 size = size_;
 }
 };

 template<typename T> class izene_deserialization_boost {
 stringbuf b;
 istream istr;
 public:
 izene_deserialization_boost(const char* ptr, const size_t size) :
 istr(&b) {
 istr.rdbuf()->pubsetbuf((char* )ptr, size);
 }
 void read_image(T& dat) {
 boost::archive::binary_iarchive ia(istr, archive_flags);
 ia & dat;
 }
 };*/

/*
*/


 template<typename T> class izene_serialization_boost1 {
 size_t size_;
 izene_streambuf b;
 ostream ostr;
 public:
 izene_serialization_boost1(const T& dat) :
 ostr(&b) {
 {
 boost::archive::binary_oarchive oa(ostr, archive_flags);
 oa & dat;
 }
 size_ = ((izene_streambuf *)ostr.rdbuf() )->size();
 }
 ~izene_serialization_boost1() {
 }
 void write_image(char * &ptr, size_t& size) {
 ptr = ((izene_streambuf *)ostr.rdbuf() )->data();
 size = size_;
 }
 };

 template<typename T> class izene_deserialization_boost1 {
 stringbuf b;
 istream istr;
 public:
 izene_deserialization_boost1(const char* ptr, const size_t size) :
 istr(&b) {
 istr.rdbuf()->pubsetbuf((char* )ptr, size);
 }
 void read_image(T& dat) {
 boost::archive::binary_iarchive ia(istr, archive_flags);
 ia & dat;
 }
 };

 template<typename T> class izene_serialization_boost2 {
 size_t size_;
 izene_streambuf b;
 ostream ostr;
 public:
 izene_serialization_boost2(const T& dat) :
 ostr(&b) {
 {
 boost::archive::text_oarchive oa(ostr, archive_flags);
 oa & dat;
 }
 size_ = ((izene_streambuf *)ostr.rdbuf() )->size();
 }
 ~izene_serialization_boost2() {
 }
 void write_image(char * &ptr, size_t& size) {
 ptr = ((izene_streambuf *)ostr.rdbuf() )->data();
 size = size_;
 }
 };

 template<typename T> class izene_deserialization_boost2 {
 stringbuf b;
 istream istr;
 public:
 izene_deserialization_boost2(const char* ptr, const size_t size) :
 istr(&b) {
 istr.rdbuf()->pubsetbuf((char* )ptr, size);
 }
 void read_image(T& dat) {
 boost::archive::text_iarchive ia(istr, archive_flags);
 ia & dat;
 }
 };

NS_IZENELIB_UTIL_END

#endif /*IZENE_SERIALIAZATION_BOOST_H_*/
