#include <cstdlib>
#include <ctime>
#include <string.h>

#include <boost/thread.hpp>

#include <3rdparty/zookeeper/ZooKeeper.hpp>
#include <3rdparty/zookeeper/ZooKeeperWatcher.hpp>
#include <3rdparty/zookeeper/ZooKeeperEvent.hpp>
#include <3rdparty/zookeeper/ZkDataPack.hpp>

#include <3rdparty/zookeeper/DoubleBarrier.h>
#include <3rdparty/zookeeper/CyclicBarrier.h>

using namespace std;
using namespace boost;
using namespace zookeeper;

class Watcher : public ZooKeeperEventHandler
{
public:
    Watcher() : childChanged_(false) {}

    virtual void process(ZooKeeperEvent& zkEvent)
    {
        cout << "[Watcher] "<< zkEvent.toString()<<endl;
    }

    virtual void onChildrenChanged(const std::string& path)
    {
        cout << "[Watcher] onChildrenChanged " << path <<endl;
        childChanged_ = true;
    }

    bool childChanged_;
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

void t_zk_watch_event(string& hosts)
{
    int recvTimeout = 2000;
    ZooKeeper cli(hosts, recvTimeout);
    sleep(2);

    cli.deleteZNode("/testchildren", true);
    cli.createZNode("/testchildren");

    Watcher watcher;
    cli.registerEventHandler(&watcher);

    std::vector<std::string> childrenList;
    cli.getZNodeChildren("/testchildren", childrenList, ZooKeeper::WATCH);
    cout << "Getting children of \"/testchildren\" "<<childrenList.size()<<" , and watching changes"<<endl;

    while(true)
    {
        sleep(1);
        if (watcher.childChanged_)
        {
            // watch again, notify once for one watch...
            childrenList.clear();
            cli.getZNodeChildren("/testchildren", childrenList, ZooKeeper::WATCH);
            watcher.childChanged_ = false;
            cout << "Getting children of \"/testchildren\" "<<childrenList.size()<<" , and watching changes"<<endl;
        }

    }
}

void t_zk_data_pack(string& hosts)
{
    ZooKeeper cli(hosts, 2000);
    sleep(2);

    ZkDataPack zdata;
    std::string sdata;

    zdata.setValue("host", "localhost");
    zdata.setValue("port", 18111);
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
    zdata2.loadZkData(sdata2, true);
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

int main(int argv, char* argc[])
{
    string hosts = "127.0.0.1:2181";
    hosts = "172.16.0.161:2181,172.16.0.162:2181,172.16.0.163:2181";

    if (argv >= 3)
    {
        if ((strcasecmp(argc[1], "delete") == 0))
        {
            ZooKeeper cli(hosts, 2000);
            if (cli.deleteZNode(argc[2], true));
        }
    }

    //t_zk_client(hosts);

    //t_DoubleBarrier(hosts);

    //t_CyclicBarrier(hosts);

    //t_zk_watch_event(hosts);

    // t_zk_data_pack(hosts);

    //t_asyn_create(hosts);

    return 0;
}
