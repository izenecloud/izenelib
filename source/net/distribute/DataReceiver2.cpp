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


DataReceiver2::DataReceiver2(const std::string& dir,
            const unsigned& port, const size_t& size, bool handleSignals) 
        : endpoint(ba::ip::tcp::v4(), port), acceptor(service, endpoint),
            signals(service), basedir(dir), bufferSize(size), running(false) {
    LOG(INFO) << "DataReceiver2 on port: " << port;
    LOG(INFO) << " - base directory: " << dir;

    if (handleSignals) {
        DLOG(INFO) << "installing signal handler";
        // registering signals
        signals.add(SIGINT);
        signals.add(SIGTERM);
#ifdef SIGQUIT
        signals.add(SIGQUIT);
#endif
        signals.async_wait(boost::bind(&DataReceiver2::handleStop, this));
    }
    DLOG(INFO) << "instantiated";
}


DataReceiver2::~DataReceiver2() {
    if (running) {
        stop();
    }
    DLOG(INFO) << "destroyed";
}


void
DataReceiver2::start() {
    startAccept();
    LOG(INFO) << "starting loop thread ...";
    boost::thread(boost::bind(&ba::io_service::run, &service));
    LOG(INFO) << "starting data-receiving service ...";
    running = true;
    LOG(INFO) << "started.";
}


void
DataReceiver2::stop() {
    LOG(INFO) << "stopping data-receiving service ...";
    acceptor.close();
    service.stop();
    running = false;
    LOG(INFO) << "stopped.";
}


void
DataReceiver2::startAccept() {
    Connection::pointer newConnection = Connection::create(service, bufferSize, basedir);
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
