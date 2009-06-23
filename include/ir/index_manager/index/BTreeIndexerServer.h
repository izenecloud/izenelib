#ifndef BTREEINDEXERSERVER_H
#define BTREEINDEXERSERVER_H

#include <3rdparty/febird/io/boostvariant.h>
#include <3rdparty/febird/io/SocketStream.h>
#include <3rdparty/febird/rpc/server.h>

#include <ir/index_manager/index/BTreeIndex.h>

using namespace febird;
using namespace febird::rpc;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

namespace server{
BEGIN_RPC_INTERFACE(BTreeIndexerStub, GlobaleScope)
    RPC_ADD_MF(add)
END_RPC_ADD_MF()
    RPC_DECLARE_MF(add, (collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid))
END_RPC_INTERFACE()
}

class BTreeIndexerStubImpl:public izenelib::ir::indexmanager::server::BTreeIndexerStub
{
public:
    BTreeIndexerStubImpl(BTreeIndexer* pBTreeIndexer);

    rpc_ret_t add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid);
private:
    BTreeIndexer* pBTreeIndexer_;
};

class BTreeIndexerServer
{
public:
    BTreeIndexerServer(string port, BTreeIndexer* pBTreeIndexer);

    ~BTreeIndexerServer();

    void run();

private:
    SocketAcceptor* acceptor_;

    rpc_server<PortableDataInput, PortableDataOutput> * server_;

    BTreeIndexer* pBTreeIndexer_;
};


}

NS_IZENELIB_IR_END


#endif

