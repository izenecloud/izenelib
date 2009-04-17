///  @file IDManagerException.h 
///  @date 7/14/2008
///  @author QuangNT
///

#ifndef _ID_MANAGER_EXCEPTION_H_
#define _ID_MANAGER_EXCEPTION_H_

#include <iostream>

namespace idmanager 
{

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
        IDManagerException(int errorNo, int errorLine, const std::string& fileName);

        /// A destructor, destroy variables if it is neccessary
        virtual ~IDManagerException();

        /// print out the exception's description to the file
        /// @param
        ///		outputStream - where the error description is written to
        virtual void output(std::ostream& outputStream = std::cerr);

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

} // end - namespace sf1v5

#endif

