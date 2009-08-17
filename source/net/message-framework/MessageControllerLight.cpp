/**
 * @author  MyungHyun Lee (Kent), Rewritten by Wei Cao 2009-3-29
 */

#include <net/message-framework/MessageControllerLight.h>
#include <net/message-framework/MessageClientLight.h>
#include <iostream>

using namespace std;
using namespace boost;

namespace messageframework {

void MessageControllerLight::processServiceRegistrationRequest() {
	boost::mutex::scoped_lock lock(processDummyMutex_);
	processDummyCond_.wait(lock);
}

void MessageControllerLight::processServicePermissionRequest() {
	boost::mutex::scoped_lock lock(processDummyMutex_);
	processDummyCond_.wait(lock);
}

void MessageControllerLight::processServiceResultFromServer() {
	boost::mutex::scoped_lock lock(processDummyMutex_);
	processDummyCond_.wait(lock);
}

void MessageControllerLight::processServiceRequestFromClient() {
	boost::mutex::scoped_lock lock(processDummyMutex_);
	processDummyCond_.wait(lock);
}

int MessageControllerLight::applyForClientId() {
	{
		boost::mutex::scoped_lock lock(nxtClientIdMutex_);
		if (nxtClientId_ == MF_LIGHT_MAX_CLIENT_ID) {
			std::cout << "Error: Max Client ID reached" << std::endl;
			return 0;
		}

		serviceIdResultMaps_[nxtClientId_] = boost::shared_ptr<std::map<unsigned int, ServiceResultPtr>  >
					(new std::map<unsigned int, ServiceResultPtr>);
			serviceIdResultMutexs_[nxtClientId_] =
				boost::shared_ptr<boost::mutex>(new boost::mutex());
			serviceIdResultConds_[nxtClientId_] =
				boost::shared_ptr<boost::condition_variable>(
					new boost::condition_variable());

			int clientId = nxtClientId_;
			nxtClientId_ ++;
			return clientId;
		}
    }

    bool MessageControllerLight::addServiceRegistrationRequest( const ServiceRegistrationMessage & message )
    {
		{
			const MessageFrameworkNode& server = message.getServer();
			boost::mutex::scoped_lock lock( availableServiceListMutex_ );
			std::string serviceName = message.getServiceName();
            if( availableServiceList_.find( serviceName ) != availableServiceList_.end() )
            {
            	availableServiceList_[serviceName].setServer(message.getAgentInfo(), server);
				std::cout << "CONTROLLER:  service exists " << message.getServiceName() << std::endl;
                return false;
            }
            ServicePermissionInfo permissionInfo;
            permissionInfo.setServiceName(serviceName);						
            permissionInfo.setServer(message.getAgentInfo(), server);
			availableServiceList_[serviceName] = permissionInfo;
		}
        //=== registering request queue for server ===
        const MessageFrameworkNode & server = message.getServer();
        {
			boost::mutex::scoped_lock lock_map( serverServiceRequestQueueMapMutex_ );
			boost::shared_ptr<vector<shared_ptr<ServiceRequestInfo> > >
				serverServiceRequestQueue( new vector<shared_ptr<ServiceRequestInfo> >() );
			boost::shared_ptr<boost::mutex>
				serverServiceRequestQueueMutex( new boost::mutex() );
			boost::shared_ptr<boost::condition_variable>
				serverServiceRequestQueueCond( new boost::condition_variable() );

			serverServiceRequestQueueMap_[server.hash()] = serverServiceRequestQueue;
			serverServiceRequestQueueMutexMap_[server.hash()] = serverServiceRequestQueueMutex;
			serverServiceRequestQueueCondMap_[server.hash()] = serverServiceRequestQueueCond;
        }
        return true;     
       
        
    }


    bool MessageControllerLight::getServicePermission(
            const std::string & serviceName,
            ServicePermissionInfo & servicePermissionInfo )
    {
#ifdef SF1_TIME_CHECK
		getPermM.begin();
#endif
        boost::mutex::scoped_lock lock( availableServiceListMutex_ );
        boost::unordered_map<std::string, ServicePermissionInfo>::iterator it;

        if( (it = availableServiceList_.find( serviceName )) != availableServiceList_.end())
        {
            //servicePermissionInfo.clear();

            //servicePermissionInfo.setServiceName( (it->second).getServiceName() );
           // servicePermissionInfo.setPermissionFlag( (it->second).getPermissionFlag() );
           // servicePermissionInfo.setServer( (it->second).getServer() );
          //  servicePermissionInfo.setServiceResultFlag( (it->second).getServiceResultFlag() );
            
            servicePermissionInfo = availableServiceList_[serviceName];
            
#ifdef SF1_TIME_CHECK
            getPermM.end();
#endif
            return true;
        }
        else
        {
			std::cout << "CONTROLLER:\t  service does *NOT* exists: " << serviceName << endl;
            servicePermissionInfo.clear();
            servicePermissionInfo.setServiceName( serviceName );
           // servicePermissionInfo.setPermissionFlag( UNKNOWN_PERMISSION_FLAG );
            return false;
        }
    }

