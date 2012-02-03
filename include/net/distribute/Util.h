#ifndef IZENE_NET_DISTRIBUTE_UTIL_H_
#define IZENE_NET_DISTRIBUTE_UTIL_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <string.h>

using namespace izenelib::util;

namespace net{
namespace distribute{


class Util
{
public:
    /// @brief get local host ip
    /// Support linux version currently
    static bool getLoalHostIp(std::string& localHostIp)
    {
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1) {
           return false;
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL) {
                continue;
            }

            family = ifa->ifa_addr->sa_family;

            if (family == AF_INET || family == AF_INET6) {

               s = getnameinfo(ifa->ifa_addr,
                       (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                             sizeof(struct sockaddr_in6),
                       host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

               if (s != 0) {
                   return false;
               }

               // support ipv4 currently, Fixme
               //printf("%s, %s\n", ifa->ifa_name, host);
               if (family == AF_INET && (strncmp(ifa->ifa_name, "eth", 3) == 0)) // e.g. eth0
               {
                   localHostIp = host;
                   return true;
               }
           }
        }

        return false;
    }
};

}}

#endif /* IZENE_NET_DISTRIBUTE_UTIL_H_ */
