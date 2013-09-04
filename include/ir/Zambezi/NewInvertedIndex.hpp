/**
 * An inverted index data structure consisting of
 * the following components:
 *
 *  - SegmentPool, which contains all segments.
 *  - Dictionary, which is a mapping from term to term id
 *  - Pointers, which contains Document Frequency, Head/Tail Pointers,
 *    etc.
 *  - DocumentVectors, which contains compressed document vector representation
 *    of documents
 *
 * @author Nima Asadi
 */

#ifndef IZENELIB_IR_ZAMBEZI_NEW_INVERTED_INDEX_HPP
#define IZENELIB_IR_ZAMBEZI_NEW_INVERTED_INDEX_HPP

#include "NewSegmentPool.hpp"
#include "Dictionary.hpp"
#include "Pointers.hpp"
#include "buffer/NewBufferMaps.hpp"
#include "Consts.hpp"

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class NewInvertedIndex
{
public:
    NewInvertedIndex(bool reverse = true);

    ~NewInvertedIndex();

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

    bool hasValidPostingsList(uint32_t termid) const;

    void insertDoc(
            uint32_t docid,
            const std::vector<std::string>& term_list,
            const std::vector<uint32_t>& score_list);

    void flush();

    void retrieval(
            const std::vector<std::string>& term_list,
            uint32_t hits,
            std::vector<uint32_t>& docid_list,
            std::vector<uint32_t>& score_list) const;

private:
    NewBufferMaps buffer_;
    NewSegmentPool pool_;
    Dictionary dictionary_;
    Pointers pointers_;

    FastPFor codec_;
};

}

NS_IZENELIB_IR_END

#endif
