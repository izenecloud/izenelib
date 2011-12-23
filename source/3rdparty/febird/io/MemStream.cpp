/* vim: set tabstop=4 : */
#include <febird/io/MemStream.h>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

namespace febird {

//void MemIO_Base::seek_cur(ptrdiff_t diff)

void throw_EndOfFile(const char* func, size_t want, size_t available)
{
	std::ostringstream oss;
	oss << "in " << func << ", want=" << want
		<< ", available=" << available
//		<< ", tell=" << tell() << ", size=" << size()
		;
	throw EndOfFileException(oss.str().c_str());
}

void throw_OutOfSpace(const char* func, size_t want, size_t available)
{
	std::ostringstream oss;
	oss << "in " << func << ", want=" << want
		<< ", available=" << available
//		<< ", tell=" << tell() << ", size=" << size()
		;
	throw OutOfSpaceException(oss.str().c_str());
}

void MemIO::throw_EndOfFile(const char* func, size_t want)
{
	febird::throw_EndOfFile(func, want, available());
}

void MemIO::throw_OutOfSpace(const char* func, size_t want)
{
	febird::throw_OutOfSpace(func, want, available());
}

//////////////////////////////////////////////////////////////////////////

void SeekableMemIO::swap(SeekableMemIO& that)
{
	std::swap(m_beg, that.m_beg);
	std::swap(m_end, that.m_end);
	std::swap(m_pos, that.m_pos);
}

void SeekableMemIO::seek(long newPos)
{
	assert(newPos >= 0);
	if (newPos < 0 || newPos > m_end - m_beg) {
		std::ostringstream oss;
		oss << "in " << BOOST_CURRENT_FUNCTION
			<< "[newPos=" << newPos << ", size=" << m_end << "]";
		throw std::invalid_argument(oss.str());
	}
	m_pos = m_beg + newPos;
}

void SeekableMemIO::seek(long offset, int origin)
{
	size_t pos;
	switch (origin)
	{
		default:
		{
			std::ostringstream oss;
			oss << "in " << BOOST_CURRENT_FUNCTION
				<< "[offset=" << offset << ", origin=" << origin << "(invalid)]";
			throw std::invalid_argument(oss.str().c_str());
		}
		case 0: pos = (size_t)(0 + offset); break;
		case 1: pos = (size_t)(tell() + offset); break;
		case 2: pos = (size_t)(size() + offset); break;
	}
	seek(pos);
}

// rarely used methods....
//

MemIO SeekableMemIO::range(size_t ibeg, size_t iend) const
{
	assert(ibeg <= iend);
	assert(ibeg <= size());
	assert(iend <= size());
	if (ibeg <= iend && ibeg <= size() && iend <= size())
	{
		return MemIO(m_beg + ibeg, m_beg + iend);
	}
	std::ostringstream oss;
	oss << BOOST_CURRENT_FUNCTION
		<< ": size=" << size()
		<< ", tell=" << tell()
		<< ", ibeg=" << ibeg
		<< ", iend=" << iend
		;
	throw std::invalid_argument(oss.str());
}

//////////////////////////////////////////////////////////////////////////

static std::allocator<byte> G_byteAlloc;

AutoGrownMemIO::AutoGrownMemIO(size_t size)
{
//	m_beg = 0 == size ? 0 : (byte*)::malloc(size);
	try {
		m_beg = 0 == size ? 0 : G_byteAlloc.allocate(size);
	}
	catch (const std::exception& exp)
	{
		std::ostringstream oss;
		oss << "at " << BOOST_CURRENT_FUNCTION << ": size=" << size
			<< ", nested-exption[, type=" << typeid(exp).name() << ", what=" << exp.what() << "]";
		throw std::runtime_error(oss.str());
	}
	m_end = m_beg + size;
	m_pos = m_beg;
}

AutoGrownMemIO::~AutoGrownMemIO()
{
//	if (m_beg) ::free(m_beg);
	if (m_beg)
		G_byteAlloc.deallocate(m_beg, size());
}

void AutoGrownMemIO::clone(const AutoGrownMemIO& src)
{
	AutoGrownMemIO t(src.size());
	memcpy(t.begin(), src.begin(), src.size());
	this->swap(t);
}

void AutoGrownMemIO::swap(AutoGrownMemIO& that)
{
	std::swap(m_beg, that.m_beg);
	std::swap(m_end, that.m_end);
	std::swap(m_pos, that.m_pos);
}

/**
 @brief �ı� buffer �ߴ�

  ���ı� buffer �е��Ѵ����ݣ����ı� pos

 @note must m_pos <= newsize
 */
void AutoGrownMemIO::resize(size_t newsize)
{
	assert(tell() <= newsize);

//	byte* newbeg = (byte*)::realloc(m_beg, newsize);
	byte* newbeg = 0;
	try {		
		newbeg = G_byteAlloc.allocate(newsize);
	}
	catch (const std::exception& exp)
	{
		std::ostringstream oss;
		oss << "at " << BOOST_CURRENT_FUNCTION << ": size=" << size() << ", newsize=" << newsize
			<< ", nested-exption[, type=" << typeid(exp).name() << ", what=" << exp.what() << "]";
		throw std::runtime_error(oss.str());
	}
	if (newbeg)
	{
		memcpy(newbeg, m_beg, size());		
		G_byteAlloc.deallocate(m_beg, size() );
		m_pos = newbeg + (m_pos - m_beg);
		m_beg = newbeg;
		m_end = newbeg + newsize;
	}
	else
	{
#ifdef _MSC_VER
		std::ostringstream oss;
		oss << "realloc failed in \"void AutoGrownMemIO::resize(newsize=" << newsize
			<< ")\", the AutoGrownMemIO object is not mutated!";
		throw std::bad_alloc(oss.str().c_str());
#else
		throw std::bad_alloc();
#endif
	}
}

/**
 @brief �ͷ�ԭ�ȵĿռ䲢���·���

  �൱�ڰ��³ߴ����¹���һ���� AutoGrownMemIO
  ����Ҫ�Ѿ����ݿ������µ�ַ
 */
void AutoGrownMemIO::init(size_t newsize)
{
//	if (m_beg)
//		::free(m_beg);
//	m_pos = m_beg = m_end = 0;
//	m_beg = (byte*)::malloc(newsize);
	size_t oldSize = size();
	try {
		if (m_beg) {
			G_byteAlloc.deallocate(m_beg, oldSize);
			m_pos = m_beg = m_end = 0;
		}
		m_beg = G_byteAlloc.allocate(newsize);
	}
	catch (const std::exception& exp)
	{
		std::ostringstream oss;
		oss << "at " << BOOST_CURRENT_FUNCTION << ": size=" << oldSize << ", newsize=" << newsize
			<< ", nested-exption[, type=" << typeid(exp).name() << ", what=" << exp.what() << "]";
		throw std::runtime_error(oss.str());
	}

	if (0 == m_beg)
	{
#ifdef _MSC_VER
		std::ostringstream oss;
		oss << "alloc failed in \"" << BOOST_CURRENT_FUNCTION
			<< "\", with capacity=" << newsize
			<< ", [this=" << (void*)(this)
			<< "] was partly mutated and is not in consistent state!";
		throw std::bad_alloc(oss.str().c_str());
#else
		throw std::bad_alloc();
#endif
	}
	m_pos = m_beg;
	m_end = m_beg + newsize;
}

void AutoGrownMemIO::growAndWrite(const void* FEBIRD_RESTRICT data, size_t length)FEBIRD_RESTRICT
{
	using namespace std;
	size_t nSize = size();
	size_t nGrow = max(length, nSize);
	resize(max(nSize + nGrow, (size_t)64u));
	memcpy(m_pos, data, length);
	m_pos += length;
}

void AutoGrownMemIO::growAndWriteByte(byte b)FEBIRD_RESTRICT
{
	using namespace std;
	resize(max(2u * size(), (size_t)64u));
	*m_pos++ = b;
}

} // namespace febird


