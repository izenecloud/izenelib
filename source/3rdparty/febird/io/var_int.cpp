/* vim: set tabstop=4 : */
#include <febird/io/var_int.h>
#include <assert.h>
#include <stdexcept>

namespace febird {

std::pair<uint16_t, int> load_var_uint16(const unsigned char* buf)
{
	std::pair<uint32_t, int> x = load_var_uint32(buf);
	assert(x.first <= 0xFFFF);
	if (x.first > 0xFFFF)
		throw std::runtime_error(BOOST_CURRENT_FUNCTION);

	return std::pair<uint16_t, int>(uint16_t(x.first), x.second);
}

std::pair<uint32_t, int> load_var_uint32(const unsigned char* buf)
{
	uint32_t v = 0;
	const unsigned char* p = buf;
	for (int shift = 0; shift < 35; shift += 7, ++p)
	{
		const unsigned char b = *p;
		v |= uint32_t(b & 0x7F) << shift;
		if ((b & 0x80) == 0)
			return std::pair<uint32_t, int>(v, int(++p-buf));
	}
	assert(0); // should not get here
	throw std::runtime_error(BOOST_CURRENT_FUNCTION);
}

std::pair<uint64_t, int> load_var_uint64(const unsigned char* buf)
{
	uint64_t v = 0;
	const unsigned char* p = buf;
	for (int shift = 0; shift < 56; shift += 7, ++p)
	{
		register unsigned char b = *p;
		v |= uint64_t(b & 0x7F) << shift;
		if ((b & 0x80) == 0)
			return std::pair<uint64_t, int>(v, int(++p-buf));
	}
	v |= uint64_t(*p++) << 56;
	return std::pair<uint64_t, int>(v, int(p-buf));
}

std::pair<int16_t, int> load_var_int16(const unsigned char* buf)
{
	std::pair<uint16_t, int> x = load_var_uint16(buf);
	return std::pair<int16_t, int>(var_int16_u2s(x.first), x.second);
}

std::pair<int32_t, int> load_var_int32(const unsigned char* buf)
{
	std::pair<uint32_t, int> x = load_var_uint32(buf);
	return std::pair<int32_t, int>(var_int32_u2s(x.first), x.second);
}

std::pair<int64_t, int> load_var_int64(const unsigned char* buf)
{
	std::pair<uint64_t, int> x = load_var_uint64(buf);
	return std::pair<int64_t, int>(var_int64_u2s(x.first), x.second);
}

//////////////////////////////////////////////////////////////////////////

template<class IntType>
unsigned char* save_var_uint(unsigned char* p, IntType x)
{
	while (x & ~0x7F)
	{
		*p++ = (unsigned char)((x & 0x7f) | 0x80);
		x >>= 7; //doing unsigned shift
	}
	*p++ = (unsigned char)(x);
	return p;
}

unsigned char* save_var_uint32(unsigned char* buf, uint32_t x) { return save_var_uint(buf, x); }
unsigned char* save_var_uint16(unsigned char* buf, uint16_t x) { return save_var_uint(buf, x); }

unsigned char* save_var_uint64(unsigned char* p, uint64_t x)
{
	for (int bytes = 0; bytes < 8; ++bytes)
	{
		if (x & ~0x7F) {
			*p++ = (unsigned char)((x & 0x7f) | 0x80);
			x >>= 7; //doing unsigned shift
		} else
			break;
	}
	*p++ = (unsigned char)x;
	return p;
}

unsigned char* save_var_int32(unsigned char* buf, int32_t x) { return save_var_uint32(buf, var_int32_s2u(x)); }
unsigned char* save_var_int16(unsigned char* buf, int16_t x) { return save_var_uint16(buf, var_int16_s2u(x)); }
unsigned char* save_var_int64(unsigned char* buf, int64_t x) { return save_var_uint64(buf, var_int64_s2u(x)); }

/**
 @brief reverse get var_uint32_t

 @note
   - if first var_int has read, *cur == buf-1
   - the sequence must all stored var_int, if not, the dilimeter byte's high bit must be 0
 */
uint32_t reverse_get_var_uint32(const unsigned char* buf, unsigned char const ** cur)
{
	assert(cur);
	assert(*cur);
	assert(*cur >= buf);

	const unsigned char* p = *cur;
	uint32_t x = 0;
	uint32_t w = *p;
	assert(!(x & 0x80));
	int shift = 0;
	--p;
	while (p >= buf && *p & 0x80)
	{
		x = x << 7 | (uint32_t)(*p & 0x7F);
		shift += 7;
		--p;
	}
	x |= w << shift;
	*cur = p;

	return x;
}

/**
 @brief reverse get var_int32_t

 @note if first var_int has read, *cur == buf-1
 */
int32_t reverse_get_var_int32(const unsigned char* buf, unsigned char const ** cur)
{
	return var_int32_u2s(reverse_get_var_uint32(buf, cur));
}

uint16_t reverse_get_var_uint16(const unsigned char* buf, unsigned char const ** cur)
{
	// same as uint32
	return (uint16_t)reverse_get_var_uint32(buf, cur);
}

int16_t reverse_get_var_int16(const unsigned char* buf, unsigned char const ** cur)
{
	return var_int16_u2s(reverse_get_var_uint16(buf, cur));
}

#if !defined(BOOST_NO_INT64_T)
/**
 @brief reverse get var_uint64_t

 @note if first var_int has read, *cur == buf-1
 */
uint64_t reverse_get_var_uint64(const unsigned char* buf, unsigned char const ** cur)
{
	assert(cur);
	assert(*cur);
	assert(*cur >= buf);

	const unsigned char* p = *cur;
	uint64_t x = 0;
	uint64_t w = *p;
	int shift = 0;
	--p;
	while (p >= buf && shift < 56 && *p & 0x80)
	{
		x = x << 7 | (uint64_t)(*p & 0x7F);
		shift += 7;
		--p;
	}
	assert(shift <= 56);

	x |= w << shift;

	*cur = p; // p now point to last byte of prev var_int

	return x;
}

/**
 @brief reverse get var_int64_t

 @note if first var_int has read, *cur == buf-1
 */
int64_t reverse_get_var_int64(const unsigned char* buf, unsigned char const ** cur)
{
	return var_int64_u2s(reverse_get_var_uint64(buf, cur));
}

#endif //BOOST_NO_INT64_T

} // namespace febird

