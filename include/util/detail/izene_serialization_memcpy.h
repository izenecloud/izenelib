#ifndef IZENE_SERIALIZATION_MEMCPY_H_
#define IZENE_SERIALIZATION_MEMCPY_H_

#include "izene_type_traits.h"

NS_IZENELIB_UTIL_BEGIN

template<typename T> class izene_serialization_memcpy {
	const T& dat_;
public:
	izene_serialization_memcpy(const T& dat) :
	dat_(dat) {

	}
	void write_image(char* &ptr, size_t& size) {
		ptr = (char*)&dat_;
		size = sizeof(dat_);
	}
};

template<typename T> class izene_deserialization_memcpy {
	const char* &ptr_;
	const size_t& size_;
public:
	izene_deserialization_memcpy(const char* ptr, const size_t size) :
		ptr_(ptr), size_(size) {

	}
	void read_image(T& dat) {
		memcpy(&dat, ptr_, sizeof(dat));
	}
};

template<> class izene_serialization_memcpy<std::string> {
	const std::string& dat_;
public:
	izene_serialization_memcpy(const std::string& dat) :
		dat_(dat) {

	}
	void write_image(char* &ptr, size_t& size) {
		ptr = (char*)dat_.c_str();
		size = dat_.size()+1;
	}
};

template<> class izene_deserialization_memcpy<std::string> {
	const char* &ptr_;
	const size_t &size_;
public:
	izene_deserialization_memcpy(const char* ptr, const size_t size) :
		ptr_(ptr), size_(size) {

	}
	void read_image(std::string& dat) {
		dat = std::string(ptr_);
	}
};

template<typename T> class izene_serialization_memcpy<std::vector<T> > {
	const std::vector<T>& dat_;
public:
	izene_serialization_memcpy(const std::vector<T>& dat) :
		dat_(dat) {

	}
	void write_image(char* &ptr, size_t& size) {
		ptr = (char*)&dat_[0];
		size = dat_.size()*sizeof(T);
	}
};

template<typename T> class izene_deserialization_memcpy< std::vector<T> > {
	const char* &ptr_;
	const size_t& size_;
public:
	izene_deserialization_memcpy(const char* ptr, const size_t size) :
		ptr_(ptr), size_(size) {

	}
	void read_image(std::vector<T>& dat) {
		dat.resize(size_/sizeof(T));
		memcpy(&dat[0], ptr_, size_);
	}
};

template<> class izene_serialization_memcpy<izenelib::am::NullType> {
public:
	izene_serialization_memcpy(const izenelib::am::NullType& dat) {

	}
	void write_image(char* &ptr, size_t& size) {
		ptr = 0;
		size = 0;
	}
};

template<> class izene_deserialization_memcpy<izenelib::am::NullType> {
public:
	izene_deserialization_memcpy(const char* ptr, const size_t size) {

	}
	void read_image(izenelib::am::NullType& dat) {

	}
};

NS_IZENELIB_UTIL_END

#endif /*IZENE_SERIALIZATION_MEMCPY_H_*/
