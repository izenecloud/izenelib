#ifndef IZENELIB_IR_ZAMBEZI_DICTIONARY_HPP
#define IZENELIB_IR_ZAMBEZI_DICTIONARY_HPP

#include "Consts.hpp"
#include <types.h>
#include <boost/unordered_map.hpp>
#include <iostream>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

template <class WordType>
class Dictionary
{
public:
    /* Create hash table, initialise ptrs to NULL */
    Dictionary(size_t vocab_size = DEFAULT_VOCAB_SIZE)
        : dict_(vocab_size)
    {
    }

    ~Dictionary()
    {
    }

    /* Search hash table for given string */
    uint32_t getTermId(const WordType& word) const
    {
        typename boost::unordered_map<WordType, uint32_t>::const_iterator it = dict_.find(word);
        if (it == dict_.end())
            return INVALID_ID;
        else
            return it->second;
    }

    /* Search hash table for given string, insert if not found */
    uint32_t insertTerm(const WordType& word)
    {
        return dict_.insert(std::make_pair(word, dict_.size())).first->second;
    }

    void save(std::ostream& ostr) const
    {
        uint32_t vocabSize = dict_.size();
        ostr.write((const char*)&vocabSize, sizeof(uint32_t));
        std::vector<std::pair<WordType, uint32_t> > seq(dict_.begin(), dict_.end());
        std::sort(seq.begin(), seq.end());
        for (typename std::vector<std::pair<WordType, uint32_t> >::const_iterator it = seq.begin();
                it != seq.end(); ++it)
        {
            uint32_t len = it->first.size();
            ostr.write((const char*)&len, sizeof(uint32_t));
            ostr.write((const char*)&it->first[0], len);
            ostr.write((const char*)&it->second, sizeof(uint32_t));
        }
    }

    void load(std::istream& istr)
    {
        uint32_t vocabSize = 0;
        istr.read((char*)&vocabSize, sizeof(uint32_t));
        for (uint32_t i = 0; i < vocabSize; ++i)
        {
            uint32_t len = 0;
            istr.read((char*)&len, sizeof(uint32_t));
            std::string word(len, '\0');
            istr.read((char*)&word[0], len);
            istr.read((char*)&dict_[word], sizeof(uint32_t));
        }
    }

private:
    boost::unordered_map<WordType, uint32_t> dict_;
};

}

NS_IZENELIB_IR_END

#endif
