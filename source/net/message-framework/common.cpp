/******************************************************************
Developer: QuangNT
Date: 21 May 2007
File: common.cpp
Description: this file declares functions, structs, and classes commonly
used by many files in the projects.
******************************************************************/

#include <net/message-framework/common.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>

void WriteToLog(const char* filename, const char* szMsg)
{
	char szMessage[1024];
	char timebuffer [80];

	struct timeval tv;
	struct timezone tz;
	struct tm* tm;

	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);
	sprintf(timebuffer, "%d:%02d:%02d %ld", tm->tm_hour, tm->tm_min,
				tm->tm_sec, tv.tv_usec);

	if(strlen(szMsg) > 500)
	{
		char szTemp[1024];
		memcpy(szTemp, szMsg, 500);
		szTemp[500] = 0;
		sprintf(szMessage, "%s: %s\n", timebuffer, szTemp);
		
	}else
		sprintf(szMessage, "%s: %s\n", timebuffer, szMsg);

	FILE* pFile = NULL;

	pFile = fopen(filename, "a");
	if(pFile)
	{
		fwrite(szMessage, 1, strlen(szMessage), pFile);
		fclose(pFile);
	}
}

