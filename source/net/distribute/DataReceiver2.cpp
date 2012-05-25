/* 
 * File:   DataReceiver2.cpp
 * Author: Paolo D'Apice
 * 
 * Created on May 24, 2012, 3:03 PM
 */

#include "net/distribute/DataReceiver2.hpp"
#include "net/distribute/Connection.hpp"

#include <boost/bind.hpp>
#include <glog/logging.h>
#include <fstream>


NS_IZENELIB_DISTRIBUTE_BEGIN


DataReceiver2::DataReceiver2(const unsigned& port) 
        : endpoint(ba::ip::tcp::v4(), port), acceptor(service, endpoint),
            signals(service) {
    LOG(INFO) << "DataReceiver2 on port: " << port;
    
    // registering signals
    signals.add(SIGINT);
    signals.add(SIGTERM);
#ifdef SIGQUIT
    signals.add(SIGQUIT);
#endif
    signals.async_wait(boost::bind(&DataReceiver2::handleStop, this));
  
    DLOG(INFO) << "instantiated";
}


DataReceiver2::~DataReceiver2() {
    DLOG(INFO) << "destroyed";
}


void
DataReceiver2::start() {
    LOG(INFO) << "starting data-receiving service ...";
    startAccept();
    LOG(INFO) << "starting loop thread ...";
    runThread = boost::thread(boost::bind(&DataReceiver2::run, this));
}


void
DataReceiver2::stop() {
    LOG(INFO) << "stopping data-receiving service ...";
    acceptor.close();
    service.stop();
    LOG(INFO) << "stopping loop thread ...";
    runThread.join();
}


void
DataReceiver2::run() {
    service.run();
}

void
DataReceiver2::startAccept() {
    // TODO: use a connection pool
    Connection::pointer newConnection = Connection::create(service);
    acceptor.async_accept(newConnection->getSocket(),
            boost::bind(&DataReceiver2::handleAccept, this, newConnection,
            boost::asio::placeholders::error));
}


void
DataReceiver2::handleAccept(Connection::pointer connection,
        const boost::system::error_code& error) {
    if (!error) {
        DLOG(INFO) << "handling incoming connection";
        connection->start();
    }
    
    startAccept();
}


void
DataReceiver2::handleStop() {
    DLOG(INFO) << "Got stop signal";
    stop();
}


NS_IZENELIB_DISTRIBUTE_END
