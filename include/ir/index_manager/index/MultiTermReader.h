#ifndef MULTITERMSEARCHER_H
#define MULTITERMSEARCHER_H

#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/index/BarrelInfo.h>

#include <map>
#include <vector>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class ReaderCache
{
public:
    ReaderCache()
    {
        pTermReader_ = NULL;
        next_ = NULL;
    }
    ReaderCache(BarrelInfo* pBarrelInfo,TermReader* pSe)
    {
        pBarrelInfo_ = pBarrelInfo;
        pTermReader_ = pSe;
        next_ = NULL;
    }
    ~ReaderCache()
    {
        pBarrelInfo_ = NULL;
        delete pTermReader_;
        pTermReader_ = NULL;
        delete next_;
        next_ = NULL;
    }
private:
    BarrelInfo* pBarrelInfo_;
    TermReader* pTermReader_;
    ReaderCache* next_;
    friend class MultiTermReader;
};

class MultiIndexBarrelReader;
class MultiTermReader : public TermReader
{
public:
    MultiTermReader(MultiIndexBarrelReader* pBarrelReader, collectionid_t colID);

    virtual ~MultiTermReader(void);
public:
    /**
     * open a index barrel
     * @param pDirectory index storage
     * @param barrelname index barrel name
     * @param pFieldInfo field information
     */
    void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo);

    /**
     * get the term iterator
     * @param field field name
     * @return term iterator, MUST be deleted by caller
     */
    TermIterator* termIterator(const char* field);

    /**
     * seek a term
     * @param pTerm term
     * @return true if success, otherwise false, if the return value is true then {@link termDocFreqs()} and
     * {@link termPositions()}can be called.
     */
    bool seek(Term* term);

    /**
     * get term's document postings,must be called after call seek() success
     * @return return document postings,need to be deleted outside
     */
    TermDocFreqs* termDocFreqs();

    /**
     * get term's position postings,must be called after call seek() success
     * @return return position postings,need to be deleted outside
     */
    TermPositions* termPositions();

    /**
     * get document frequency of a term
     * @return document frequency
     */
    freq_t docFreq(Term* term);

    /**
     * close term reader
     */
    void close();

    /**
     * clone the term reader
     * @return term reader, MUST be deleted by caller.
     */
    TermReader* clone() ;

private:

    ReaderCache* loadReader(const char* field);

private:
    collectionid_t colID_;

    MultiIndexBarrelReader* pBarrelReader_;

    ReaderCache* pCurReader_;

    map<string,ReaderCache*> readerCache_;
};

}

NS_IZENELIB_IR_END

#endif
