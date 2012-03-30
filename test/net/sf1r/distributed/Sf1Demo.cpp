/* 
 * File:   Sf1Demo.cpp
 * Author: Paolo D'Apice
 * 
 * Created on March 14, 2012, 3:22 PM
 */

#undef ENABLE_ZK_TEST
#undef ENABLE_SF1_TEST

/**
 * Sf1DistributedDriver demo.
 *
 * Using this application it is possible to verify the dynamic requests routing
 * and the ZooKeeper interaction.
 * The main cycle periodycally sends a request to the distributed driver. 
 * So starting this demo app and changing the SF1 topology registered with 
 * the ZooKeeper server, it is possible to see from the logs how the whole 
 * thing works.
 */

#include "../common.h"
#include "net/sf1r/distributed/Sf1DistributedDriver.hpp"
#include <boost/thread.hpp>
#include <string>
#include <csignal>

using namespace NS_IZENELIB_SF1R;
using namespace std;


bool stop = false;

void handler(int) {
    stop = true;
    cout << "waiting for stop ...";
}

void sendrequest(Sf1DriverBase* driver, const string& uri, 
        const string& tokens, string body) {
    try {
        driver->call(uri, tokens, body);
    } catch(runtime_error& e) {
        cout << "error: " << e.what() << endl;
    }
}

int main() {
    const string host = "localhost:2181";
    const Sf1DistributedConfig conf;
    const int PAUSE = 500; // milliseconds
    
    const string uri    = "/documents/search"; 
    const string tokens = "";
          string body   = "{ \"collection\":\"b5mm\","
                          "  \"header\":{\"check_time\":true},"
                          "  \"search\":{\"keywords\":\"america\"},"
                          "  \"limit\":10"
                          "}";
          
    signal(SIGTERM, &handler);
    signal(SIGINT, &handler);
    
    Sf1DriverBase* driver = new Sf1DistributedDriver(host, conf);
    sendrequest(driver, uri, tokens, body);
    
    cout << "CTRL-C to stop" << endl;
    do {
        boost::this_thread::sleep(boost::posix_time::milliseconds(PAUSE));
        sendrequest(driver, uri, tokens, body);
        
        cout << endl << endl;
    } while(not stop);
    
    delete driver;
}
