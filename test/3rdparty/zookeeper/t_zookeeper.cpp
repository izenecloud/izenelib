#include <boost/test/unit_test.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <iostream>
#include <sstream>

#include <3rdparty/zookeeper/ZooKeeper.hpp>
#include <3rdparty/zookeeper/ZooKeeperWatcher.hpp>
#include <3rdparty/zookeeper/ZooKeeperEvent.hpp>

using namespace std;
using namespace boost;
using namespace boost::lambda;
using namespace izenelib::zookeeper;


class WorkerSearch : public ZooKeeperEventHandler
{
public:
    virtual void onSessionConnected()
    {
    }

    virtual void onNodeCreated(const std::string& path)
    {
        cout << "[WorkerSearch] onNodeCreated " << path <<endl;
        path_ = path;
    }

    virtual void onNodeDeleted(const std::string& path)
    {
        cout << "[WorkerSearch] onNodeDeleted " << path <<endl;
    }

    virtual void onDataChanged(const std::string& path)
    {
        cout << "[WorkerSearch] onDataChanged " << path <<endl;
        path_ = path;
    }

    std::string path_;
};

class WorkerMining : public ZooKeeperEventHandler
{
public:
    virtual void onSessionConnected()
    {
    }

    virtual void onNodeCreated(const std::string& path)
    {
        cout << "[WorkerMining] onNodeCreated " << path <<endl;
    }

    virtual void onNodeDeleted(const std::string& path)
    {
        cout << "[WorkerMining] onNodeDeleted " << path <<endl;
    }

    virtual void onDataChanged(const std::string& path)
    {
        cout << "[WorkerMining] onDataChanged " << path <<endl;
    }
};

static const std::string gHosts = "localhost:2181"; //"127.16.0.161:2181,127.16.0.162:2181,127.16.0.163:2181";
//#define ENABLE_ZK_TEST

BOOST_AUTO_TEST_SUITE( t_zookeeper )

BOOST_AUTO_TEST_CASE( check_zookeeper_service )
{
    std::cout << "---> Note: start ZooKeeper Service firstly before test." << std::endl;
    std::cout << "     ZooKeeper Service: "<< gHosts << std::endl;
}

BOOST_AUTO_TEST_CASE( zookeeper_client_basic )
{
    std::cout << "---> Test ZooKeeper Client basic functions" << std::endl;

#ifndef ENABLE_ZK_TEST
    return;
#endif

    std::string hosts = gHosts;
    int recvTimeout = 3000;

    // Zookeeper Client
    ZooKeeper cli(hosts, recvTimeout);
    sleep(2);

    if (!cli.isConnected())
        return;

    // remove all
    cli.deleteZNode("/SF1", true);

    // create
    std::string path = "/SF1";
    std::string data = "distributed search";
    cli.createZNode(path, data, ZooKeeper::ZNODE_NORMAL);
    BOOST_CHECK_EQUAL(cli.isZNodeExists(path), true);
    BOOST_CHECK_EQUAL(cli.createZNode(path, data, ZooKeeper::ZNODE_NORMAL), false);

    // create ephemeral node
    if (false) // disable
    {
        ZooKeeper tmpCli(hosts, recvTimeout);
        tmpCli.createZNode("/SF1/ephemeral", "", ZooKeeper::ZNODE_EPHEMERAL);
        BOOST_CHECK_EQUAL(tmpCli.isZNodeExists("/SF1/ephemeral"), true);
    }
    //tmpCli exited...
    sleep(1);
    BOOST_CHECK_EQUAL(cli.isZNodeExists("/SF1/ephemeral"), false);

    // create sequence node
    cli.createZNode("/SF1/sequence", "", ZooKeeper::ZNODE_SEQUENCE);
    string s1 = cli.getLastCreatedNodePath();
    BOOST_CHECK_EQUAL(cli.isZNodeExists(s1), true);
    cli.createZNode("/SF1/sequence", "", ZooKeeper::ZNODE_SEQUENCE);
    string s2 = cli.getLastCreatedNodePath();
    BOOST_CHECK_EQUAL(cli.isZNodeExists(s2), true);
    cli.deleteZNode(s1);
    cli.deleteZNode(s2);

    // get
    std::string data_get;
    cli.getZNodeData(path, data_get);
    BOOST_CHECK_EQUAL(data_get, data);

    // set
    std::string data2 = "distributed search (sf1-kite)";
    BOOST_CHECK_EQUAL(cli.setZNodeData(path, data2), true);
    cli.getZNodeData(path, data_get);
    BOOST_CHECK_EQUAL(data_get, data2);

    // children
    std::string master = "/SF1/Master";
    std::string worker1 = "/SF1/Worker1";
    std::string worker2 = "/SF1/Worker2";
    BOOST_CHECK_EQUAL(cli.createZNode(master, "this is master node"), true);
    BOOST_CHECK_EQUAL(cli.createZNode(worker1, "remote worker1"), true);
    BOOST_CHECK_EQUAL(cli.createZNode(worker2, "remote worker2"), true);
    std::vector<std::string> children;
    cli.getZNodeChildren("/SF1", children);
    BOOST_CHECK_EQUAL(children.size(), 3);
    BOOST_CHECK_EQUAL(children[0], master);
    BOOST_CHECK_EQUAL(children[1], worker1);
    BOOST_CHECK_EQUAL(children[2], worker2);

    // display
    //cli.showZKNamespace("/SF1");
}

BOOST_AUTO_TEST_CASE( zookeeper_watch )
{
    std::cout << "---> Test ZooKeeper Watcher" << std::endl;

#ifndef ENABLE_ZK_TEST
    return;
#endif

    // Client
    std::string hosts = gHosts;
    int recvTimeout = 2000;
    ZooKeeper cli(hosts, recvTimeout);
    sleep(1);

    if (!cli.isConnected())
        return;

    // set event handlers for watcher
    WorkerSearch wkSearch;
    WorkerMining wkMining;
    cli.registerEventHandler(&wkSearch);
    cli.registerEventHandler(&wkMining);

    // 1. get and watch znode for changes
    std::string path = "/SF1/Master";
    std::string data_get;
    cli.getZNodeData(path, data_get, ZooKeeper::WATCH);
    BOOST_CHECK_EQUAL(data_get, "this is master node"); // set in former test case

    cli.setZNodeData(path, "master data changed!");
    sleep(1); //ensure watcher notified

    // master was notified by watcher on znode changed
    BOOST_CHECK_EQUAL(wkSearch.path_, path);
    cli.getZNodeData(wkSearch.path_, data_get);
    BOOST_CHECK_EQUAL(data_get, "master data changed!");

    // 2. check exists and watch znode for creation
    std::string path2 = "/NotExistedNode";
    cli.deleteZNode(path2, true);
    BOOST_CHECK_EQUAL(cli.isZNodeExists(path2, ZooKeeper::WATCH), false);

    cli.createZNode(path2, "nodata");
    sleep(1); //ensure watcher notified

    // master was notified by watcher on znode created
    BOOST_CHECK_EQUAL(wkSearch.path_, path2);
    cli.getZNodeData(wkSearch.path_, data_get);
    BOOST_CHECK_EQUAL(data_get, "nodata");

    // clear test data from zookeeper servers
    cli.deleteZNode(path2, true);
    cli.deleteZNode("/SF1", true);
}

BOOST_AUTO_TEST_SUITE_END()
