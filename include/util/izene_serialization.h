#ifndef IZENE_SERIALIZATION_H_
#define IZENE_SERIALIZATION_H_

#include "detail/izene_serialization_boost.h"
#include "detail/izene_serialization_febird.h"
#include "detail/izene_serialization_memcpy.h"

NS_IZENELIB_UTIL_BEGIN

template<typename T, bool isMemcpy = false, bool isFeBird = false>
struct izene_serial_type
{
	typedef izene_serialization_boost<T> stype;
	typedef izene_deserialization_boost<T> dtype;
};

template<typename T>
struct izene_serial_type<T, true, false>
{
	typedef izene_serialization_memcpy<T> stype;
	typedef izene_deserialization_memcpy<T> dtype;
};

template<typename T>
struct izene_serial_type<T, true, true>
{
	typedef izene_serialization_memcpy<T> stype;
	typedef izene_deserialization_memcpy<T> dtype;
};

template<typename T>
struct izene_serial_type<T, false, true>
{
	typedef izene_serialization_febird<T> stype;
	typedef izene_deserialization_febird<T> dtype;
};

template <typename T> class izene_serialization {
	typedef typename izene_serial_type< T, IsMemcpySerial<T>::yes, IsFebirdSerial<T>::yes>::stype
			stype;
	stype impl;
public:
	izene_serialization(const T& dat) :
		impl(dat) {

	}
	void write_image(char* &ptr, size_t &size) {
			impl.write_image(ptr, size);
		}
};

template <typename T> class izene_deserialization {
	typedef typename izene_serial_type<T, IsMemcpySerial<T>::yes, IsFebirdSerial<T>::yes>::dtype
			dtype;
	dtype impl;
public:
	izene_deserialization(const char* ptr, const size_t size) :
		impl(ptr, size) {
	}
	void read_image(T& dat) {
		impl.read_image(dat);
	}
};

/*
 template<> class izene_serialization<string> {
 const string& dat_;
 public:
 izene_serialization(const string& dat) :
 dat_(dat) {

 }
 void write_image(void* &ptr, size_t& size) {
 ptr = (void*)dat_.c_str();
 size = dat_.size()+1;
 }
 };

 template<> class izene_deserialization<string> {
 void* &ptr_;
 size_t& size_;
 public:
 izene_deserialization(void* &ptr, size_t& size) :
 ptr_(ptr), size_(size) {

 }
 void read_image(string& dat) {
 dat = string((char*)ptr_);
 }
 };
 */

NS_IZENELIB_UTIL_END

#endif /*IZENE_SERIALIZATION_H_*/
