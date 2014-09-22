///
/// @file   IPRestrictor.h
/// @brief  Header file of IPRestrictor class.
/// @author Dohyun Yun
/// @date   2009.11.19
///

#ifndef _IPRESTRICTOR_H_
#define _IPRESTRICTOR_H_

#include <util/singleton.h>

#include <boost/asio/detail/array.hpp>
#include <iostream>
#include <vector>

namespace izenelib
{
namespace driver
{


///
/// @details
/// - With this class, following list is possible.
///     -# Register allowing IP and deny IP in any class type (A,B,C or normal type.
///        D class is called as a full ip address type in this source)
///     -# Determine given IP is allowable by looking at the table.
/// - Rule of IP restriction.
///     - Detail level (Low -> High) : Class A < Class B < Class C < Class D
///     - If there are some collisions in deny IP and allow IP list,
///         - Higher detail leveled class has priority. For example, given IP is
///           (1.1.1.1) is denied with allow List(1.1, 132.23.1) and deny List(1.1.1,
///           23.1.2.4). Because the deny List Level is class C and allow List Level is class B.
///     - If the level of them is same, given IP is denied.
///     - If none of them matches, given IP is allowed.
/// - Speed : IP matching is based on the byte comparing so it is fast.
///
class IPRestrictor
{
public:
    ///
    /// @brief One byte indicates one ip in IPV4.
    ///
    typedef boost::asio::detail::array<unsigned char,4> IP_BYTES_TYPE;

    ///
    /// @brief List of IP_BYTES_TYPE to store class A[0] ~ D[3].
    ///
    typedef boost::asio::detail::array<std::vector<IP_BYTES_TYPE> ,4> IP_LIST_TYPE;

private:
    static const int    UNKNOWN_IP_TYPE = 0; ///< Unknown IP type.
    static const int    CLASS_A_TYPE    = 1; ///< Class A : XXX
    static const int    CLASS_B_TYPE    = 2; ///< Class B : XXX.XXX
    static const int    CLASS_C_TYPE    = 3; ///< Class C : XXX.XXX.XXX
    static const int    CLASS_D_TYPE    = 4; ///< Class D : XXX.XXX.XXX.XXX

public:
    static IPRestrictor* getInstance()
    {
        return ::izenelib::util::Singleton<IPRestrictor>::get();
    }

    // -------------------------------------------- [ Interfaces ]
    ///
    /// @brief an interface which clears all member variables.
    ///
    void clear();

    ///
    /// @brief a member function to set allow ip.
    /// @param  ipAddress   string object which contains allow ip.
    /// @return true        sucess to store allow ip.
    /// @return false       invalid format of ip.
    ///
    bool registerAllowIP(const std::string& ipAddress);

    ///
    /// @brief a member function to set reject ip.
    /// @param  ipAddress   string object which contains reject ip.
    /// @return true        sucess to store ip.
    /// @return false       invalid format of ip.
    ///
    bool registerDenyIP(const std::string& ipAddress);

    ///
    /// @brief a member function to check whether given IP address can be allowed or not.
    /// @details    If matched levels of allow and deny list is equal to 0, it will return true(allow).
    ///             If allow level is higher than deny level, it will return true  (allow).
    ///             If deny level is higher than allow level, it will return false (deny).
    ///             If allow level is same as the deny level, it will return false (deny).
    /// @param  ipByte      bytes object to contain ip address.
    /// @return true        Given IP is allowed.
    /// @return false       Given IP is denied.
    ///
    bool isThisIPAllowed(const IP_BYTES_TYPE& ipByte);


    // -------------------------------------------- [ Debug ]

    ///
    /// @brief an interface which displays all member variables.
    ///
    void print();

    ///
    /// @brief an interface which returns the value of one list as a string type.
    /// @param  message     Title of list group.
    /// @param  ipListGroup A list of IP address which is displayed.
    /// @return String value of ip list.
    ///
    std::string printListGroupToString(const std::string& message, const IP_LIST_TYPE& ipListGroup);

    ///
    /// @brief a member function to set reject ip.
    /// @param  ipAddress   string object which contains reject ip.
    /// @param  result      byte array which contains ip address.
    /// @return true        sucess to parse.
    /// @return false       fail to parse.
    ///
    int parseIPStringToBytes(const std::string& ipAddress, IP_BYTES_TYPE& result) const;

private:

    ///
    /// @brief This member function write given ip to the proper ip list.
    /// @param  type        Range of type value (CLASS_A_TYPE  ~ CLASS_D_TYPE). It will
    ///                     indicates where( [type-1] in the list ) and how many numbers
    ///                     (as many as type value) to store.
    /// @param  listGroup   Target ip list object to store ipByte.
    /// @param  result      Byte form ipAddress which is stored into listGroup.
    /// @return true        Success to store.
    /// @return false       The value of type is out of range.
    ///
    bool addToList(int type, IP_LIST_TYPE& listGroup, IP_BYTES_TYPE& result);

    ///
    /// @brief This interface checks if the input IP address in the given list. It will
    ///        return the matched level as high as possible.
    /// @details if allow IP list has these value( 123.* , 123.123.* ) and given ip va-
    ///          lue is 123.123.0.1, it will return 2 because matched highlevel is 2.
    ///          (123.123.*)
    /// @return 0           Not in the list
    /// @return 1~4         Return high matched level. ClassA level(1)~ ClassD level(4).
    ///
    int isThisIPInList(const IP_LIST_TYPE& classAList, const IP_BYTES_TYPE& ipByte);


private:

    IP_LIST_TYPE allowIPList_;  ///< Allow IP list [0] : Class A list ~ [3] : Class D list.
    IP_LIST_TYPE denyIPList_;   ///< Deny  IP list [0] : Class A list ~ [3] : Class D list.

}; // end - class IPRestrictor

}
}

#endif // _IPRESTRICTOR_H_
