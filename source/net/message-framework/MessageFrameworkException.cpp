#include <net/message-framework/MessageFrameworkException.h>
#include <net/message-framework/MessageFrameworkErrorString.h>

#include <stdio.h>
#include <string.h>
#include <time.h>


namespace messageframework
{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*******************************************************************************
Input:
	nErrorID: error code of the exception
	nInputLine: the line number where the exception is thrown
	szInputFile: the file name where the exception is thrown
Output: none
Description: creat a new MessageFrameworkException given error code, file name
and line number where the exception is thrown
*******************************************************************************/
MessageFrameworkException::MessageFrameworkException(int errorNo,
					int errorLine, const std::string& fileName)
{
	errorCodeNo_ = errorNo;
	line_ = errorLine;
	errorString_ = MessageFrameworkErrorString[errorNo];
	exceptionLocation_ = fileName;
//	output(std::cout);
}

/*******************************************************************************
Destructor, destroy variables if it is neccessary
*******************************************************************************/
MessageFrameworkException::~MessageFrameworkException()
{
}

/*******************************************************************************
Function:
	void MessageFrameworkException::(ostream& outputStream)
Input:
	file - file where the exception's description is written to
Description: the function writes description of the exception ino a file
*******************************************************************************/
void MessageFrameworkException::output(std::ostream& outputStream)
{
	time_t rawtime;
	struct tm * timeinfo;
	char timebuffer [80];

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	strftime (timebuffer,80,"%Y.%m.%d %H:%M:%S",timeinfo);

	outputStream << timebuffer << ":" << errorString_.c_str();
	outputStream << ": Exception occurs at line " << line_ << " in file ";
	outputStream <<  exceptionLocation_.c_str() << std::endl;
}
}// end of namespace messageframework

