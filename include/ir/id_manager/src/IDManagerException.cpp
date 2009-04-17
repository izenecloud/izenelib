// IDManagerException.cpp: implementation of the CIDManagerException class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <IDManagerException.h>
#include <IDManagerErrorString.h>

namespace idmanager {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*******************************************************************************
Input:
	nErrorID: error code of the exception
	nInputLine: the line number where the exception is thrown
	szInputFile: the file name where the exception is thrown
Output: none
Description: creat a new IDManagerException given error code, file name
and line number where the exception is thrown
*******************************************************************************/
IDManagerException::IDManagerException(int errorNo, 
					int errorLine, const std::string& fileName)
{
	errorCodeNo_ = errorNo;
	line_ = errorLine;
	errorString_ = IDManagerErrorString[errorNo];
	exceptionLocation_ = fileName;
}

/*******************************************************************************
Destructor, destroy variables if it is neccessary
*******************************************************************************/
IDManagerException::~IDManagerException()
{
}

/*******************************************************************************
Function:
	void IDManagerException::(ostream& outputStream)
Input:
	file - file where the exception's description is written to
Description: the function writes description of the exception ino a file
*******************************************************************************/
void IDManagerException::output(std::ostream& outputStream)
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

} // end - namespace sf1v5
