#ifndef IZENELIB_UTIL_COMPRESSION_COMPRESSED_SET_LAZY_AND_SET_H__
#define IZENELIB_UTIL_COMPRESSION_COMPRESSED_SET_LAZY_AND_SET_H__
#include "CompressedSet.h"

namespace izenelib
{
namespace util
{
namespace compression
{

class LazyAndSet;

class LazyAndSetIterator : public Set::Iterator
{
private:
    unsigned lastReturn;
    vector<boost::shared_ptr<Set::Iterator> > iterators;
    const LazyAndSet& set;
public:
    LazyAndSetIterator(const LazyAndSet* parent);
    unsigned int docID();
    unsigned int nextDoc();
    unsigned int Advance(unsigned int target);
};

class LazyAndSet : Set
{
public:
    vector<boost::shared_ptr<Set> > sets_;
    int nonNullSize;
    mutable unsigned int setSize;
    mutable bool init;;
    LazyAndSet();

    LazyAndSet(vector<boost::shared_ptr<Set> >& sets);

    inline bool find(unsigned int val) const;

    unsigned int size() const ;

    boost::shared_ptr<Set::Iterator> iterator() const;

};

}
}
}
#endif  // IZENELIB_UTIL_COMPRESSION_COMPRESSED_SET_LAZY_AND_SET_H__
