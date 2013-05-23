#include <boost/date_time/posix_time/posix_time.hpp>
#include <am/approximate_matching/MatchIndex.h>

#include <util/ustring/UString.h>

#include <iostream>
#include <map>

using namespace std;
using namespace izenelib::util;
using namespace izenelib::am;
string toString(UString us)
{
    string str;
    us.convertString(str, izenelib::util::UString::UTF_8);
    return str;
}

int main()
{

    MatchIndex Mi;
    string scd_file="SPU.SCD";
    ifstream in;

    /*
    int i=0;
    if(!scd_file.empty())
    {
        in.open(scd_file.c_str(),ios::in);
        string title;


        vector<string> param;
        while( getline(in, title))
        {
            //cout<<line<<endl;
            if(title.find("<Title>")==0)
            {
                i++;
                //if(i>2) break;
                Mi.Add(UString(title.substr(7),UString::UTF_8));
            }

        }


    }
    Mi.BuildIndex();
    //Mi.Match(UString("酒鬼酒 五星湘魂酒 馥郁香型白酒 52度 500ml",UString::UTF_8),4);


    Mi.BuildIndex();
    ofstream doc;
    doc.open("doc",ios::out);
    doc<<Mi;
    doc.close();
    cout<<"save"<<endl;
    Mi.Clear();
   */
    ifstream docin;
    docin.open("doc",ios::in);
    docin>>Mi;
    docin.close();
    cout<<"begin"<<endl;
    string match_file="unmatch";
    ofstream out;
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
    cout<<"MatchBegin"<<boost::posix_time::to_iso_string(time_now)<<endl;
    out.open("outputd",ios::out);
    in.close();


    if(!match_file.empty())
    {
        //cout<<"asdasdd"<<endl;
        in.open(match_file.c_str(),ios::in);
        string title;
        while( getline(in, title))
        {
            {
                vector<UString> can;
                Mi.Match(UString(title,UString::UTF_8),5,can);

                out<<title<<endl;
                for(unsigned i=0; i<can.size(); i++)
                {
                    out<<toString(can[i])<<endl;
                }
                out<<endl;
                /*
                vector<UString> cannaive;
                Mi.NaiveMatch(UString(title,UString::UTF_8),5,cannaive);//Right Test
                if(can.size()!=cannaive.size())
                {
                    out<<"title:"<<title<<"   "<<can.size()<<"    "<<cannaive.size()<<endl;
                    for(unsigned i=0; i<can.size(); i++)
                    {
                        out<<toString(can[i])<<endl;
                    }
                    out<<"naive----------------------------------------------------------"<<endl;
                    for(unsigned i=0; i<cannaive.size(); i++)
                    {
                        out<<toString(cannaive[i])<<endl;
                    }

                }
                */


            }

        }


    }

    /*
    //酷奇 15.4宽屏(16:10)防眩护眼膜 屏幕膜 液晶屏保护膜
    vector<UString> can;
    Mi.Match(UString("酷奇 15.4宽屏(16:10)防眩护眼膜 屏幕膜 液晶屏保护膜",UString::UTF_8),5,can);

    for(unsigned i=0; i<can.size(); i++)
    {
        cout<<toString(can[i])<<endl;
    }
    */

    time_now = boost::posix_time::microsec_clock::local_time();
    cout<<"MatchEnd"<<boost::posix_time::to_iso_string(time_now)<<endl;


    Mi.Show();

}

