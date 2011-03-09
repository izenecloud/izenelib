/** 
 * @file ProcMemInfo.h
 * @brief ProcMemInfo class. Part of "sys' library
 * @author MyungHyun (Kent)
 * @date 2008-06-05
 *
 * @brief Revise to get realMem from the 2nd item in /proc/<PID>/statm, and remove the useless procMaxMem.
 * @author Jun
 * @date 2011-03-09
 */

#if !defined(_PROC_MEM_INFO_H_)
#define _PROC_MEM_INFO_H_

#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../types.h"

using namespace std;

NS_IZENELIB_UTIL_BEGIN

/**
 * @brief ProcMemInfo reads the current process' memory usage status
 *
 * ProcMemInfo reads the current process' following setting through /proc/<PID>/statm file:
 *  - Virtual memory(bytes): The amount of virtual memory the process has. It includes all code, 
 *  data and  shared  libraries  plus  pages  that  have  been swapped  out.
 *  - Real memory(bytes): "Resident Set Size(rss): number of pages the process has in real memory, minus 3 for  
 *  administrative  purposes. This is just the pages which count towards text, data, or stack space.  
 *  This does not include pages which have not been demand-loaded in, or which are swapped out." - proc manual
 *
 * Virtual memory is mostly used to know one process' memory usage.
 */
class ProcMemInfo {
public:
	//public methods

	ProcMemInfo() {
	}
	~ProcMemInfo() {
	}

	/**
	 * Opens the statm file and parses the virtual memory, rss in bytes. 
	 * Rss is multiplies by 4K(page size) to get real memory. 
	 * - Virtual memory is located at the 1st in the statm file
	 * - Rss is located at the 2nd item in the statm file
	 *
	 * @param virtualMem The size of virtual memory that the process is currently using.
	 * @param realMem The real memory that this process is actually using.
	 * @exception std::ios_base::failure Throws an exception when an error occures when opening the stat file.
	 *
	 */
	static void getProcMemInfo(unsigned long & virtualMem, unsigned long & realMem)
			throw (ios_base::failure);

	/**
	 * Opens the statm file and parses the virtual memory, rss in bytes. 
	 * Rss is multiplies by 4(page size) to get real memory. 
	 * - Virtual memory is located at the 1st in the statm file
	 * - Rss is located at the 2nd item in the statm file
	 * 
	 * This method allows the user to specify a process id to scan for. 
	 *
	 * @param pid The pid that the user wishes to check the memory of. 
	 * @param virtualMem The size of virtual memory that the process is currently using.
	 * @param realMem The real memory that this process is actually using.
	 * @exception std::ios_base::failure Throws an exception when an error occures when opening the stat file.
	 *
	 */
	static void getProcMemInfo(pid_t pid, unsigned long & virtualMem, unsigned long & realMem)
			throw (ios_base::failure);

private:
	//private methods

	/**
	 * Opens the "statm" file for the current/specified process and reads the content onto a buffer.
	 *
	 * @param path Path to the stat file.
	 * @param buffer A buffer to hold the results
	 */
	static void getStatFile(const string path, string & buffer)
			throw (ios_base::failure);

	/**
	 * Reads the Resident Set Size(rss), and virtual memory information from the statm file into the member variables.
	 *
	 * @param buffer The buffer which contains the stat file in string format.
	 */
	static void readProcStatus(const string & buffer,
			unsigned long & virtualMem, unsigned long & realMem);

private:
//private member variables

};

NS_IZENELIB_UTIL_END
#endif
