#include <3rdparty/zookeeper/ZooKeeper.hpp>
#include <3rdparty/zookeeper/ZooKeeperWatcher.hpp>
#include <3rdparty/zookeeper/ZooKeeperEvent.hpp>

#include <iostream>
#include <sstream>
#include "string.h"

using namespace std;
using namespace izenelib::zookeeper;

void printUsage(char* argv0)
{
    cout << "Usage: " << argv0 << " host:port[,host:port]" <<endl;
    cout << "   help                "<<endl;
    cout << "   ls path             "<<endl;
    cout << "   quit|exit           "<<endl;
    cout << "   create [-s] [-e] path data"<<endl;
    cout << "   set path data       "<<endl;
    cout << "   get path            "<<endl;
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

        bool res = true;

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
        // ls
        else if (strcasecmp(command.c_str(), "ls") == 0)
        {
            std::string path;
            iss >> path;
            zk.showZKNamespace(path);
        }
        // create
        else if (strcasecmp(command.c_str(), "create") == 0)
        {
            std::string option;
            std::string path;
            std::string data;

            int flag = ZooKeeper::ZNODE_NORMAL;
            while (true)
            {
                iss >> option;
                if (strcasecmp(option.c_str(), "-s") == 0)
                {
                    flag |= ZooKeeper::ZNODE_SEQUENCE;
                }
                else if (strcasecmp(option.c_str(), "-e") == 0)
                {
                    flag |= ZooKeeper::ZNODE_EPHEMERAL;
                }
                else
                {
                    // assume is path
                    break;
                }
            }

            path = option;
            iss >> data;

            res = zk.createZNode(path, data, (ZooKeeper::ZNodeCreateType)flag);
        }
        // get
        else if (strcasecmp(command.c_str(), "set") == 0)
        {
            std::string path;
            std::string data;
            iss >> path >> data;

            res = zk.setZNodeData(path, data);
        }
        // get
        else if (strcasecmp(command.c_str(), "get") == 0)
        {
            std::string path;
            iss >> path;

            std::string data;
            res = zk.getZNodeData(path, data);

            if (res)
                std::cout << data <<endl;
        }
        // delete
        else if (strcasecmp(command.c_str(), "delete") == 0)
        {
            std::string path;
            std::string recur;
            iss >> path >> recur;

            if (strcasecmp(recur.c_str(), "true") == 0)
                res = zk.deleteZNode(path, true);
            else
                res = zk.deleteZNode(path, false);
        }
        else
        {
            printUsage(argv[0]);
        }

        // result for executing command
        if (!res)
        {
            cout <<"Failed to "<<command<<"!  "<<zk.getErrorString()<<endl;
        }
    }

    return 0;
}
