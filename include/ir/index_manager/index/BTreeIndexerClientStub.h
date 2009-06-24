#ifndef BTREEINDEXERCLIENTSTUB_H
#define BTREEINDEXERCLIENTSTUB_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/IndexerDocument.h>

#include <3rdparty/febird/io/boostvariant.h>
#include <3rdparty/febird/io/SocketStream.h>
#include <3rdparty/febird/rpc/client.h>

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


struct BTreeIndexerClientImpl
{
    boost::shared_ptr<SocketStream> cs_;
    boost::shared_ptr<rpc_client<PortableDataInput, PortableDataOutput> > client_;
    izenelib::ir::indexmanager::client::BTreeIndexerStubPtr stubPtr_; //
};
}

NS_IZENELIB_IR_END


#endif
