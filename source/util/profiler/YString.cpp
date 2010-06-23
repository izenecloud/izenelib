
/**
 * @file YString.cc
 *
 * @brief The YString class is an usual YString class, providing
                    almost all the possible operations appliable to
		    character string.

    The original codes were based on 'C++ Components and
                    Algorithms' by Scott Robert Ladd.
 *
 */

#include <util/profiler/YString.h>
#include <util/DArray.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>


using namespace std;
NS_IZENELIB_UTIL_BEGIN

  // class al constant initialization.
  const int TAB_SIZE = 8;

  const YString SPACE_ENCODE = "&-";
  const YString TAB_ENCODE = "&+";
  const YString NEWLINE_ENCODE = "&=";

  const char ENCODE_SPACE_CHAR = '-';
  const char ENCODE_TAB_CHAR = '+';
  const char ENCODE_NEWLINE_CHAR = '=';
  const char ENCODE_CHAR = '&';
  const int DEFAULT_PRECISION = 2;


  void error(const YString& message)
  {
    std::cerr << message << std::endl;
    abort();
  }


  /**
   *
   * @brief  The YString constructor that takes a real also takes a
                  width (which is the number of non-NULL characters)
		  and a precision).

          Note that if the width and/or
		  precisions are 0, the maximum values are used.
         Note that width should not be given a default value of
                  zero.  To do so would ignore one of the original
		  motivations of having extra arguments--namely that
		  construction from a real cannot occur as an accident.
		The extra space in the string is filled with the ' '
		  character.
   *
   */
  YString::YString(double source, int precision, int width)
  {
    stringstream tempNumStream;
    tempNumStream << setprecision(precision);

    if(precision)
      tempNumStream.setf(ios::floatfield, ios::fixed);
    else
      tempNumStream << setprecision(DEFAULT_PRECISION);
    tempNumStream << source;

    *this = tempNumStream.str();
  }


  /**
   * @brief Compare the given YString with the current one.
   */
  StrCompVal YString::compare(StrCompMode caseChk,
			      const YString& str) const

  {
    if (is_null())
      return SC_ERROR;

    if (str.is_null())
      return SC_ERROR;

    int count;
    if (str.length() < length())
      count = str.length();
    else
      count = length();

    char c1, c2;
    int i;

    if (caseChk == SM_IGNORE) {
      for (i = 0; i < count; i++) {
	c1 = (char)tolower((*this)[i]);
	c2 = (char)tolower(str[i]);

	// if characters differ
	if (c1 != c2) {
	  if (c1 < c2)
	    return SC_LESS;
	  else
	    return SC_GREATER;
	}
      }
    }
    else {
      for (i = 0; i < count; i++) {
	c1 = (*this)[i];
	c2 = str[i];

	if (c1 != c2) {
	  if (c1 < c2)
	    return SC_LESS;
	  else
	    return SC_GREATER;
	}
      }
    }

    if (length() == str.length())
      return SC_EQUAL;
    else {
      if (length() < str.length())
	return SC_LESS;
      else
	return SC_GREATER;
    }
  }

  /**
   *  @brief Compare two string after cutting head and tail spaces out.
   */
  bool YString::equal_ignore_case_space(const YString& str) const
  {
    YString self = *this;
    self.compact_tail();
    self.compact_head();
    YString other = str;
    other.compact_tail();
    other.compact_head();
    return self.equal_ignore_case(other);
  }


  /**
   * @brief Substitute to character for from.
   */
  void YString::substitute_char(char from, char to)
  {
    for (size_t i = 0; i < length(); i++)
      if ((*this)[i] == from)
	(*this)[i] = to;
  }


  /**
   * @brief Substitute the to YString for from.
   *
   Substitute the to YString for from. Beware that it will
                  substitute only one time!
		Returns the resulting YString. Aborts if there is no
		  'from' YString.

   *
   */
  YString YString::substitute_string(StrCompMode caseChk,
				     const YString& from,
				     const YString& to) const
  {
    if (from.is_null())
      return *this;

    int pos;
    char* temp = new char[size() + 1 + to.size() - from.size()];
    if (find(caseChk, from, pos)) {
      int cnt = 0;
      int i;
      for (i = 0; i < pos; i++)
	temp[cnt++] = (*this)[i];

      for (i = 0; i < (int)to.size(); i++)
	temp[cnt++] = to[i];

      for (i = pos + from.size(); i < (int)size(); i++)
	temp[cnt++] = (*this)[i];

      temp[cnt] = (char)NULL;
    }
    else {
      if (temp) delete [] temp;
      return *this;
    }

    YString output = temp;
    if (temp) delete [] temp;
    return output;
  }


  /**
   * @brief Substitute the to YString for from.
   *
     Substitute the to YString for from. Beware that it will
                  substitute as many times as necessary.
		Returns the resulting YString.

   *
   */
  YString YString::multi_substitute_string(StrCompMode caseChk,
					   const YString& from,
					   const YString& to) const
  {
    if (from.is_null())
      return *this;

    stringstream buf;
    int start = 0;
    int offset = 0;

    BoyerMoore bm(from);
    while (find_bm(caseChk, bm, offset, start)) {
      if (offset > start)
	buf << cut_range(start, offset - 1);

      buf << to;
      start = offset + from.size();
    }

    if (offset == start)
      return *this;

    if (start < (int)size())
      buf << cut_range(start, size() - 1);

    return YString(buf);
  }



  /**
   * @brief Find a YString, returns true iff found.
   */
  bool YString::find(StrCompMode caseChk,
		     const YString& str,
		     int& pos,
		     int start) const
  {
    // this is too slow.
    if (is_null())
      return false;

    if (str.is_null())
      return false;

    if (length() < str.length())
      return false;

    if (start > high())
      return false;

    bool result;
    int tpPos = 0;

    if (caseChk == SM_IGNORE) {
      char *target = new char[length() + 1 - start];
      memcpy(target, &(*this)[start], length() - start);
      target[length() - start] = (char) NULL;

      char *pattern = new char[str.length() + 1];
      memcpy(pattern, str.c_str(), str.length());
      pattern[str.length()] = (char) NULL;

      strlwr(target, strlen(target));
      strlwr(pattern, strlen(pattern));

      int end = length() - str.length() - start;
      int p, t;

      for (;;) {
	p = 0;
	t = tpPos;

	while ((pattern[p] == target[t])
	       && (pattern[p] != 0)
	       && (target[t] != 0)) {
	  p++;
	  t++;
	}

	if (pattern[p] == 0) {
	  result = true;
	  break;
	}


	if ((target[t] == 0) || (tpPos >= end)) {
	  result = false;
	  break;
	}

	tpPos++;
      }

      if (target) delete [] target;
      if (pattern) delete [] pattern;
    }
    else {
      const char* target = &(*this)[start];
      int end = length() - str.length() - start;
      int p, t;

      for (;;) {
	p = 0;
	t = tpPos;

	while ((str[p] == target[t])
	       && (str[p] != 0)
	       && (target[t] != 0)) {
	  p++;
	  t++;
	}

	if (str[p] == 0) {
	  result = true;
	  break;
	}


	if ((target[t] == 0) || (tpPos >= end)) {
	  result = false;
	  break;
	}

	tpPos++;
      }
    }

    tpPos += start;
    if (result)
      pos = tpPos;
    return result;
  }


  /**
   * @brief The kmp algorithm.
   */
  int KMP_matcher(const char* text, const char* pattern, int n, int m)
  {
    int i, j;

    int* next = new int[m+1];
    next[0] = -1;
    for (i = 0, j = -1; i < m; i++, j++, next[i] = j)
      while ((j>=0) && (pattern[i] != pattern[j]))
	j = next[j];

    for (i = 0, j = 0; j < m && i < n; i++, j++)
      while ((j >= 0) && (text[i] != pattern[j]))
	j = next[j];

    delete [] next;
    if (j == m)
      return i-m;
    else
      return i;
  }


  int KMP_matcher_case_ignore(const char* text, const char* pattern, int n, int m)
  {
    int i, j;
    int* next = new int[m+1];
    next[0] = -1;
    for (i = 0, j = -1; i < m; i++, j++, next[i] = j)
      while ((j>=0) && (tolower(pattern[i]) != tolower(pattern[j])))
	j = next[j];

    for (i = 0, j = 0; j < m && i < n; i++, j++)
      while ((j >= 0) && (tolower(text[i]) != tolower(pattern[j])))
	j = next[j];

    delete [] next;

    if (j == m)
      return i-m;
    else
      return i;
  }


  bool YString::find_kmp(StrCompMode caseChk,
			 const YString& str,
			 int& pos,
			 int start) const
  {
    if (is_null())
      return false;

    if (str.is_null())
      return false;

    if (length() < str.length())
      return false;

    size_t ret;
    if (caseChk == SM_SENSITIVE)
      ret = KMP_matcher(&(*this)[start], str.as_const_char(), length() - start, str.size());
    else
      ret = KMP_matcher_case_ignore(&(*this)[start], str.as_const_char(), length() - start, str.size());

    if (ret < length() - start) {
      pos = ret + start;
      return true;
    }
    else
      return false;
  }


  int YString::find(StrCompMode caseChk,
		    const YString& str,
		    DynamicArray<int>&
		    indices) const
  {
    int index = 0;
    int cnt = 0;
    BoyerMoore bm(str);
    while (find_bm(caseChk, bm, index, index))
      indices[cnt++] = index++;

    return cnt;
  }


  bool YString::find_bm(StrCompMode caseChk,
			const BoyerMoore& bm,
			int& pos,
			int start) const
  {
    if (is_null())
      return false;

    if ((int)length() < bm.m)
      return false;

    int ret;
    if (caseChk == SM_SENSITIVE) {
      ret = bm.find(&(*this)[start], length()-start);
      if (ret != -1) {
	pos = ret + start;
	return true;
      }
      else
	return false;
    }
    else {
      ret = KMP_matcher_case_ignore(&(*this)[start], bm.x, length()-start, bm.m);
      if (ret < (int)length() - start) {
	pos = ret + start;
	return true;
      }
      else
	return false;
    }
  }


  /**
   *  @brief YString deletion method.
   */
  void YString::del(int pos, int count)
  {
    *this = cut_range(0, pos - 1) + cut_range(pos+count, high());
  }


  /**
   * @brief Replace all the occurrences of the given characters with
                   spaces.
   *
    Replace all the occurrences of the given characters with
                   spaces.
   *
   */
  void YString::remove_duplicate_chars(const char ch)
  {
    int count = 0;
    char* temp = new char[size() + 1];
    int cnt = 0;
    size_t i;
    for (i = 0; i < length(); i++) {
      if ((*this)[i] == ch) {
	if (count > 0)
	  (*this)[i] = ' ';
	else
	  temp[cnt++] = ch;
	count++;
      }
      else {
	temp[cnt++] = (*this)[i];
	count = 0;
      }
    }
    *this = temp;
    if (temp) delete [] temp;
  }


  /**
   * @brief YString retrieval method.
   *
    YString retrieval method. Returns the YString from
                    start to (count+start) position.
    Does not affect the receiver itself.
   *
   */
  YString YString::cut(int start, int count) const
  {
    return substr(start, count);
  }


  /**
   * @brief YString retrieval method.
   *
   YString retrieval method. Returns the YString from
                    start to right before the delimiter character.
    Does not affect the receiver itself.
   *
   */
  YString YString::cut_delimiter(int start, char delimiter) const
  {
    size_t i = 0;
    while ((*this)[start + i] != delimiter) {
      i++;
      if (start + i >= length()) {
	return "";
      }
    }
    return cut(start,i);
  }




  /**
   * @brief YString retrieval method.
   *
    YString retrieval method. Returns the YString from
                    the first count characters.
    Does not affect the receiver itself.

   */
  YString YString::cut_head(int count) const
  {
    return substr(0, count);
  }



  /**
   * @brief YString retrieval method.
   *
    YString retrieval method. Returns the YString from
                    the last count characters.
    Does not affect the receiver itself.
   *
   */
  YString YString::cut_tail(int count) const
  {
    return substr(length() - count, count);
  }



  /**
   @brief Cut trailing spaces in the tail.
   */
  void YString::compact_tail()
  {
    if (is_null())
      return;

    int i = high();
    while (isspace((*this)[i])) {
      if (i == 0) {
	release();
	return;
      }
      else
	i--;
    }

    *this = cut_range(0, i);
  }


  void YString::compact_head()
  {
    if (is_null())
      return;

    int i = 0;
    while (isspace((*this)[i])) {
      if (i == high()) {
	release();
	return;
      }
      else
	i++;
    }

    *this = cut_range(i, high());
  }


  bool YString::is_alpha() const
  {
    if (is_null())
      return false;

    for (size_t i = 0; i < size(); i++)
      if (!isalpha((*this)[i]))
	return false;

    return true;
  }

  bool YString::is_digit() const
  {
    if (is_null())
      return false;

    for (size_t i = 0; i < size(); i++)
      if (isdigit((*this)[i]) == 0)
	return false;

    return true;
  }


  bool YString::is_whitespace() const
  {
    if (is_null())
      return false;

    for (size_t i = 0; i < size(); i++)
      if (!isspace((*this)[i]))
	return false;

    return true;
  }

  void YString::read_from_file(const YString& filename)
  {
    long size;
    std::ifstream file (filename.as_const_char(), std::ios::in|std::ios::binary|std::ios::ate);
    if (!file.good()) {
      std::cerr << "YString::read_from_file: file not good: " << filename << std::endl;
      abort();
    }

    size = file.tellg();
    file.seekg (0, std::ios::beg);
    char* buffer = new char [size+1];
    file.read (buffer, size);
    buffer[size] = (char)NULL;
    file.close();

    *this = buffer;
    delete [] buffer;
  }

  void YString::write_to_file(const YString& filename) const
  {
    ofstream file(filename.as_const_char(), ios::out|ios::binary);
    file.write(as_const_char(), size());
    file.close();
  }

  void YString::write_to_file(const YString& filename, const YString& str)
  {
    str.write_to_file(filename);
  }

