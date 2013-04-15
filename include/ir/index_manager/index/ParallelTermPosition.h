/**
* @file        ParallelTermPosition.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Used by getDocsByTermsInProperties in Indexer
*/
#ifndef PARALLELTERMPOSITION_H
#define PARALLELTERMPOSITION_H

#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/index/TermPositions.h>
#include <ir/index_manager/index/CommonItem.h>
#include <ir/index_manager/utility/PriorityQueue.h>

//#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <map>
#include <vector>

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
    class TermPositionEntry
    {
    public:
        TermPositionEntry(string property)
            :property(property)
            ,pTermReader(NULL)
            ,pPositions(NULL)
        {}

        ~TermPositionEntry()
        {
            if(pPositions)
                delete pPositions;
            if(pTermReader)
                delete pTermReader;
        }

        bool next()
        {
            if(pPositions == NULL)
                return false;
            return pPositions->next();
        }

	void setCurrent(bool bCurrent){current = bCurrent;}
	
	bool isCurrent(){return current;}

    public:
        std::string property;
        TermReader* pTermReader;
        TermPositions* pPositions;
        bool current;
    };

    class TermPositionQueue : public PriorityQueue<ParallelTermPosition::TermPositionEntry*>
    {
    public:
        TermPositionQueue(size_t size)
        {
            initialize(size,false);
        }
    protected:
        bool lessThan(ParallelTermPosition::TermPositionEntry* o1, ParallelTermPosition::TermPositionEntry* o2)
        {
            return (o1->pPositions->doc() < o2->pPositions->doc());
        }
    };


public:
    ParallelTermPosition(collectionid_t colID, IndexReader* pIndexReader, const std::vector<std::string>& properties);

    ~ParallelTermPosition();
public:
    bool seek(termid_t termID);

    docid_t doc(){return currDoc_;}

    count_t maxdf();

    count_t ctf();	

    count_t tf(){return currTf_;}

    bool next();

    void getPositions(std::map<string, PropertyItem>& result);

    void get_df_and_ctf(termid_t termID, DocumentFrequencyInProperties& dfmap, CollectionTermFrequencyInProperties& ctfmap);

    bool isValid()
    {
        return pIndexReader_!=NULL;
    }

private:
    void initQueue();
	
private:
    collectionid_t colID_;

    IndexReader* pIndexReader_;

    vector<string> properties_;

    ParallelTermPosition::TermPositionQueue* positionsQueue_;

    std::vector<ParallelTermPosition::TermPositionEntry*> positions_;

    docid_t currDoc_;

    size_t currTf_;
};
}

NS_IZENELIB_IR_END

#endif
