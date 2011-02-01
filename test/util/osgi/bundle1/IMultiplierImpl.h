#ifndef IMULTIPLIER_IMPL_H
#define IMULTIPLIER_IMPL_H

#include <iostream>

#include "../IMultiplier.h"

using namespace std;

class IMultiplierImpl : public IMultiplier
{
public:
    virtual int multiply( int x, int y );
};

int IMultiplierImpl::multiply( int x, int y )
{
    cout << "[IMultiplierImpl#multiply] Called." << endl;
    return x*y;
}

#endif
