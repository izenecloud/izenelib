#ifndef DOCUMENTMANAGERCLIENT_REMOTE_H
#define DOCUMENTMANAGERCLIENT_REMOTE_H

#include <ir/index_manager/index/adaptor/DocumentManagerClient.h>

#include <sf1v5/document-manager/Document.h>
#include <sf1v5/document-manager/UniqueDocIdentifier.h>
#include <sf1v5/message_framework.h>
#include <sf1v5/message-framework/MessageAgent.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

template<> DocumentManagerClient<Distribute>
{
public:

    DocumentManagerClient() {}

    DocumentManagerClient(const sf1v5::MessageFrameworkNode& controllerInfo);

    ~DocumentManagerClient(){}

    unsigned int getDocumentCountByCollectionId(unsigned int collectionId);

    bool getDocIdListByCollectionId(unsigned int collectionId, std::vector<unsigned int> & docIdList);

    IndexerDocument* getDocumentByDocId(unsigned int collectionId, unsigned int docId){return NULL;}

    IndexerDocument* getDeletedDocumentByDocId(unsigned int collectionId, unsigned int docId){return NULL;}


#ifdef SF1_TIME_CHECK
    void printDocumentProcessTime(void);
#endif

private:
    bool requestServiceResult(const std::string& serviceName,
                              const std::vector<boost::shared_ptr<sf1v5::VariantType> >& parameterList,
                              sf1v5::ServiceResult& serviceResult);

    bool connectDocumentManager(const sf1v5::MessageFrameworkNode& controllerInfo);
private:

    sf1v5::MessageClient* messageClient_;
};

template<>
DocumentManagerClient<Distribute>::DocumentManagerClient(const MessageFrameworkNode& controllerInfo)
{
    messageClient_ = NULL;
    connectDocumentManager(controllerInfo);
}


template<>
bool DocumentManagerClient<Distribute>::connectDocumentManager(const MessageFrameworkNode& controllerInfo)
{
    std::string clientName("IndexManager");
    messageClient_ = new MessageClient(MF_CLIENT_ARG(clientName, controllerInfo));

    return true;
}

template<> 
DocumentManagerClient<Distribute>::~DocumentManagerClient()
{
    if (messageClient_)
        delete messageClient_;
    messageClient_ = NULL;
}

template<>
unsigned int DocumentManagerClient<Distribute>::getDocumentCountByCollectionId(unsigned int collectionId)
{
    ServiceRequestInfo  request;
    ServiceResult       serviceResult;

    unsigned int nDocCount = 0;
    boost::shared_ptr<unsigned int> collectionIdData(new unsigned int(collectionId) );
    boost::shared_ptr<VariantType> data(new VariantType(collectionIdData) );
    request.appendParameter(data);

    if (requestService("getDocumentCountByCollectionId", request, serviceResult, *messageClient_ ))
    {
        // std::vector<VariantType> values;

        // modified by TuanQuang Nguyen, Dec 15th 2008, interface is modified
        // old code: serviceResult.getServiceResult(values);
        const std::vector<boost::shared_ptr<VariantType> >& values =
            serviceResult.getServiceResult();
        if (values.size() != 1)
            SF1V5_THROW(ERROR_BAD_PARAMETER, "Wrong size");

        boost::shared_ptr<unsigned int> docCount;
        values[0]->getData(docCount);


        return *docCount;
    }

    return nDocCount;
}

template<>
bool DocumentManagerClient<Distribute>::getDocIdListByCollectionId(unsigned int collectionId,
        boost::shared_ptr<std::vector<UniqueDocIdentifier> >& docIdList)
{
    ServiceRequestInfo  request;
    ServiceResult       serviceResult;

    boost::shared_ptr<unsigned int> tempData(new unsigned int(collectionId));
    boost::shared_ptr<VariantType> collectionIdData(
        new VariantType(tempData) );
    request.appendParameter(collectionIdData);

    if (requestService("getDocIdListByCollectionId", request, serviceResult, *messageClient_))
    {
        // std::vector<VariantType> values;

        // modified by TuanQuang Nguyen, Dec 15th 2008, interface is modified
        // old code: serviceResult.getServiceResult(values);
        const std::vector<boost::shared_ptr<VariantType> >& values =
            serviceResult.getServiceResult();
        if (values.size() != 1)
            SF1V5_THROW(ERROR_BAD_PARAMETER, "Wrong size");

        values[0]->getData(docIdList);

        return true;
    }

    return false;
}

template<>
Document* DocumentManagerClient<Distribute>::getDocumentByDocId(unsigned int collectionId, unsigned int docId)
{
    ServiceRequestInfo  request;
    ServiceResult       serviceResult;

    boost::shared_ptr<UniqueDocIdentifier>  uniqueDocId(new UniqueDocIdentifier() );
    uniqueDocId->collId = collectionId;
    uniqueDocId->docId = docId;
    boost::shared_ptr<VariantType> uniqueDocIdData(new VariantType(uniqueDocId) );
    request.appendParameter(uniqueDocIdData);

    if (requestService("getDocumentByDocId", request, serviceResult, *messageClient_))
    {
        const std::vector<boost::shared_ptr<VariantType> >& values =
            serviceResult.getServiceResult();
        if (values.size() != 1)
            SF1V5_THROW(ERROR_BAD_PARAMETER, "Wrong size");

        boost::shared_ptr<Document> document;
        if (!values[0]->getData(document))
            SF1V5_THROW(ERROR_BAD_PARAMETER, "Wrong Type");
        Document* pDocument = new Document();
        *pDocument = *document;

        return pDocument;
    }

    SF1V5_THROW(ERROR_BAD_PARAMETER, "Fail to get result");
    return NULL;
}

template<>
Document* DocumentManagerClient<Distribute>::getDeletedDocumentByDocId(unsigned int collectionId, unsigned int docId)
{
    ServiceRequestInfo  request;
    ServiceResult       serviceResult;

    boost::shared_ptr<UniqueDocIdentifier> uniqueDocId( new UniqueDocIdentifier() );
    uniqueDocId->collId = collectionId;
    uniqueDocId->docId = docId;
    boost::shared_ptr<VariantType> uniqueDocIdData(
        new VariantType(uniqueDocId) );
    request.appendParameter(uniqueDocIdData);

    if (requestService("getDeletedDocumentByDocId", request, serviceResult, *messageClient_))
    {
        // std::vector<VariantType> values;

        // modified by TuanQuang Nguyen, Dec 15th 2008, interface is modified
        const std::vector<boost::shared_ptr<VariantType> >& values = serviceResult.getServiceResult();
        if (values.size() != 1)
            SF1V5_THROW(ERROR_BAD_PARAMETER, "Wrong size");

        boost::shared_ptr<Document> document;
        values[0]->getData(document);
        Document* pDocument = new Document();
        *pDocument = *document;


        return pDocument;
    }


    return NULL;
}

#ifdef SF1_TIME_CHECK
// @by MyungHyun Lee (Kent) - Feb 6, 2009
template<>
void DocumentManagerClient<Distribute>::printDocumentProcessTime(void)
{
    ServiceRequestInfo  request;

    if (requestService("printDocumentTime", request, *messageClient_) == false)
    {
        cerr <<  "Failed to notify Document Process to print Time Check" << endl;
    }
}
#endif


}

NS_IZENELIB_IR_END

#endif
