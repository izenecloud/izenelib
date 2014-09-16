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
    static bool getLocalHostIp(std::string& localHostIp, const std::string& interface = "")
    {
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];

        localHostIp.clear();
        if (getifaddrs(&ifaddr) == -1)
        {
           return false;
        }

        bool ret = false;
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
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
                   ret = false;
                   break;
               }

               // support ipv4 currently, Fixme
               //printf("%s, %s\n", ifa->ifa_name, host);
               if (family == AF_INET)
               {
                   if (strncmp(ifa->ifa_name, interface.c_str(), interface.length()) == 0)
                   {
                       localHostIp = host;
                       ret = true;
                       break;
                   }
                   else if ((strncmp(ifa->ifa_name, "eth1", 4) == 0))
                   {
                       localHostIp = host;
                       ret = true;
                       if (interface.empty())
                       {
                           break; // Usually we use LAN address firstly (eth1)
                       }
                   }
                   else if (!ret && (strncmp(ifa->ifa_name, "eth0", 4) == 0))
                   {
                       localHostIp = host;
                       ret = true;
                   }
               }
           }
        }

        freeifaddrs(ifaddr);
        return ret;
    }
};

}}

#endif /* IZENE_NET_DISTRIBUTE_UTIL_H_ */
