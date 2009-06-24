#include <ir/index_manager/index/BTreeIndexerServerStub.h>
#include <ir/index_manager/index/BTreeIndexerServer.h>
#include <iostream>

using namespace std;
using namespace izenelib::ir::indexmanager;
using namespace izenelib::ir::indexmanager::server;

BTreeIndexerStubImpl::BTreeIndexerStubImpl(BTreeIndexer* pBTreeIndexer)
    :pBTreeIndexer_(pBTreeIndexer)
{}

rpc_ret_t BTreeIndexerStubImpl::add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid)
{
    pBTreeIndexer_->add(colID, fid, value, docid);
    return 0;
}

BTreeIndexerServerImpl::BTreeIndexerServerImpl(string port, BTreeIndexer* pBTreeIndexer)
    :pBTreeIndexer_(pBTreeIndexer)
{
    try{
        string url("0.0.0.0:");
        url += port;
        acceptor_ = new SocketAcceptor(url.c_str());
        server_ = new rpc_server<PortableDataInput, PortableDataOutput>(acceptor_);
        server_->add_servant(
            new BTreeIndexerStubImpl(pBTreeIndexer_),
            "BTreeIndexerStubObj",
            0 // 0 will not auto create GlobaleScope Object
        );
    }catch (const std::exception& exp)
    {
        cout<<exp.what()<<endl;
    }
}

BTreeIndexerServerImpl::~BTreeIndexerServerImpl()
{
    delete acceptor_;
    delete server_;
}

BTreeIndexerServer::BTreeIndexerServer(string port, BTreeIndexer* pBTreeIndexer)
{
    pImpl_ = new BTreeIndexerServerImpl(port, pBTreeIndexer);
}

BTreeIndexerServer::~BTreeIndexerServer()
{
    delete pImpl_;
}


void BTreeIndexerServer::run()
{
    try{
        pImpl_->server_->start();
    }catch (const std::exception& exp)
    {
        cout<<exp.what()<<endl;
    }
}


