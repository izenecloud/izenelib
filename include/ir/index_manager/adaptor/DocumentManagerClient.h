#ifndef DOCUMENT_MANAGER_CLIENT_H
#define DOCUMENT_MANAGER_CLIENT_H

#include <ir/index_manager/index/IndexerDocument.h>

#include <util/BoostVariantUtil.h>

#include <time.h>
#include <boost/variant.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

struct StandAlone{};
struct Local{};
struct Distribute{};

template<typename IndexerType=StandAlone>
class DocumentManagerClient
{
public:

    DocumentManagerClient() {}

    ~DocumentManagerClient(){}

    unsigned int getDocumentCountByCollectionId(unsigned int collectionId){return 0;}

    bool getDocIdListByCollectionId(unsigned int collectionId, std::vector<unsigned int> & docIdList){return true;}

    IndexerDocument* getDocumentByDocId(unsigned int collectionId, unsigned int docId){return NULL;}

    IndexerDocument* getDeletedDocumentByDocId(unsigned int collectionId, unsigned int docId){return NULL;}


#ifdef SF1_TIME_CHECK
    void printDocumentProcessTime(void){}
#endif

};


typedef boost::variant<DocumentManagerClient<StandAlone>, DocumentManagerClient<Local>, DocumentManagerClient<Distribute> > DocumentManagerClientType;

class document_manager_visitor:public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(T& v, unsigned int collectionId, unsigned int& count)
    {
        count = v.getDocumentCountByCollectionId(collectionId);
    }

    template<typename T>
    void operator()(T& v, unsigned int collectionId, std::vector<unsigned int> & docIdList, bool& ret)
    {
	ret = v.getDocIdListByCollectionId(collectionId, docIdList);
    }

    template<typename T>
    void operator()(T& v, unsigned int collectionId, unsigned int docId, IndexerDocument* pDoc)
    {
        pDoc = v.getDocumentByDocId(collectionId, docId);
    }

    template<typename T>
    void operator()(T& v, unsigned int collectionId, unsigned int docId, IndexerDocument* pDoc, int dummy)
    {
        pDoc = v.getDeletedDocumentByDocId(collectionId, docId);
    }
};

}

NS_IZENELIB_IR_END

#endif
