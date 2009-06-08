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
#include <util/streambuf.h>

NS_IZENELIB_UTIL_BEGIN

const int archive_flags = archive::no_header | archive::no_codecvt;

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
};

NS_IZENELIB_UTIL_END

#endif /*IZENE_SERIALIAZATION_BOOST_H_*/
