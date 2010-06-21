/**
 * @file  YString.h
 * @brief Define the operations for manipulating strings.
 */
#ifndef _STR_ING_H_
#define _STR_ING_H_


#include <util/MRandom.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//using namespace std;
NS_IZENELIB_UTIL_BEGIN

  enum StrCompVal { SC_LESS, SC_EQUAL, SC_GREATER, SC_ERROR };
  enum StrCompMode { SM_SENSITIVE, SM_IGNORE };

  template <class Type> class DynamicArray;
  template <class Type> class Array;

  const int CHAR_SIZE = 128;

  /**
   * @brief Wrapper for the operations for manipulating strings.
   */
  class YString : public std::string {
  public:
  /**
   * @brief Simple wrapper for BoyerMoore algorithm.
   *
   */
  class BoyerMoore {
  public:
  /**
   * @brief Constructor.
   */
  BoyerMoore(const YString& pat)
    : x(pat.as_const_char()), m(pat.size())
  {
    bm_gs = new int[m +1];
    pre_gs();
    pre_bc();
  }

  void copy(const BoyerMoore& bm)
  {
    x = bm.x;
    m = bm.m;
    bm_gs = new int[m+1];
    memcpy(bm_gs, bm.bm_gs, (m+1)*sizeof(int));
    memcpy(bm_bc, bm.bm_bc, (256)*sizeof(int));
  }

  BoyerMoore(const BoyerMoore& bm)
  {
    copy(bm);
  }

  BoyerMoore& operator=(const BoyerMoore& bm)
  {
    if (this != &bm) {
      delete bm_gs;
      copy(bm);
    }

    return *this;
  }
    const char* x;
    int m;
    int *bm_gs;
    int bm_bc[256];

  /**
   * @brief Preprocessing of the Bad Character function shift.
   */
  void pre_bc()
  {
    int a, j;

    for (a=0; a < 256; a++) bm_bc[a]=m;
    for (j=0; j < m-1; j++) bm_bc[(int)x[j]]=m-j-1;
  }


  /**
   * @brief Preprocessing of the Good Suffix function shift.
   */
  void pre_gs()
  {
    int i, j, p;
    int* f = new int[m+1];

    memset(bm_gs, 0, (m+1)*sizeof(int));
    f[m]=j=m+1;
    for (i=m; i > 0; i--) {
      while (j <= m && x[i-1] != x[j-1]) {
	if (bm_gs[j] == 0) bm_gs[j]=j-i;
	j=f[j];
      }
      f[i-1]=--j;
    }
    p=f[0];
    for (j=0; j <= m; ++j) {
      if (bm_gs[j] == 0) bm_gs[j]=p;
      if (j == p) p=f[p];
    }
    delete f;
  }


  /**
   * @brief Boyer-Moore string matching algorithm.
   */
  int find(const char *y, int n) const
  {
    int i, j;

    i=0;
    while(i <= n-m) {
      for (j=m-1; j >= 0 && y[i+j]==x[j]; --j);
      if (j < 0)
	return (i);
      else
	i+=std::max(bm_gs[j+1],bm_bc[(int)y[i+j]]-m+j+1);
    }

    return -1;
  }
  }; // end of BoyerMoore
  private:
    int num_occurrences(StrCompMode caseChk,
			const YString& pattern,
			const YString& source,
			int numOcc) const;
    static void strlwr(char* buffer, int n);
    static void strupr(char* buffer, int n);

  public:
    void OK() const;

    YString(const YString& str) : std::string(str) {}
    YString() : std::string() { }
    YString(const char* cstr): std::string(cstr) { }
    YString(const std::string& str) : std::string(str) { }
    YString(std::stringstream& strBuf) : std::string(strBuf.str()) { }
    YString(char v) { std::stringstream strbuf; strbuf << v; *this = strbuf.str(); }
    YString(unsigned char v) { std::stringstream strbuf; strbuf << v; *this = strbuf.str(); }
    YString(int v) { std::stringstream strbuf; strbuf << v; *this = strbuf.str(); }
    YString(unsigned int v) { std::stringstream strbuf; strbuf << v; *this = strbuf.str(); }
    YString(long v) { std::stringstream strbuf; strbuf << v; *this = strbuf.str(); }
    YString(float v) { std::stringstream strbuf; strbuf << v; *this = strbuf.str(); }
    YString(double v) { std::stringstream strbuf; strbuf << v; *this = strbuf.str(); }
    YString(double source, int precision, int width = 0);
    ~YString() {}

    inline int high() const { return (size() - 1); }
    inline int low() const { return 0; }
    inline bool is_null() const { return empty(); }

    bool is_alpha() const;
    bool is_digit() const;
    bool is_whitespace() const;

    inline int integer() const { return atoi(c_str()); }
    inline long to_long() const { return atol(c_str()); }
    inline unsigned long to_ulong() const { return strtoul(c_str(), NULL, 10); }
    inline double real() const { return (double)atof(c_str()); }

    inline void release() { clear(); }

    // conversion functions instead of conversion operators.
    inline const char* as_const_char() const { return (const char*)(*this).c_str(); }
    inline char* as_char() { return (char*)(*this).c_str(); }
    inline const unsigned char* as_const_unsigned_char() const { return (const unsigned char*)(*this).c_str(); }
    inline unsigned char* as_unsigned_char() { return (unsigned char*)(*this).c_str(); }

    inline YString& operator=(const YString& str) { std::string::operator=(str.get_key()); return *this; }
    inline void assign(const YString& str) { std::string::assign(str); }
    inline YString& operator+=(const YString& str) { std::string::operator+=(str.get_key()); return *this; }
    std::string as_append(const std::string& str) const;
    inline void swap(YString& str) { std::string::swap(str); }

    StrCompVal compare(StrCompMode caseChk, const YString& str) const;
    inline int compare(const std::string& key) const { return std::string::compare(key); }
    inline int operator-(const std::string& key) const { return std::string::compare(key); }

    bool equal_ignore_case(const YString& str) const{ return (compare(SM_IGNORE, str) == SC_EQUAL); }
    bool equal_ignore_case_space(const YString& str) const;

    bool starts_with(StrCompMode caseChk, const YString& key, int pos = 0) const;
    bool ends_with(StrCompMode caseChk, const YString& key) const;

    bool find(StrCompMode caseChk, const YString& str, int& pos, int start = 0) const;
    bool find_kmp(StrCompMode caseChk, const YString& str, int& pos, int start = 0) const;
    bool find_bm(StrCompMode caseChk, const BoyerMoore& bm, int& pos, int start = 0) const;
    inline bool find(const BoyerMoore& bm, int& pos, int start = 0) const { return find_bm(SM_SENSITIVE, bm, pos, start); }
    int find(StrCompMode caseChk, const YString& str, DynamicArray<int>& indices) const;
    inline int contains(StrCompMode caseChk, const YString& str) const { int d; return find(caseChk, str, d);  }

    // this methods need to be declared to use YString as string
    // don't know whyneed to call string:: methods explictly since YString can be
    // automatically cast to string, which is its parent class.
    inline int find(const YString& str) const { return std::string::find(str); }
    inline int find(const YString& str, int idx) const { return std::string::find(str, idx); }
    inline int rfind(const YString& str) const { return std::string::rfind(str); }
    inline int rfind(const YString& str, int idx) const { return std::string::rfind(str, idx); }
    inline int find_last_of(const YString& str) const { return std::string::find_last_of(str); }
    inline int find_last_of(const YString& str, int idx) const { return std::string::find_last_of(str, idx); }
    inline int find_last_not_of(const YString& str) const { return std::string::find_last_not_of(str); }
    inline int find_last_not_of(const YString& str, int idx) const { return std::string::find_last_not_of(str, idx); }

    // this is simple version of regular expression.
    bool find_patterns(StrCompMode caseChk,
		       int startPos,
		       const DynamicArray<YString>& patterns,
		       DynamicArray<int>& indices,
		       int patternIndex = 0) const;

    // this finds each location of multiple patterns.
    int multi_find(StrCompMode caseChk,
		   const DynamicArray<YString>& keywordsList,
		   DynamicArray<int>& returnIndices,
		   int starrtPos = 0) const;

    // this finds multiple locations of one patterns.
    int multi_find(StrCompMode caseChk,
		   const YString& pattern,
		   DynamicArray<int>& indices,
		   int startPos = 0) const;

    int num_occurrences(StrCompMode caseChk, const YString& str) const;
    int num_occurrences(StrCompMode caseChk,
			const BoyerMoore& bm,
			const YString& source,
			int numOcc) const;

    void frequency_counter(StrCompMode caseChk,
			   const DynamicArray<YString>& patterns,
			   DynamicArray<int>& result) const;

    void substitute_char(char from, char to);
    YString substitute_string(StrCompMode caseChk,
			      const YString& from,
			      const YString& to) const;
    YString multi_substitute_string(StrCompMode caseChk,
				    const YString& from,
				    const YString& to) const;

    // substring deletion method.
    void del(int pos, int count);
    // replace newlinew with spaces.
    void remove_newlines() { remove_chars('\n'); }
    // replace all the occurrences of the given character with spaces.
    void replace_char(size_t pos, const char ch);
    void remove_chars(const char ch) { substitute_char(ch, ' '); }
    void remove_duplicate_chars(const char ch);
    YString remove_multi_whitespace() const;
    YString paragraph(int lineLength) const;
    YString cut(int start, int count) const;
    inline YString cut_range(int start, int end) const {  return cut(start, end - start + 1); }
    YString cut_delimiter(int start, char delimiter) const;
    YString cut_head(int count) const;
    inline YString cut_through(int start) const { return cut_tail(size() - start); }
    YString cut_through_with(const YString& mark) const;
    YString cut_tail_with(const YString& mark) const;
    YString cut_through_with(StrCompMode caseChk, const YString& mark) const;
    YString cut_tail_with(StrCompMode caseChk, const YString& mark) const;
    YString cut_tail(int count) const;
    YString cut_between_words(char beg, char end) const;
    YString cut_between_words(const YString& p1, const YString& p2) const;
    YString cut_between_words(StrCompMode caseChk, const YString& p1,
			      const YString& p2) const;
    YString cut_outside_words(const YString& p1, const YString& p2) const;
    YString cut_outside_words(StrCompMode caseChk, const YString& p1,
			      const YString& p2) const;
    YString cut_word_after(StrCompMode caseChk, const YString& tag) const;
    YString cut_line_after(StrCompMode caseChk, const YString& tag) const;
    void take_between_mark(char mark,
			   DynamicArray<YString>& arrayOfstrings) const;
    void take_between_marks(char beg, char end,
			    DynamicArray<YString>& arrayOfstrings) const;
    YString reverse_cut_rear_with(StrCompMode caseChk,
				  char tag,
				  bool includeTag = true) const;
    YString reverse_cut_front_with(StrCompMode caseChk,
				   char tag,
				   bool includeTag = true) const;

    int reverse_find(StrCompMode caseChk,
		     const YString& tag,
		     int& index,
		     int rearOffset = -1) const;

    inline void compact() { compact_tail(); compact_head(); }
    inline YString trim() { compact(); return *this; }

    void compact_tail();
    void compact_head();
    void make_tokens(char delimiter,
		     DynamicArray<YString>& tokens,
		     bool noNull = false) const;
    void make_tokens(char delimiter,
		     DynamicArray<std::string>& tokens,
		     bool noNull = false) const;
    void make_tokens(const YString& delimiter,
		     DynamicArray<YString>& tokens,
		     bool noNull = false) const;
    void make_tokens_with_delimiters(const YString& delimiter,
				     DynamicArray<YString>& tokens) const;
    void make_tokens(const DynamicArray<YString>& delimiters,
		     DynamicArray<YString>& tokens,
		     bool noNull = false) const;

    void make_tokens(char delimiter,
		     std::vector<YString>& tokens,
		     bool noNull = false) const;
    void make_tokens(char delimiter,
		     std::vector<std::string>& tokens,
		     bool noNull = false) const;
    void make_tokens(const YString& delimiter,
		     std::vector<YString>& tokens,
		     bool noNull = false) const;

    void to_upper();
    void to_lower();

    YString as_upper() const;
    YString as_lower() const;

    /* Base64 encoding */
    YString encode_base64() const;

    // AccessMethods interfaces
    const std::string& get_key() const { return *this; }
    void set_key(const YString& k) { *this = k; }
    int hash_x33_u4() const;

    // HashDB interface
    int size_image() const;
    void read_image(const char*& p);
    void write_image(char*& p) const;

    void encode_whitespace();
    void decode_whitespace();

    // simple file read interface
    void read_from_file(const YString& filename);
    void write_to_file(const YString& filename) const;
    static void write_to_file(const YString& filename, const YString& str);

    static void read_from_file(const YString& filename, DynamicArray<YString>& lines);
    static void read_from_file(const YString& filename, DynamicArray<std::string>& lines);
    static void read_from_file(const YString& filename, std::vector<YString>& lines);
    static void read_from_file(const YString& filename, std::vector<std::string>& lines);

    static void write_to_file(const YString& filename, const Array<YString>& lines);
    static void write_to_file(const YString& filename, const Array<std::string>& lines);
    static void write_to_file(const YString& filename, const std::vector<YString>& lines);
    static void write_to_file(const YString& filename, const std::vector<std::string>& lines);

    static void read_filenames(const YString& dir, DynamicArray<YString>& lines);
    static void read_filenames(const YString& dir, DynamicArray<std::string>& lines);
    static void read_filenames(const YString& dir, std::vector<YString>& lines);
    static void read_filenames(const YString& dir, std::vector<std::string>& lines);

    // random generator of a string; used when testing.
    static YString generate_random_string(MRandom& rand, int size = 10);
  };

  // this is necessary for AccessMethods APIs...to extend APIs of string.
  inline int operator-(const std::string& str1, const std::string& str2) { return str1.compare(str2); }
NS_IZENELIB_UTIL_END

// for serialization
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace boost {
    namespace serialization {
      template<typename Archive> void serialize(Archive & ar, izenelib::util::YString & t,
                const unsigned int) {
            std::string temp;
            temp = t;
            ar & temp;
            t = temp;
        }
    }
}

#endif
