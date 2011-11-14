#include <3rdparty/zookeeper/ZooKeeper.hpp>
#include <3rdparty/zookeeper/ZooKeeperWatcher.hpp>
#include <3rdparty/zookeeper/ZooKeeperEvent.hpp>
#include <3rdparty/zookeeper/ZkDataPack.hpp>

#include <iostream>
#include <sstream>
#include "string.h"

using namespace std;
using namespace zookeeper;

void printUsage(char* argv0)
{
    cout << "Usage: " << argv0 << " host:port[,host:port]" <<endl;
    cout << "   help                "<<endl;
    cout << "   show path           "<<endl;
    cout << "   quit|exit           "<<endl;
    cout << "   delete path [true|false]        option: whether delete recursively" <<endl;
    cout <<endl;
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 0;
    }

    // connect to zookeeper
    string hosts = argv[1];
    cout << "Connecting to ZooKeeper..." <<endl;
    ZooKeeper zk(hosts, 2000, true);

    while (true)
    {
        if (!zk.isConnected())
        {
            cout << "Connecting to ZooKeeper..." <<endl;
            zk.connect(true);
        }

        cout<<"[zk: "<<hosts<<"("<<zk.getStateString()<<")] ";

        static char buffer[1024];
        cin.getline(buffer, 1024);

        istringstream iss(buffer);
        std::string command;
        iss >> command;
        //cout<<"command: "<<command<<endl;

        // help
        if (strcasecmp(command.c_str(), "help") == 0)
        {
            printUsage(argv[0]);
        }
        // quit|exit
        else if ((strcasecmp(command.c_str(), "quit") == 0)
                || (strcasecmp(command.c_str(), "exit") == 0))
        {
            break;
        }
        // show
        else if (strcasecmp(command.c_str(), "show") == 0)
        {
            std::string path;
            iss >> path;
            zk.showZKNamespace(path);
        }
        // delete
        else if (strcasecmp(command.c_str(), "delete") == 0)
        {
            std::string path;
            std::string recur;
            iss >> path >> recur;

            bool ret;
            if (strcasecmp(recur.c_str(), "true") == 0)
                ret = zk.deleteZNode(path, true);
            else
                ret = zk.deleteZNode(path, false);

            if (!ret)
            {
                cout <<"Failed to delete!  "<<zk.getErrorString()<<endl;
            }
        }
        // ...
        else
        {
            printUsage(argv[0]);
        }
    }

    return 0;
}
