#ifndef MRLITE_UTIL_H_
#define MRLITE_UTIL_H_

#include <set>
#include <string>
#include <vector>

namespace net{namespace mrlite{

// Delete elements (in pointer type) in a STL container like vector,
// list, and deque.
template <class Container>
void STLDeleteElementsAndClear(Container* c)
{
    for (typename Container::iterator iter = c->begin();
            iter != c->end(); ++iter)
    {
        if (*iter != NULL)
        {
            delete *iter;
        }
    }
    c->clear();
}

// Delete elements (in pointer type) in a STL associative container
// like map and hash_map.
template <class AssocContainer>
void STLDeleteValuesAndClear(AssocContainer* c)
{
    for (typename AssocContainer::iterator iter = c->begin();
            iter != c->end(); ++iter)
    {
        if (iter->second != NULL)
        {
            delete iter->second;
        }
    }
    c->clear();
}

// Subdivide string |full| into substrings according to delimitors
// given in |delim|.  |delim| should pointing to a string including
// one or more characters.  Each character is considerred a possible
// delimitor.  For example,
//   vector<string> substrings;
//   SplitStringUsing("apple orange\tbanana", "\t ", &substrings);
// results in three substrings:
//   substrings.size() == 3
//   substrings[0] == "apple"
//   substrings[1] == "orange"
//   substrings[2] == "banana"
void SplitStringUsing(const std::string& full,
                      const char* delim,
                      std::vector<std::string>* result);

// This function has the same semnatic as SplitStringUsing.  Results
// are saved in an STL set container.
void SplitStringToSetUsing(const std::string& full,
                           const char* delim,
                           std::set<std::string>* result);


template <typename T>
struct simple_insert_iterator
{
    explicit simple_insert_iterator(T* t) : t_(t) { }

    simple_insert_iterator<T>& operator=(const typename T::value_type& value)
    {
        t_->insert(value);
        return *this;
    }

    simple_insert_iterator<T>& operator*()
    {
        return *this;
    }
    simple_insert_iterator<T>& operator++()
    {
        return *this;
    }
    simple_insert_iterator<T>& operator++(int placeholder)
    {
        return *this;
    }

    T* t_;
};

template <typename T>
struct back_insert_iterator
{
    explicit back_insert_iterator(T& t) : t_(t) {}

    back_insert_iterator<T>& operator=(const typename T::value_type& value)
    {
        t_.push_back(value);
        return *this;
    }

    back_insert_iterator<T>& operator*()
    {
        return *this;
    }
    back_insert_iterator<T>& operator++()
    {
        return *this;
    }
    back_insert_iterator<T> operator++(int placeholder)
    {
        return *this;
    }

    T& t_;
};


template <typename StringType, typename ITR>
static inline
void SplitStringToIteratorUsing(const StringType& full,
                                const char* delim,
                                ITR& result)
{
    // Optimize the common case where delim is a single character.
    if (delim[0] != '\0' && delim[1] == '\0')
    {
        char c = delim[0];
        const char* p = full.data();
        const char* end = p + full.size();
        while (p != end)
        {
            if (*p == c)
            {
                ++p;
            }
            else
            {
                const char* start = p;
                while (++p != end && *p != c)
                {
                    // Skip to the next occurence of the delimiter.
                }
                *result++ = StringType(start, p - start);
            }
        }
        return;
    }

    std::string::size_type begin_index, end_index;
    begin_index = full.find_first_not_of(delim);
    while (begin_index != std::string::npos)
    {
        end_index = full.find_first_of(delim, begin_index);
        if (end_index == std::string::npos)
        {
            *result++ = full.substr(begin_index);
            return;
        }
        *result++ = full.substr(begin_index, (end_index - begin_index));
        begin_index = full.find_first_not_of(delim, end_index);
    }
}

template <class ConstForwardIterator>
void JoinStrings(const ConstForwardIterator& begin,
                 const ConstForwardIterator& end,
                 const std::string& delimiter,
                 std::string* output)
{
    output->clear();
    for (ConstForwardIterator iter = begin; iter != end; ++iter)
    {
        if (iter != begin)
        {
            output->append(delimiter);
        }
        output->append(*iter);
    }
}

template <class ConstForwardIterator>
std::string JoinStrings(const ConstForwardIterator& begin,
                        const ConstForwardIterator& end,
                        const std::string& delimiter)
{
    std::string output;
    JoinStrings(begin, end, delimiter, &output);
    return output;
}

template <class Container>
std::string JoinStrings(const Container& container,
                        const std::string& delimiter = " ")
{
    return JoinStrings(container.begin(), container.end(), delimiter);
}


}}

#endif

