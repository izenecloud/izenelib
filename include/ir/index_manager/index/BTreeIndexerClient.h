#ifndef BTREEINDEXERCLIENT_H
#define BTREEINDEXERCLIENT_H

#include <3rdparty/febird/io/boostvariant.h>
#include <3rdparty/febird/io/SocketStream.h>
#include <3rdparty/febird/rpc/client.h>

#include <ir/index_manager/index/BTreeIndex.h>

using namespace febird;
using namespace febird::rpc;


NS_IZENELIB_IR_BEGIN

namespace indexmanager{

namespace client{

BEGIN_RPC_INTERFACE(BTreeIndexerStub, GlobaleScope)
    RPC_ADD_MF(add)
END_RPC_ADD_MF()
    RPC_DECLARE_MF(add, (collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid))
END_RPC_INTERFACE()
}

class BTreeIndexerClient:public BTreeIndexerInterface
{
public:
    void switchServer(string ip, string port);

    void add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid);

    void remove(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid){}
private:
    boost::shared_ptr<SocketStream> cs_;
    boost::shared_ptr<rpc_client<PortableDataInput, PortableDataOutput> > client_;
    izenelib::ir::indexmanager::client::BTreeIndexerStubPtr stubPtr_;
};


}

NS_IZENELIB_IR_END


#endif
