/* 
 * File:   Sf1Demo.cpp
 * Author: Paolo D'Apice
 * 
 * Created on March 14, 2012, 3:22 PM
 */

#include "../common.h"
#include "net/sf1r/distributed/Sf1DistributedDriver.hpp"
#include <string>
#include <csignal>

using namespace NS_IZENELIB_SF1R;
using namespace std;


bool stop = false;

void handler(int) {
    stop = true;
    cout << "waiting for stop ...";
}

int main() {
    const string host = "localhost:2181";
    const Sf1Config conf;
    const int PAUSE = 10;
    
    
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
    
    cout << "CTRL-C to stop" << endl;
    do {
        sleep(PAUSE);
        try {
            string response = driver->call(uri, tokens, body);
            cout << endl << endl << endl;
        } catch(runtime_error& e) {
            cout << "error: " << e.what() << endl;
        }
    } while(not stop);
    
    delete driver;
}
