#include <ir/Zambezi/Dictionary.hpp>
#include <ir/Zambezi/Consts.hpp>

#include <vector>
#include <algorithm>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

Dictionary::Dictionary()
    : dict_(DEFAULT_VOCAB_SIZE)
{
}

Dictionary::~Dictionary()
{
}

uint32_t Dictionary::getTermId(const std::string& word) const
{
    boost::unordered_map<std::string, uint32_t>::const_iterator it = dict_.find(word);
    if (it == dict_.end())
        return INVALID_ID;
    else
        return it->second;
}

uint32_t Dictionary::insertTerm(const std::string& word)
{
    return dict_.insert(std::make_pair(word, dict_.size())).first->second;
}

void Dictionary::save(std::ostream& ostr) const
{
    uint32_t vocabSize = dict_.size();
    ostr.write((const char*)&vocabSize, sizeof(uint32_t));
    std::vector<std::pair<std::string, uint32_t> > seq(dict_.begin(), dict_.end());
    std::sort(seq.begin(), seq.end());
    for (std::vector<std::pair<std::string, uint32_t> >::const_iterator it = seq.begin();
            it != seq.end(); ++it)
    {
        uint32_t len = it->first.size();
        ostr.write((const char*)&len, sizeof(uint32_t));
        ostr.write((const char*)&it->first[0], len);
        ostr.write((const char*)&it->second, sizeof(uint32_t));
    }
}

void Dictionary::load(std::istream& istr)
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

}

NS_IZENELIB_IR_END
