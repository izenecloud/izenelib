#include <cstdlib>
#include <ctime>
#include <string.h>

#include <boost/thread.hpp>

#include <3rdparty/zookeeper/ZooKeeper.hpp>
#include <3rdparty/zookeeper/ZooKeeperWatcher.hpp>
#include <3rdparty/zookeeper/ZooKeeperEvent.hpp>
#include <3rdparty/zookeeper/DoubleBarrier.h>
#include <3rdparty/zookeeper/CyclicBarrier.h>
#include <util/kv2string.h>

using namespace std;
using namespace boost;
using namespace izenelib::zookeeper;

typedef izenelib::util::kv2string ZkDataPack;

class Watcher : public ZooKeeperEventHandler
{
public:
    Watcher(ZooKeeper* zk, bool rewatch=true)
    : zk_(zk)
    , rewatch_(rewatch)
    {}

    virtual void process(ZooKeeperEvent& zkEvent)
    {
        cout << "[Watcher] "<< zkEvent.toString();
    }

    virtual void onNodeCreated(const std::string& path)
    {
        cout << "[Watcher] onNodeCreated " << path <<endl;
        if (rewatch_)
            zk_->isZNodeExists(path, ZooKeeper::WATCH);
    }
    virtual void onNodeDeleted(const std::string& path)
    {
        cout << "[Watcher] onNodeDeleted " << path <<endl;
        if (rewatch_)
            zk_->isZNodeExists(path, ZooKeeper::WATCH);
    }
    virtual void onDataChanged(const std::string& path)
    {
        cout << "[Watcher] onDataChanged " << path <<endl;
        if (rewatch_) {
            std::string data;
            zk_->getZNodeData(path, data, ZooKeeper::WATCH);
        }
    }

    virtual void onChildrenChanged(const std::string& path)
    {
        cout << "[Watcher] onChildrenChanged " << path <<endl;
        if (rewatch_) {
            std::vector<std::string> childrenList;
            zk_->getZNodeChildren(path, childrenList, ZooKeeper::WATCH);
        }
    }

    ZooKeeper* zk_;
    bool rewatch_;
};

void t_zk_client(string& hosts)
{
    int recvTimeout = 2000;
    ZooKeeper cli(hosts, recvTimeout);
    sleep(2);

    cli.isConnected();

    cli.deleteZNode("/testchildren", true);
    cli.createZNode("/testchildren");
}


void set_watchers(ZooKeeper& cli)
{
    // zoo_exists
    cli.isZNodeExists("/a", ZooKeeper::WATCH);
    cli.isZNodeExists("/a/b", ZooKeeper::WATCH);
    cli.isZNodeExists("/a/b/c", ZooKeeper::WATCH);

    // zoo_get
    std::string data;
    cli.getZNodeData("/a", data, ZooKeeper::WATCH);
    cli.getZNodeData("/a/b", data, ZooKeeper::WATCH);
    cli.getZNodeData("/a/b/c", data, ZooKeeper::WATCH);

    // zoo_get_children
    std::vector<std::string> childrenList;
    cli.getZNodeChildren("/a", childrenList, ZooKeeper::WATCH);
    cli.getZNodeChildren("/a/b", childrenList, ZooKeeper::WATCH);
    cli.getZNodeChildren("/a/b/c", childrenList, ZooKeeper::WATCH);
}

void t_zk_watch_event(string& hosts)
{
    int recvTimeout = 2000;
    ZooKeeper cli(hosts, recvTimeout);
    sleep(2);

    Watcher watcher(&cli);
    cli.registerEventHandler(&watcher);

    cli.deleteZNode("/a", true);
    cli.createZNode("/a");

    //*
    set_watchers(cli);

    while(true)
    {
        cout << "Press anykey to exit..." <<endl;
        std::string s;
        cin >> s;
        break;
    }
    //*/

    /* double watch just as watch once
    watcher.rewatch_ = false;
    cli.isZNodeExists("/a", ZooKeeper::WATCH);
    cli.isZNodeExists("/a", ZooKeeper::WATCH);
    std::string data;
    cli.getZNodeData("/a/b", data, ZooKeeper::WATCH);
    cli.getZNodeData("/a/b", data, ZooKeeper::WATCH);
    std::vector<std::string> childrenList;
    cli.getZNodeChildren("/a/b", childrenList, ZooKeeper::WATCH);
    cli.getZNodeChildren("/a/b", childrenList, ZooKeeper::WATCH);
    //*/

    char ch;
    cin >> ch;

    cli.deleteZNode("/a", true);
}

void t_zk_data_pack(string& hosts)
{
    ZooKeeper cli(hosts, 2000);
    sleep(2);

    ZkDataPack zdata;
    std::string sdata;

    zdata.setValue("host", "localhost");
    zdata.setValue("port", (unsigned int)18111);
    zdata.setValue("file", "");
    zdata.setValue("", "");
    zdata.setValue("file", "/test/hello world");
    sdata = zdata.serialize(true);
    cout <<"pack: "<<sdata <<endl;

    cli.createZNode("/testdata", sdata);
    cli.setZNodeData("/testdata", sdata);

    ZkDataPack zdata2;
    std::string sdata2;
    cli.getZNodeData("/testdata", sdata2);
    cout <<"unpack: "<<sdata2 <<endl;
    zdata2.loadKvString(sdata2, true);
}

void t_asyn_create(string& hosts)
{
    ZooKeeper cli(hosts, 2000);

    cli.createZNode("/syn_create", "hear me s");
    cli.createZNode("/asyn_create", "hear me a");

    while (!cli.isConnected())
        sleep(1);
}

void t_DoubleBarrier(string& hosts)
{
    cout << "enter process name:"<<endl;

    std::string name;
    std::cin >> name;
    DoubleBarrier barrier(hosts, "/barrier", 3, name);

    cout << "try to enter barrier.."<<endl;
    while (!barrier.enter())
    {
        sleep(1);
    }
    cout << "entered!"<<endl;

    srand(int(name[2]));
    int i = rand() % 10 + 1;
    sleep(i);
    cout << "slept "<<i<<" seconds in barrier."<<endl;

    cout << "try to leave barrier.."<<endl;
    while (!barrier.leave())
    {
        sleep(2);
    }
    cout << "left!" <<endl;
}

void t_CyclicBarrier(string& hosts)
{
    cout << "enter process name:"<<endl;

    std::string name;
    std::cin >> name;
    CyclicBarrier cbarrier(3, hosts, "/cyclic", name);

    cout << "try to lock.."<<endl;
    time_t t1 = time(NULL); cout << t1 << endl;
    cbarrier.await();
    time_t t2 = time(NULL); cout <<"finished "<<t2 << endl;

    //sleep(1);
    cbarrier.reset();
}

int main(int argc, char* argv[])
{
    string hosts = "127.0.0.1:2181"; //"172.16.0.161:2181,172.16.0.162:2181,172.16.0.163:2181";
    if (argc > 1)
        hosts = argv[1];

    //t_zk_client(hosts);

    //t_DoubleBarrier(hosts);

    //t_CyclicBarrier(hosts);

    //t_zk_watch_event(hosts);

    // t_zk_data_pack(hosts);

    //t_asyn_create(hosts);

    return 0;
}
