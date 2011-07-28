/*
 * \file strutil.h
 * \author vernkin
 *
 * \date December 5, 2008, 11:00 AM
 */

#ifndef _STRUTIL_H
#define	_STRUTIL_H
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <functional>
#include <string.h>
#include <types.h>

NS_IZENELIB_AM_BEGIN

using namespace std;

#define SPACES " \t\r\n"

inline string trimRight (const string & s, const string & t = SPACES)
{
    string d (s);
    string::size_type i (d.find_last_not_of (t));

    if (i == string::npos)
        return "";
    else
        return d.erase (d.find_last_not_of (t) + 1);
}

inline void trimRightSelf (string & d, const string & t = SPACES)
{
    string::size_type i (d.find_last_not_of (t));

    if (i != string::npos)
        d.erase (d.find_last_not_of (t) + 1);
}

inline string trimLeft (const string & s, const string & t = SPACES)
{
    string d (s);
    return d.erase (0, s.find_first_not_of (t));
}

inline void trimLeftSelf (string & d, const string & t = SPACES)
{
    d.erase (0, d.find_first_not_of (t));
}


inline string trim (const string & s, const string & t = SPACES)
{
    string d (s);
    return trimLeft (trimRight (d, t), t);
}

inline void trimSelf (string & d, const string & t = SPACES)
{
    trimRightSelf (d, t);
    trimLeftSelf (d, t);
}

inline void normalizeSpace(string& s)
{
    string norm;

    size_t i = 0;
    size_t n = s.length();

    // remove head spaces
    while (i < n && s[i] == ' ')
        i ++;

    while (i < n)
    {
        if (s[i] != ' ')
        {
            norm.push_back(s[i++]);
        }
        else
        {
            // remove end spaces, change middle spaces to one
            while (i < n && s[i] == ' ')
                i ++;
            if (i < n) norm.push_back(' ');
        }
    }

    s.swap(norm);
}

// returns a lower case version of the string
inline string toLower (const string & s)
{
    string d (s);
    transform (d.begin (), d.end (), d.begin (), (int(*)(int)) tolower);
    return d;
}

// returns a lower case version of the string
inline void toLowerSelf(string & d)
{
    //transform (d.begin (), d.end (), d.begin (), (int(*)(int)) tolower);

    string::iterator itr = d.begin();
    size_t size = d.length();
    for(size_t i = 0; i < size; ++i)
    {
        *itr = tolower( *itr );
        ++itr;
    }

/*
    size_t size = d.length();
    for(size_t i = 0; i < size; ++i)
    {
        d[i] = tolower(d[i]);
    }*/
}

// returns an upper case version of the string
inline string toUpper (const string & s)
{
    string d (s);
    transform (d.begin (), d.end (), d.begin (), (int(*)(int)) toupper);
    return d;
}

// transformation function for toCapitals that has a "state"
// so it can capitalise a sequence
class fCapitals : public unary_function<char,char>
{
    bool bUpper;

public:

    // first letter in string will be in capitals
    fCapitals () : bUpper (true) {}; // constructor

    char operator() (const char & c)
    {
        char c1;
        // capitalise depending on previous letter
        if (bUpper)
            c1 = toUpper (c+"").at(0);
        else
            c1 = toLower (c+"").at(0);

        // work out whether next letter should be capitals
        bUpper = isalnum (c) == 0;
        return c1;
    }
};

// returns a capitalized version of the string
inline string toCapitals (const string & s)
{
    string d (s);
    transform (d.begin (), d.end (), d.begin (), fCapitals ());
    return d;
}

// replace first str2 in str1 with str3
inline bool replace(string& str1, const string& str2, const string& str3)
{
    if (str1.size() == 0 || str2.size() == 0)
        return false;

    if (str1.size() < str2.size())
        return false;

    size_t pos = str1.find(str2);
    if (pos == string::npos)
        return false;
    str1.replace(pos, str2.size(), str3);
    return true;
}

// replace first str2 in str1 with str3
inline bool replaceAll(string& str1, const string& str2, const string& str3)
{
    if(str2 == str3)
        return false;
    unsigned int count = 0;

    while (replace(str1, str2, str3))
        ++count;

    return count != 0;
}

