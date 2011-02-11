/**
 * @file    utils.h
 * @brief   
 * @author  MyungHyun Lee (Kent)
 * @date    Feb 02, 2009
 */


#ifndef _DUMMY_UTILS_H_
#define _DUMMY_UTILS_H_

#include <boost/thread.hpp>

inline boost::system_time delay(unsigned int milliseconds)
{
	boost::system_time wakeupTime = boost::get_system_time() + boost::posix_time::milliseconds(milliseconds);
	return wakeupTime;
}

#endif  //_DUMMY_UTILS_H_
