#ifndef POSTINGS_LIST_H_GUARD
#define POSTINGS_LIST_H_GUARD

#include "InvertedIndex.hpp"

#include <vector>
#include <boost/shared_ptr.hpp>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class InvertedIndex;
class PostingsList
{
public:
    PostingsList(const boost::shared_ptr<InvertedIndex>& index, uint32_t termid);
    ~PostingsList();

    void nextPosting();
    bool hasNext() const;

    uint32_t getDocumentId() const;
    uint32_t getTermFrequency() const;
    uint32_t getDocumentFrequency() const;

private:
    boost::shared_ptr<InvertedIndex> index_;
    uint32_t termid_;
    uint32_t df_;

    long pointer_;
    uint32_t length_;
    uint32_t position_;

    std::vector<uint32_t> docid_list_;
    std::vector<uint32_t> tf_list_;
};

}

NS_IZENELIB_IR_END

#endif
