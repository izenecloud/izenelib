#ifndef IZENELIB_UTIL_FILE_IO_H
#define IZENELIB_UTIL_FILE_IO_H

#include <string>
#include <fstream>
#include <assert.h>

#include <boost/iterator/iterator_facade.hpp>

namespace izenelib{ namespace util {

inline void trim_newline_chars(std::string& s)
{
    size_t l = s.size();
    while (l && (s[l-1] == '\r' ||
                 s[l-1] == '\n'))
    {
        --l;
    }
    s.resize(l);
}

// this is considerably faster than std::getline
inline bool fast_getline(std::string& line, FILE* input = stdin, bool trim_newline = false)
{
    line.clear();
    static const size_t max_buffer = 65536;
    char buffer[max_buffer];
    bool done = false;
    while (!done)
    {
        if (!fgets(buffer, max_buffer, input))
        {
            if (!line.size())
            {
                return false;
            }
            else
            {
                done = true;
            }
        }
        line += buffer;
        if (*line.rbegin() == '\n')
        {
            done = true;
        }
    }
    if (trim_newline)
    {
        trim_newline_chars(line);
    }
    return true;
}

class line_iterator
    : public boost::iterator_facade<line_iterator
    , std::string const
    , boost::forward_traversal_tag
    >
{
public:
    line_iterator()
        : file_(0)
    {}

    explicit line_iterator(FILE* input, bool trim_newline = false)
        : file_(input)
        , trim_newline_(trim_newline)
    {}

private:
    friend class boost::iterator_core_access;

    void increment()
    {
        assert(file_);
        if (!fast_getline(line_, file_, trim_newline_))
        {
            file_ = 0;
        }
    }

    bool equal(line_iterator const& other) const
    {
        return this->file_ == other.file_;
    }

    std::string const& dereference() const
    {
        return line_;
    }

    std::string line_;
    FILE* file_;
    bool trim_newline_;
};

typedef std::pair<line_iterator, line_iterator> line_range_t;

inline line_range_t lines(FILE* ifs, bool trim_newline = false)
{
    return std::make_pair(line_iterator(ifs, trim_newline), line_iterator());
}

class buffer_line_iterator
    : public boost::iterator_facade<buffer_line_iterator
    , std::string const
    , boost::forward_traversal_tag
    >
{
public:
    buffer_line_iterator()
        : buffer_(0)
        , end_(0)
        , cur_pos_(0)
    {}

    buffer_line_iterator(const char* buffer, size_t size)
        : buffer_(buffer)
        , end_(buffer + size)
        , cur_pos_(buffer)
    {
        increment();
    }

private:
    friend class boost::iterator_core_access;

    void increment()
    {
        assert(cur_pos_);
        if (cur_pos_ >= end_)
        {
            cur_pos_ = 0;
            return;
        }
        const char* begin = cur_pos_;
        while (cur_pos_ < end_ && *cur_pos_ != '\n')
        {
            ++cur_pos_;
        }
        const char* end = cur_pos_;
        ++cur_pos_; // skip the newline

        if (begin != end && *(end - 1) == '\r')
        {
            --end;
        }
        cur_value_ = std::string(begin, end - begin);
    }

    bool equal(buffer_line_iterator const& other) const
    {
        return cur_pos_ == other.cur_pos_;
    }

    std::string const& dereference() const
    {
        assert(cur_pos_);
        return cur_value_;
    }

    const char* buffer_;
    const char* end_;
    const char* cur_pos_;
    std::string cur_value_;
};


}}

#endif
