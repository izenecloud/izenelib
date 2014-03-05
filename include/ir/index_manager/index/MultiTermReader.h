#ifndef MULTITERMSEARCHER_H
#define MULTITERMSEARCHER_H

#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/TermInfo.h>

#include <utility> // std::pair
#include <vector>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class MultiIndexBarrelReader;
class MultiTermReader : public TermReader
{
public:
    MultiTermReader(MultiIndexBarrelReader* pBarrelReader, collectionid_t colID);

    MultiTermReader(const MultiTermReader& multiTermReader);

    virtual ~MultiTermReader(void);
public:
    /**
     * open a index barrel
     * @param pDirectory index storage
     * @param barrelname index barrel name
     * @param pFieldInfo field information
     */
    void open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo);


    void reopen();

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
     * get term information of a term
     * @param term term
     * @return term information
     * @note as there are multiple barrels, in return value,
     * only @c docFreq_, @c ctf_ and @c lastDocID_ is valid.
     */
    TermInfo* termInfo(Term* term);

    /**
     * close term reader
     */
    void close();

    /**
     * clone the term reader
     * @return term reader, MUST be deleted by caller.
     */
    TermReader* clone() ;

    void setSkipInterval(int skipInterval);

    void setMaxSkipLevel(int maxSkipLevel);

    void setDocFilter(Bitset* pFilter);

private:
    collectionid_t colID_;

    typedef std::pair<BarrelInfo*, TermReader*> BarrelTermReaderEntry;
    std::vector<BarrelTermReaderEntry> termReaders_;

    bool isOwnTermReaders_;

    TermInfo termInfo_;
};

}

NS_IZENELIB_IR_END

#endif
