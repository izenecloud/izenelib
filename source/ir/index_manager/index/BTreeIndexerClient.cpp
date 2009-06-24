#include <ir/index_manager/index/BTreeIndexerClientStub.h>
#include <ir/index_manager/index/BTreeIndexerClient.h>

using namespace izenelib::ir::indexmanager;
using namespace izenelib::ir::indexmanager::client;

BTreeIndexerClient::BTreeIndexerClient()
{
    pImpl_ = new BTreeIndexerClientImpl;
}

BTreeIndexerClient::~BTreeIndexerClient()
{
    delete pImpl_;
}

void BTreeIndexerClient::switchServer(string ip, string port)
{
	string url = ip.append(":");
	url += port;
	pImpl_->cs_.reset(ConnectSocket(url.c_str()));
	pImpl_->client_.reset(new rpc_client<PortableDataInput, PortableDataOutput>(pImpl_->cs_.get()));
	pImpl_->client_->create(pImpl_->stubPtr_, "BTreeIndexerStubObj");	
}


void BTreeIndexerClient::add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid)
{
	pImpl_->stubPtr_->add(colID,fid,value,docid);
}


