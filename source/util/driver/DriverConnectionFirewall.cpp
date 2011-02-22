/**
 * @file util/driver/DriverConnectionFirewall.cpp
 * @author Ian Yang
 * @date Created <2010-05-28 17:53:17>
 */
#include <util/driver/DriverConnectionFirewall.h>


namespace izenelib {
namespace driver {

bool DriverConnectionFirewall::operator()(
    const boost::asio::ip::address& ipAddress
)
{
    if (ipAddress.is_v4())
    {
        return ipRestrictor.isThisIPAllowed(
            ipAddress.to_v4().to_bytes()
        );
    }

    // Disable IPv6
    return false;
}

} 
}

