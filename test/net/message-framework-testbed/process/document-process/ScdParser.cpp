#include "ScdParser.h"

#include <fstream>
#include <iostream>

using namespace std;


namespace sf1v5_dummy
{

    void parseScdFile( const char * filePath, std::vector<Document> & docList, int max )
    {
        ifstream fin( filePath );

        char    buf[1024];
        string  str;
        bool    bContent = false;   //whether the line belongs to <Content>
                                    //only <Content> is multiline

        string title;
        string content;

        Document document;

        int num = 0;
   
        //int max = 100;

        memset(buf, 0, 1024);

        while( !fin.eof() )
        {
            fin.getline( buf, 1024 );

            if( buf[ strlen(buf) -1] == '\r' )
            {
                buf[ strlen(buf) -1 ] = '\0';
            }

            str = buf;

            if( str.find( "<DOCID>" ) != string::npos )
            {
                if( bContent )
                {
                    bContent = false;

                    document.clear();
                    //if is set by DocumentManager
                    document.setTitle( title );
                    document.setContent( content );

                    docList.push_back( document );
                    num ++;
                    if(num >max)
                    	break;
                }
            }
            else if( str.find( "<Title>" ) != string::npos )
            {
                title = str.substr( strlen("<Title>") );
            }
            else if( str.find( "<Content>" ) != string::npos )
            {
                content = str.substr( strlen( "<Content>") );
                
                //from here, the text belongs to <Content>
                bContent = true;
            }
            else
            {
                if( bContent )
                {
                    content += str;
                }
            }
        }

        fin.close();
    }

}
