/**
 * Implementation of the union set of multiple DocIdSets (which essentially is a merged set of thes DocIdSets).
 */
#ifndef IZENELIB_UTIL_COMPRESSION_COMPRESSED_SET_LAZY_OR_SET_H__
#define IZENELIB_UTIL_COMPRESSION_COMPRESSED_SET_LAZY_OR_SET_H__

#include "CompressedSet.h"

namespace izenelib
{
namespace util
{
namespace compression
{

class LazyOrSetIterator : public Set::Iterator
{
private:
    class Item
    {
    public:
        boost::shared_ptr<Set::Iterator> iter;
        unsigned int doc;
        Item(boost::shared_ptr<Set::Iterator> it)
        {
            iter = it;
            doc = 0;
        }
    };
    unsigned _curDoc;
    vector<boost::shared_ptr<Item> > _heap;
    int _size;
    void heapRemoveRoot();
    void heapAdjust();
public:
    LazyOrSetIterator(vector<boost::shared_ptr<Set> > sets);
    unsigned int docID();
    unsigned int nextDoc();
    unsigned int Advance(unsigned int target);
};

class LazyOrSet : public Set
{
private:
    static int INVALID;
    vector<boost::shared_ptr<Set> > sets;
    mutable int _size;

public:

    LazyOrSet(vector<boost::shared_ptr<Set> > docSets);

    boost::shared_ptr<Set::Iterator>  iterator()  const;

    //Override
    unsigned int size() const;

    bool find(unsigned int val) const;
};

}
}
}
#endif  //IZENELIB_UTIL_COMPRESSION_COMPRESSED_SET_LAZY_OR_SET_H__
