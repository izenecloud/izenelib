#ifndef serialization_h
#define serialization_h

#include <am/concept/DataType.h>
#include "Wrapper.h"

#include <sstream>
// for serialization
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

NS_IZENELIB_AM_BEGIN

namespace util {

	template<typename T>
	inline void write_image(const T& dat, char* &str, size_t& size)
	{
		DbObjPtr ptr;
		ptr.reset(new DbObj);
		write_image(dat, ptr);
		str = (char*)ptr->getData();
		size = ptr->getSize();
	}

	template<typename T>
	inline void read_image(T& dat, const char* str, const size_t& size)
	{
		DbObjPtr ptr;
		ptr.reset(new DbObj(str, size));
		read_image(dat, ptr);
	}

	template<>
	inline void write_image<std::string>(const std::string& dat, char* &str, size_t& size)
	{
		str = (char*)dat.c_str();
		size = dat.size();
	}

	template<>
	inline void read_image<std::string>(std::string& dat, const char* str, const size_t& size)
	{
		dat = str;
	}

	template<>
	inline void write_image<int>(const int& dat, char* &str, size_t& size)
	{
		str = (char*)(&dat);
		size = sizeof(dat);
	}

	template<>
	inline void read_image<int>(int& dat, const char* str, const size_t& size)
	{
		memcpy(&dat, str,sizeof(int));
	}

	inline int uint_to_bytes(unsigned int val, char* dest)
    {
		unsigned int i;
		for (i = 0; i < sizeof(int); i++)
        {
			dest[i] = (char)((val) & 0xFF);
			val >>= 8;
			if (val == 0)
			break;
		}
		return i;
	}

	inline unsigned int bytes_to_uint(int len, char * vals)
    {
		int dest = 0;
		for(int i = 0; i < len; i++)
        {
			dest = (dest << 8) | (vals[sizeof(int) - i - 1] & 0xFF);
		}
		return dest;
	}

	template<typename T>
	inline void to_string(const T& dat, std::string& str)
    {
		DbObjPtr ptr;
		ptr.reset(new DbObj);
		write_image(dat, ptr);
		str.assign((const char*)ptr->getData());
	}

	template<typename T>
	inline void from_string(T& dat, const std::string& str)
    {
		DbObjPtr ptr;
		ptr.reset(new DbObj(str.c_str(), str.size() + 1));
		read_image(dat, ptr);
	}

	template<>
	inline void to_string<std::string>(const std::string& dat, std::string& str)
    {
		str = dat;
	}

	template<>
	inline void from_string<std::string>(std::string& dat, const std::string& str)
    {
		dat = str;
	}

	template<typename T>
	inline std::string to_string(const T& dat)
    {
		DbObjPtr ptr;
		ptr.reset(new DbObj);
		write_image(dat, ptr);
		return std::string((const char*)ptr->getData());
	}

	template<typename T>
	inline T from_string(const std::string& str)
    {
		T dat;
		DbObjPtr ptr;
		ptr.reset(new DbObj(str.c_str(), str.size() + 1));
		read_image(dat, ptr);
		return dat;
	}

	template<>
	inline std::string to_string<std::string>(const std::string& dat)
    {
		return dat;
	}

	template<>
	inline std::string from_string<std::string>(const std::string& str)
    {
		return str;
	}

	template<typename T>
	inline void serialize_boost(const T& dat, boost::archive::text_oarchive& ar)
    {
		ar & dat;

	}

	template<typename T>
	inline void deserialize_boost(T& dat, const boost::archive::text_iarchive& ar)
    {
		ar & dat;
	}

	template<typename T>
	inline void serialize_ss(const T& dat, stringstream& ar)
    {
		ar << dat;

	}

	template<typename T>
	inline void deserialize_ss(T& dat, const stringstream& ar)
    {
		ar >> dat;
	}

}

NS_IZENELIB_AM_END

#endif /*serialization_h*/
