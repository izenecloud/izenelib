/* 
 * File:   DataTransfer2Test.cpp
 * Author: Paolo D'Apice
 * 
 * Created on May 22, 2012, 2:34 PM
 */

#define BOOST_TEST_MODULE DataReceiver2
#include <boost/test/unit_test.hpp>

#ifdef ENABLE_TEST

#include "net/distribute/DataReceiver2.hpp"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

using namespace NS_IZENELIB_DISTRIBUTE;
using namespace boost::filesystem;

namespace {
const std::string BASEDIR = "/tmp";
const unsigned PORT = 8121;
}


BOOST_AUTO_TEST_CASE(boost_fs) {
    const string filenames[] = {
        "file",
        "./file",
        "dest/file",
        "dest/./file",
        "dest/./tmp/file"
    };
    
    BOOST_FOREACH(const string& filename, filenames) {
        BOOST_TEST_MESSAGE("filename: " << filename);
        path p(filename);
        BOOST_TEST_MESSAGE("path: " << p);
        
        path parent = p.parent_path();
        if (not parent.empty() and not exists(parent)) {
            create_directories(parent);
        }
        bfs::ofstream output(p);
        BOOST_CHECK(output.good());
        output.write("this is a test", 14);
        output.close();
        remove(p);
    }
}


BOOST_AUTO_TEST_CASE(service_test) {
    DataReceiver2 server(BASEDIR, PORT);
    server.start();
    cout << "Press ENTER to stop" << endl;
    cin.get();
    server.stop();
}

#else

BOOST_AUTO_TEST_CASE(dummy) {
    // avoid test failure due to empty test tree
}

#endif