#define WRITE_TO_FILE(container, arg) \
  void YString::write_to_file(const YString& filename, const container<arg>& lines) \
  { \
    stringstream ss; \
    for (int i = 0; i < (int)lines.size(); i++) \
      ss << lines[i] << endl; \
    write_to_file(filename, ss.str()); \
  }
  WRITE_TO_FILE(Array, YString);
  WRITE_TO_FILE(Array, string);
  WRITE_TO_FILE(vector, YString);
  WRITE_TO_FILE(vector, string);
#undef WRITE_TO_FILE



  /**
   * @brief Conveniece routine to read words in a file into the DynamicArray
                   object.

   */
#define READ_FROM_FILE(container, arg) \
  void YString::read_from_file(const YString& filename, container<arg>& lines) \
  { \
    YString t; \
    t.read_from_file(filename); \
    t.make_tokens('\n', lines, true); \
  }
  READ_FROM_FILE(DynamicArray, YString);
  READ_FROM_FILE(DynamicArray, string);
  READ_FROM_FILE(vector, YString);
  READ_FROM_FILE(vector, string);
#undef READ_FROM_FILE

  /**
   * @brief Read all the names of the files in the specified directory.
   *
       Read all the names of the files in the specified directory.
                    If there are any errors such as no such directory, it
              just returns leaving the reference argument unchanged.
            It discards '.' and '..' files.

   *
   */