    bool MessageControllerLight::addServiceRequest(
			const MessageFrameworkNode & server,
			const ServiceRequestInfoPtr & requestInfo )
    {
#ifdef SF1_TIME_CHECK
		addReqM.begin();
#endif
        {
            boost::mutex::scoped_lock lock( availableServiceListMutex_ );
            if( availableServiceList_.find( requestInfo->getServiceName() ) == availableServiceList_.end() )
            {
				cout << "CONTROLLER:\t  service does *NOT* exists: " << requestInfo->getServiceName() << endl;
                return false;
            }
        }

		boost::shared_ptr<std::vector<ServiceRequestInfoPtr > > serverServiceRequestQueue;
		boost::shared_ptr<boost::mutex> serverServiceRequestQueueMutex;
		boost::shared_ptr<boost::condition_variable> serverServiceRequestQueueCond;
		{
			boost::mutex::scoped_lock lock_map( serverServiceRequestQueueMapMutex_ );
			serverServiceRequestQueue = serverServiceRequestQueueMap_[server.hash()];
			serverServiceRequestQueueMutex = serverServiceRequestQueueMutexMap_[server.hash()];
			serverServiceRequestQueueCond = serverServiceRequestQueueCondMap_[server.hash()];
		}

		{
			boost::mutex::scoped_lock lock_queue(*serverServiceRequestQueueMutex);
			serverServiceRequestQueue->push_back( requestInfo );
		}
		serverServiceRequestQueueCond->notify_all();

#ifdef SF1_TIME_CHECK
		addReqM.end();
#endif
        return true;
    }


    bool MessageControllerLight::addServiceRequests(
			const MessageFrameworkNode & server,
			const std::vector<ServiceRequestInfoPtr > & requestInfos )
    {
#ifdef SF1_TIME_CHECK
		addReqM.begin();
#endif

		// sanity checks
		if( requestInfos.size() == 0)
		{
			std::cout << "Warning: requestInfos.size() == 0 in addServiceRequests" << std::endl;
			return true;
		}

		std::string serviceName = requestInfos[0]->getServiceName();
		for(int i=1; i<requestInfos.size(); i++)
		{
			if(requestInfos[i]->getServiceName() != serviceName )
			{
				std::cout << "Error: all requetInfos should be with the same service in addServiceRequests" << std::endl;
				return true;
			}
		}

        {
            boost::mutex::scoped_lock lock( availableServiceListMutex_ );
            if( availableServiceList_.find( serviceName ) == availableServiceList_.end() )
            {
				cout << "Error: service does *NOT* exists: " << serviceName << endl;
                return false;
            }
        }

        // insert requests into queue
		boost::shared_ptr<std::vector<ServiceRequestInfoPtr > > serverServiceRequestQueue;
		boost::shared_ptr<boost::mutex> serverServiceRequestQueueMutex;
		boost::shared_ptr<boost::condition_variable> serverServiceRequestQueueCond;
		{
			boost::mutex::scoped_lock lock_map( serverServiceRequestQueueMapMutex_ );
			serverServiceRequestQueue = serverServiceRequestQueueMap_[server.hash()];
			serverServiceRequestQueueMutex = serverServiceRequestQueueMutexMap_[server.hash()];
			serverServiceRequestQueueCond = serverServiceRequestQueueCondMap_[server.hash()];
		}

		{
			boost::mutex::scoped_lock lock_queue(*serverServiceRequestQueueMutex);
			serverServiceRequestQueue->reserve( serverServiceRequestQueue->size() + requestInfos.size() );
			for(int i=0; i<requestInfos.size(); i++ )
				serverServiceRequestQueue->push_back( requestInfos[i] );
		}
		serverServiceRequestQueueCond->notify_all();

#ifdef SF1_TIME_CHECK
		addReqM.end();
#endif
        return true;
    }


