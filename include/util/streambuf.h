#ifndef IZENELIB_STREAMBUF_H
#define IZENELIB_STREAMBUF_H

#include <types.h>

#include <boost/asio/streambuf.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/regex.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/utility/enable_if.hpp>

#include <iostream>

NS_IZENELIB_UTIL_BEGIN

namespace detail{
#if BOOST_WORKAROUND(__CODEGEARC__, BOOST_TESTED_AT(0x610))
  template <typename T>
  struct has_result_type
  {
    template <typename U> struct inner
    {
        struct big { char a[100]; };
        static big helper(U, ...);
        static char helper(U, typename U::result_type* = 0);
    };
    static const T& ref();
    enum { value = (sizeof((inner<const T&>::helper)((ref)())) == 1) };
  };
#else // BOOST_WORKAROUND(__CODEGEARC__, BOOST_TESTED_AT(0x610))
  template <typename T>
  struct has_result_type
  {
    struct big { char a[100]; };
    template <typename U> static big helper(U, ...);
    template <typename U> static char helper(U, typename U::result_type* = 0);
    static const T& ref();
    enum { value = (sizeof((helper)((ref)())) == 1) };
  };
#endif // BOOST_WORKAROUND(__CODEGEARC__, BOOST_TESTED_AT(0x610))
}

/// Type trait used to determine whether a type can be used as a match condition
/// function with izene_read_until
template <typename T>
struct is_match_condition
{
  enum
  {
    value = boost::is_function<typename boost::remove_pointer<T>::type>::value
      || detail::has_result_type<T>::value
  };
};


class izene_streambuf : public boost::asio::streambuf
{
public:		
    char* gptr(){ return std::streambuf::gptr();}

    size_t size() { return (std::streambuf::pptr() - std::streambuf::gptr());}

    char * eback () const { return std::streambuf::eback();}

    char * pptr () const { return std::streambuf::pptr();}

    void reserve(std::size_t n) { boost::asio::streambuf::reserve(n);}
};

/// Read data into a streambuf until it contains a specified delimiter.
/**
 * This function is used to read data into the specified streambuf until the
 * streambuf's get area contains the specified delimiter. The call will block
 * until one of the following conditions is true:
 *
 * @li The get area of the streambuf contains the specified delimiter.
 *
 *
 * This operation is implemented in terms of zero or more calls to the stream's
 * read_some function. If the streambuf's get area already contains the
 * delimiter, the function returns immediately.
 *
 * @param s The stream from which the data is to be read. 
 *
 * @param b A streambuf object into which the data will be read.
 *
 * @param delim The delimiter string.
 *
 * @returns The number of bytes in the streambuf's get area up to and including
 * the delimiter.
 *
 *
 * @note After a successful izene_read_until operation, the streambuf may contain
 * additional data beyond the delimiter. An application will typically leave
 * that data in the streambuf for a subsequent read_until operation to examine.
 */

std::size_t izene_read_until(std::istream& s, izene_streambuf& b, const std::string& delim);
/**
 * @defgroup izene_read_until
 *
 * @brief Read data into a streambuf until it contains a delimiter, matches a
 * regular expression, or a function object indicates a match.
 */
/*@{*/

/// Read data into a streambuf until it contains a specified delimiter.
/**
 * This function is used to read data into the specified streambuf until the
 * streambuf's get area contains the specified delimiter. The call will block
 * until one of the following conditions is true:
 *
 * @li The get area of the streambuf contains the specified delimiter.
 *
 * This operation is implemented in terms of zero or more calls to the stream's
 * read_some function. If the streambuf's get area already contains the
 * delimiter, the function returns immediately.
 *
 * @param s The stream from which the data is to be read. 
 *
 * @param b A streambuf object into which the data will be read.
 *
 * @param delim The delimiter character.
 *
 * @returns The number of bytes in the streambuf's get area up to and including
 * the delimiter.
 *
 * @note After a successful read_until operation, the streambuf may contain
 * additional data beyond the delimiter. An application will typically leave
 * that data in the streambuf for a subsequent read_until operation to examine.
 *
 */
std::size_t izene_read_until(std::istream& s, izene_streambuf& b, char delim);
/// Read data into a streambuf until some part of the data it contains matches
/// a regular expression.
/**
 * This function is used to read data into the specified streambuf until the
 * streambuf's get area contains some data that matches a regular expression.
 * The call will block until one of the following conditions is true:
 *
 * @li A substring of the streambuf's get area matches the regular expression.
 *
 *
 * This operation is implemented in terms of zero or more calls to the stream's
 * read_some function. If the streambuf's get area already contains data that
 * matches the regular expression, the function returns immediately.
 *
 * @param s The stream from which the data is to be read. The type must support
 * the SyncReadStream concept.
 *
 * @param b A streambuf object into which the data will be read.
 *
 * @param expr The regular expression.
 *
 * @returns The number of bytes in the streambuf's get area up to and including
 * the substring that matches the regular expression.
 *
 * @note After a successful izene_read_until operation, the streambuf may contain
 * additional data beyond that which matched the regular expression. An
 * application will typically leave that data in the streambuf for a subsequent
 * read_until operation to examine.
 */
