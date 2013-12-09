#ifndef SF1V5_ZAMBEZI_BASE_INDEX_H
#define SF1V5_ZAMBEZI_BASE_INDEX_H

#include "Consts.hpp"

#include <iostream>
#include <vector>
#include <string>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

// Define the interface for all kindes of zambezi index;

class ZambeziIndex
{
public:
    virtual ~ZambeziIndex() {}

    virtual void save(std::ostream& ostr) const = 0;
    virtual void load(std::istream& istr) = 0;

    virtual void flush() = 0;

    /// @brief For different zambezi index, the usage is different;
    virtual void insertDoc(
            uint32_t docid,
            const std::vector<std::string>& term_list,
            const std::vector<uint32_t>& score_list) = 0;

    /// @brief For different zambezi index, not all the parameters need to be used;
    virtual void retrievalWithBuffer(
            Algorithm algorithm,
            const std::vector<std::pair<std::string, int> >& term_list,
            uint32_t hits,
            bool search_buffer,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const = 0;

    virtual uint32_t totalDocNum() const = 0;

};

}

NS_IZENELIB_IR_END
#endif
