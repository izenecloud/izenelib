#include <LAManager.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace sf1v5_dummy
{
    LAManager::LAManager()
    {
    }

    void LAManager::parseString( const std::string & text, std::vector<std::string> & termList )
    {
        stringstream ss(text);

        // a temporary storage
        string str;
       // cout<<"parseString:"<<endl;

        while( !ss.eof() )
        {
            ss >> str;
       //     cout<<str<<endl;
            termList.push_back( str );
        }
    }
}
