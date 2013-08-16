#include <ir/Zambezi/Dictionary.hpp>
#include <ir/Zambezi/Consts.hpp>

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

uint32_t Dictionary::getId(const std::string& word) const
{
    boost::unordered_map<std::string, uint32_t>::const_iterator it = dict_.find(word);
    if (it == dict_.end())
        return INVALID_ID;
    else
        return it->second;
}

uint32_t Dictionary::insertTerm(const std::string& word)
{
    uint32_t& id = dict_[word];
    if (id == 0)
        id = dict_.size() - 1;
    return id;
}

void Dictionary::save(std::ostream& ostr) const
{
    uint32_t vocabSize = dict_.size();
    ostr.write((const char*)&vocabSize, sizeof(uint32_t));
    for (boost::unordered_map<std::string, uint32_t>::const_iterator it = dict_.begin();
            it != dict_.end(); ++it)
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
