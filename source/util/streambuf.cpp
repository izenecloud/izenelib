#include <util/streambuf.h>

NS_IZENELIB_UTIL_BEGIN

template <typename Iterator1, typename Iterator2>
std::pair<Iterator1, bool> izene_partial_search(
	Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2)
{
  for (Iterator1 iter1 = first1; iter1 != last1; ++iter1)
  {
	Iterator1 test_iter1 = iter1;
	Iterator2 test_iter2 = first2;
	for (;; ++test_iter1, ++test_iter2)
	{
	  if (test_iter2 == last2)
		return std::make_pair(iter1, true);
	  if (test_iter1 == last1)
	  {
		if (test_iter2 != first2)
		  return std::make_pair(iter1, false);
		else
		  break;
	  }
	  if (*test_iter1 != *test_iter2)
		break;
	}
  }
  return std::make_pair(last1, false);
}


std::size_t izene_read_until(std::istream& s, izene_streambuf& b, const std::string& delim)
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
    std::pair<iterator, bool> result = izene_partial_search(
        start, end, delim.begin(), delim.end());
    if (result.first != end)
    {
      if (result.second)
      {
        // Full match. We're done.
        return result.first - begin + delim.length();
      }
      else
      {
        // Partial match. Next search needs to start from beginning of match.
        next_search_start = result.first - begin;
      }
    }
    else
    {
      // No match. Next search can start with the new data.
      next_search_start = end - begin;
    }

    // Check if buffer is full.
    if (b.size() == b.max_size())
    {
      std::cerr << "izene_streambuf is full, b.size(): " << b.size()
                << ", b.max_size(): " << b.max_size() << std::endl;
      return 0;
    }
	
    // Need more data.
    std::size_t bytes_available =
      std::min<std::size_t>(512, b.max_size() - b.size());
    b.reserve(bytes_available);

    s.read(b.pptr(), bytes_available);
    std::size_t bytes_read = s.gcount();
    if(bytes_read <=0)
    {
        return 0;
    }

    b.commit(bytes_read);
  }
}


std::size_t izene_read_until(std::istream& s, izene_streambuf& b, char delim)
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
    iterator iter = std::find(start, end, delim);
    if (iter != end)
    {
      // Found a match. We're done.
      return iter - begin + 1;
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

    s.read(b.pptr(), bytes_available);
    std::size_t bytes_read = s.gcount();
    if(bytes_read <=0)
    {
        return 0;
    }

    b.commit(bytes_read);
  }
}

std::size_t izene_read_until(std::istream& s, izene_streambuf& b, const boost::regex& expr)
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
    boost::match_results<iterator> match_results;
    if (boost::regex_search(start, end, match_results, expr,
          boost::match_default | boost::match_partial))
    {
      if (match_results[0].matched)
      {
        // Full match. We're done.
        return match_results[0].second - begin;
      }
      else
      {
        // Partial match. Next search needs to start from beginning of match.
        next_search_start = match_results[0].first - begin;
      }
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

    s.read(b.pptr(), bytes_available);
    std::size_t bytes_read = s.gcount();
    if(bytes_read <=0)
    {
        return 0;
    }

    b.commit(bytes_read);
  }
}


NS_IZENELIB_UTIL_END