#define READ_FILENAMES(container, arg) \
  void YString::read_filenames(const YString& dir, container<arg>& files) \
  { \
    DIR *dirp; \
    struct dirent *direntp; \
    dirp = opendir(dir.as_const_char()); \
    if (dirp == NULL) { \
      cerr << "Warning:YString::read_filenames : invalid directory: " \
	   << dir << endl; \
      return; \
    } \
    while ( (direntp = readdir( dirp )) != NULL ) \
      if (!((strcmp(direntp->d_name, ".") == 0 \
	     || strcmp(direntp->d_name, "..") == 0))) \
	files.push_back(direntp->d_name); \
    (void)closedir( dirp ); \
  }
  READ_FILENAMES(DynamicArray, YString);
  READ_FILENAMES(DynamicArray, string);
  READ_FILENAMES(vector, YString);
  READ_FILENAMES(vector, string);
#undef READ_FILENAMES

  /**
   * @brief Change all the lower case to upper caes in the _text.
   *
    Change all the lower case to upper caes in the _text.
    It forces const char* to char*. Do more research on std::string
   *
   */
  void YString::to_upper()
  {
    const char* ptr = c_str();
    for (unsigned int i = 0; i < length(); i++)
      at(i) = (char)toupper(ptr[i]);
  }


  /**
   * @brief Change all the upper case to lower case in the _text.
   *
    Change all the upper case to lower case in the _text.
    It forces const char* to char*. Do more research on std::string
   *
   */
  void YString::to_lower()
  {
    const char* ptr = c_str();
    for (unsigned int i = 0; i < length(); i++)
      at(i) = (char)tolower(ptr[i]);
  }


  /**
       @brief  Return the all upper case YString of the _text.
   */
  YString YString::as_upper() const
  {
    YString temp = *this;
    temp.to_upper();
    return temp;
  }


  /**
   * @brief Change all the lower case to upper caes in the given YString.
   *
      Change all the lower case to upper caes in the given YString.
      Static, protected member.
   *
   */
  void YString::strupr(char *buffer, int n)
  {
    for (int i = 0; i < n; i++)
      if (islower(buffer[i]))
	buffer[i] -= 'a' - 'A';
  }



  /**
   * @brief Change all the upper case to lower caes in the given YString.
   *
    Change all the upper case to lower caes in the given YString.
    Static, protected member.
   *
   */
  void YString::strlwr(char *buffer, int n)
  {
    for (int i = 0; i < n; i++)
      if (isupper(buffer[i]))
	buffer[i] += 'a' - 'A';
  }


  /**
   @brief Return the all upper case YString of the _text.

   */
  YString YString::as_lower() const
  {
    YString temp = *this;
    temp.to_lower();
    return temp;
  }



  /**
   *
     @brief : Returns an array of YStrings each containing tokens separated
                  by the delimiter argument.
   */
  void YString::make_tokens(char delimiter,
			    DynamicArray<YString>& tokens,
			    bool noNull) const
  {
    if (is_null())
      return;

    int j = 0;
    int k = 0;
    size_t i;
    for (i = 0; i < size(); i++)
      if ((*this)[i] == delimiter) {
	if (noNull && k == 0)
	  continue;
	tokens[j++] = cut(i-k, k);
	k = 0;
      }
      else
	k++;

    if (!(noNull && k == 0))
      tokens[j++] = cut(i - k, k) ;
  }


  void YString::make_tokens(char delimiter,
			    vector<YString>& tokens,
			    bool noNull) const
  {
    if (is_null())
      return;

    int k = 0;
    size_t i;
    for (i = 0; i < size(); i++)
      if ((*this)[i] == delimiter) {
	if (noNull && k == 0)
	  continue;
	tokens.push_back(cut(i-k, k));
	k = 0;
      }
      else
	k++;

    if (!(noNull && k == 0))
      tokens.push_back(cut(i - k, k));
  }


  /**
     @brief : Returns an array of YStrings each containing tokens separated
                  by the delimiter argument.
   */
  void YString::make_tokens(char delimiter,
			    DynamicArray<string>& tokens,
			    bool noNull) const
  {
    if (is_null())
      return;

    int j = 0;
    int k = 0;
    size_t i;
    for (i = 0; i < size(); i++)
      if ((*this)[i] == delimiter) {
	if (noNull && k == 0)
	  continue;
	tokens[j++] = cut(i-k, k);
	k = 0;
      }
      else
	k++;

    if (!(noNull && k == 0))
      tokens[j++] = cut(i - k, k) ;
  }



  void YString::make_tokens(char delimiter,
			    vector<string>& tokens,
			    bool noNull) const
  {
    if (is_null())
      return;

    int k = 0;
    size_t i;
    for (i = 0; i < size(); i++)
      if ((*this)[i] == delimiter) {
	if (noNull && k == 0)
	  continue;
	tokens.push_back(cut(i-k, k));
	k = 0;
      }
      else
	k++;

    if (!(noNull && k == 0))
      tokens.push_back(cut(i - k, k));
  }


  /**
    @brief Returns an array of YStrings each containing tokens separated
                  by the set of delimiters.

   */
  void YString::make_tokens(const YString& delimiter,
			    DynamicArray<YString>& tokens,
			    bool noNull) const
  {
    if (is_null())
      return;

    int j = 0;
    int k = 0;
    size_t i;
    for (i = 0; i < size(); i++)
      if (delimiter.contains(SM_SENSITIVE, (*this)[i])) {
	if (noNull && k == 0)
	  continue;
	tokens[j++] = cut(i-k, k);
	k = 0;
      }
      else
	k++;

    if (!(noNull && k == 0))
      tokens[j++] = cut(i - k, k);
  }


  void YString::make_tokens(const YString& delimiter,
			    vector<YString>& tokens,
			    bool noNull) const
  {
    if (is_null())
      return;

    int k = 0;
    size_t i;
    for (i = 0; i < size(); i++)
      if (delimiter.contains(SM_SENSITIVE, (*this)[i])) {
	if (noNull && k == 0)
	  continue;
	tokens.push_back(cut(i-k, k));
	k = 0;
      }
      else
	k++;

    if (!(noNull && k == 0))
      tokens.push_back(cut(i - k, k));
  }


  /**
   *
   * @brief : make a token based on the delimiters.
   *
   */
  void YString::make_tokens(const DynamicArray<YString>& delimiters,
			    DynamicArray<YString>& tokens,
			    bool noNull) const
  {
    if (is_null())
      return;

    int offset = 0;
    while (offset < (int)size()) {
      DynamicArray<int> indices;

      if (!multi_find(SM_SENSITIVE,
		      delimiters,
		      indices,
		      offset)) {
	tokens[tokens.size()] = cut_range(offset, high());
	return;
      }

      // found a pattern.
      int delimiterIndex = 0;
      int nextOffset = size();

      for (int j = 0; j < indices.size(); j++) {
	if (indices[j] != -1 && nextOffset > indices[j]) {
	  nextOffset = indices[j];
	  delimiterIndex = j;
	}
      }

      if (noNull && nextOffset == offset) {
	offset+= delimiters[delimiterIndex].size();
	continue;
      }

      tokens[tokens.size()] = cut(offset, nextOffset - offset);
      offset = nextOffset + delimiters[delimiterIndex].size();
    }
  }




  /**
   *   @brief : Returns an array of YStrings, including delimiters, each
   *                   containing tokens separated by the delimiter argument.
   *
   */
  void YString::make_tokens_with_delimiters(const YString& delimiter,
					    DynamicArray<YString>& tokens) const
  {
    if (is_null())
      return;

    int j = 0;
    int k = 0;
    size_t i;
    for (i = 0; i < size(); i++)
      if (delimiter.contains(SM_SENSITIVE, (*this)[i])) {
	if (k != 0)
	  tokens[j++] = cut(i-k, k);
	tokens[j++] = (*this)[i];
	k = 0;
      }
      else
	k++;

    if (k != 0)
      tokens[j++] = cut(i - k, k);
  }



  /**
   *
   *  @brief : Cut between the specified two characters. The inclusion of
   *               the delimiting characters depends on the value of option.
   */
  YString YString::cut_between_words(char beg, char end) const
  {
    stringstream str;

    if (beg == end)
      return "";

    size_t i;
    size_t level = 0;
    for (i = 0; i < size(); i++) {
      if ((*this)[i] == beg)
	level++;
      if (level == 0)
	str << (*this)[i];
      if ((*this)[i] == end)
	if (level > 0)
	  level--;
    }

    return YString(str);
  }


  /**
   * @brief Cut between the specified two patterns.
   *
     Cut between the specified two patterns.
                   The return values starts with the first pattern and ends
              with the second one.

   */
  YString YString::cut_between_words(const YString& p1, const YString& p2) const
  {
    return cut_between_words(SM_SENSITIVE, p1, p2);
  }


  YString YString::cut_between_words(StrCompMode caseChk,
				     const YString& p1, const YString& p2) const
  {
    return cut_through_with(caseChk, p1).cut_tail_with(caseChk, p2) + p2;
  }


  /**
   *
     @brief Returns the string except the substring that is formed by
                  the beginning pattern and the ending pattern.
		It replaces only one such substring occurrence.
   *
   */
  YString YString::cut_outside_words(const YString& p1, const YString& p2) const
  {
    return cut_outside_words(SM_SENSITIVE, p1, p2);
  }


  YString YString::cut_outside_words(StrCompMode caseChk,
				     const YString& p1, const YString& p2) const
  {
    int prev = 0;
    int index1 = -1;
    int index2 = -1;
    YString ret;
    BoyerMoore bm1(p1);
    BoyerMoore bm2(p2);
    while (1) {
      if (find_bm(caseChk, bm1, index1, index2+1)) {
	if (find_bm(caseChk, bm2, index2, index1+1)) {
	  ret += cut_range(prev, index1 -1);
	  prev = index2+p2.size();
	  continue;
	}
      }

      if (!is_null())
	ret += cut_range(prev, high());
      break;
    }

    return ret;
  }


  /**
   *
     @brief : Returns n DynamicArray of YStrings of which elements contain
                 the words between the marks.
	       It doesn't contain the marks itself.
   *
   */
  void YString::take_between_marks(char beg, char end,
				   DynamicArray<YString>& arrayOfstrings) const
  {
    if (beg == end)
      return;

    size_t i;
    int index = -1;
    size_t level = 0;
    size_t step = 0;
    for (i = 0; i < size(); i++) {
      if ((*this)[i] == beg) {
	index++;
	level++;
      }
      else if ((*this)[i] == end) {
	if (level > 0) {
	  level--;
	  if (level == 0)
	    step = 0;
	  else
	    step++;
	}
      }
      else if (level > 0) {
	YString temp = (*this)[i];
	int finalIndex = index - step;
	arrayOfstrings.index(finalIndex) += temp;
      }
      else;
    }
  }



  void YString::take_between_mark(char mark,
				  DynamicArray<YString>& arrayOfstrings) const
  {
    int index = 0;
    int prev = 0;
    int cnt = 0;
    find(SM_SENSITIVE, mark, index, index);
    prev = index;
    while (find(SM_SENSITIVE, mark, index, index + 1)) {
      arrayOfstrings[cnt++] = cut_range(prev+1, index - 1);
      prev = index;
    }
  }



  /**
   *
      @brief Returns the number of occurrences for a given pattern in the
                     string.
   *
   */
  int YString::num_occurrences(StrCompMode caseChk, const YString& pattern) const
  {
    return num_occurrences(caseChk, pattern, *this, 0);
  }



  int YString::num_occurrences(StrCompMode caseChk,
			       const YString& pattern,
			       const YString& source,
			       int numOcc) const
  {
    return num_occurrences(caseChk,BoyerMoore(pattern), source, numOcc);
  }



  int YString::num_occurrences(StrCompMode caseChk,
			       const BoyerMoore& bm,
			       const YString& source,
			       int numOcc) const
  {
    int index;
    if (source.find_bm(caseChk, bm, index))
      return num_occurrences(caseChk, bm,
			     source.cut_through(index+bm.m),
			     ++numOcc);
    else
      return numOcc;
  }



  /**
   * @brief  Returns the counter array (reference argument) each array
                 element representing the number of occurrences of the
		 keyword in the patterns array.
   */
  void YString::frequency_counter(StrCompMode caseChk,
				  const DynamicArray<YString>& pattern,
				  DynamicArray<int>& result) const
  {
    for (int i = 0; i < pattern.size(); i++)
      result[i] = num_occurrences(caseChk, pattern[i]);
  }


  /**
   * @brief Insert the given character so that it appears in the string
                 at least at the given interval.
   *
   */
  int print_string_nospace(stringstream& buf, const YString& temp)
  {
    int test = false;
    int cnt = 0;
    for (size_t k = 0; k < temp.size(); k++)
      if (temp[k] != ' ' || test) {
	test = true;
	buf << temp[k];
	if (temp[k] == '\t')
	  cnt += TAB_SIZE;
	else
	  cnt++;
      }
    return cnt;
  }


  YString YString::paragraph(int interval) const
  {
    if (is_null())
      return *this;

    DynamicArray<YString> tokens;
    make_tokens_with_delimiters(" ", tokens);

    stringstream buf;
    int acc = 0;
    int newLinePrinted = false;
    int i;
    for (i = 0; i < tokens.size(); i++) {
      if (tokens[i].is_null())
	continue;

      int offset = 0;
      if (tokens[i].find(SM_SENSITIVE, "\n", offset)) {
	while (tokens[i].find(SM_SENSITIVE, '\n', offset, offset+1));
	if (newLinePrinted) {
	  print_string_nospace(buf, tokens[i]);
	  newLinePrinted = false;
	}
	else
	  buf << tokens[i];

	acc = tokens[i].size() - offset - 1;
	if (acc == 0)
	  newLinePrinted = true;
	else {
	  YString tp = tokens[i].cut_range(offset+1, tokens[i].high());
	  int numTabs = tp.num_occurrences(SM_SENSITIVE, '\t');
	  acc += (TAB_SIZE - 1) * numTabs;
	}
      }
      else {
	size_t diff = acc + tokens[i].size() - interval;
	if (diff > 0) {
	  if (diff > tokens[i].size()/3) {
	    // put the newline here.
	    buf << std::endl;
	    acc = print_string_nospace(buf, tokens[i]);
	  }
	  else {
	    buf << tokens[i] << std::endl;
	    newLinePrinted = true;
	    acc = 0;
	  }
	}
	else {
	  if (newLinePrinted) {
	    acc += print_string_nospace(buf, tokens[i]);
	    newLinePrinted = false;
	  }
	  else {
	    buf << tokens[i];
	    acc += tokens[i].size();
	    int numTabs = tokens[i].num_occurrences(SM_SENSITIVE, '\t');
	    acc += (TAB_SIZE - 1) * numTabs;
	  }
	}
      }
    }

    return YString(buf);
  }



  void YString::replace_char(size_t pos, const char ch)
  {
    if (pos < 0 || pos >= length())
      error("YString::replace_char() : illegal position :" + YString((unsigned int)pos));

    (*this)[pos] = ch;
  }



  YString YString::cut_through_with(const YString& mark) const
  {
    return cut_through_with(SM_SENSITIVE, mark);
  }



  YString YString::cut_through_with(StrCompMode caseChk, const YString& mark) const
  {
    int index;
    if (find(caseChk, mark, index))
      return cut_through(index);
    else
      return "";
  }


  YString YString::cut_tail_with(const YString& mark) const
  {
    return cut_tail_with(SM_SENSITIVE, mark);
  }


  YString YString::cut_tail_with(StrCompMode caseChk, const YString& mark) const
  {
    int index;
    if (find(caseChk, mark, index))
      return cut_range(0, index-1);
    else
      return *this;
  }


  /**
   * @brief Find a pattern that is formed by concatenating the patterns
                  passed in DynamicArray<YString> patterns argument.
   *
     Find a pattern that is formed by concatenating the patterns
                  passed in DynamicArray<YString> patterns argument.
		Returns each starting offsets of the sub-patterns via
		  DynamicArray<int> indices argument.
                Returns true iff such patterns are found.
		Same sub-patterns are allowed and second patterns or later
		 ones are searched right after the previous ones in the object.

   */
  int previous_offset(const DynamicArray<int>& indices,
		      int i)
  {
    if (i > 0)
      return indices[i - 1] + 1;
    else
      return 0;
  }



  bool YString::find_patterns(StrCompMode caseChk,
			      int startPos,
			      const DynamicArray<YString>& patterns,
			      DynamicArray<int>& indices,
			      int patternIndex) const
  {
    for (int i = patternIndex; i < patterns.size(); i++) {
      if (!find(caseChk,
		patterns[i],
		indices[i],
		std::max(startPos, previous_offset(indices, i)))) {
	return false;
      }

      if (i != 0 && indices[i] <= indices[i-1]) {
	return false;
      }
    }


    return true;
  }


  char base64_code(char t)
  {
    if (t >= 0 && t <= 25)
      return char(t + 'A');

    if (t >= 26 && t <= 51)
      return char(t - 26 + 'a');

    if (t >= 52 && t <= 61)
      return char(t - 52 + '0');

    if (t == 62)
      return '+';
    if (t == 63)
      return '/';

    assert(false);
    return 'A'; // dummy.
  }



  YString get_four_octet(const YString& str, int& index)
  {
    unsigned char b1, b2, b3;

    if (index < (int) str.size())
      b1 = str[index++];
    else
      assert(false);

    YString ret;
    unsigned char t = b1 >> 2;
    ret = YString(base64_code(t));
    t = b1 << 4;
    t &= 0x3f;

    if (index < (int)str.size())
      b2 = str[index++];
    else {
      ret += YString(base64_code(t));
      ret += "==";
      return ret;
    }

    t += b2 >> 4;
    ret += YString(base64_code(t));
    t = b2 << 2;
    t &= 0x3f;

    if (index < (int)str.size())
      b3 = str[index++];
    else {
      ret += YString(base64_code(t));
      ret += "=";
      return ret;
    }

    t += b3 >> 6;
    ret += YString(base64_code(t));
    t = b3 & 0x3f;
    ret += YString(base64_code(t));
    return ret;
  }




  YString YString::encode_base64() const
  {
    int index = 0;

    stringstream buf;
    int numIters = size()/3;
    if (size()%3 != 0)
      numIters++;

    for (int i = 0; i < numIters; i++)
      buf << get_four_octet(*this, index);

    return YString(buf);
  }




  /**
      @brief : Returns the number of keywords found.
                    returnIndices contains the offsets for the keywords.
            If there is no such keyword, it sets the offset to -1.
   *
   */
  int YString::multi_find(StrCompMode caseChk,
			  const DynamicArray<YString>& keywordsList,
			  DynamicArray<int>& returnIndices,
			  int startPos) const
  {
    int numFinds = 0;

    for (int i = 0; i < keywordsList.size(); i++) {
      int offset = -1;
      if (find(caseChk, keywordsList[i], offset, startPos))
	numFinds++;
      returnIndices[i] = offset;
    }

    return numFinds;
  }

  void YString::encode_whitespace()
  {
    char* temp = new char[size() * 2 + 1];
    int cnt = 0;
    size_t i;
    for (i = 0; i < size(); i++) {
      if ((*this)[i] == ' ') {
	temp[cnt++] = ENCODE_CHAR;
	temp[cnt++] = ENCODE_SPACE_CHAR;
      }
      else if ((*this)[i] == '\t') {
	temp[cnt++] = ENCODE_CHAR;
	temp[cnt++] = ENCODE_TAB_CHAR;
      }
      else if ((*this)[i] == '\n') {
	temp[cnt++] = ENCODE_CHAR;
	temp[cnt++] = ENCODE_NEWLINE_CHAR;
      }
      else if (isspace((*this)[i])) {
	std::cerr << "Warning - something that I didn't expect is being"
	  " encoded into space :" << (*this)[i] << ":" << std::endl;
	temp[cnt++] = ENCODE_CHAR;
	temp[cnt++] = ENCODE_SPACE_CHAR;
      }
      else
	temp[cnt++] = (*this)[i];
    }
    temp[cnt] = (char)NULL;
    *this = temp;
    if (temp) delete [] temp;
  }



  void YString::decode_whitespace()
  {
    char* temp = new char[size() + 1];
    int cnt = 0;
    size_t i;
    for (i = 0; i < size(); i++) {
      if ((*this)[i] == ENCODE_CHAR) {
	if (i < size() - 1) {
	  if ((*this)[i+1] == ENCODE_SPACE_CHAR) {
	    temp[cnt++] = ' ';
	    i++;
	  }
	  else if ((*this)[i+1] == ENCODE_TAB_CHAR) {
	    temp[cnt++] = '\t';
	    i++;
	  }
	  else if ((*this)[i+1] == ENCODE_NEWLINE_CHAR) {
	    temp[cnt++] = '\n';
	    i++;
	  }
	  else
	    temp[cnt++] = (*this)[i];
	}
	else
	  temp[cnt++] = (*this)[i];
      }
      else
	temp[cnt++] = (*this)[i];
    }

    temp[cnt] = (char)NULL;
    *this = temp;
    if (temp) delete [] temp;
  }



  YString YString::cut_word_after(StrCompMode caseChk, const YString& tag) const
  {
    int index;
    if (find(caseChk, tag, index)) {
      size_t i = index + tag.size();
      for (; i < size() && isspace((*this)[i]); i++);
      if (i == size())
	return "";
      int start = i;
      for (; i < size() && !isspace((*this)[i]); i++);
      return cut_range(start, i - 1);
    }
    return "";
  }


  YString YString::cut_line_after(StrCompMode caseChk, const YString& tag) const
  {
    int index;
    if (find(caseChk, tag, index)) {
      size_t i = index + tag.size();
      if (i == size())
	return "";
      int start = i;
      for (; i < size() && (*this)[i] != '\n'; i++);
      return cut_range(start, i - 1);
    }
    return "";
  }

  YString YString::remove_multi_whitespace() const
  {
    stringstream buf;
    for (size_t i = 0; i < size(); i++) {
      if (isspace((*this)[i])) {
	if (i > 0 && !isspace((*this)[i-1]))  // only one space.
	  buf << " ";
      }
      else
	buf << (*this)[i];
    }
    return YString(buf);
  }



  /**
      @brief Returns the rear part that begins with the tag, including
                      the tag depending on the option.
   */
  YString YString::reverse_cut_rear_with(StrCompMode caseChk,
					 char tag,
					 bool includeTag) const
  {
    int i;
    for (i = high(); i >= 0; i--) {
      if (caseChk == SM_SENSITIVE) {
	if ((*this)[i] == tag)
	  break;
      }
      else {
	if (tolower((*this)[i]) == tolower(tag))
	  break;
      }
    }

    if (i >= 0) {
      if (includeTag)
	return cut_range(i, high());
      else
	return cut_range(i+1, high());
    }

    return "";
  }



  YString YString::reverse_cut_front_with(StrCompMode caseChk,
					  char tag,
					  bool includeTag) const
  {
    int i;
    for (i = high(); i >= 0; i--) {
      if (caseChk == SM_SENSITIVE) {
	if ((*this)[i] == tag)
	  break;
      }
      else {
	if (tolower((*this)[i]) == tolower(tag))
	  break;
      }
    }

    if (i >= 1) {
      if (includeTag)
	return cut_range(0, i);
      else
	return cut_range(0, i-1);
    }

    return "";
  }



  int YString::multi_find(StrCompMode caseChk,
			  const YString& pattern,
			  DynamicArray<int>& indices,
			  int startPos) const
  {
    int index;
    int start = startPos;
    BoyerMoore bm(pattern);
    while (find_bm(caseChk, bm, index, start)) {
      indices[indices.size()] = index;
      start = index + 1;
    }

    return indices.size();
  }



  /**
   * @brief Returns true if the string starts with the given key in the
                      position pos.
   */
  bool YString::starts_with(StrCompMode caseChk, const YString& key, int pos)
    const
  {
    int index = -1;
    find(caseChk, key, index);

    return (index == pos);
  }


  bool YString::ends_with(StrCompMode caseChk, const YString& key)
    const
  {
    int index = size(); // value not to be reached.
    find(caseChk, key, index);

    return (index == (int)(size() - key.size()));
  }


  int YString::reverse_find(StrCompMode caseChk,
			    const YString& tag,
			    int& indexRet,
			    int offset) const
  {
    if (offset == -1) // default value
      offset = high();

    // find from the beginning and down to the offset value.
    int index = -1;
    int prevFoundIndex = index;
    while (find(caseChk, tag, index, index+1)) {
      // found.
      if (index + (int)tag.size() - 1 > offset)
	break;
      else
	prevFoundIndex = index;
    }

    if (prevFoundIndex != -1) {
      indexRet = prevFoundIndex;
      return true;
    }
    else
      return false;
  }



  /**
   * @brief Returns an integer to be used by EfficientLHT
   *
   */
  int YString::hash_x33_u4() const
  {
    register unsigned int h =0;
    int len = length();
    const char* str = as_const_char();

    for (; len >= 4; len -= 4) {
      h = (h << 5) + h + *str++;
      h = (h << 5) + h + *str++;
      h = (h << 5) + h + *str++;
      h = (h << 5) + h + *str++;
    }

    switch (len) {
    case 3:
      h = (h << 5) + h + *str++;
    case 2:
      h = (h << 5) + h + *str++;
    case 1:
      h = (h << 5) + h + *str++;
      break;
    default:
      break;
    }

    return h;
  }


  int YString::size_image() const
  {
    return sizeof(int) + sizeof(char)*size();
  }


  void YString::read_image(const char*& p)
  {
    int l;
    memcpy((char*)&l, p, sizeof(l));
    char *buf = new char[l+1];
    p += sizeof(l);
    memcpy(buf, p, l);
    p += l;
    buf[l] = (char)NULL;
    assign(buf);
    delete [] buf;
  }

  void YString::write_image(char*& p) const
  {
    int l = size();
    memcpy(p, (const char*)&l, sizeof(l));
    p += sizeof(l);
    memcpy(p, c_str(), l);
    p += l;
  }


  string YString::as_append(const string& str) const
  {
    string t = *this;
    t.append(str);
    return t;
  }


  YString YString::generate_random_string(MRandom& rand, int size)
  {
    static const int alpha_span = 'z'-'a';

    string t;
    char* buf = new char[size+1];
    for (int i = 0; i < size; i++)
      buf[i] = (char)(rand.integer()%alpha_span + 'a');
    buf[size] = 0;
    t = buf;
    delete [] buf;
    return t;
  }

NS_IZENELIB_UTIL_END
