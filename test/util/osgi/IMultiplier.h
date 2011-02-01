#ifndef IMULTIPLIER_H
#define IMULTIPLIER_H

#include <util/osgi/IService.h>

using namespace izenelib::osgi;

class IMultiplier : public IService
{
public:
    virtual ~IMultiplier(){}

    virtual int multiply( int x, int y ) = 0;
};

#endif

