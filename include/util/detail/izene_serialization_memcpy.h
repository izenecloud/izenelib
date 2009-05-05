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
	void write_image(void* &ptr, size_t& size) {
		ptr = (void*)&dat_;
		size = sizeof(dat_);
	}
};

template<typename T> class izene_deserialization_memcpy {
	void* &ptr_;
	size_t& size_;
public:
	izene_deserialization_memcpy(void* &ptr, size_t& size) :
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
	void write_image(void* &ptr, size_t& size) {
		ptr = (void*)dat_.c_str();
		size = dat_.size()+1;
	}
};

template<> class izene_deserialization_memcpy<std::string> {
	void* &ptr_;
	size_t& size_;
public:
	izene_deserialization_memcpy(void* &ptr, size_t& size) :
		ptr_(ptr), size_(size) {

	}
	void read_image(std::string& dat) {
		dat = std::string((char*)ptr_);
	}
};

template<typename T> class izene_serialization_memcpy<std::vector<T> > {
	const std::vector<T>& dat_;
public:
	izene_serialization_memcpy(const std::vector<T>& dat) :
		dat_(dat) {

	}
	void write_image(void* &ptr, size_t& size) {
		ptr = &dat_[0];
	}
};

template<typename T> class izene_deserialization_memcpy< std::vector<T> > {
	void* &ptr_;
	size_t& size_;
public:
	izene_deserialization_memcpy(void* &ptr, size_t& size) :
		ptr_(ptr), size_(size) {

	}
	void read_image(std::vector<T>& dat) {
		dat.resize(size_/sizeof(T));
		memcpy(&dat[0], ptr_, size_);
	}
};

NS_IZENELIB_UTIL_END


#endif /*IZENE_SERIALIZATION_MEMCPY_H_*/
