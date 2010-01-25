#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <net/message_framework.h>

using namespace std;
using namespace boost;
using namespace boost::lambda;
using namespace messageframework;

BOOST_AUTO_TEST_SUITE( t_ServiceResolver )

BOOST_AUTO_TEST_CASE(basic) {

    // Init Controller
    MessageFrameworkNode ctrlNode("localhost", 10080);
    messageframework::MessageControllerFullSingleton::init( "controller", 10080);
    MessageControllerFull& controller = messageframework::MessageControllerFullSingleton::instance();
    thread* t1 = new thread((lambda::bind(&MessageControllerFull::processServiceRegistrationRequest, var(controller)) ));
    thread* t2 = new thread((lambda::bind(&MessageControllerFull::processServicePermissionRequest, var(controller)) ));

    // Init Client
    map<string, MessageFrameworkNode> response;
    MessageClientFull client("client", ctrlNode);

    ::sleep(1);

    {
        // Init service
        ServiceInfo service("service");
        service.setServer("localhost", 10086);
        MessageServerFull server("server", 10086, ctrlNode);
        server.registerService(service);
        // Resolved service address == localhost::10086
        client.getHostsOfService("service", response);
        BOOST_CHECK_EQUAL( response[""].nodeIP_, "127.0.0.1" );
        BOOST_CHECK_EQUAL( response[""].nodePort_, 10086U );
    }

    delete t1;
    delete t2;
}

BOOST_AUTO_TEST_CASE(service_name_changed) {

    // Init Controller
    MessageFrameworkNode ctrlNode("localhost", 10080);
    messageframework::MessageControllerFullSingleton::init( "controller", 10080);
    MessageControllerFull& controller = messageframework::MessageControllerFullSingleton::instance();
    thread* t1 = new thread((lambda::bind(&MessageControllerFull::processServiceRegistrationRequest, var(controller)) ));
    thread* t2 = new thread((lambda::bind(&MessageControllerFull::processServicePermissionRequest, var(controller)) ));

    // Init Client
    map<string, MessageFrameworkNode> response;
    MessageClientFull client("client", ctrlNode);

    ::sleep(1);

    {
        // Init service @10086
        ServiceInfo service("service");
        service.setServer("localhost", 10086);
        MessageServerFull server("server", 10086, ctrlNode);
        server.registerService(service);
        // Resolved service address == localhost::10086
        client.getHostsOfService("service", response);
        BOOST_CHECK_EQUAL( response[""].nodeIP_, "127.0.0.1" );
        BOOST_CHECK_EQUAL( response[""].nodePort_, 10086U );
    }

    ::sleep(1);

    {
        // Init service @10087
        ServiceInfo service("service");
        service.setServer("localhost", 10087);
        MessageServerFull server("server", 10087, ctrlNode);
        server.registerService(service);
        // Resolved service address == localhost::10087
        client.getHostsOfService("service", response);
        BOOST_CHECK_EQUAL( response[""].nodeIP_, "127.0.0.1" );
        BOOST_CHECK_EQUAL( response[""].nodePort_, 10087U );
    }

    delete t1;
    delete t2;
}

BOOST_AUTO_TEST_SUITE_END()
