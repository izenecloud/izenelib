#include <util/cronexpression.h>

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include <boost/tokenizer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

namespace izenelib{
namespace util{

std::string format(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char temp[2048];
    vsprintf(temp,fmt, args);
    std::string result = temp;
    va_end(args);
    return result;
}

// debug util func
static std::string vector_dump(std::vector<int> &iv)
{
    std::string tmp = "<";
    for (std::vector<int>::iterator itr = iv.begin(); itr != iv.end(); ++itr)
        tmp += format(" %d", *itr);
    tmp += " >";
    return tmp;
}

CronExpression::~CronExpression()
{
}

bool CronExpression::setExpression(const std::string& expression, bool show)
{
  typedef boost::tokenizer<boost::char_separator<char> > ExpTokenizer;

  // parse the string we're given into five vectors of n ints and a command
  // note: this is rather expensive
  // first bust it up into tokens based on whitespace.
  // the first five are the timing values and the 'sixth through nth' is the command.
  boost::char_separator<char> sep(" \t");
  ExpTokenizer tokens(expression, sep);
  std::vector<std::string> toks;
  for (ExpTokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter)
  {
      toks.push_back(*tok_iter);
  }
  if(toks.size() < 5) return false;
  // hokey dokey.  now we have six strings and we need five arrays of ints and one string out of them.
  minutes = parseTimeList(toks[0], 0, 59);
  hours = parseTimeList(toks[1], 0, 23);
  days = parseTimeList(toks[2], 1, 31);
  months = parseTimeList(toks[3], 1, 12);
  weekdays = parseTimeList(toks[4], 0, 7);

  // sunday is both 7 and 0, make sure we have both or neither
  if (isInVector(weekdays, 0) && !isInVector(weekdays, 7)) weekdays.push_back(7);
  else if (isInVector(weekdays, 7) && !isInVector(weekdays, 0)) weekdays.push_back(0);
  if(show)
  {
      std::cout << "expression minutes: " << vector_dump(minutes) << std::endl;
      std::cout << "expression hours: " << vector_dump(hours) << std::endl;
      std::cout << "expression days: " << vector_dump(days) << std::endl;
      std::cout << "expression months: " << vector_dump(months) << std::endl;
      std::cout << "expression weekdays: " << vector_dump(weekdays) << std::endl;
  }

  return true;
}

std::vector<int> CronExpression::parseTimeList(const std::string in, const int min, const int max)
{
    typedef boost::tokenizer<boost::char_separator<char> > token;

    std::vector<int> vi;
    std::string list = in;

    // First things first.  Find out if there's a periodicity and trim it off.
    size_t pos = in.find("/");
    int period = 1;
    if (pos != std::string::npos)
    {
        period = atoi(in.substr(pos + 1).c_str());
        list = in.substr(0, pos);
    }

    boost::char_separator<char> sep(",");
    token tokens(in, sep);
    std::vector<std::string> stage1;
    for (token::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter)
    {
        stage1.push_back(*tok_iter);
    }

    // Now tokenize on ","
    //std::vector<std::string> stage1 = tokenize(list, ",",0,false);
    // No tokens?  That's cool too.
    if (stage1.size() == 0) stage1.push_back(list);

    // And for each token, blow up any "-" ranges and "*" ranges.
    for (std::vector<std::string>::iterator itr = stage1.begin(); itr != stage1.end(); ++itr)
    {
        if ((*itr).find("*") != std::string::npos)
        {
            for (int i = min; i <= max; ++i)
                vi.push_back(i);
        }
        else if ((pos = (int)(*itr).find("-")) != std::string::npos)
        {
            int rmin = 0, rmax = 0;
            rmin = atoi((*itr).substr(0, pos).c_str());
            rmax = atoi((*itr).substr(pos + 1).c_str());
            if (rmin < min) rmin = min;
            if (rmax > max) rmax = max;
            for (int i = rmin; i <= rmax; ++i)
                vi.push_back(i);
        }
        else
        {
            vi.push_back(atoi((*itr).c_str()));
        }
    }

    // Remember that periodicity we got rid of earlier?  Now we need it.
    // Eliminate any elements which disagree with the periodicity.
    if (period > 1)
    {
        std::vector<int> vp;
        for (std::vector<int>::iterator itr2 = vi.begin(); itr2 != vi.end(); ++itr2)
        {
            if (((*itr2) == 0) || ((*itr2) % period == 0))
                vp.push_back(*itr2);
        }
        return vp;
    }
    else
    {
        return vi;
    }
}

bool CronExpression::matches(int n, int h, int d, int m, int w) const
{
    // if we are supposed to execute now, return true, otherwise return false
    return (isInVector(minutes, n) &&
            isInVector(hours, h) &&
            isInVector(days, d) &&
            isInVector(months, m) &&
            isInVector(weekdays, w));
}

bool CronExpression::matches_now() const
{
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  using namespace izenelib::util;

  boost::posix_time::ptime now = second_clock::local_time();
  boost::gregorian::date date = now.date();
  boost::posix_time::time_duration du = now.time_of_day();
  int dow = date.day_of_week();
  int month = date.month();
  int day = date.day();
  int hour = du.hours();
  int minute = du.minutes();
  return matches(minute, hour, day, month, dow);
}

bool CronExpression::isInVector(const std::vector<int> &iv, const int x)
{
    for (std::vector<int>::const_iterator itr = iv.begin(); itr != iv.end(); ++itr)
    {
        if (*itr == x)
            return true;
    }
    return false;
}

int CronExpression::day_of_week(int m, int d, int y) 
{
    // W = (k + floor(2.6m - 0.2) - 2C + Y + floor(Y/4) + floor(C/4)) mod 7

    m -= 2; // march is base month
    if (m < 1) {
        y -= 1;
        m += 12;
    }

    const int c = (int)((y - 50.0f) / 100.0f); // century

    // whew!
    return ((int)(d + floor(2.6f * m - 0.2f) - 2 * c + y + floor(y / 4.0f) + floor(c / 4.0f)) % 7);
}

}
}
