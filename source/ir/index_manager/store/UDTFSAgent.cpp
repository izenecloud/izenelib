#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread.hpp>

#include <ir/index_manager/store/UDTFSAgent.h>
#ifndef UDT3
#include <ir/index_manager/store/cc.h>
#endif

#include <ir/index_manager/index/Indexer.h>


using namespace izenelib::ir::indexmanager;

UDTFSAgent::UDTFSAgent(string port, Indexer* pIndexer)
    :pIndexer_(pIndexer)
{
#ifndef UDT3
    UDT::startup();
#endif

    serv_ = UDT::socket(AF_INET, SOCK_STREAM, 0);


#ifndef UDT3
    bool reuseaddr = false;
    UDT::setsockopt(serv_, 0, UDT_REUSEADDR, &reuseaddr, sizeof(bool));
    CCCVirtualFactory* pCCFactory = new CCCFactory<CUDPBlast>;
    UDT::setsockopt(serv_, 0, UDT_CC, pCCFactory , sizeof(CCCFactory<CUDPBlast>));
    delete pCCFactory;
#endif

#ifdef WIN32
    int mss = 1052;
    UDT::setsockopt(serv_, 0, UDT_MSS, &mss, sizeof(int));
#endif
    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(port.c_str()));
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8);

    if (UDT::ERROR == UDT::bind(serv_, (sockaddr*)&my_addr, sizeof(my_addr)))
    {
        cout << "bind: " << UDT::getlasterror().getErrorMessage() << endl;
        return;
    }
    UDT::listen(serv_, 1);

    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();

    home_ = pIndexer_->getIndexManagerConfig()->indexStrategy_.indexLocation_;

    pIndexMergerAgent_ = new IndexMergerAgent(pIndexer_);
}


UDTFSAgent::~UDTFSAgent()
{
    UDT::close(serv_);
#ifndef UDT3
    UDT::cleanup();
#endif
    delete pIndexMergerAgent_;
}


UDTSOCKET UDTFSAgent::accept(sockaddr* addr, int* addrlen)
{
#ifndef UDT3
    UDT::UDSET readfds;
	
    UD_ZERO(&readfds);
    UD_SET(serv_, &readfds);
	
    int res = UDT::select(1, &readfds, NULL, NULL, NULL);
	
    if ((res == UDT::ERROR)||(!UD_ISSET(serv_, &readfds)) )
        return UDT::INVALID_SOCK;
#endif

    return UDT::accept(serv_, addr, addrlen);
}

void UDTFSAgent::run()
{
    UDTSOCKET recver;
    sockaddr_in clientaddr;
    int addrlen = sizeof(sockaddr_in);

    while (true)
    {
        //if (UDT::INVALID_SOCK == (recver = UDT::accept(serv_, (sockaddr*)&clientaddr, &addrlen)))
        if(UDT::INVALID_SOCK == (recver = accept((sockaddr*)&clientaddr, &addrlen)))
        {
            cout << "accept error: " << UDT::getlasterror().getErrorMessage() << endl;
            continue;
        }
        else
        {
            boost::thread rcvthread(boost::bind(&UDTFSAgent::agentHandler, this, recver));
            //rcvthread.join();
            rcvthread.detach();
        }
    }
}

int UDTFSAgent::recv(UDTSOCKET recver, char* buf, int size)
{
   int rsize = 0;
   while (rsize < size)
   {
      int rs = UDT::recv(recver, buf + rsize, size - rsize, 0);
      if (UDT::ERROR == rs)
         return -1;

      rsize += rs;
   }
   return rsize;
}


int UDTFSAgent::recvMsg(UDTSOCKET recver, UDTFSMessage* msg)
{
    //if (UDT::ERROR == UDT::recv(recver, msg->pcBuffer_, msg->hdrSize_, 0))
    if (recv(recver, msg->pcBuffer_, msg->hdrSize_) < 0)
    {
        return -1;
    }

    int32_t dataLength = msg->getDataLength();
    if ( dataLength > 0)
    {
        //if (UDT::ERROR == UDT::recv(recver, msg->pcBuffer_ + msg->hdrSize_, dataLength, 0))
        if (recv(recver, msg->pcBuffer_ + msg->hdrSize_, dataLength) < 0)
        {
            cout << "recv cmd data error: " << UDT::getlasterror().getErrorMessage() << endl;
            return -1;
        }
    }
    return 0;
}

void UDTFSAgent::agentHandler(UDTSOCKET recver)
{
    string filename;

    UDTFSMessage msg;

    bool isBarrel = false;

    bool run = true;	

#ifndef UDT3
    boost::filesystem::fstream ofs;
#else
    boost::filesystem::ofstream ofs;
#endif
    while (run)
    {
        if (recvMsg(recver, &msg) < 0)
        {
            break;
        }

        switch (msg.getType())
        {
        case UDTFS_OPEN:
        {
            char* data = msg.getData();
            filename = data+1;
            char openmode = *data;
            string filetype;
            boost::filesystem::path path;			
            if(!strcasecmp(filename.c_str(),"barrels" ))
            {
                isBarrel = true;
                path = home_ + "/" + "barrels.tmp";
            }
            else if(filename.find("forward")!=string::npos)
            {
                path = home_ + "/" + filename;
            }
            else
            {
                size_t p = filename.rfind('.');
                if (p == string::npos)
                    assert(false);
                else
                    filetype = filename.substr(p , filename.length() - p);
                path = home_ + "/" + pIndexMergerAgent_->getCurrentBarrelName()+filetype;
            }
            cout<<"open "<<path<<endl;
            if(openmode == FILEOPEN_MODE_TRUNK)
                ofs.open(path, ios::out | ios::binary | ios::trunc);
            else if(openmode == FILEOPEN_MODE_APPEND)
                ofs.open(path, ios::out | ios::binary | ios::app);
            else if(openmode == FILEOPEN_MODE_MODIFY)
                ofs.open(path, ios::out | ios::binary | ios::in |ios::ate);
	     else
                ofs.open(path, ios::out | ios::binary | ios::trunc); //trunk by default, error information received here

            continue;
        }
        case UDTFS_WRITE:
        {
            int64_t offset = * (int64_t *)(msg.getData());
            int64_t size =* (int64_t *)(msg.getData() + sizeof(int64_t));
            if (UDT::ERROR == UDT::recvfile(recver, ofs, offset, size))
            {
                cout << "recv file data error: "<<recver<<" "<<filename << " " << UDT::getlasterror().getErrorMessage() << endl;
                run = false;
                break;
            }
            continue;
        }
        case UDTFS_UPLOAD:
        {
            int64_t size =* (int64_t *)(msg.getData());
            if (UDT::ERROR == UDT::recvfile(recver, ofs, 0, size))
            {
                cout << "recvfile error: "<<filename<<" "<< UDT::getlasterror().getErrorMessage() << endl;
                run = false;
                break;
            }
            continue;
        }
        case UDTFS_CLOSE:
        {
            ofs.close();
            cout<<"close "<<recver<<" "<<filename<<endl;			
            run = false;
            break;
        }
        }

    }

    UDT::close(recver);

    if(isBarrel)
    {
        pIndexMergerAgent_->mergeBarrelsInfo();
        pIndexMergerAgent_->triggerMerge();
    }

}

