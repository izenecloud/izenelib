#include <am/vsynonym/QueryNormalize.h>

NS_IZENELIB_AM_BEGIN

QueryNormalize::QueryNormalize()
{
    count_ = 0;
    filePath_ = "./QUERYNORMALIZE";
    keyStr_ = new string[MAX_NORMALIZE_SIZE];
    valueStr_ = new string[MAX_NORMALIZE_SIZE];
    keyString_ = "";
}

QueryNormalize::~QueryNormalize()
{
    delete [] keyStr_;
    delete [] valueStr_;
}

bool QueryNormalize::query_Normalize(string &str)
{
    if(count_ == 0)
	return false;
    stanrd_raw(str);
    if(str.length() < 3)
        return false;
    string firstthree = str.substr(0, 3);
    if(keyString_.find(firstthree) == string::npos)
    {
        return true;
    }
    else
    {
        string tmp = "";
        uint32_t i, pos;//uint32_t
        for(i = 0; i < count_; i++)
        {
            if(str.find(keyStr_[i]) == 0)
            {
                tmp.append(keyStr_[i]);
                if(tmp.length() == str.length())
                    return true;
                tmp.append(" ");
                break;
            }
        }
        pos = keyStr_[i].length();
        while(str[pos] == ' ')
            pos++;
        string nextpart = str.substr(pos);
        uint32_t len = nextpart.length();
        uint32_t size;
        if( len < MAX_TYPE_LENGTH)
            size = len;
        else
            size = MAX_TYPE_LENGTH;
        uint32_t j;
        for(j = size; j > 0; j--)
        {
            string tmpstr = nextpart.substr(0, j);
            if(isTypeString(valueStr_[i], tmpstr))
            {
                tmp.append(tmpstr);
                if(tmp.length() != str.length() + 1)
                    tmp.append(" ");
                break;
            }
        }
        while(nextpart[j] == ' ')
            j++;
        tmp.append(nextpart.substr(j));
        str = tmp;
    }
    return true;
}

bool QueryNormalize::load(string nameFile)
{
    filePath_ = nameFile;
    fstream in,out;
    const char* path = filePath_.c_str();
    in.open(path, ios::in);
    string strline;
    if(in.good())
    {
	while(!in.eof())
        {
            getline(in, strline);
            if(strline[0] != '#')
            {
                 if(!build(strline));
            }
        }
    }
    else
    {
        return false;
    }
    in.close();
    buildkeystring();
    return true;
}

bool QueryNormalize::build(string& str)
{
    uint32_t len;
    if((len = str.find_first_of('#')) == string::npos)
        return false;
    keyStr_[count_] = str.substr(0, len);
    valueStr_[count_] = str.substr(len + 1);
    count_++;
    return true;
}

void QueryNormalize::buildkeystring()
{
    for(uint32_t i = 0; i < count_; i++)
    {
        string str = keyStr_[i].substr(0, 3);
        keyString_.append(str);
        keyString_.append("#");
    }
}

bool QueryNormalize::isTypeString(const string& valuestring, const string& type)
{
    uint32_t pos = 0;
    uint32_t valuelen = valuestring.length();
    uint32_t i = 0;
    while(i < valuelen)
    {
        if(valuestring[i] != '#')
            i++;
        else
        {
            string subtype = valuestring.substr(pos, i - pos);
            if(subtype == type)
                return true;
            pos = i + 1;
            i++;
        }
    }
    return false;
}

void QueryNormalize::stanrd_raw(string& str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    uint32_t len = str.length();
    char * c = new char[len];
    uint32_t k = 0;
    uint32_t i = 0;
    while(str[i] == ' ')
        i++;
    while(str[len - 1] == ' ')
        len--;
    for(; i < len; i++)
    {
        if(str[i] == ' ')
        {
            if(str[i - 1] != ' ')
            {
                c[k] = str[i];
                k++;
            }
        }
        else
        {
            c[k] = str[i];
            k++;
        }
    }
    string s(c,k);
    str = s;
}

NS_IZENELIB_AM_END