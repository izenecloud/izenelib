#include <ir/index_manager/index/ForwardIndex.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

ForwardIndex::ForwardIndex()
    :numTerms_(0)
{
}


ForwardIndex::~ForwardIndex()
{
    DynForwardIndexArray::array_iterator iter = fastTermOffsetListMap_.elements();
	std::deque<std::pair<WordOffset, CharOffset> >* pForward;
    while (iter.next())
    {
        pForward = iter.element();
        delete pForward;		///clear posting data
    }
}

const std::deque<unsigned int>& ForwardIndex::getTermIdList() const
{
    return termList_;
}

TermArray& ForwardIndex::getTermIdArray()
{
    return termArray_;
}

size_t ForwardIndex::getNumTerms()
{
    return numTerms_;
}

std::deque<std::pair<WordOffset, CharOffset> >* ForwardIndex::getTermOffsetListByTermId(unsigned int termId)
{
    return fastTermOffsetListMap_[termId];
}

bool ForwardIndex::insertTermOffset(unsigned int termId, pair<WordOffset, CharOffset>& termOffset)
{
    std::deque<std::pair<WordOffset, CharOffset> >* pForward = fastTermOffsetListMap_[termId];
    if(pForward == NULL)
    {
        // append the term offset to the list
        pForward = new std::deque<std::pair<WordOffset, CharOffset> >;
        pForward->push_back(termOffset);
        fastTermOffsetListMap_[termId] = pForward;
        //termList_.push_back(termId);
        termArray_[termId] = termId;
        numTerms_++;
    }
    else
    {
        // if the term does not exist
        // insert the term and its offset
        pForward->push_back(termOffset);
    }
    return true;
}

