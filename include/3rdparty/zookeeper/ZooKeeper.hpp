/**
 * @file ZooKeeper.hpp
 * @author Zhongxia Li
 * @date Sep 7, 2011
 * @brief ZooKeeper is a wrapper of zookeeper c client library.
 */
#ifndef ZOO_KEEPER_HPP_
#define ZOO_KEEPER_HPP_

#include "zookeeper.h"

#include <iostream>
#include <string>
#include <vector>

namespace zookeeper {

class ZooKeeperWatcher;

/**
 * ZooKeeper Client
 */
class ZooKeeper
{
public:
    /**
     * @param host comma separated host:port pairs, each corresponding to a zk server.
     *             e.g. "127.0.0.1:3000,127.0.0.1:3001,127.0.0.1:3002"
     * @param recvTimeout
     * @param watcher reference to the watcher object for the znode.
     */
    ZooKeeper(const std::string& hosts, const int recvTimeout, void* watcher = NULL);

    ~ZooKeeper();

    /**
     * Set watcher for a client (only one watcher can be set at a time)
     * @param path path for the znode
     * @param zkWatcher reference to the watcher object for the znode.
     */
    void setZNodeWatcher(const std::string &path, ZooKeeperWatcher* zkWatcher);

    /**
     * @return true if this client has been connected to Zookeeper server, or false.
     */
    bool isConnected();

    /**
     * Synchronous API
     * @{
     */

    bool connect();

    void disconnect();

    /**
     * Create a znode
     * @param path path for the znode.
     * @param value data for the znode.
     * @param flags this parameter can be set to 0 for normal create or an OR of the Create Flags..xxx
     * @return true if success, or false.
     */
    bool createZNode(const std::string &path, const std::string &data, int flags = 0);

    /**
     * Delete a znode
     * @param path path for the znode.
     * @param version the expected version of the node. If -1 is used the version check will not take place.
     * @return true if success, or false.
     */
    bool deleteZNode(const std::string &path, int version = -1);

    /**
     * Whether a znode exists
     * @param path path for the znode.
     * @return true if exists, or false.
     */
    bool isZNodeExists(const std::string &path);

    /**
     * Get data of a znode
     * @param path path for the znode
     * @param data[OUT] return data of the znode
     * @return true if success, or false;
     */
    bool getZNodeData(const std::string &path, std::string& data);

    /**
     * Set data for a znode
     * @param path path for the znode
     * @param data data for the znode
     * @return true if success, or false;
     */
    bool setZNodeData(const std::string &path, const std::string& data, int version = -1);


    /**
     * Get children of a znode
     * @param path path of the znode
     * @param childrens[OUT] return childrens of the znode
     */
    void getZNodeChildren(const std::string &path, std::vector<std::string>& childrens);

    /**
     * @}
     */

    /**
     * Asynchronous API
     * @{
     */

    // TODO

    /**
     * @}
     */


private:
    std::string hosts_;
    int recvTimeout_;
    clientid_t* sessionId_; //pass 0 if not reconnecting to a previous session.
    void* context_;
    int flags_;            //reserved, set to 0.

    zhandle_t* zk_;

    static const int MAX_PATH_LENGTH = 1024;
    char newNodePath_[MAX_PATH_LENGTH];

    static const int MAX_DATA_LENGTH = 128 * 1024;
    char buffer_[MAX_DATA_LENGTH];
};


}

#endif /* ZOO_KEEPER_HPP_ */
