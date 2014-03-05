#ifndef IZENELIB_IR_ZAMBEZI_INDEX_BASE_HPP
#define IZENELIB_IR_ZAMBEZI_INDEX_BASE_HPP

#include "FilterBase.hpp"

#include <iostream>
#include <vector>
#include <string>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

// Define the interface for all kindes of zambezi index;

class IndexBase
{
public:
    virtual ~IndexBase() {}

    virtual void save(std::ostream& ostr) const = 0;
    virtual void load(std::istream& istr) = 0;

    virtual void flush() = 0;

    virtual void insertDoc(
            uint32_t docid,
            const std::vector<std::string>& term_list,
            const std::vector<uint32_t>& score_list) = 0;

    virtual void retrieve(
            Algorithm algorithm,
            const std::vector<std::pair<std::string, int> >& term_list,
            const FilterBase* filter,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<float>& score_list) const = 0;

    virtual uint32_t totalDocNum() const = 0;
};

}

NS_IZENELIB_IR_END

#endif
