#ifndef WIN32
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <string>
#include <udt/udt.h>

using namespace std;

class SimpleFTPClient
{
public:
    SimpleFTPClient(string server_ip,string server_port)
    {
        fhandle_ = UDT::socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(short(atoi(server_port.c_str())));
#ifndef WIN32
        if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0)
#else
        if (INADDR_NONE == (serv_addr.sin_addr.s_addr = inet_addr(server_ip.c_str())))
#endif
        {
            cout << "incorrect network address.\n";
            //return 0;
        }
        memset(&(serv_addr.sin_zero), '\0', 8);

        if (UDT::ERROR == UDT::connect(fhandle_, (sockaddr*)&serv_addr, sizeof(serv_addr)))
        {
            cout << "connect: " << UDT::getlasterror().getErrorMessage() << endl;
            //return 0;
        }

    }

    ~SimpleFTPClient()
    {
        UDT::close(fhandle_);
        UDT::cleanup();
    }

    int upload(string path)
    {
        size_t p = path.rfind('/');
        string filename;
        if (p == string::npos)
            return -1;
        else
        {
            filename = path.substr(p + 1, path.length() - p);
            cout<<"uploading "<<filename<<endl;
        }


        int64_t size;
        fstream ifs;
        ifs.open(path.c_str(), ios::in | ios::binary);

        if (ifs.fail() || ifs.bad())
            return 0;

        ifs.seekg(0, ios::end);
        size = ifs.tellg();
        ifs.seekg(0);

        int len = filename.size();
        // send filename length
        if (UDT::ERROR == UDT::send(fhandle_, (char*)&len, sizeof(int), 0))
        {
            cout << "send: "<<" " << UDT::getlasterror().getErrorMessage() << endl;
            return 0;
        }
        // send filename
        if (UDT::ERROR == UDT::send(fhandle_, filename.c_str(), len, 0))
        {
            cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
            return 0;
        }
        // send file length
        if (UDT::ERROR == UDT::send(fhandle_, (char*)&size, sizeof(int64_t), 0))
        {
            cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
            return 0;
        }

        // send the file
        if (UDT::ERROR == UDT::sendfile(fhandle_, ifs, 0, size))
        {
            cout << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;
            return 0;
        }
        return 1;
    }


public:
    static void init()
    {
        UDT::startup();
    }

    static void close()
    {
        // use this function to release the UDT library
        //UDT::cleanup();
    }


private:
    UDTSOCKET fhandle_;
};


int main(int argc, char* argv[])
{
    //SimpleFTPClient::init();
    UDT::startup();

    SimpleFTPClient client(argv[1],argv[2]);

    client.upload(argv[3]);

    SimpleFTPClient::close();
    return 1;
}
