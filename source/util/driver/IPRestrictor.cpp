///
/// @file   IPRestrictor.cpp
/// @brief  Source file of IPRestrictor class.
/// @author Dohyun yun
/// @date   2009.11.19
/// @Log
///     - 2009.12.09 SFlog Information added by Deepesh
///

#include <util/driver/IPRestrictor.h>

#include <string>
#include <sstream>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include <cstdio> // sprintf
#include <cstring> // memcpy

using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;

namespace izenelib
{
namespace driver
{
void IPRestrictor::clear()
{
    // boost array doesn't have clear() interface
    allowIPList_[0].clear();
    allowIPList_[1].clear();
    allowIPList_[2].clear();
    allowIPList_[3].clear();

    denyIPList_[0].clear();
    denyIPList_[1].clear();
    denyIPList_[2].clear();
    denyIPList_[3].clear();

} // end - clear()

bool IPRestrictor::registerAllowIP(const std::string& ipAddress)
{
    IP_BYTES_TYPE ipByte;
    int type = parseIPStringToBytes(ipAddress, ipByte);
    if (type == 0) return false;
    this->addToList( type, allowIPList_, ipByte );

    return true;
} // end - registerAllowIP()

bool IPRestrictor::registerDenyIP(const std::string& ipAddress)
{
    IP_BYTES_TYPE ipByte;
    int type = parseIPStringToBytes(ipAddress, ipByte);
    if (type == 0) return false;
    this->addToList( type, denyIPList_, ipByte );
    return true;
} // end - registerDenyIP()

bool IPRestrictor::isThisIPAllowed(const IP_BYTES_TYPE& ipByte)
{
    int allowLevel = this->isThisIPInList(allowIPList_, ipByte);
    int denyLevel  = this->isThisIPInList(denyIPList_, ipByte);

    if ( allowLevel > denyLevel )
        return true;
    else if ( allowLevel < denyLevel )
        return false;
    else  // ( allowLevel == denyLevel )
    {
        if ( allowLevel == 0 )
            return true;
        else
            return false;
    }
} // end - isThisIPAllowed()

void IPRestrictor::print()
{
    stringstream ss;
    ss << this->printListGroupToString( "allow", allowIPList_ );
    ss << this->printListGroupToString( "deny" , denyIPList_  );
    cout << endl << ss.str() << endl;
} // end - print()

std::string IPRestrictor::printListGroupToString(const std::string& message, const IP_LIST_TYPE& ipListGroup)
{
    char classChar('A');
    char ipChar[5];

    stringstream ss;
    for (size_t i = 0; i < ipListGroup.size(); i++)
    {
        ss << message << " " << (char)(classChar + i) << " Class list" << endl;
        std::vector<IP_BYTES_TYPE>::const_iterator iter = ipListGroup[i].begin();
        for (;iter != ipListGroup[i].end(); iter++)
        {
            for (size_t j = 0; j < i + 1; j++)
            {
                sprintf(ipChar, "%d", (*iter)[j]);
                if ( j == 0 )
                    ss << ipChar;
                else
                    ss << '.' << ipChar;
            } // end - for ( ip element )
            ss << endl;
        } // end - for ( ip address )
        ss << endl;
    } // end - for (ip list )
    return ss.str();
} // end - printListGroupToString()


int IPRestrictor::parseIPStringToBytes(const std::string& ipAddress, IP_BYTES_TYPE& result) const
{
    int type = UNKNOWN_IP_TYPE;

    typedef boost::tokenizer< boost::char_separator<char> > tokenizer_type;
    boost::char_separator<char> sep(".");
    tokenizer_type ipTokenizer(ipAddress, sep);

    // Check wrong usage of . in the ipAddress string.
    {
        // empty, and first and last . checking
        if ( ipAddress.empty() || ipAddress[0] == '.' || ipAddress[ ipAddress.length() -1 ] == '.' )
            return false;
        // continuous . checking
        size_t prev_pos(ipAddress.find('.')), pos(prev_pos + 1);
        while ( (pos = ipAddress.find('.', pos)) != std::string::npos )
        {
            if ( (prev_pos + 1) == pos )
            {
                return false;
            }
            prev_pos = pos;
            pos++;
        } // end - while()
    }

    for (tokenizer_type::iterator iter = ipTokenizer.begin(); iter != ipTokenizer.end(); iter++)
    {
        if ( type > 3 )
        {
            return UNKNOWN_IP_TYPE;
        }

        try
        {
            int integerCastValue = boost::lexical_cast<int>(*iter);
            if ( integerCastValue < 0 || integerCastValue > 255 )
            {
                return UNKNOWN_IP_TYPE;
            }
            unsigned char value = static_cast<unsigned char>(integerCastValue);
            result[type++] = value;
        }
        catch (boost::bad_lexical_cast &)
        {
            return UNKNOWN_IP_TYPE;
        }
    }
    return type;
} // end - parseIPStringToBytes

bool IPRestrictor::addToList(int type, IP_LIST_TYPE& listGroup, IP_BYTES_TYPE& result)
{
    if ( type < CLASS_A_TYPE || CLASS_D_TYPE < type )
    {
        return false;
    } // end - if

    IP_BYTES_TYPE ipInfo;
    //memcpy(&ipInfo.elems, &result.elems, type);
	ipInfo = result;
    listGroup[type-1].push_back( ipInfo );

    return true;
} // end - addToList()

int IPRestrictor::isThisIPInList( const IP_LIST_TYPE& listGroup, const IP_BYTES_TYPE& ipByte)
{
    size_t i = listGroup.size();
    do
    {
        i--;
        std::vector<IP_BYTES_TYPE>::const_iterator ipListIter = listGroup[i].begin();
        for (; ipListIter != listGroup[i].end(); ipListIter++)
        {
			for(size_t j = 0; j < i + 1; ++j)
				if((*ipListIter)[j] != ipByte[j])
					return i + 1;
            //if ( !memcmp(ipListIter->elems, ipByte.elems, i+1) )
            //    return i+1;
        } // end - ipListIter
    }
    while (i != 0);// end - listIter

    return 0;
} // end - isThisIPInListA()

}
}

