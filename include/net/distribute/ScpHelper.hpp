/* 
 * File:   ScpHelper.hpp
 * Author: Paolo D'Apice
 *
 * Created on May 30, 2012, 11:48 AM
 */

#ifndef IZENELIB_NET_DISTRIBUTE_SCPHELPER_HPP
#define	IZENELIB_NET_DISTRIBUTE_SCPHELPER_HPP

#include "common.hpp"
#include <boost/format.hpp>
#include <glog/logging.h>
#include <cstdlib>
#include <string>

NS_IZENELIB_DISTRIBUTE_BEGIN

/**
 * @brief Simple adapter for calling the scp command on the system.
 * This class contains only a static method performing a system call
 * to the \c scp command.
 */
class ScpHelper {
public:

    /**
     * Execute the \c scp command.
     * @param source The source file list in the format `[[user@]host1:]file`
     * @param destination The destination in the format `[[user@]host1:]file`
     * @param options A string containing \c scp options. Default: empty
     * @return a status code defined as:
     * \li -2 if there is no available command processor
     * \li 0 on success
     * \li >0 if an error occurs
     * @see \c scp(1)
     * @see \c system(3)
     */
    static int scp(const std::string& source, const std::string& destination,
            const std::string& options = "");
    
private: // prevent instantiation
    ScpHelper() {}
    ~ScpHelper() {}
};

int
ScpHelper::scp(const std::string& source, const std::string& destination,
        const std::string& options) {
    if (system(NULL) == 0) {
        LOG(ERROR) << "No command processor available.";
        return -2;
    }

    boost::format command("scp %1% %2% %3%");
    command % options % source % destination;
    LOG(INFO) << "executing command: " << command;

    int ret = ::system(command.str().c_str());
    DLOG(INFO) << "command returned: " << ret;
    
    return ret;
}

NS_IZENELIB_DISTRIBUTE_END
        
#endif	/* IZENELIB_NET_DISTRIBUTE_SCPHELPER_HPP */
