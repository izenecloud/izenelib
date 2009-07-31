/**
 * @file    MessageControllerLight.h
 * @brief
 * @author  MyungHyun Lee (Kent), Rewritten by Wei Cao 2009-3-29
 * @date    2008-11-18
 */


#ifndef _MESSAGE_CONTROLLER_LIGHT_H_
#define _MESSAGE_CONTROLLER_LIGHT_H_

#include <net/message-framework/MFSingleton.h>

#include <net/message-framework/ServiceInfo.h>
#include <net/message-framework/ServicePermissionInfo.h>
#include <net/message-framework/ServiceMessage.h>

#ifdef SF1_TIME_CHECK
#include <profiler/Profiler.h>
#endif

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

#include <boost/smart_ptr.hpp>

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#include <queue>
#include <vector>
#include <map>
#include <iostream>

namespace messageframework
{
	#define MF_LIGHT_MAX_CLIENT_ID 255
	#define MF_LIGHT_CLIENT_NUMBER (MF_LIGHT_MAX_CLIENT_ID+1)

    class MessageControllerLight : public MFSingleton<MessageControllerLight>
    {
#ifdef SF1_TIME_CHECK
        static sf1lib::Profiler contextSwitchProf;
        static sf1lib::Profiler::Profile getRequestCS;
        static sf1lib::Profiler::Profile addRequestCS;
        static sf1lib::Profiler::Profile resultCS;

        static sf1lib::Profiler::Profile getPermM;
        static sf1lib::Profiler::Profile addReqM;
        static sf1lib::Profiler::Profile processReqM;
        static sf1lib::Profiler::Profile getReqM;
        static sf1lib::Profiler::Profile addResultM;
        static sf1lib::Profiler::Profile getResultM;
#endif

        friend class MFSingleton<MessageControllerLight>;
        friend class MessageClientLight;
        friend class MessageServerLight;

        //PRIVATE CONSTRUCTOR
        private:

            MessageControllerLight()
			: controllerName_("mf_light-controller")
            {
                nxtClientId_ = 1;
            }

        public:

            //nothing to do
            void processServiceRegistrationRequest();

            //nothing to do
            void processServicePermissionRequest();

            //nothing to do
            void processServiceRequestFromClient();

            //nothing to do
            void processServiceResultFromServer();

            std::string getControllerName()
            {
                return controllerName_;
            }

#ifdef SF1_TIME_CHECK
            void printTime();
#endif

        protected:

            bool addServiceRegistrationRequest( const ServiceInfo & service );

            bool getServicePermission( const std::string & serviceName,
                    ServicePermissionInfo & servicePermissionInfo );

            bool addServiceRequest( const MessageFrameworkNode & serverInfo,
				const ServiceRequestInfoPtr & requestInfo );

			bool addServiceRequests( const MessageFrameworkNode & server,
				const std::vector<ServiceRequestInfoPtr > & requestInfos );

            bool getRequestListByServer( const MessageFrameworkNode & server,
				std::vector<ServiceRequestInfoPtr> & requestList );

            bool addResultOfService( const ServiceRequestInfoPtr & serviceRequest,
				const ServiceResultPtr & serviceResult );
            
            bool addResultOfService( const ServiceResultPtr & serviceResult );

            bool getResultByRequestId( const unsigned int requestId,
				ServiceResultPtr & result );

			bool getResultsByRequestIds( const std::vector<unsigned int> & requestIds,
			std::vector<ServiceResultPtr> & results );

			int applyForClientId();

            //TEMPORARY
            void printAvailServiceList();

        private:

            const std::string           controllerName_;

            boost::mutex                processDummyMutex_;
            boost::condition_variable   processDummyCond_;

			/**
			 * @brief Sequential number used to generate client id
			 */
			int nxtClientId_;

			boost::mutex nxtClientIdMutex_;

            /**
             * @brief The service names mapped to the service information
             *
             * @details
             * <service name> mapped to <ServiceInfo>
             */
            boost::unordered_map<std::string, ServiceInfo> availableServiceList_;
            boost::mutex availableServiceListMutex_;

            /**
             * @brief Maps each server to the list of service requests that are directed to those servers
             *
             * @details
             * <servername> mapped to <service requests for that server>
             */
            std::map<std::size_t, boost::shared_ptr<std::vector<ServiceRequestInfoPtr > > >
				serverServiceRequestQueueMap_;
            std::map<std::size_t, boost::shared_ptr<boost::mutex> >
				serverServiceRequestQueueMutexMap_;
            std::map<std::size_t, boost::shared_ptr<boost::condition_variable> >
				serverServiceRequestQueueCondMap_;
            boost::mutex serverServiceRequestQueueMapMutex_;

			/**
			 * @brief
			 */
			boost::array< boost::shared_ptr<std::map<unsigned int, ServiceResultPtr> >,
				MF_LIGHT_CLIENT_NUMBER > serviceIdResultMaps_;
			boost::array< boost::shared_ptr<boost::mutex>,
				MF_LIGHT_CLIENT_NUMBER > serviceIdResultMutexs_;
			boost::array< boost::shared_ptr<boost::condition_variable>,
				MF_LIGHT_CLIENT_NUMBER > serviceIdResultConds_;

    };

}

#endif  //_MESSAGE_CONTROLLER_LIGHT_H_
