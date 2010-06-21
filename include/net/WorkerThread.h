/**
 * @file    WorkerThread.h
 * @brief   Defines WorkerThread class
 * @author  MyungHyun Lee (Kent). (originally by Quang)
 * @date    2009-03-24
 * @details
 *  - Log
 */

#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include <net/message_framework.h>
#include <net/ServiceItem.h>
#include <util/thread-pool/ThreadObject.h>

#include <exception>

namespace messageframework{

/// @brief This class is a work thread of all the manager.
/// And each work thread servers only one request.
template <typename ServiceHandler>
class WorkerThread : public izenelib::util::thread_pool::ThreadObject
{
	typedef typename boost::shared_ptr<ServiceHandler> ServiceHandlerPtr;
    private:
        /// @brief the pointer of index service handler object.
    	ServiceHandlerPtr serviceHandlerPtr_;

        /// @brief the information of request which worker thread should deal with.
        ServiceRequestInfoPtr requestInfo_;

        /// @brief the pointer of message server which transfer the result data.
        MessageServerPtr messageServerPtr_;

        /// @brief service item which contains service information.
        ServiceItem<ServiceHandler> serviceItem_;

    public:
        WorkerThread(void)            
        {}

        /// @brief an interface which sets the service handler object.
        void setServiceHandler( ServiceHandlerPtr handler )
        {
            serviceHandlerPtr_ = handler;
        } 

        /// @brief an interface which sets the request info object.
        void setRequestInfo( const ServiceRequestInfoPtr& request )
        {
            requestInfo_ = request;
        }

        /// @brief an interface which sets the service item object.
        void setServiceItem( const ServiceItem<ServiceHandler>& serviceItem )
        {
            serviceItem_ = serviceItem;
        }

        /// @brief an interface which sets the message server object.
        void setMessageServer(const MessageServerPtr messageServer )
        {
            messageServerPtr_ = messageServer;
        }

    private:
        void start(void);
}; // end - class WorkerThread


template <typename ServiceHandler>
void WorkerThread<ServiceHandler>::start(void)
{
    try
    {  
    	if( !( serviceHandlerPtr_.get()->*(serviceItem_.callback_))(*messageServerPtr_, requestInfo_) )
        {

            DLOG(ERROR) << "Error while processing service : " << requestInfo_->getServiceName() << std::endl; 
        }  
    }
    catch(std::exception& e)
    {
        DLOG(ERROR) << "Exception while processing service : " << requestInfo_->getServiceName() << std::endl;
        DLOG(ERROR) << "  Detail: " << e.what() << std::endl;
    }
    catch(...)
    {
        DLOG(ERROR) << "Exception while processing service : " << requestInfo_->getServiceName() << std::endl;
        return;
    }
}


}


#endif  //_WORKER_THREAD_H_

//eof
