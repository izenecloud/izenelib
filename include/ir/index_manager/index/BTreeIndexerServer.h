#ifndef BTREEINDEXERSERVER_H
#define BTREEINDEXERSERVER_H

#include <ir/index_manager/index/BTreeIndex.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class BTreeIndexerServerImpl;

class BTreeIndexerServer
{
public:
    BTreeIndexerServer(string port, BTreeIndexer* pBTreeIndexer);

    ~BTreeIndexerServer();

    void run();

private:
    ///we have to provide a class BTreeIndexerServerImpl because there exists macro definition confliction
    ///in febird/rpc/client.h and febird/rpc/server.h. Febird is reasonable because the general rpc based 
    ///application would have separated instances, however index-manager is going to be built into a single
    ///library which could serve for both client and server.
    BTreeIndexerServerImpl* pImpl_;
};

}

NS_IZENELIB_IR_END

#endif

