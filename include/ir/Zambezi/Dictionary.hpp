#ifndef IZENELIB_IR_ZAMBEZI_DICTIONARY_HPP
#define IZENELIB_IR_ZAMBEZI_DICTIONARY_HPP

#include "Consts.hpp"
#include <util/izene_serialization.h>

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
    Dictionary(size_t vocab_size)
        : dict_(vocab_size)
    {
    }

    ~Dictionary()
    {
    }

    std::size_t size() const
    {
        return dict_.size();
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
        if (dict_.load_factor() < dict_.max_load_factor())
            return dict_.insert(std::make_pair(word, dict_.size())).first->second;

        // not to insert as dictionary is full
        return getTermId(word);
    }

    void save(std::ostream& ostr) const
    {
        uint32_t vocabSize = dict_.size();
        ostr.write((const char*)&vocabSize, sizeof(vocabSize));
        std::vector<std::pair<WordType, uint32_t> > seq(dict_.begin(), dict_.end());
        std::sort(seq.begin(), seq.end());
        for (typename std::vector<std::pair<WordType, uint32_t> >::const_iterator it = seq.begin();
                it != seq.end(); ++it)
        {
            char* buf;
            size_t len = 0;
            izenelib::util::izene_serialization<WordType> izsKey(it->first);
            izsKey.write_image(buf, len);
            uint32_t slen = len;
            ostr.write((const char*)(&slen), sizeof(slen));
            ostr.write(buf, len);
            ostr.write((const char*)&it->second, sizeof(it->second));
        }
    }

    void load(std::istream& istr)
    {
        uint32_t vocabSize = 0;
        istr.read((char*)&vocabSize, sizeof(vocabSize));
        for (uint32_t i = 0; i < vocabSize; ++i)
        {
            uint32_t slen = 0;
            istr.read((char*)(&slen), sizeof(slen));
            size_t len = slen;
            char buf[len];
            istr.read(buf, len);
            izenelib::util::izene_deserialization<WordType> izsKey(buf, len);
            WordType word;
            izsKey.read_image(word);
            istr.read((char*)&dict_[word], sizeof(dict_[0]));
        }
    }

private:
    boost::unordered_map<WordType, uint32_t> dict_;
};

}

NS_IZENELIB_IR_END

#endif
