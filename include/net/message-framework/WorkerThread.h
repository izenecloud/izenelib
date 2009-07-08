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

#include "ServiceItem.h"
//#include "ServiceHandler.h"

#include <net/message_framework.h>
#include <wiselib/thread-pool/ThreadObject.h>


/// @brief This class is a work thread of all the manager.
/// And each work thread servers only one request.
template <typename ServiceHandler>
class WorkerThread : public wiselib::thread_pool::ThreadObject
{
    private:
        /// @brief the pointer of index service handler object.
        ServiceHandler* serviceHandlerPtr_;

        /// @brief the information of request which worker thread should deal with.
        ServiceRequestInfoPtr requestInfo_;

        /// @brief the pointer of message server which transfer the result data.
        MessageServer* messageServerPtr_;

        /// @brief service item which contains service information.
        ServiceItem<ServiceHandler> serviceItem_;

    public:
        WorkerThread(void)
            : serviceHandlerPtr_(NULL), messageServerPtr_(NULL)
        {}

        /// @brief an interface which sets the service handler object.
        void setServiceHandler( ServiceHandler* handler )
        {
            serviceHandlerPtr_ = handler;
        } 

        /// @brief an interface which sets the request info object.
        void setRequestInfo( const ServiceRequestInfoPtr& request )
        {
            requestInfo_ = request;
        }

        /// @brief an interface which sets the service item object.
        void setServiceItem( const ServiceItem<ServiceHandler> & serviceItem )
        {
            serviceItem_ = serviceItem;
        }

        /// @brief an interface which sets the message server object.
        void setMessageServer( MessageServer* messageServer )
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
        if( !(serviceHandlerPtr_->*(serviceItem_.callback_))(*messageServerPtr_, requestInfo_) )
        {
#ifdef SF1_DEBUG
            std::cout << "Error while processing service : " << requestInfo_.getServiceName() << std::endl;
#endif 
        }
    }
    catch(...)
    {
        return;
    }
} 




#endif  //_WORKER_THREAD_H_

//eof