std::size_t izene_read_until(std::istream& s, izene_streambuf& b, const boost::regex& expr);
/// Read data into a streambuf until a function object indicates a match.
/**
 * This function is used to read data into the specified streambuf until a
 * user-defined match condition function object, when applied to the data
 * contained in the streambuf, indicates a successful match. The call will
 * block until one of the following conditions is true:
 *
 * @li The match condition function object returns a std::pair where the second
 * element evaluates to true.
 *
 *
 * This operation is implemented in terms of zero or more calls to the stream's
 * read_some function. If the match condition function object already indicates
 * a match, the function returns immediately.
 *
 * @param s The stream from which the data is to be read. The type must support
 * the SyncReadStream concept.
 *
 * @param b A streambuf object into which the data will be read.
 *
 * @param match_condition The function object to be called to determine whether
 * a match exists. The signature of the function object must be:
 * @code pair<iterator, bool> match_condition(iterator begin, iterator end);
 * @endcode
 * where @c iterator represents the type:
 * @code buffers_iterator<basic_streambuf<Allocator>::const_buffers_type>
 * @endcode
 * The iterator parameters @c begin and @c end define the range of bytes to be
 * scanned to determine whether there is a match. The @c first member of the
 * return value is an iterator marking one-past-the-end of the bytes that have
 * been consumed by the match function. This iterator is used to calculate the
 * @c begin parameter for any subsequent invocation of the match condition. The
 * @c second member of the return value is true if a match has been found, false
 * otherwise.
 *
 * @returns The number of bytes in the streambuf's get area that have been fully
 * consumed by the match function. Returns 0 if an error occurred.
 *
 * @note After a successful read_until operation, the streambuf may contain
 * additional data beyond that which matched the function object. An application
 * will typically leave that data in the streambuf for a subsequent
 *
 * @note The default implementation of the @c is_match_condition type trait
 * evaluates to true for function pointers and function objects with a
 * @c result_type typedef. It must be specialised for other user-defined
 * function objects.
 * @par Examples
 * To read data into a streambuf until whitespace is encountered:
 * @code typedef boost::asio::buffers_iterator<
 *     izene_streambuf::const_buffers_type> iterator;
 *
 * std::pair<iterator, bool>
 * match_whitespace(iterator begin, iterator end)
 * {
 *   iterator i = begin;
 *   while (i != end)
 *     if (std::isspace(*i++))
 *       return std::make_pair(i, true);
 *   return std::make_pair(i, false);
 * }
 * ...
 * izene_streambuf b;
 * std::ifstream("path");
 * izene_read_until(s, b, match_whitespace);
 * @endcode
 *
 * To read data into a streambuf until a matching character is found:
 * @code class match_char
 * {
 * public:
 *   explicit match_char(char c) : c_(c) {}
 *
 *   template <typename Iterator>
 *   std::pair<Iterator, bool> operator()(
 *       Iterator begin, Iterator end) const
 *   {
 *     Iterator i = begin;
 *     while (i != end)
 *       if (c_ == *i++)
 *         return std::make_pair(i, true);
 *     return std::make_pair(i, false);
 *   }
 *
 * private:
 *   char c_;
 * };
 *
 * namespace izenelib{namespace util{
 *   template <> struct is_match_condition<match_char>
 *     : public boost::true_type {};
 * }}
 * ...
 * izene_streambuf b;
 * izene_read_until(s, b, match_char('a'));
 * @endcode
 */
 
template <typename MatchCondition>
std::size_t izene_read_until(std::istream& s, izene_streambuf& b,  MatchCondition match_condition,
    typename boost::enable_if<is_match_condition<MatchCondition> >::type* = 0)
{
  std::size_t next_search_start = 0;
  for (;;)
  {
    // Determine the range of the data to be searched.
    typedef izene_streambuf::const_buffers_type const_buffers_type;
    typedef boost::asio::buffers_iterator<const_buffers_type> iterator;
    const_buffers_type buffers = b.data();
    iterator begin = iterator::begin(buffers);
    iterator start = begin + next_search_start;
    iterator end = iterator::end(buffers);

    // Look for a match.
    std::pair<iterator, bool> result = match_condition(start, end);
    if (result.second)
    {
      // Full match. We're done.
      return result.first - begin;
    }
    else if (result.first != end)
    {
      // Partial match. Next search needs to start from beginning of match.
      next_search_start = result.first - begin;
    }
    else
    {
      // No match. Next search can start with the new data.
      next_search_start = end - begin;
    }

    // Check if buffer is full.
    if (b.size() == b.max_size())
    {
      return 0;
    }

    // Need more data.
    std::size_t bytes_available =
      std::min<std::size_t>(512, b.max_size() - b.size());
    b.reserve(bytes_available);
    std::size_t bytes_read = s.readsome(b.pptr(), bytes_available);
    if(bytes_read <=0 || (s.rdstate() == std::ios_base::eofbit))
        return 0;
    b.commit(bytes_read);
  }
}

NS_IZENELIB_UTIL_END

#endif //End of IZENELIB_STREAMBUF_H