// split a line into the first word, and rest-of-the-line
inline string getWord (string& s, const string delim = " ", const bool trim_spaces = true)
{
    // find delimiter
    string::size_type i (s.find (delim));

    // split into before and after delimiter
    string w (s.substr (0, i));


    if (i == string::npos) // if no delimiter, remainder is empty
        s.erase ();
    else // erase up to the delimiter
        s.erase (0, i + delim.size ());

    // trim spaces if required
    if (trim_spaces)
    {
        w = trim (w);
        s = trim (s);
    }

    // return first word in line
    return w;

}

// To be symmetric, we assume an empty string (after trimming spaces)
// will give an empty vector.
// However, a non-empty string (with no delimiter) will give one item
// After that, you get an item per delimiter, plus 1.
// eg.  ""      => empty
//      "a"     => 1 item
//      "a,b"   => 2 items
//      "a,b,"  => 3 items (last one empty)

inline void stringToVector (const string s, vector<string> & v, const string delim = " ", const bool trim_spaces = true)
{
    // start with initial string, trimmed of leading/trailing spaces if required
    string s1 (trim_spaces ? trim (s) : s);

    // ensure vector empty
    v.clear ();

    // no string? no elements
    if (s1.empty ())
        return;

    // add to vector while we have a delimiter
    while (!s1.empty () && s1.find (delim) != string::npos)
        v.push_back (getWord (s1, delim, trim_spaces));

    // add final element
    v.push_back (s1);
}

// Takes a vector of strings and converts it to a string
// like "apples,peaches,pears"
// Should be symmetric with stringToVector (excepting any spaces that might have
//  been trimmed).

inline string vectorToString (const vector<string> & v, const string delim = " ")
{
    // vector empty gives empty string
    if (v.empty ())
        return "";

    // for copying results into
    ostringstream os;

    // copy all but last one, with delimiter after each one
    copy (v.begin (), v.end () - 1,
          ostream_iterator<string> (os, delim.c_str ()));

    // return string with final element appended
    return os.str () + *(v.end () - 1);
}

inline string depTrim (const string & s, const string & t = SPACES)
{
	string ret;
	vector<string> v;
	stringToVector(s, v, t, true);

	bool add_space = false;
	vector<string>::iterator idx = v.begin();
	vector<string>::iterator end = v.end();
	for(; idx!=end; ++idx)
	{
		ret += *idx;

		if(add_space)
			ret += " ";
		else
			add_space = true;
	}

	return ret;
}

inline void tokenizeAndLowerCase(vector<string>& vec, string& text, const char* delimiter){
    string::size_type last = 0;
    string::size_type next = 0;
    string::size_type dellen = strlen(delimiter);
    string::size_type strlen = text.size();
    while(last < strlen){
        next = text.find_first_of(delimiter, last);
        if(next == string::npos){
            next = strlen;
        }
        string tmp = text.substr(last, next-last);
        trimSelf(tmp);
        //normalizeSpace(tmp);
        if(tmp.size() > 0){
            toLowerSelf(tmp);
            vec.push_back(tmp);
        }
        last = next + dellen;
    }

}

static const string PUNTCUATION = " ,./<>?;':\"[]\\{}|~!@#$%^&*()_+`-=";

inline size_t computeLength(const string& str, bool isWordLan){
    if(!isWordLan)
        return str.size();
    unsigned int count = 0;
    bool notpunc = false; //true if occur one notpunc
    for(string::const_iterator itr = str.begin(); itr != str.end(); ++itr){
        if(PUNTCUATION.find_first_of(*itr, 0) != string::npos){
            if(notpunc){
                ++count;
                notpunc = false;
            }

            ++count;
        }else{
            notpunc = true;
        }
    }

    if(notpunc){
        ++count;
    }

    return count;
}

inline vector<size_t>* findTokens(const string& str, bool isWordLan){
    vector<size_t> *ret = new vector<size_t>();
    ret->push_back(0);
    size_t len = str.length();
    if(!isWordLan){
        for(size_t i=0;i<len;++i){
            ret->push_back(i);
        }
        return ret;
    }

    bool notpunc = false; //true if occur one notpunc
    size_t length = str.length();
    for(size_t i=0; i<length; ++i){
        if(PUNTCUATION.find_first_of(str[i], 0) != string::npos){
            if(notpunc){
                ret->push_back(i-1);
                notpunc = false;
            }
            ret->push_back(i);
        }else{
            notpunc = true;
        }
    }

    if(notpunc){
        ret->push_back(length-1);
    }
    return ret;
}

NS_IZENELIB_AM_END

#endif	/* _STRUTIL_H */

