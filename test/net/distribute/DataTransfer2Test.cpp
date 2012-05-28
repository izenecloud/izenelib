/* 
 * File:   DataTransfer2Test.cpp
 * Author: Paolo D'Apice
 * 
 * Created on May 22, 2012, 2:34 PM
 */

#define BOOST_TEST_MODULE DataTransfer2
#include <boost/test/unit_test.hpp>

#ifdef ENABLE_TEST

#include "net/distribute/DataTransfer2.hpp"
#include "net/distribute/DataReceiver2.hpp"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

using namespace NS_IZENELIB_DISTRIBUTE;
using namespace boost::filesystem;
using namespace std;
using boost::tuple;

namespace {
const string BASEDIR = "/tmp";
const string   HOST = "localhost";
const unsigned PORT = 8121;
}


// Test Fixture.
struct Receiver {
    Receiver() : server(BASEDIR, PORT) {
        server.start();
    }
    
    ~Receiver() {
        server.stop();
    }
    
private:
    DataReceiver2 server;
};


BOOST_AUTO_TEST_CASE(connection_fail) {
    DataTransfer2 client(HOST, PORT);
    BOOST_CHECK_EQUAL(false, client.probe());
}


BOOST_FIXTURE_TEST_CASE(connection_success, Receiver) {
    DataTransfer2 client(HOST, PORT);
    BOOST_CHECK(client.probe());
}


BOOST_AUTO_TEST_CASE(file_error) {
    DataTransfer2 client(HOST, PORT);
    BOOST_CHECK_EQUAL(false, client.syncSend("/path/to/somefile", "tmp"));
}


/// @see Connection
#define DEFAULT_DEST "/tmp"

BOOST_FIXTURE_TEST_CASE(send_file, Receiver) {
    DataTransfer2 client(HOST, PORT);
    
    typedef tuple<string, string> input_t;
    const input_t values[] = {
        input_t("testfile"),                     // default destination
        input_t("testfile", ""),                 // empty destination
        input_t("testfile", "dest"),             // without trailing slash
        input_t("testfile", "dest/"),            // with trailing slash
        input_t("tmp/testfile", "dest"),         // nested input 
        input_t("tmp/testfile", "dest/nested"),  // nested input and dest
    };
    
    BOOST_FOREACH(const input_t& input, values) {
        const string& file = input.get<0>();
        const string& dest = input.get<1>();
        
        const path destpath(dest);
        const path filepath(file);
        
        const path output = path(DEFAULT_DEST)/destpath/filepath;
        BOOST_TEST_MESSAGE("output: " << output);
        
        BOOST_WARN(not exists(output));
        BOOST_REQUIRE(client.syncSend(file, dest));
        BOOST_REQUIRE(exists(output) and is_regular_file(output));
        
        // cleanup
        BOOST_CHECK(remove_all(output) > 0);
    }
    // wait for stdout
    sleep(1);
}


BOOST_FIXTURE_TEST_CASE(send_dir, Receiver) {
    DataTransfer2 transfer(HOST, PORT);
    
    typedef tuple<string, string, bool> input_t;
    const input_t values[] = {
        input_t("tmp"),                         // default destination
        input_t("tmp", ""),                     // empty destination
        input_t("tmp", "newdest"),              // without trailing slash
        input_t("tmp", "newdest/"),             // with trailing slash
        input_t("tmp", "newdest/", true),       // recursive
    };
    
    BOOST_FOREACH(const input_t& input, values) {
        const string& dir = input.get<0>();
        const string& dest = input.get<1>();
        const bool& recursive = input.get<2>();
        
        const path destpath(dest);
        const path dirpath(dir);
        
        const path output = path(DEFAULT_DEST)/destpath/dirpath;
        BOOST_TEST_MESSAGE("output: " << output);
        
        BOOST_WARN(not exists(output));
        BOOST_REQUIRE(transfer.syncSend(dir, dest, recursive));
        BOOST_REQUIRE(exists(output) and is_directory(output));
        
        // cleanup
        BOOST_CHECK(remove_all(output) > 0);
    }
    // wait for stdout
    sleep(1);
}

#else

BOOST_AUTO_TEST_CASE(dummy) {
    // avoid test failure due to empty test tree
}

#endif