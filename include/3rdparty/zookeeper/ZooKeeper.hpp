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
class ZooKeeperEventHandler;

/**
 * ZooKeeper Client
 */
class ZooKeeper
{
public:
    enum ZNodeCreateType
    {
       ZNODE_NORMAL = 0,
       ZNODE_EPHEMERAL = 1 << 0, // equal to ZOO_EPHEMERAL,
                                 // ephemeral znode, not allowed to have children.
       ZNODE_SEQUENCE = 1 << 1,  // equal to ZOO_SEQUENCE,
                                 // sequence znode, will be appended a monotonically increasing counter to the end of path.
                                 // i.e., creating "/node" mutliple times, will get "/node0000000001", "/node0000000002" ..
    };

    enum ZNodeWatchType
    {
        NOT_WATCH = 0,
        WATCH = 1
    };

public:
    /**
     * @param host comma separated host:port pairs, each corresponding to a zk server.
     *             e.g. "127.0.0.1:3000,127.0.0.1:3001,127.0.0.1:3002"
     * @param recvTimeout
     */
    ZooKeeper(const std::string& hosts, const int recvTimeout);

    ~ZooKeeper();

    /**
     * Register EventHandler which will be notified when watched a event.
     * @param evtHandler
     */
    void registerEventHandler(ZooKeeperEventHandler* evtHandler);

    /**
     * @return true if this client has been connected to Zookeeper server, or false.
     */
    bool isConnected();

    /**
     * Set debug level
     * @param logLevel see enum {ZOO_LOG_LEVEL_ERROR=1,ZOO_LOG_LEVEL_WARN=2,ZOO_LOG_LEVEL_INFO=3,ZOO_LOG_LEVEL_DEBUG=4}
     */
    static void setDebugLevel(ZooLogLevel logLevel);

    /**
     * Set log output stream
     * @param logFile
     */
    static void setLogFile(const std::string& logFile);

public:
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
    bool createZNode(const std::string &path, const std::string &data="", ZNodeCreateType flags = ZNODE_NORMAL);

    /// Note: this function may be available for the last call of createZNode with flags = ZNODE_SEQUENCE
    std::string getLastCreatedNodePath();

    /**
     * Delete a znode
     * @param path path for the znode.
     * @param recursive Whether delete recursively if the znode has children
     * @param version the expected version of the node. If -1 is used the version check will not take place.
     * @return true if success, or false.
     */
    bool deleteZNode(const std::string &path, bool recursive = false, int version = -1);

    /**
     * Whether a znode exists
     * @param path path for the znode.
     * @param watch If nonzero, a watch will be set at the server to notify the client if the node changes.
     * @return true if exists, or false.
     */
    bool isZNodeExists(const std::string &path, ZNodeWatchType watch = NOT_WATCH);

    /**
     * Get data of a znode
     * @param path path for the znode
     * @param data[OUT] return data of the znode
     * @param watch If nonzero, a watch will be set at the server to notify the client if the node changes.
     * @return true if success, or false;
     */
    bool getZNodeData(const std::string &path, std::string& data, ZNodeWatchType watch = NOT_WATCH);

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
     * @param childrenList[OUT] return children of the znode
     * @param watch If nonzero, a watch will be set at the server to notify the client if the node changes.
     * @param inAbsPath  Whether get children in absolute path or relative path
     */
    void getZNodeChildren(
            const std::string &path, std::vector<std::string>& childrenList,
            ZNodeWatchType watch = NOT_WATCH,
            bool inAbsPath = true);

    /**
     * @}
     */

public:
    /**
     * Asynchronous API
     * @{
     */

    // TODO

    /**
     * @}
     */

    /**
     * Show ZooKeeper's Hierarchical Namespace
     * @param path  the path of the root znode
     * @param level current level of znode in znodes' tree (top-to-bottom)
     */
    void showZKNamespace(const std::string& path = "/", int level = 0, std::ostream& out = std::cout);



private:
    std::string hosts_;
    int recvTimeout_;
    clientid_t* sessionId_; //pass 0 if not reconnecting to a previous session.
    void* context_;
    int flags_;            //reserved, set to 0.

    zhandle_t* zk_;

    static const int MAX_PATH_LENGTH = 1024;
    char realNodePath_[MAX_PATH_LENGTH];

    static const int MAX_DATA_LENGTH = 128 * 1024;
    char buffer_[MAX_DATA_LENGTH];
};


}

#endif /* ZOO_KEEPER_HPP_ */
