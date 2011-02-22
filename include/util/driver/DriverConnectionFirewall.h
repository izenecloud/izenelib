#ifndef IZENELIB_DRIVER_CONNECTION_FIREWALL_H
#define IZENELIB_DRIVER_CONNECTION_FIREWALL_H
/**
 * @file util/driver/DriverConnectionFirewall.h
 * @author Ian Yang
 * @date Created <2010-05-28 17:51:48>
 */
#include <boost/asio/ip/address.hpp>

#include <util/driver/IPRestrictor.h>

namespace izenelib {
namespace driver {

struct DriverConnectionFirewall
{
    bool operator()(const boost::asio::ip::address& ipAddress);
    IPRestrictor ipRestrictor;
};

}
}

#endif // IZENELIB_DRIVER_CONNECTION_FIREWALL_H