    bool MessageControllerLight::getRequestListByServer(
            const MessageFrameworkNode & server,
            std::vector<ServiceRequestInfoPtr> & requestList )
    {
#ifdef SF1_TIME_CHECK
		getReqM.begin();
#endif
        requestList.clear();

		boost::shared_ptr<std::vector<ServiceRequestInfoPtr > > serverServiceRequestQueue;
		boost::shared_ptr<boost::mutex> serverServiceRequestQueueMutex;
		boost::shared_ptr<boost::condition_variable> serverServiceRequestQueueCond;
		{
			boost::mutex::scoped_lock lock_map( serverServiceRequestQueueMapMutex_ );
			serverServiceRequestQueue = serverServiceRequestQueueMap_[server.hash()];
			serverServiceRequestQueueMutex = serverServiceRequestQueueMutexMap_[server.hash()];
			serverServiceRequestQueueCond = serverServiceRequestQueueCondMap_[server.hash()];
		}

		{
			boost::mutex::scoped_lock lock_queue(*serverServiceRequestQueueMutex);

			while(serverServiceRequestQueue->size() == 0 )
				serverServiceRequestQueueCond->wait(lock_queue);

			requestList.reserve(serverServiceRequestQueue->size());
			for (int i=0; i<serverServiceRequestQueue->size(); i++)
			{
				requestList.push_back( ((*serverServiceRequestQueue)[i]) );
			}
			serverServiceRequestQueue->clear();

#ifdef SF1_TIME_CHECK
			getReqM.end();
#endif

			if(requestList.size() > 0)
				return true;
			return false;
		}
    }

    bool MessageControllerLight::addResultOfService(     
            const ServiceResultPtr & serviceResult )
    {
#ifdef SF1_TIME_CHECK
        addResultM.begin();
#endif
		unsigned int requestId = serviceResult->getRequestId();
		unsigned int clientId = ( requestId >> MF_LIGHT_REQUEST_ID_CLIENT_ID_SHIFT );

		if(clientId == 0 || clientId > MF_LIGHT_MAX_CLIENT_ID)
		{
			std::cout << "Bad clientId " << clientId << std::endl;
			return false;
		}

		boost::shared_ptr<std::map<unsigned int, ServiceResultPtr> >
			serviceIdResultMap = serviceIdResultMaps_[clientId];
		boost::shared_ptr<boost::mutex> serviceIdResultMutex =
			serviceIdResultMutexs_[clientId];
		boost::shared_ptr<boost::condition_variable> serviceIdResultCond =
			serviceIdResultConds_[clientId];

		{
			boost::mutex::scoped_lock lock( *serviceIdResultMutex);
			(*serviceIdResultMap)[requestId] = serviceResult;
		}
		serviceIdResultCond->notify_all();

#ifdef SF1_TIME_CHECK
        addResultM.end();
#endif
        return true;
    }

    bool MessageControllerLight::getResultByRequestId(
			const unsigned int requestId, ServiceResultPtr & result )
    {
#ifdef SF1_TIME_CHECK
        getResultM.begin();
#endif

		unsigned int clientId = ( requestId >> MF_LIGHT_REQUEST_ID_CLIENT_ID_SHIFT );
		if(clientId == 0 || clientId > MF_LIGHT_MAX_CLIENT_ID)
		{
			std::cout << "Bad clientId " << clientId << std::endl;
			return false;
		}

		boost::shared_ptr<std::map<unsigned int, ServiceResultPtr> >
			serviceIdResultMap = serviceIdResultMaps_[clientId];
		boost::shared_ptr<boost::mutex> serviceIdResultMutex =
			serviceIdResultMutexs_[clientId];
		boost::shared_ptr<boost::condition_variable> serviceIdResultCond =
			serviceIdResultConds_[clientId];

		{
			boost::mutex::scoped_lock lock( *serviceIdResultMutex);
			while(serviceIdResultMap->find(requestId) == serviceIdResultMap->end() )
				serviceIdResultCond->wait(lock);

			result = (*serviceIdResultMap)[requestId];
			serviceIdResultMap->erase(requestId);
		}

#ifdef SF1_TIME_CHECK
		getResultM.end();
#endif
		return true;
    }

