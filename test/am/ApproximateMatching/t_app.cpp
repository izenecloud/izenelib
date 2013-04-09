#include <am/ApproximateMatching/MatchIndex.h>

#include <util/ustring/UString.h>
//#include <common/ScdParser.h>
#include <iostream>
#include <map>
using namespace std;
using namespace izenelib::util;
using namespace izenelib;
string toString(UString us)
{
    string str;
    us.convertString(str, izenelib::util::UString::UTF_8);
    return str;
}

int main()
{

MatchIndex Mi;
string scd_file="unmatch";

ifstream in;
    if(!scd_file.empty())
    {
        in.open(scd_file.c_str(),ios::in);
        string title;


        vector<string> param;
        while( getline(in, title))
        {
            //cout<<line<<endl;
             Mi.add(UString(title,UString::UTF_8));
        }


    }
/*
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
                      Mi.add(p->second);
              };
        }

     }
*/
/*
Mi.BuildIndex();
Mi.Match(UString("酒鬼酒 五星湘魂酒 馥郁香型白酒 52度 500ml",UString::UTF_8),5);
  
*/
UString a("abcdefdsewdda",UString::UTF_8);
UString b("dsadaefe",UString::UTF_8);
UString c("dsadfeffsa",UString::UTF_8);
UString d("fdsfedd",UString::UTF_8);
UString e("ddcsdefre",UString::UTF_8);
cout<<Mi.EditDistance(a,b)<<endl;
Mi.add(a);
Mi.add(b);
Mi.add(c);
Mi.add(d);
Mi.add(e);    
Mi.BuildIndex();

vector<UString> can=Mi.Match(UString("酒鬼酒 五星湘魂酒 馥郁香型白酒 52度 500ml",UString::UTF_8),5);
    for(unsigned i=0;i<can.size();i++)
    {  
         cout<<toString(can[i])<<endl;
    }
//Mi.show();

}

/*
 
*/
