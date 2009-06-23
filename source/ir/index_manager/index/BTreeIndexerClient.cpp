#include <ir/index_manager/index/BTreeIndexerClient.h>

using namespace izenelib::ir::indexmanager;

void BTreeIndexerClient::switchServer(string ip, string port)
{
	string url = ip.append(":");
	url += port;
	cs_.reset(ConnectSocket(url.c_str()));
	client_.reset(new rpc_client<PortableDataInput, PortableDataOutput>(cs_.get()));
	client_->create(stubPtr_, "BTreeIndexerStubObj");	
}

void BTreeIndexerClient::add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid)
{
	stubPtr_->add(colID,fid,value,docid);
}


