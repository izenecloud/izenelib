#ifndef STRING_CHAR_SEPARATOR_H
#define STRING_CHAR_SEPARATOR_H
/**
 * @file string/CharSeparator.h
 * @author Ian Yang
 * @date Created <2010-02-01 17:26:32>
 * @date Updated <2010-02-03 16:57:56>
 * @brief Tokonize using char separator
 */

#include <set>
#include <utility>

namespace izenelib {namespace util{

template<typename Char>
class CharSeparator
{
public:
    CharSeparator()
    : delimiters_(), outputDone_(false)
    {}

    template<typename Iterator>
    CharSeparator(Iterator delimitersFirst,
                  Iterator delimitersLast)
    : delimiters_(delimitersFirst, delimitersLast), outputDone_(false)
    {}

    template<typename Iterator>
    void setDelimiters(Iterator first, Iterator last)
    {
        delimiters_.assign(first, last);
    }

    void reset()
    {}

    template<typename Iterator>
    bool operator()(Iterator& next, Iterator end,
                    std::pair<Iterator, Iterator>& tok)
    {
        if (next == end)
        {
            if (outputDone_)
            {
                return false;
            }
            else
            {
                outputDone_ = true;
                tok.first = next;
                tok.second = end;
                return true;
            }
        };

        tok.first = next;
        while (next != end && !isDelimiter(*next))
        {
            ++next;
        }
        tok.second = next;

        if (next == end)
        {
            outputDone_ = true;
        }
        else
        {
            ++next;
            outputDone_ = false;
        }

        return true;
    }

private:
    bool isDelimiter(Char c) const
    {
        return delimiters_.find(c) != delimiters_.end();
    }

    std::set<Char> delimiters_;
    bool outputDone_;
};

}} // namespace izenelib::util

#endif // STRING_CHAR_SEPARATOR_H
