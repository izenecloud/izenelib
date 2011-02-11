/**
 * @file t_MessageController.cpp
 * @brief A test unit of MessageController
 * @author TuanQuang Nguyen
 * @date 2008-08-20
 * @details
 * This file implements test code for MessageController. Here is test scheme:
 * every 10 milliseconds, it calls 4 main interfaces of MessageController
 * to process underlying data.
 * - processServiceRegistrationRequest() - process all the service registration requests
 *   from servers. It registers services. MessageClient can access to only
 *   registered services.
 * - processServicePermissionRequest() - process all the requests from client
 *   to access to specific services
 * - processServiceRequestFromClient() - process all requests from client
 *   to retrieve results of a service. Those requests wil be forwarded to
 *   servers of the requested services.
 * - processServiceResultFromServer() - process all replies from servers to serve
 *   results of the requested services in processServiceRequestFromClient(). The
 *   resutls will be forwarded to clients that requested the results.
 */

/**************** Include module header files *******************/
// #include "MessageController.h"
// #include "MessageFrameworkConfiguration.h"
#include <net/message_framework.h>
#include <ProcessOptions.h>

/**************** Include sf1lib header files *******************/

/**************** Include boost header files *******************/
#include <boost/thread.hpp>
// #include <boost/channel/channel.hpp>

/**************** Include std header files *******************/

using namespace std;
using namespace messageframework;

namespace ControllerProcess
{

//	int processServiceRegistration(MessageController& messageController)
//	{
//		try
//		{
//			boost::system_time wakeupTime;
//			MessageController& controller = messageController;
//
//			while(true)
//			{
//				controller.processServiceRegistrationRequest();
//
//				// Sleep 5 milliseconds
//				wakeupTime = boost::get_system_time() + boost::posix_time::milliseconds(1);
//				boost::thread::sleep( wakeupTime );
//			}
//		}
//		catch(MessageFrameworkException& e)
//		{
//			e.output(std::cout);
//		}
//
//		return 1;
//	}
//
//	int processServicePermission(MessageController& messageController)
//	{
//		try
//		{
//			MessageController& controller = messageController;
//			boost::system_time wakeupTime;
//
//			while(true)
//			{
//				controller.processServicePermissionRequest();
//
//				// Sleep 5 milliseconds
//				wakeupTime = boost::get_system_time() + boost::posix_time::milliseconds(1);
//				boost::thread::sleep( wakeupTime );
//			}
//		}
//		catch(MessageFrameworkException& e)
//		{
//			e.output(std::cout);
//		}
//
//		return 1;
//	}
//
//	int processServiceRequest(MessageController& messageController)
//	{
//		try
//		{
//			MessageController& controller = messageController;
//			boost::system_time wakeupTime;
//
//			while(true)
//			{
//				controller.processServiceRequestFromClient();
//
//				// Sleep 5 milliseconds
//				wakeupTime = boost::get_system_time() + boost::posix_time::milliseconds(1);
//				boost::thread::sleep( wakeupTime );
//			}
//		}
//		catch(MessageFrameworkException& e)
//		{
//			e.output(std::cout);
//		}
//
//		return 1;
//	}
//
//
//	int processServiceResult(MessageController& messageController)
//	{
//		try
//		{
//			MessageController& controller = messageController;
//			boost::system_time wakeupTime;
//
//			while(true)
//			{
//				controller.processServiceResultFromServer();
//
//				// Sleep 5 milliseconds
//				wakeupTime = boost::get_system_time() + boost::posix_time::milliseconds(1);
//				boost::thread::sleep( wakeupTime );
//			}
//		}
//		catch(MessageFrameworkException& e)
//		{
//			e.output(std::cout);
//		}
//
//		return 1;
//	}

	void _usage(void)
	{
		std::cout << "controller controllerName controllerPort" << std::endl;
	}

}

/**
 * @brief it creates 4 threads for 4 main routines of Message Controller
 * - processServiceRegistrationRequest() - process all the registration requests
 *   from servers
 * - processServicePermissionRequest() - process all the requests from client
 *   to access to specific services
 * - processServiceRequestFromClient() - process all requests from client
 *   to retrieve results of a service. Those requests wil be forwarded to
 *   specific servers.
 * - processServiceResultFromServer() - process all replies from servers to serve
 *   results of the requested services in processServiceRequestFromClient(). The
 *   resutls will be forwarded to specific clients.
 */
//int main(int argc, char* argv[])
MF_AUTO_MAIN( controllerProcess )
{
	using namespace ControllerProcess;

	sf1v5_dummy::ProcessOptions argument;

	int port;

	if(argument.setControllerOptions(argc, argv) && argument.getNumberOfOptions() == 1)
	{

			port = argument.getControllerPort();
			INIT_MF_CONTROLLER("controller", port);

			MessageController& controller = GET_CONTROLLER_INSTANCE();
			controller.run();
			
//			boost::thread* ProcessServiceRegistrationThread;
//			boost::thread* ProcessServicePermissionThread;
//			boost::thread* ProcessServiceRequestThread;
//			boost::thread* ProcessServiceResultThread;
//
//			ProcessServiceRegistrationThread = new boost::thread(boost::bind(processServiceRegistration,
//							boost::ref(controller)));
//
//			ProcessServicePermissionThread = new boost::thread(boost::bind(processServicePermission,
//							boost::ref(controller)));
//
//			ProcessServiceRequestThread = new boost::thread(boost::bind(processServiceRequest,
//							boost::ref(controller)));
//
//			ProcessServiceResultThread = new boost::thread(boost::bind(processServiceResult,
//							boost::ref(controller)));

//			ProcessServiceRegistrationThread->join();
//			ProcessServicePermissionThread->join();
//			ProcessServiceRequestThread->join();
//			ProcessServiceResultThread->join();

//			delete ProcessServiceResultThread;
//			delete ProcessServiceRequestThread;
//			delete ProcessServicePermissionThread;
//			delete ProcessServiceRegistrationThread;

	}

	return 0;
}

