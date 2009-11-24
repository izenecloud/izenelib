/**
* @file        BTreeIndexerStubImpl.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief 
*/
#ifndef BTREEINDEXERSERVERSTUB_H
#define BTREEINDEXERSERVERSTUB_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/IndexerDocument.h>

#include <3rdparty/febird/io/boostvariant.h>
#include <3rdparty/febird/io/SocketStream.h>
#include <3rdparty/febird/rpc/server.h>

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

class BTreeIndexer;
class BTreeIndexerStubImpl:public izenelib::ir::indexmanager::server::BTreeIndexerStub
{
public:
    BTreeIndexerStubImpl(BTreeIndexer* pBTreeIndexer);

    rpc_ret_t add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid);
private:
    BTreeIndexer* pBTreeIndexer_;
};

struct BTreeIndexerServerImpl
{
BTreeIndexerServerImpl(string port, BTreeIndexer* pBTreeIndexer);

~BTreeIndexerServerImpl();

SocketAcceptor* acceptor_;

rpc_server<PortableDataInput, PortableDataOutput> * server_;

BTreeIndexer* pBTreeIndexer_;
};

}

NS_IZENELIB_IR_END

#endif
