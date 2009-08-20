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

//#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <map>


NS_IZENELIB_IR_BEGIN

namespace indexmanager{
typedef boost::unordered_map <unsigned int,float> ID_FREQ_MAP_T;
//typedef rde::hash_map <unsigned int,float> ID_FREQ_MAP_T;
typedef std::map<std::string, ID_FREQ_MAP_T > DocumentFrequencyInProperties;
typedef std::map<std::string, ID_FREQ_MAP_T > CollectionTermFrequencyInProperties;

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
    void getPositions(string& property, boost::shared_ptr<std::deque<unsigned int> >& positions, freq_t& tf, freq_t& doclen);

    void getPositions(string& property, boost::shared_ptr<std::deque< std::pair<unsigned int,unsigned int> > >& positions, freq_t& tf, freq_t& doclen);

    void get_df_and_ctf(termid_t termID, DocumentFrequencyInProperties& dfmap, CollectionTermFrequencyInProperties& ctfmap);

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
