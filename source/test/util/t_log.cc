#define IZENE_LOG

#define DBG2CONSOLE



#include <iostream>
#include <util/log.h>

using namespace std;

USING_IZENE_LOG();


int main()
{

	LDBG_ << "Output debug information!";
	LERR_ << "Output error information!";
	LAPP_ << "Output application information!";
	
    return 0;
}
