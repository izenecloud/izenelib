///
///  @file : MessageServerFull.cpp
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///


/**************** Include module header files *******************/
// #include <BasicMessage.h>
#include <net/message-framework/MessageServerFull.h>
#include <net/message-framework/MessageFrameworkConfiguration.h>
#include <net/message-framework/ServiceRegistrationMessage.h>
#include <net/message-framework/MessageType.h>
#include <net/message-framework/ServiceMessage.h>
#include <net/message-framework/common.h>

/**************** Include std header files *******************/
#include <iostream>

/**************** Include boost header files *******************/


#define  _LOGGING_

namespace messageframework
{

	/***************************************************************************
 	Description: Initialize variables with default values. It initialize resources
	for communication.
    Input:
		serverName - name of server
		servicePort - port  of service that serves results of services
		controllerInfo - information of controller
	***************************************************************************/

	MessageServerFull::MessageServerFull(
					const std::string& serverName, unsigned int serverPort,
					const MessageFrameworkNode& controllerInfo):
		messageDispatcher_(this, this),
		connector_(this, io_service_),
		connectionEstablished_(false)
	{
		ownerManager_ = serverName;
		timeOutMilliSecond_ = TIME_OUT_IN_MILISECOND;

		server_.nodePort_ = serverPort;
		server_.nodeIP_ = getLocalHostIp(io_service_);

		availableServiceList_.clear();

		serviceRegistrationServer_ = controllerInfo;
		connector_.listen(serverPort);
		controllerNode_.nodeIP_ = getHostIp(io_service_, controllerInfo.nodeIP_);
		controllerNode_.nodePort_ = controllerInfo.nodePort_;
		controllerNode_.nodeName_ = controllerInfo.nodeName_;
		connector_.connect(controllerInfo.nodeIP_, controllerInfo.nodePort_);

		// create new thread for I/O operations
		ioThread_ = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));

		// Added by Wei Cao, 2009-02-20
		// To slove the problem of service registration timeout.
		// waiting until connection to controller is established.
		boost::system_time const timeout = boost::get_system_time()
			+ boost::posix_time::milliseconds(timeOutMilliSecond_);
		boost::mutex::scoped_lock connectionEstablishedLock(connectionEstablishedMutex_);
		while(!connectionEstablished_)
		{
			if (!connectionEstablishedEvent_.timed_wait(connectionEstablishedLock, timeout))
				throw MessageFrameworkException(SF1_MSGFRK_CONNECTION_TIMEOUT, __LINE__, __FILE__);
		}
		// connect to controller successfully now
	}

	/******************************************************************************
    Description: Destroy communication resources.
	******************************************************************************/
	MessageServerFull::~MessageServerFull()
	{
		connector_.shutdown();
		ioThread_->join();
		delete ioThread_;
	}

	/*********************************************************************************
	Input:
		 serviceName - The service name
		 parameterList - The list of parameter types of the service
		 permissionFlag - this flags tell how to serve the service (through Message
				Controller or directly at MessageServerFull)
	Return
		 true - the service has been successfully registered.
	Description: This function register a service of the current MessageServerFull. It will
	submit the registration information of the service to the MessageController.
	*********************************************************************************/
	bool MessageServerFull::registerService(const std::string& serviceName,
					const std::vector<ServiceParameterType>& parameterList,
					const PermissionFlag& permissionFlag,
                    const ServiceResultFlag serviceResultFlag )
	{
		try
		{
			ServiceInfo serviceInfo;
			std::map<std::string, ServiceInfo>::iterator it;

			// connection has not been established
			if(!connectionEstablished_)
				return false;

			// Check if the service has been registered
			it = availableServiceList_.find(serviceName);
			if(it != availableServiceList_.end())
			{
				// service has been registered
				return true;
			}

			sendServiceRegistrationRequest(serviceName, parameterList,
				permissionFlag, serviceResultFlag);

			boost::system_time const timeout = boost::get_system_time() +
				boost::posix_time::milliseconds(timeOutMilliSecond_);
			boost::mutex::scoped_lock lock(serviceRegistrationReplyMutex_);
			// Modified by Wei Cao, 2009-02-20
			// To slove the problem of service registration timeout
			// Remove serviceRegistrationWaitingQueue_
			// When wake up from a condition variable, you should check whether
			// the condition fulfills, if not, go on waiting.
			while(serviceRegistrationResult_.find(serviceName) ==
				serviceRegistrationResult_.end())
			{
				if(!serviceRegistrationReplyAccessor_.timed_wait(lock, timeout))
				{
  #ifdef SF1_DEBUG
					std::cout << "[Server: " << getName() << "]Timed out!!! Connection is going to be closed." << std::endl;
  #endif
					return false;
				}
			}

			std::map<std::string, bool>::const_iterator iter =
				serviceRegistrationResult_.find(serviceName);
			if(iter != serviceRegistrationResult_.end())
			{
				serviceInfo.setServiceName(serviceName);
				serviceInfo.setParameterList(parameterList);
				serviceInfo.setPermissionFlag(permissionFlag);
                serviceInfo.setServiceResultFlag( serviceResultFlag );

				// The service has been successfully registered, the service
				// is added to the available service list
				availableServiceList_.insert(std::pair<std::string, ServiceInfo>(serviceName, serviceInfo));
				serviceRegistrationResult_.erase(serviceName);
#ifdef SF1_DEBUG
				std::cout << "[Server: " << getName() << "] Service " << serviceName;
				std::cout << " is successfully registered."<< std::endl;
#endif
#ifdef _LOGGING_
				WriteToLog("log.log", "successfully registered.");
#endif
				return true;
			}

			// it's strange to reach here
			throw MessageFrameworkException(SF1_MSGFRK_UNKNOWN_ERROR, __LINE__, __FILE__);
		}
		catch(MessageFrameworkException& e)
		{
			e.output(std::cerr);
		}
		catch(boost::system::error_code& e)
		{
			std::cerr << e << std::endl;
		}
		catch(std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << std::endl;
		}

		return false;
	}


    bool MessageServerFull::registerService( const ServiceInfo & serviceInfo )
    {
        return registerService(
			serviceInfo.getServiceName(),
			serviceInfo.getParameterList(),
			serviceInfo.getPermissionFlag(),
			serviceInfo.getServiceResultFlag()
		);
    }

	/*********************************************************************************
	Output:
		 requestList - the list of waiting service requests
	Return:
		 true - the list is retrieved successfully
	Description: This function copies all requests to the parameter
	requestList. Then, it deletes the requests in the internal queue.
	*********************************************************************************/
	bool MessageServerFull::getServiceRequestList(std::vector<ServiceRequestInfoPtr>& requestList)
	{
		try
		{
			if(requestList.size() > 0)
				requestList.clear();
			// connection has not been established
			if(!connectionEstablished_)
				return false;

			/* >>>>>>>>>>>
			 * TODO: NEED TO DECIDE WHETHER THERE SHOULD BE A TIMEOUT FOR THE RESULT    @by MyungHyun - 2009-01-29
			 */
			// otherwise, wait for new request
			//newRequestEvent_.wait(requestQueueLock);
			
			boost::system_time const timeout = boost::get_system_time()
				+ boost::posix_time::milliseconds(500); //waits 0.5 secs

			boost::mutex::scoped_lock requestQueueLock(requestQueueMutex_);
			while( requestQueue_.empty() )
				newRequestEvent_.wait(requestQueueLock);
		  //newRequestEvent_.timed_wait(requestQueueLock, timeout);
		  //<<<<<<<<<<<<<<
			
			while(!requestQueue_.empty())
			{
				requestList.push_back(requestQueue_.front() );
				requestQueue_.pop();
			}
			return true;
		}
		catch(boost::system::error_code& e)
		{
			std::cerr << e << std::endl;
		}
		catch(std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << std::endl;
		}

		return false;
	}


	/**
	 * @brief This function answers a service request by putting the ServiceResult to the
	 * requester of the service.
	 * @param
	 *	serviceRequest - information about the service request (service name, requester, parameter list)
	 * @param
	 *	result - the result of the service
	 * @return
	 *	true - the service result has been successfully put to the requester
	 */
	bool MessageServerFull::putResultOfService(const ServiceRequestInfoPtr& serviceRequestInfo, const ServiceResultPtr& result)
	{

		//the service provides no return value  @by MyungHyun - 2009-01-28
		/*if( serviceRequestInfo.getServiceResultFlag() ==  messageframework::SERVICE_WITHOUT_RESULT)
		{
		  return false;
		}*/
#ifdef SF1_DEBUG
		std::cout << "[Server: " << getName() << "] start to send result of ";
		std::cout << serviceRequestInfo->getServiceName() << std::endl;
#endif

		// connection has not been established
		if(!connectionEstablished_)
			return false;

		sendResultOfService(serviceRequestInfo->getRequester(), result);

#ifdef SF1_DEBUG
		std::cout << "[Server: " << getName() << "] Successfull to send result of ";
		std::cout << serviceRequestInfo->getServiceName() << std::endl;
#endif
		return true;
	}


	bool MessageServerFull::putResultOfService(const ServiceResultPtr& result)
	{
#ifdef SF1_DEBUG
		std::cout << "[Server: " << getName() << "] start to send result of ";
			std::cout << result->getServiceName() << std::endl;
#endif
		// connection has not been established
		if(!connectionEstablished_)
			return false;
		sendResultOfService(result->getRequester(), result);
		return true;
	}

	/*********************************************************************************
	Description: This function answers a service request by putting the ServiceResult
	to the requester of the service.
	*********************************************************************************/
	void MessageServerFull::sendResultOfService(const MessageFrameworkNode& requester,				
				const ServiceResultPtr& result)
	{
#ifdef _LOGGING_
		WriteToLog("log.log", "============= MessageServer::sendResultOfService ===========");		
#endif	
		//boost::mutex::scoped_lock resultCacheLock(resultCacheMutex_);
		messageDispatcher_.sendDataToLowerLayer1(SERVICE_RESULT_MSG, result, requester);
/*
		try
		{
		   //std::cout << "Debug: send result for request#"
			//			<< result->getRequestId()  << " minor#" << result->getMinorId() << std::endl;
			//if( result->getMinorId() == 0 )
			{
				ServiceMessage message;

				message.setServiceName(requestInfo.getServiceName());
				message.setRequestId(requestInfo.getRequestId());

				boost::shared_ptr<ServiceResult> serviceResult(new ServiceResult(result));
				serviceResult->setServiceName(requestInfo.getServiceName() );
				serviceResult->setRequestId(requestInfo.getRequestId() );
				serviceResult->setMinorId(requestInfo.getMinorId() );

				boost::shared_ptr<VariantType> tmp( new VariantType() );
				tmp->putCustomData(serviceResult);
				message.appendData(tmp);
				//messageDispatcher_.sendDataToLowerLayer(SERVICE_RESULT_MSG, message, requester);
				messageDispatcher_.sendDataToLowerLayer1(SERVICE_RESULT_MSG, *result, requester);
			}
			else
			{
			
				unsigned int requestId = requestInfo.getRequestId();
				unsigned int minorId = requestInfo.getMinorId();
				boost::mutex::scoped_lock resultCacheLock(resultCacheMutex_);
				if( resultCacheMap_[requestId][minorId-1] == NULL )
				{
					boost::shared_ptr<ServiceResult> serviceResult(new ServiceResult(result));
					serviceResult->setServiceName(requestInfo.getServiceName() );
					serviceResult->setRequestId(requestInfo.getRequestId() );
					serviceResult->setMinorId(requestInfo.getMinorId() );

					resultCacheMap_[requestId][minorId-1] = serviceResult;
					actualResultCountMap_[requestId] ++;
				}
				else
				{
					std::cout << "Warning: send duplicated result for request#"
						<< requestId << " minor#" << minorId << std::endl;
				}
				if(actualResultCountMap_[requestId] == expectedResultNumberMap_[requestId])
				{
					ServiceMessage message;

					message.setServiceName(requestInfo.getServiceName());
					message.setRequestId(requestInfo.getRequestId());

					for( int i=0; i<resultCacheMap_[requestId].size(); i++ )
					{
						boost::shared_ptr<VariantType> tmp( new VariantType() );
						tmp->putCustomData(resultCacheMap_[requestId][i]);
						message.appendData(tmp);
					}
					resultCacheMap_.erase(requestId);
					expectedResultNumberMap_.erase(requestId);
					actualResultCountMap_.erase(requestId);

					resultCacheLock.unlock();
					messageDispatcher_.sendDataToLowerLayer(SERVICE_RESULT_MSG, message, requester);
				}
			}
		}
		catch(MessageFrameworkException& e)
		{
			e.output(std::cerr);
		}
		catch(boost::system::error_code& e)
		{
			std::cerr << e << std::endl;
		}
		catch(std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << std::endl;
		}*/
	}

	/******************************************************************************
 	Description: This function put the request for a service result in the waiting list.
	******************************************************************************/
	/*void MessageServerFull::receiveServiceRequest(const MessageFrameworkNode& requester,
						unsigned int requestId,
						const std::string& serviceName,
						const std::vector<boost::shared_ptr<VariantType> >& data)*/
						
  void MessageServerFull::receiveServiceRequest(const MessageFrameworkNode& requester,
						 ServiceRequestInfoPtr & requestInfo) 	
	{
#ifdef SF1_DEBUG
			std::cout << "[Server: " << getName() << "] Receive request to retrieve result of service ";
			std::cout << requestInfo->getServiceName();
			std::cout << "[requestId = " << requestInfo->getRequestId() << "]"  << std::endl;
			requestInfo->display();
#endif
		try
		{
			//int requestNumber = data.size();

			//if(requestNumber == 1)
			//{
				//boost::shared_ptr<ServiceRequestInfo> serviceRequestInfo;
				//data[0]->getCustomData(serviceRequestInfo);

				// if there is only one request and its minor id is one
				//if( requestInfo->getMinorId() == 0 )
				{
					requestInfo->setRequester(requester);
				//	std::cout<<"receiveServiceRequest : after set Requester"<<endl;
				//	requestInfo->display();

					boost::mutex::scoped_lock requestQueueLock(requestQueueMutex_);
					// add to the requestQueue_
					requestQueue_.push(requestInfo);

					requestQueueLock.unlock();
					newRequestEvent_.notify_all();
					return;
				}
			//}

			// a multi-request
			/*{			   
				boost::mutex::scoped_lock resultCacheLock(resultCacheMutex_);
				resultCacheMap_[requestId].resize(requestNumber);
				expectedResultNumberMap_[requestId] = requestNumber;
				actualResultCountMap_[requestId] = 0;
			}
			boost::mutex::scoped_lock requestQueueLock(requestQueueMutex_);
			for( int i=0; i<requestNumber; i++ )
			{
				boost::shared_ptr<ServiceRequestInfo> serviceRequestInfo;
				data[i]->getCustomData(serviceRequestInfo);
				serviceRequestInfo.setRequester(requester);

				// add to the requestQueue_
				requestQueue_.push(serviceRequestInfo);
			}
			requestQueueLock.unlock();
			newRequestEvent_.notify_all();*/
		}
		catch (boost::system::error_code& e)
		{
			std::cerr << e << std::endl;
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: "<< e.what() << std::endl;
		}

	}


	/**
	 * @brief This function sends request to ServiceRegistrationServer
	 * in order to register a service
	 * @param
	 *	serviceName - The service name
	 * @param
	 *  parameterList - The list of parameter types of the service
	 * @param
	 *	permissionFlag - this flags tell how to serve the service (through Message Controller or
	 *				directly at MessageServerFull)
	 */
	void MessageServerFull::sendServiceRegistrationRequest(const std::string& serviceName,
				const std::vector<ServiceParameterType>& parameterList,
				const PermissionFlag& permissionFlag,
                const ServiceResultFlag serviceResultFlag )
	{
		ServiceRegistrationMessage message(serviceName, parameterList, permissionFlag, serviceResultFlag);
		message.setServer(server_);
		message.setRequester(ownerManager_);

		messageDispatcher_.sendDataToLowerLayer(SERVICE_REGISTRATION_REQUEST_MSG, message, controllerNode_);
	}

	/*********************************************************************************
 	Description: This function set the reply of the recent request of service registration
 	Input:
 		 registrationSuccess - true if the registration is successful
	*********************************************************************************/
	void MessageServerFull::receiveServiceRegistrationReply(const std::string& serviceName,
						bool registrationSuccess)
	{
		try
		{
#ifdef _LOGGING_
			WriteToLog("log.log", "receiveServiceRegistrationReply...");
#endif
			{
				boost::mutex::scoped_lock serviceRegistrationReplyLock(
					serviceRegistrationReplyMutex_);
				serviceRegistrationResult_.insert(
					std::pair<std::string, bool>(serviceName, registrationSuccess));
			}
			serviceRegistrationReplyAccessor_.notify_all();
		}
		catch(boost::system::error_code& e)
		{
			std::cerr << e << std::endl;
		}
		catch(std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << std::endl;
		}
	}

	/**
	 * @brief This function create a new AsyncStream that is based on tcp::socket
	 */
	AsyncStream* MessageServerFull::createAsyncStream(boost::shared_ptr<tcp::socket> sock)
	{
		tcp::endpoint endpoint = sock->remote_endpoint();
		std::cout << "Remote IP = " << endpoint.address().to_string();
		std::cout << ", port = " << endpoint.port() << std::endl;

		if(!connectionEstablished_)
		{
			{
				boost::mutex::scoped_lock connectionEstablishedLock(connectionEstablishedMutex_);
				connectionEstablished_ = true;
			}
			connectionEstablishedEvent_.notify_all();
		}

		return new AsyncStream(&messageDispatcher_, sock);
	}

}// end of namespace messageframework

