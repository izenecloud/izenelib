#ifndef ISERVICE_B_H
#define ISERVICE_B_H

#include <util/osgi/IService.h>

using namespace izenelib::osgi;

class IServiceB : public IService
{
public:
    virtual ~IServiceB(){}
    virtual int getValue() = 0;
};
#endif

