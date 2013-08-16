#ifndef IZENELIB_IR_ZAMBEZI_DICTIONARY_HPP
#define IZENELIB_IR_ZAMBEZI_DICTIONARY_HPP

#include <types.h>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <string>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class Dictionary
{
public:
    /* Create hash table, initialise ptrs to NULL */
    Dictionary();
    ~Dictionary();

    /* Search hash table for given string */
    uint32_t getId(const std::string& word) const;
    /* Search hash table for given string, insert if not found */
    uint32_t insertTerm(const std::string& word);

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

private:
    boost::unordered_map<std::string, uint32_t> dict_;
};

}

NS_IZENELIB_IR_END

#endif
