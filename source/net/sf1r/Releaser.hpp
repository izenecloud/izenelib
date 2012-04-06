/* 
 * File:   Releaser.hpp
 * Author: Paolo D'Apice
 *
 * Created on March 26, 2012, 11:13 AM
 */

#ifndef RELEASER_HPP
#define	RELEASER_HPP

#include "net/sf1r/config.h"
#include "net/sf1r/Sf1DriverBase.hpp"
#include "RawClient.hpp"

NS_IZENELIB_SF1R_BEGIN

/**
 * Cleaner class for releasing a connection via the RAII idiom.
 * Used in order to reproduce the Java \c finally clause.
 */
class Releaser {
public:
    Releaser(Sf1DriverBase& drv, const RawClient& cli) : driver(drv), client(cli) {}
    
    ~Releaser() {
        driver.release(client);
    }
    
private:
    Sf1DriverBase& driver;
    const RawClient& client;
};

NS_IZENELIB_SF1R_END

#endif	/* RELEASER_HPP */
