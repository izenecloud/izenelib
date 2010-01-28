#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <util/izene_log.h>

#include <net/message_framework.h>

using namespace std;
using namespace boost;
using namespace boost::lambda;
using namespace messageframework;

BOOST_AUTO_TEST_SUITE( t_ServiceResolver )

BOOST_AUTO_TEST_CASE(basic) {

    google::InitGoogleLogging("t_mf");

    ::remove("_ctr_services.10080.dat");

    MessageFrameworkNode ctrlNode("localhost", 10080);

    // Init Controller
    MessageControllerFull controller("controller", 10080);
    thread t(lambda::bind(&MessageControllerFull::run, var(controller)));

    // Init Client
    map<string, MessageFrameworkNode> response;
    MessageClientFull client("client", ctrlNode);

    ::sleep(1);

    // Init service
    ServiceInfo service("service");
    service.setServer("localhost", 10086);
    MessageServerFull server("server", 10086, ctrlNode);
    server.setAgentInfo("0");
    server.registerService(service);
    // Resolved service address == localhost::10086
    client.getPermissionOfService("service", response);
    BOOST_CHECK_EQUAL( response["0"].nodeIP_, "127.0.0.1" );
    BOOST_CHECK_EQUAL( response["0"].nodePort_, 10086U );

    controller.shutdown();
    t.join();
}

BOOST_AUTO_TEST_CASE(multiple_collection) {

    ::remove("_ctr_services.10080.dat");

    MessageFrameworkNode ctrlNode("localhost", 10080);

    // Init Controller
    MessageControllerFull controller("controller", 10080);
    thread t(lambda::bind(&MessageControllerFull::run, var(controller)));

    // Init Client
    map<string, MessageFrameworkNode> response;
    MessageClientFull client("client", ctrlNode);

    ::sleep(1);

    // Init service 0
    ServiceInfo service0("service");
    service0.setServer("localhost", 10086);
    MessageServerFull server0("server", 10086, ctrlNode);
    server0.setAgentInfo("0");
    server0.registerService(service0);

    client.getPermissionOfService("service", response);
    BOOST_CHECK_EQUAL( response["0"].nodeIP_, "127.0.0.1" );
    BOOST_CHECK_EQUAL( response["0"].nodePort_, 10086U );

    // Init service 1
    ServiceInfo service1("service");
    service1.setServer("localhost", 10087);
    MessageServerFull server1("server", 10087, ctrlNode);
    server1.setAgentInfo("1");
    server1.registerService(service1);

    client.getPermissionOfService("service", response);
    BOOST_CHECK_EQUAL( response["0"].nodeIP_, "127.0.0.1" );
    BOOST_CHECK_EQUAL( response["0"].nodePort_, 10086U );

    BOOST_CHECK_EQUAL( response["1"].nodeIP_, "127.0.0.1" );
    BOOST_CHECK_EQUAL( response["1"].nodePort_, 10087U );
    controller.shutdown();
    t.join();
}

BOOST_AUTO_TEST_CASE(service_name_changed) {

    ::remove("_ctr_services.10080.dat");

    // Init Controller
    MessageFrameworkNode ctrlNode("localhost", 10080);
    MessageControllerFull controller("controller", 10080);
    thread t(lambda::bind(&MessageControllerFull::run, var(controller)));

    // Init Client
    map<string, MessageFrameworkNode> response;
    MessageClientFull client("client", ctrlNode);

    ::sleep(1);

    {
        // Init service @10086
        ServiceInfo service("service");
        service.setServer("localhost", 10086);
        MessageServerFull server("server", 10086, ctrlNode);
        server.setAgentInfo("0");
        server.registerService(service);
        // Resolved service address == localhost::10086
        client.getPermissionOfService("service", response);
        BOOST_CHECK_EQUAL( response["0"].nodeIP_, "127.0.0.1" );
        BOOST_CHECK_EQUAL( response["0"].nodePort_, 10086U );
    }

    ::sleep(1);

    {
        // Init service @10087
        ServiceInfo service("service");
        service.setServer("localhost", 10087);
        MessageServerFull server("server", 10087, ctrlNode);
        server.setAgentInfo("0");
        server.registerService(service);
        // Resolved service address == localhost::10087
        client.getPermissionOfService("service", response);
        BOOST_CHECK_EQUAL( response["0"].nodeIP_, "127.0.0.1" );
        BOOST_CHECK_EQUAL( response["0"].nodePort_, 10087U );
    }

    controller.shutdown();
    t.join();
}


BOOST_AUTO_TEST_CASE(controller_reboot) {

    ::remove("_ctr_services.10080.dat");

    MessageFrameworkNode ctrlNode("localhost", 10080);

    // Init Controller
    MessageControllerFull controller("controller", 10080);
    thread t(lambda::bind(&MessageControllerFull::run, var(controller)));
    ::sleep(1);

    // Init service @10086
    ServiceInfo service("service");
    service.setServer("localhost", 10086);
    MessageServerFull server("server", 10086, ctrlNode);
    server.setAgentInfo("0");
    server.registerService(service);

    {
        // Init Client
        map<string, MessageFrameworkNode> response;
        MessageClientFull client("client", ctrlNode);
        // Resolved service address == localhost::10086
        client.getPermissionOfService("service", response);
        BOOST_CHECK_EQUAL( response["0"].nodeIP_, "127.0.0.1" );
        BOOST_CHECK_EQUAL( response["0"].nodePort_, 10086U );
    }

    controller.shutdown();
    t.join();

    MessageControllerFull rebootedController("controller", 10080);
    thread rt(lambda::bind(&MessageControllerFull::run, var(rebootedController)));
    ::sleep(1);

    {
        // Init Client
        map<string, MessageFrameworkNode> response;
        MessageClientFull client("client", ctrlNode);
        // Resolved service address == localhost::10086
        client.getPermissionOfService("service", response);
        BOOST_CHECK_EQUAL( response["0"].nodeIP_, "127.0.0.1" );
        BOOST_CHECK_EQUAL( response["0"].nodePort_, 10086U );
    }

    rebootedController.shutdown();
    rt.join();
}


BOOST_AUTO_TEST_SUITE_END()
