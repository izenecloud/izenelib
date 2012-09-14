#include <am/vsynonym/QueryNormalize.h>

#include <boost/algorithm/string/trim.hpp>

NS_IZENELIB_AM_BEGIN

QueryNormalize::QueryNormalize()
{
    count_ = 0;
    filePath_ = "./QUERYNORMALIZE";
    keyStr_ = new string[MAX_NORMALIZE_SIZE];
    valueStr_ = new string[MAX_NORMALIZE_SIZE];
    keyString_ = "";
    LoadOK_ = false;
}

QueryNormalize::~QueryNormalize()
{
    delete [] keyStr_;
    delete [] valueStr_;
}

bool QueryNormalize::query_Normalize(string &str)
{
    stanrd_raw(str);

    if(count_ == 0)
	return false;
    if(str.length() < 3)
        return false;
    string firstthree = str.substr(0, 3);
    if(LoadOK_)
    {
    if(keyString_.find(firstthree) == string::npos)
    {
        return true;
    }
    else
    {
        string tmp = "";
        uint32_t i, pos;
        for(i = 0; i < count_; i++)
        {
            if(str.length() < keyStr_[i].length())
                continue;
            if(str.find(keyStr_[i]) != string::npos)//check
            {
                tmp.append(keyStr_[i]);
                if(tmp.length() == str.length())
                    return true;
                break;
            }
        }

        if (count_ == i)//only sub, return
        {
            return true;
        }
        
        if(str.length() == keyStr_[i].length())
            return true;
        pos = keyStr_[i].length();
        if(str[pos] == ' ')
            pos++;
        if(str.length() > pos)
        {
            tmp.append(" ");
            string nextpart = str.substr(pos);
            uint32_t len = nextpart.length();//the next must be its type, like "nokia 800c jiage"
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
                    pos += j;
                    tmp.append(tmpstr);
                    if(str.length() > pos)
                        tmp.append(" ");
                    break;
                }
            }
            if(nextpart[j] == ' ')
                j++;
            tmp.append(nextpart.substr(j));
        }
        str = tmp;
    }
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
    size_t count = 0;
    getline(in, strline);
    if(in.good())
    {
	while(!in.eof())
        {
            count++;
            if(count >= MAX_NORMALIZE_SIZE)
            {
                cout<<"[error]: file size is more than MAX_NORMALIZE_SIZE"<<endl;
                LoadOK_ = false;
                return false;
            }
            if(strline.length() > 3)
            {
                if(strline[0] != '#')
                {
                    build(strline);
                }
            }
	    getline(in, strline);
        }
    }
    else
    {
        cout<<"[error]:Load QUERYNORMALIZE FILE error"<<endl;
        LoadOK_ = false;
        return false;
    }
    in.close();
    buildkeystring();
    LoadOK_ = true;
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
        if(keyStr_[i].length() < 3)
            continue;
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
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    boost::algorithm::trim(str);
    if(str.empty()) 
        return;
    size_t i=0;
    size_t j = 0;
    bool lastspace = false; 
    for (; i < str.length(); i++)
    {
        if (str[i] != ' ')
        {
            str[j] = str[i];
            j++;
            lastspace = false;
            continue;
        }

	if(lastspace == false)
        {
             str[j] = str[i];
	     j++;
             lastspace = true;
        }
    }
    str.resize(j);
}

NS_IZENELIB_AM_END
