/**
* @file        MultiFieldTermReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   Read term in a single barrel with multi fields exist
*/

#ifndef MULTIFIELDTERMREADER_H
#define MULTIFIELDTERMREADER_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/index/FieldInfo.h>
#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/index/Term.h>
#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
* TermReader in a single barrel with multi fields exist
*/
class MultiFieldTermReader : public TermReader
{
public:
    MultiFieldTermReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldsInfo* pFieldsInfo, IndexLevel indexLevel);

    MultiFieldTermReader();

    virtual ~MultiFieldTermReader();
public:
    /**
     * open a index barrel
     * @param pDirectory index storage
     * @param barrelname index barrel name
     * @param pFieldInfo field information
     */
    void open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo);

    void reopen();
    /*
     *  set delete documents filter
     */
    void setDocFilter(Bitset* pFilter);

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
     * get term reader of a field
     * @param field field name
     * @return term reader, MUST be deleted by caller.
     */
    TermReader* termReader(const char* field);

    /**
     * close term reader
     */
    void close();

    /**
     * clone the term reader
     * @return term reader, MUST be deleted by caller.
     */
    TermReader* clone() ;

    /**
     * add a term reader
     * @param field field name
     * @param pTermReader term reader
     */
    void addTermReader(const char* field,TermReader* pTermReader);

    void setSkipInterval(int skipInterval);

    void setMaxSkipLevel(int maxSkipLevel);

protected:
    /**
     * get term information of a term
     * @param term term
     * @return term information
     */
    TermInfo* termInfo(Term* term);

protected:
    typedef map<string,TermReader*> reader_map;

    reader_map fieldsTermReaders_;
    TermReader* pCurReader_;
};

}

NS_IZENELIB_IR_END

#endif
