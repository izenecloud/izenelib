/// @file ForwardIndex.h
/// @brief ForwardIndex class that stores term word and byte offsets.
/// @author Yingfeng Zhang
/// @date 2009/05/21

#ifndef _FORWARD_INDEX_H_
#define _FORWARD_INDEX_H_


#include <util/DynamicArray.h>

#include <iostream>
#include <vector>
#include <deque>
#include <utility>

using namespace izenelib::util;

NS_IZENELIB_IR_BEGIN

namespace indexmanager {

typedef unsigned int WordOffset;
typedef unsigned int CharOffset;

typedef DynamicArray<std::deque<std::pair<WordOffset, CharOffset> >*, Const_NullValue<std::deque<std::pair<WordOffset, CharOffset> >*> > DynForwardIndexArray;
typedef DynamicArray<unsigned int, Const_NullValue<unsigned int > > TermArray;

///
/// @brief ForwardIndex stores terms of a document along with their
/// word offsets, and byte offsets. The class also provides such information
/// as term frequency.
///

class ForwardIndex
{
public:
    /// Constructor of a class. Initializes the data structure
    /// that stores the list of terms and their offsets.
    ForwardIndex();

    /// Destructor of a class. Clears the contents of a data structure
    /// that stores the list of terms and their offsets.
    ~ForwardIndex();

    size_t getNumTerms();

    /// The function retrieves the list of termId's
    /// stored in the forward index.
    /// @desc Get a list of term identifiers in this ForwardIndex.
    const std::deque<unsigned int>&  getTermIdList() const;

    /// The function retrieves the list of termId's
    /// stored in the forward index.
    /// @desc Use DynamicArray to guarantee the order of term, which is useful for index's compression.
    TermArray& getTermIdArray();
	
    /// The function retrieves the list of term offsets given a term identifier.
    std::deque<std::pair<WordOffset, CharOffset> >* getTermOffsetListByTermId(unsigned int termId);

    /// The function inserts term offset information given
    /// a term identifier into the Forward Index.
    bool insertTermOffset(unsigned int termId, std::pair<WordOffset, CharOffset>&  termOffset);

    inline void print(){};

private:
    size_t numTerms_;	

    DynForwardIndexArray	 fastTermOffsetListMap_;

    std::deque<unsigned int> termList_;

    TermArray termArray_;
};

} // end - namespace indexmanager

NS_IZENELIB_IR_END

#endif // _FORWARD_INDEX_H_
