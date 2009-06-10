#ifndef DOCUMENTMANAGER_CLIENT_LOCAL_H
#define DOCUMENTMANAGER_CLIENT_LOCAL_H

#include <ir/index_manager/index/adaptor/DocumentManagerClient.h>

#include <sf1v5/document-manager/DocumentManager.h>
#include <sf1v5/document-manager/Document.h>
#include <sf1v5/document-manager/UniqueDocIdentifier.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

template<> DocumentManagerClient<Local>
{
public:

    DocumentManagerClient(boost::shared_ptr<DocumentManager> documentManager):documentManager_(documentManager) {}

    ~DocumentManagerClient(){}

    unsigned int getDocumentCountByCollectionId(unsigned int collectionId);

    bool getDocIdListByCollectionId(unsigned int collectionId, std::vector<unsigned int> & docIdList);

    IndexerDocument* getDocumentByDocId(unsigned int collectionId, unsigned int docId);

    IndexerDocument* getDeletedDocumentByDocId(unsigned int collectionId, unsigned int docId);

#ifdef SF1_TIME_CHECK
    void printDocumentProcessTime(void);
#endif

private:
    boost::shared_ptr<DocumentManager> documentManager_;
};


}

NS_IZENELIB_IR_END

#endif
