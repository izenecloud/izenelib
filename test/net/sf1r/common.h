/* 
 * File:   common.h
 * Author: Paolo D'Apice
 *
 * Created on February 3, 2012, 2:36 PM
 */

#ifndef COMMON_H
#define	COMMON_H

#include <glog/logging.h>
#include <boost/test/unit_test_suite.hpp>


/** Fixture initializing the logging library. */
struct Glog {
    Glog() { 
        google::InitGoogleLogging("test");
        google::LogToStderr();
    }
    
    ~Glog() { 
        google::ShutdownGoogleLogging(); 
    }
};


BOOST_GLOBAL_FIXTURE(Glog);


#endif	/* COMMON_H */

