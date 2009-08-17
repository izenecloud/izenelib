///  @file IDManagerException.h
///  @date 7/14/2008
///  @author QuangNT
///

#ifndef _ID_MANAGER_EXCEPTION_H_
#define _ID_MANAGER_EXCEPTION_H_

#include <iostream>

#include <types.h>

#include "IDManagerErrorString.h"

NS_IZENELIB_IR_BEGIN

namespace idmanager{

    /// @brief This file defines a general exception in the Document Manager
    class IDManagerException
    {
    public:
        /// creat a new IDManagerException given error code, file name, and
        /// line number where the exception is throw
        /// @param
        /// 	errorNo - error code
        /// @param
        ///		errorLine - Line number where the exception occurs
        /// @param
        ///		fileName - file where the exception occurs
        IDManagerException(int errorNo, int errorLine, const std::string& fileName)
        {
        	errorCodeNo_ = errorNo;
        	line_ = errorLine;
        	errorString_ = IDManagerErrorString[errorNo];
        	exceptionLocation_ = fileName;
        }


        /// A destructor, destroy variables if it is neccessary
        virtual ~IDManagerException(){

        }

        /// print out the exception's description to the file
        /// @param
        ///		outputStream - where the error description is written to
        virtual void output(std::ostream& outputStream = std::cerr)
        {
        	time_t rawtime;
        	struct tm * timeinfo;
        	char timebuffer [80];

        	time ( &rawtime );
        	timeinfo = localtime ( &rawtime );
        	strftime (timebuffer,80,"%Y.%m.%d %H:%M:%S",timeinfo);

        	outputStream << timebuffer << ":" << errorString_;
        	outputStream << ": Exception occurs at line " << line_ << " in file ";
        	outputStream <<  exceptionLocation_ << std::endl;
        }

        /// get the error code
        /// @return
        ///		Exception Code
        int getCode(){ return errorCodeNo_;}

        /// get the error string
        /// @return
        ///		string that describes the exception
        virtual const std::string& getString()const {return errorString_;}

    protected:
        /// file where the exception occurs
        std::string exceptionLocation_;

        /// line number where the exception occurs
        int line_;

        /// Error code of the exception
        int errorCodeNo_;

        /// Error string of the exception
        std::string errorString_;

    }; // end - class IDManagerException

} // end - namespace idmanager

NS_IZENELIB_IR_END

#endif