    bool MessageControllerLight::getResultsByRequestIds(
			const std::vector<unsigned int> & requestIds,
			std::vector<ServiceResultPtr> & results )
    {
#ifdef SF1_TIME_CHECK
        getResultM.begin();
#endif

		// sanity check
		if(requestIds.size() == 0)
		{
			std::cout << "Warning: requestIds.size() == 0 in getResultsByRequestIds" << std::endl;
			return true;
		}

		unsigned int clientId = (requestIds[0] >> MF_LIGHT_REQUEST_ID_CLIENT_ID_SHIFT);
		unsigned int lastRequestId = requestIds[0];
		for( int i=1; i<requestIds.size(); i++ )
		{
			if( (requestIds[i] >> MF_LIGHT_REQUEST_ID_CLIENT_ID_SHIFT) != clientId )
			{
				std::cout << "Error: requestIds should have the same clientId in getResultsByRequestIds" << std::endl;
				return false;
			}
			if(requestIds[i] <= lastRequestId )
			{				
				std::cout << "Error: requestIds should be sorted ascendingly (" <<requestIds[i] <<" <= "<<lastRequestId<< " ) "<<std::endl;
				return false;
			}
		}

		if(clientId == 0 || clientId > MF_LIGHT_MAX_CLIENT_ID)
		{
			std::cout << "Bad clientId " << clientId << std::endl;
			return false;
		}

		results.clear();
		results.resize(requestIds.size());

		boost::shared_ptr<std::map<unsigned int, ServiceResultPtr> >
			serviceIdResultMap = serviceIdResultMaps_[clientId];
		boost::shared_ptr<boost::mutex> serviceIdResultMutex =
			serviceIdResultMutexs_[clientId];
		boost::shared_ptr<boost::condition_variable> serviceIdResultCond =
			serviceIdResultConds_[clientId];

		int hit = 0;
		// Elements poped from serviceIdResultMaps_
		std::vector<ServiceResultPtr> cache;
		// Elements to be pushed back into serviceIdResultMaps_
		std::vector<ServiceResultPtr> back;
		ServiceResultCompare compare;
		while( hit < requestIds.size() )
		{
			// pop
			{
				boost::mutex::scoped_lock lock( *serviceIdResultMutex);
				while(serviceIdResultMap->size() == 0 )
					serviceIdResultCond->wait(lock);
				cache.reserve(serviceIdResultMap->size());
				back.reserve(serviceIdResultMap->size());
				for(std::map<unsigned int, ServiceResultPtr>::iterator it = serviceIdResultMap->begin();
					it != serviceIdResultMap->end(); it ++)
				{
					it->second->setRequestId(it->first);
					cache.push_back( it->second );
				}
				serviceIdResultMap->clear();
			}

			// match
			std::sort (cache.begin(), cache.end(), compare );
			int iIter = 0;
			int iEnd = requestIds.size();
			int jIter = 0;
			int jEnd = cache.size();
			while( iIter < iEnd && jIter < jEnd )
			{
				while( requestIds[iIter] < cache[jIter]->getRequestId() && iIter < iEnd )
					iIter ++;
				if( iIter == iEnd )
					break;

				if( requestIds[iIter] == cache[jIter]->getRequestId() )
				{
					hit++;
					results[iIter] = cache[jIter];
					iIter++;
					jIter++;
					continue;
				}

				// should be exectued rarely
				while( cache[jIter]->getRequestId() < requestIds[iIter]  && jIter < jEnd )
				{
					back.push_back(cache[jIter]);
					jIter ++;
				}
				if( jIter == jEnd )
					break;
			}

			// push back
			// should be exectued rarely
			if( back.size() > 0 )
			{
				boost::mutex::scoped_lock lock( *serviceIdResultMutex);
				for( int i=0; i<back.size(); i++ )
					(*serviceIdResultMap)[back[i]->getRequestId()] =  back[i];
			}

			cache.clear();
			back.clear();

			// sleep 1 millisecond to let producer insert results into queue.
			if( hit < requestIds.size() )
				boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(1) );
		}

#ifdef SF1_TIME_CHECK
		getResultM.end();
#endif
		return true;
    }

    void MessageControllerLight::printAvailServiceList()
    {
        using namespace std;
        boost::unordered_map<std::string, ServicePermissionInfo>::iterator it;
        for( it = availableServiceList_.begin(); it != availableServiceList_.end(); it++ )
        {
            cout << "service name: " << it->first << endl;
        }
    }

#ifdef SF1_TIME_CHECK
    void MessageControllerLight::printTime()
    {
        contextSwitchProf.print();
    }
#endif
}
