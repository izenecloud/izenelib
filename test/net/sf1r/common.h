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
#include <iostream>

/** Fixture initializing the logging library. */
struct Glog {
    Glog() { 
        google::InitGoogleLogging("test");
#if 0
        google::SetLogDestination(google::FATAL, "pippo");
        
        std::cout << "logdirs: ";
        std::vector<std::string> dirs = google::GetLoggingDirectories();
        for (std::vector<std::string>::iterator i = dirs.begin(); i != dirs.end(); ++i)
            std::cout << "  " << *i;
        std::cout << std::endl;
#endif   
        google::LogToStderr();
    }
    
    ~Glog() { 
        google::ShutdownGoogleLogging(); 
    }
};


BOOST_GLOBAL_FIXTURE(Glog);


#endif	/* COMMON_H */

