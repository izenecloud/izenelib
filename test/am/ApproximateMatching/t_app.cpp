#include <boost/date_time/posix_time/posix_time.hpp>
#include <am/ApproximateMatching/MatchIndex.h>

#include <util/ustring/UString.h>
//#include <common/ScdParser.h>
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
int i=0;
/*
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

    ScdParser parser(izenelib::util::UString::UTF_8);
    parser.load(scd_file);
    uint32_t n=0;
    for( ScdParser::iterator doc_iter = parser.begin();
            doc_iter!= parser.end(); ++doc_iter, ++n)
    {

        if(n%10==0)
        {
            //LOG(INFO)<<"Find Documents "<<n<<std::endl;
        }
        if(n>docnum)
        {
            break;
        }
        Document doc;
        SCDDoc& scddoc = *(*doc_iter);
        SCDDoc::iterator p = scddoc.begin();

        for(; p!=scddoc.end(); ++p)
        {
              const std::string& property_name = p->first;
              if(property_name=="Title")
              {
                      Mi.Add(p->second);
              };
        }

     }
*/
/*
Mi.BuildIndex();
Mi.Match(UString("酒鬼酒 五星湘魂酒 馥郁香型白酒 52度 500ml",UString::UTF_8),4);
  /*

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
/**/
out.open("outputd",ios::out);
in.close();


    if(!match_file.empty())
    {
        //cout<<"asdasdd"<<endl;
        in.open(match_file.c_str(),ios::in);
        string title;

        while( getline(in, title))
        {
             //cout<<title<<endl;
             {
                  out<<title<<endl;
                  vector<UString> can=Mi.Match(UString(title,UString::UTF_8),5);
                  for(unsigned i=0;i<can.size();i++)
                  {  
                       out<<toString(can[i])<<endl;
                  }
             }
             out<<endl;
        }


    }

/*
 vector<UString> can=Mi.Match(UString("Lenovo/联想 A210",UString::UTF_8),3);
                  for(unsigned i=0;i<can.size();i++)
                  {  
                       cout<<toString(can[i])<<endl;
                  }
*/
    time_now = boost::posix_time::microsec_clock::local_time(); 
    cout<<"MatchEnd"<<boost::posix_time::to_iso_string(time_now)<<endl;

//Mi.show();

}

/*
 
*/
