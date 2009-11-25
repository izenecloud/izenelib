/**
* @file        BTreeIndexerClient.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief BTreeIndexerClient is used to flush BTree index remotely
*/
#ifndef BTREEINDEXERCLIENT_H
#define BTREEINDEXERCLIENT_H

#include <ir/index_manager/index/BTreeIndex.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class BTreeIndexerClientImpl;

class BTreeIndexerClient:public BTreeIndexerInterface
{
public:
    BTreeIndexerClient();

    ~BTreeIndexerClient();	

    void switchServer(string ip, string port);

    void add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid);

    void remove(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid){}
private:
    ///we have to provide a class BTreeIndexerClientImpl because there exists macro definition confliction
    ///in febird/rpc/client.h and febird/rpc/server.h. Febird is reasonable because the general rpc based 
    ///application would have separated instances, however index-manager is going to be built into a single
    ///library which could serve for both client and server.
    BTreeIndexerClientImpl* pImpl_;
};

}

NS_IZENELIB_IR_END

#endif
