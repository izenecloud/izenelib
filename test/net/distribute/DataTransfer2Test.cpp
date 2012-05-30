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
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

using namespace NS_IZENELIB_DISTRIBUTE;
using namespace boost::filesystem;
using boost::tuple;

namespace {
const path BASEPATH = temp_directory_path();
const path DESTPATH = temp_directory_path()/path("DataReceiver2");
const string   HOST = "localhost";
const unsigned PORT = 8121;
}

#define PATH(X) BASEPATH/path(X)

// Global fixture: creates test files.
struct Files {
    Files() {
        createFile(PATH("testfile"));
        createFile(PATH("tempdir/testfile"), true);
        createFile(PATH("tempdir/nesteddir/testfile"), true);
    }
    ~Files() {
        removeFile(PATH("testfile"));
        removeFile(PATH("tempdir"));
    }
private:
    void createFile(const path& filepath, const bool& createDir = false, 
            const size_t& filesize = 1024 * 1024) {
        BOOST_TEST_MESSAGE("creating file " << filepath);
        if (createDir) {
            path parent = filepath.parent_path();
            BOOST_TEST_MESSAGE("creating parent dir: " << parent);
            create_directories(parent);
            BOOST_ASSERT(exists(parent));
        }
        bfs::ofstream ofs(filepath);
        for (size_t i = 0; i < filesize; i++) {
            ofs << random(); // random data
        }
        BOOST_ASSERT(exists(filepath) and is_regular_file(filepath)
                and not bfs::is_empty(filepath));
    }
    void removeFile(const path& filepath) {
        BOOST_TEST_MESSAGE("deleting file " << filepath);
        remove_all(filepath);
        BOOST_ASSERT(not exists(filepath));
    }
};

BOOST_GLOBAL_FIXTURE(Files);


// Test fixture: start and stop the server
struct Receiver {
    Receiver() : server(DESTPATH.string(), PORT) {
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


BOOST_FIXTURE_TEST_CASE(send_file, Receiver) {
    DataTransfer2 client(HOST, PORT);
    
    typedef tuple<string, string> input_t;
    const input_t values[] = {
        input_t("testfile"),                     // default destination
        input_t("testfile", ""),                 // empty destination
        input_t("testfile", "dest"),             // without trailing slash
        input_t("testfile", "dest/"),            // with trailing slash
        input_t("tempdir/testfile", "dest"),         // nested input 
        input_t("tempdir/testfile", "dest/nested"),  // nested input and dest
    };
    
    BOOST_FOREACH(const input_t& input, values) {
        const string& file = input.get<0>();
        const string& dest = input.get<1>();

        const path filepath = PATH(file); // in basedir
        const path destpath = path(dest);
        BOOST_TEST_MESSAGE("file: " << filepath << ", dest: " << destpath);
        
        const path output = DESTPATH/destpath/filepath; // actual copied file
        BOOST_TEST_MESSAGE("output: " << output);
        BOOST_WARN(not exists(output));
        
        BOOST_REQUIRE(client.syncSend(filepath.string(), destpath.string()));
        BOOST_REQUIRE(exists(output) and is_regular_file(output));
        
        BOOST_WARN(remove_all(output) > 0); // cleanup
    }
    // wait for stdout
    sleep(1);
}


BOOST_FIXTURE_TEST_CASE(send_dir, Receiver) {
    DataTransfer2 transfer(HOST, PORT);
    
    typedef tuple<string, string, bool> input_t;
    const input_t values[] = {
        input_t("tempdir"),                         // default destination
        input_t("tempdir", ""),                     // empty destination
        input_t("tempdir", "newdest"),              // without trailing slash
        input_t("tempdir", "newdest/"),             // with trailing slash
        input_t("tempdir", "newdest/", true),       // recursive
    };
    
    BOOST_FOREACH(const input_t& input, values) {
        const string& dir = input.get<0>();
        const string& dest = input.get<1>();
        const bool& recursive = input.get<2>();
        
        const path dirpath = PATH(dir); // in basedir
        const path destpath = path(dest);
        BOOST_TEST_MESSAGE("dir: " << dirpath << ", dest: " << destpath);

        const path output = DESTPATH/destpath/dirpath; // actual copied file
        BOOST_TEST_MESSAGE("output: " << output);
        BOOST_WARN(not exists(output));
        
		BOOST_REQUIRE(transfer.syncSend(dirpath.string(), destpath.string(), recursive));
        BOOST_REQUIRE(exists(output) and is_directory(output));
        
        BOOST_WARN(remove_all(output) > 0); // cleanup
    }
    // wait for stdout
    sleep(1);
}

#else

BOOST_AUTO_TEST_CASE(dummy) {
    // avoid test failure due to empty test tree
}

#endif