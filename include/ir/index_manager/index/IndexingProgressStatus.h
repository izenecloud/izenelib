/**
* @file        IndexingProgressStatus.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Define some common types of Indexer, these types or classes are the interfaces between IndexManager and other Managers.
*/

#ifndef INDEXING_PROGRESS_INDEXTYPE_H
#define INDEXING_PROGRESS_INDEXTYPE_H

#include <boost/serialization/serialization.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class IndexingProgressStatus
{
public:
    IndexingProgressStatus() {}

    IndexingProgressStatus(const IndexingProgressStatus& clone)
            :totalDocumentCount(clone.totalDocumentCount)
            ,indexedCount(clone.indexedCount)
            ,waitTime(clone.waitTime)
            ,timeElapsed(clone.timeElapsed)
    {}

    IndexingProgressStatus& operator=(const IndexingProgressStatus clone)
    {
        totalDocumentCount = clone.totalDocumentCount;
        indexedCount = clone.indexedCount;
        waitTime = clone.waitTime;
        timeElapsed = clone.timeElapsed;

        return *this;
    }

    ~IndexingProgressStatus() {}

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & totalDocumentCount & indexedCount & waitTime & timeElapsed;
    }
public:
    count_t totalDocumentCount;

    count_t indexedCount;

    double waitTime;

    double timeElapsed;
};


}

NS_IZENELIB_IR_END

#endif
