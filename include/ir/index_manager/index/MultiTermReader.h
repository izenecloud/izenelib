#ifndef MULTITERMSEARCHER_H
#define MULTITERMSEARCHER_H

#include <ir/index_manager/index/TermReader.h>
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
        pTermReader = NULL;
        next = NULL;
    }
    ReaderCache(BarrelInfo* barrelInfo,TermReader* pSe)
    {
        this->barrelInfo = barrelInfo;
        pTermReader = pSe;
        next = NULL;
    }
    ~ReaderCache()
    {
        barrelInfo = NULL;
        delete pTermReader;
        pTermReader = NULL;
        delete next;
        next = NULL;
    }
protected:
    BarrelInfo*	barrelInfo;
    TermReader*	pTermReader;
    ReaderCache*	next;
    friend class MultiTermReader;
};

class MultiIndexBarrelReader;
class MultiTermReader :  public TermReader
{
public:
    MultiTermReader(void);

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

protected:
    /**
     * load reader
     */
    ReaderCache* loadReader(const char* field);

protected:
    collectionid_t colID;

    MultiIndexBarrelReader* pBarrelReader;

    ReaderCache* pCurReader;

    map<string,ReaderCache*> readerCache;	//search cache
};

}

NS_IZENELIB_IR_END

#endif
