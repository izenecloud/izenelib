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
    server.registerService(service);

    {
        // Init Client
        map<string, MessageFrameworkNode> response;
        MessageClientFull client("client", ctrlNode);
        // Resolved service address == localhost::10086
        client.getHostsOfService("service", response);
        BOOST_CHECK_EQUAL( response[""].nodeIP_, "127.0.0.1" );
        BOOST_CHECK_EQUAL( response[""].nodePort_, 10086U );
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
        client.getHostsOfService("service", response);
        BOOST_CHECK_EQUAL( response[""].nodeIP_, "127.0.0.1" );
        BOOST_CHECK_EQUAL( response[""].nodePort_, 10086U );
    }

    rebootedController.shutdown();
    rt.join();
}


BOOST_AUTO_TEST_SUITE_END()
