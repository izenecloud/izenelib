#ifndef WIN32
#include <cstdlib>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <fstream>
#include <iostream>
#include <string>
#include <udt/udt.h>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace std;

class SimpleFTPServer
{
public:
    SimpleFTPServer(string port)
            :home_("./")
    {
        serv_ = UDT::socket(AF_INET, SOCK_STREAM, 0);

#ifdef WIN32
        int mss = 1052;
        UDT::setsockopt(serv, 0, UDT_MSS, &mss, sizeof(int));
#endif
        sockaddr_in my_addr;
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(atoi(port.c_str()));
        my_addr.sin_addr.s_addr = INADDR_ANY;
        memset(&(my_addr.sin_zero), '\0', 8);

        if (UDT::ERROR == UDT::bind(serv_, (sockaddr*)&my_addr, sizeof(my_addr)))
        {
            cout << "bind: " << UDT::getlasterror().getErrorMessage() << endl;
            //return 0;
        }

        cout << "server is ready at port: " << port << endl;

        UDT::listen(serv_, 1);
    }

    ~SimpleFTPServer()
    {
        UDT::close(serv_);

        // use this function to release the UDT library
        UDT::cleanup();
    }

public:
    void run()
    {
        UDTSOCKET recver;
        sockaddr_storage clientaddr;
        int addrlen = sizeof(clientaddr);
        while (true)
        {
            if (UDT::INVALID_SOCK == (recver = UDT::accept(serv_, (sockaddr*)&clientaddr, &addrlen)))
            {
                cout << "accept: " << UDT::getlasterror().getErrorMessage() << endl;
                return;
            }
		cout<<"accepted "<<recver<<endl;

           boost::thread rcvthread(boost::bind(&SimpleFTPServer::recvdata, this, recver));
           rcvthread.join();
        }

    }

    void* recvdata(UDTSOCKET recver)
    {

//        while (true)
        {
            char file[1024];
            int len;

            // get len of file name

            if (UDT::ERROR == UDT::recv(recver, (char*)&len, sizeof(int), 0))
            {
                cout << "recv1: " << UDT::getlasterror().getErrorMessage() << endl;
                //break;
                return NULL;
            }

            // get file name
            if (UDT::ERROR == UDT::recv(recver, file, len, 0))
            {
                cout << "recv2: " << UDT::getlasterror().getErrorMessage() << endl;
                //break;
                return NULL;                
            }
            file[len] = '\0';

            // get size information
            int64_t size;

            if (UDT::ERROR == UDT::recv(recver, (char*)&size, sizeof(int64_t), 0))
            {
                cout << "send: " << UDT::getlasterror().getErrorMessage() << endl;
                //break;
                return NULL;
            }

            string path("./tmp/");
            path += file;
            ofstream ofs(path.c_str(), ios::out | ios::binary | ios::trunc);
            int64_t recvsize;

            if (UDT::ERROR == (recvsize = UDT::recvfile(recver, ofs, 0, size)))
            {
                cout << "recvfile: " << UDT::getlasterror().getErrorMessage() << endl;
                //break;
                return NULL;
            }

            ofs.close();
        }


        UDT::close(recver);

#ifndef WIN32
        return NULL;
#else
        return 0;
#endif
    }

private:
    UDTSOCKET serv_;
    string home_;
};


int main(int argc, char* argv[])
{
    SimpleFTPServer server(argv[1]);
    server.run();
    return 1;
}
