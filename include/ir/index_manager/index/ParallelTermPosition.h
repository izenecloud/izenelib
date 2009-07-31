/**
* @file        ParallelTermPosition.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Used by getDocsByTermsInProperties in Indexer
*/
#ifndef PARALLELTERMPOSITION_H
#define PARALLELTERMPOSITION_H

#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/TermPositions.h>

#include <string>
#include <map>


NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
*  If there exist multi fields to be indexed in the inverted files, adopting this class can iterate indexes of multi fields parallelly,
*  which can output the results much more fast, or else each time we add an element in CommonItem, there has to be a search process
* , which will lead to about terrible performance decrease.
*/
class ParallelTermPosition
{
public:
    ParallelTermPosition(collectionid_t colID, IndexReader* pIndexReader, vector<string>& properties);

    ~ParallelTermPosition();
public:
    /**
    * Where the term exist in all of the fields that have been indexed.
    */
    bool seek(termid_t termID);
    /**
    * Index iterator
    * @param properties: Fields list that the returned docid exists
    * @param docid: Next docid to be returned
    */
    bool next(vector<string>& properties, docid_t& docid);
    /**
    * Retrieve the position information of a certain Field
    */
    void getPositions(string& property, vector<loc_t>* positions);

    bool isValid()
    {
        return pIndexReader_!=NULL;
    }

private:
    collectionid_t colID_;

    IndexReader* pIndexReader_;

    vector<string> properties_;

    map<string, TermReader*> termReaderMap_;

    map<string, TermPositions*> termPositionMap_;

    map<string, bool> flagMap_;

    map<string, docid_t> currDocMap_;
};
}

NS_IZENELIB_IR_END

#endif
