#ifndef IZENE_UTIL_LOG_H
#define IZENE_UTIL_LOG_H

#ifdef IZENE_LOG

#include <ostream>
#include <types.h>
#include <string>
#include <boost/logging/format_fwd.hpp>
#include <boost/logging/format.hpp>

BOOST_LOG_FORMAT_MSG(optimize::cache_string_one_str<> )

using namespace boost::logging;

typedef logger_format_write< > logger_type;
 
#ifndef ERR_TAG
#define ERR_TAG "[ERR] "//defult error log tag
#endif

#ifndef APP_TAG
#define APP_TAG "[APP] "//defult app log tag
#endif

#ifndef DBG_TAG
#define DBG_TAG "[DBG] "//defult debug log tag
#endif

#define LDBG__ BOOST_LOG_USE_LOG_IF_FILTER(g_log_dbg(), g_dbg_filter()->is_enabled() ) << DBG_TAG
#define LERR__ BOOST_LOG_USE_LOG_IF_FILTER(g_log_err(), g_err_filter()->is_enabled() ) << ERR_TAG
#define LAPP__ BOOST_LOG_USE_LOG_IF_FILTER(g_log_app(), g_app_filter()->is_enabled() ) << APP_TAG
#define VACANCY_ BOOST_LOG_USE_LOG_IF_FILTER(vacancy_log(), g_block()->is_enabled() )
 
#ifndef ERR_LOG_NAME
#define ERR_LOG_NAME "./err.txt"//defult error log file name
#endif

#ifndef APP_LOG_NAME
#define APP_LOG_NAME "./app.txt"//defult application output log file name
#endif

#ifndef DBG_LOG_NAME
#define DBG_LOG_NAME "./dbg.txt"
#endif

class log
{
 public:

  virtual ~log()
  {
  }
  
  virtual log& operator << (const char* in){return *this;}
  virtual log& operator << (const std::string in){return *this;}
  virtual log& operator << (int in){return *this;}
  virtual log& operator << (double in){return *this;}
  virtual log& operator << (long double in){return *this;}
  virtual log& operator << (unsigned long in){return *this;}
  virtual log& operator << (float in){return *this;}
  virtual log& operator << (char in){return *this;}
  virtual log& operator << (unsigned short in){return *this;}
  virtual log& operator << (short in){return *this;}
  virtual log& operator << (unsigned int in){return *this;}
  virtual log& operator << (bool in){return *this;}
  
}
;

class no_log:public log
{

 BOOST_DEFINE_LOG_FILTER(g_block, filter::no_ts )
   BOOST_DEFINE_LOG(vacancy_log, logger_type)
   public:
  no_log()
  {
    g_block()->set_enabled(false);
  }


  virtual ~no_log()
  {
  }
  
  virtual no_log& operator << (const char* in){VACANCY_<<in;return *this;}
  virtual no_log& operator << (const std::string in){VACANCY_<<in.c_str();return *this;}
  virtual no_log& operator << (int in){VACANCY_<<in;return *this;}
  virtual no_log& operator << (double in){VACANCY_<<in;return *this;}
  virtual no_log& operator << (long double in){VACANCY_<<in;return *this;}
  virtual no_log& operator << (unsigned long in){VACANCY_<<in;return *this;}
  virtual no_log& operator << (float in){VACANCY_<<in;return *this;}
  virtual no_log& operator << (char in){VACANCY_<<in;return *this;}
  virtual no_log& operator << (unsigned short in){VACANCY_<<in;return *this;}
  virtual no_log& operator << (short in){VACANCY_<<in;return *this;}
  virtual no_log& operator << (unsigned int in){VACANCY_<<in;return *this;}
  virtual no_log& operator << (bool in){VACANCY_<<in;return *this;}  
};

class dbg_log
  :public log
{

 BOOST_DEFINE_LOG_FILTER(g_dbg_filter, filter::no_ts )
   BOOST_DEFINE_LOG(g_log_dbg, logger_type)
      
   public:
  dbg_log()
  {
      
    // Debug log
    g_log_dbg()->writer().add_formatter( formatter::time("$hh:$mm.$ss ") );
    g_log_dbg()->writer().add_formatter( formatter::append_newline() );
    //g_log_dbg()->writer().add_destination( destination::dbg_window() );
    g_log_dbg()->writer().add_destination( destination::file(DBG_LOG_NAME) );
#ifdef DBG2CONSOLE // put debug info into console
    g_log_dbg()->writer().add_destination( destination::cout() );
#endif

    g_log_dbg()->mark_as_initialized();
    
#ifndef DBG_DISABLE //disable the debug info
    g_dbg_filter()->set_enabled(true);
#else
    g_dbg_filter()->set_enabled(false);
#endif

  }

  virtual ~dbg_log()
  {
  }
  
  virtual dbg_log& operator << (const char* in){LDBG__<<in;return *this;}
  virtual dbg_log& operator << (const std::string in){LDBG__<<in.c_str();return *this;}
  virtual dbg_log& operator << (int in){LDBG__<<in;return *this;}
  virtual dbg_log& operator << (double in){LDBG__<<in;return *this;}
  virtual dbg_log& operator << (long double in){LDBG__<<in;return *this;}
  virtual dbg_log& operator << (unsigned long in){LDBG__<<in;return *this;}
  virtual dbg_log& operator << (float in){LDBG__<<in;return *this;}
  virtual dbg_log& operator << (char in){LDBG__<<in;return *this;}
  virtual dbg_log& operator << (unsigned short in){LDBG__<<in;return *this;}
  virtual dbg_log& operator << (short in){LDBG__<<in;return *this;}
  virtual dbg_log& operator << (unsigned int in){LDBG__<<in;return *this;}
  virtual dbg_log& operator << (bool in){LDBG__<<in;return *this;}
    
};

class app_log :public log
{
 BOOST_DEFINE_LOG_FILTER(g_app_filter, filter::no_ts )
   BOOST_DEFINE_LOG(g_log_app, logger_type)
   public:
  app_log()
  {
      
    // App log
    g_log_app()->writer().add_formatter( formatter::time("$hh:$mm.$ss ") );
    g_log_app()->writer().add_formatter( formatter::append_newline() );
    g_log_app()->writer().add_destination( destination::file(APP_LOG_NAME) );
#ifdef APP2CONSOLE // put application info to console
    g_log_app()->writer().add_destination( destination::cout() );
#endif
    g_log_app()->mark_as_initialized();
    
#ifndef APP_DISABLE // disable the app info
    g_app_filter()->set_enabled(true);
#else
    g_app_filter()->set_enabled(false);
#endif

  }

  virtual ~app_log()
  {
  }
  
  virtual app_log& operator << (const char* in){LAPP__<<in;return *this;}
  virtual app_log& operator << (const std::string in){LAPP__<<in.c_str();return *this;}
  virtual app_log& operator << (int in){LAPP__<<in;return *this;}
  virtual app_log& operator << (double in){LAPP__<<in;return *this;}
  virtual app_log& operator << (long double in){LAPP__<<in;return *this;}
  virtual app_log& operator << (unsigned long in){LAPP__<<in;return *this;}
  virtual app_log& operator << (float in){LAPP__<<in;return *this;}
  virtual app_log& operator << (char in){LAPP__<<in;return *this;}
  virtual app_log& operator << (unsigned short in){LAPP__<<in;return *this;}
  virtual app_log& operator << (short in){LAPP__<<in;return *this;}
  virtual app_log& operator << (unsigned int in){LAPP__<<in;return *this;}
  virtual app_log& operator << (bool in){LAPP__<<in;return *this;}
      
};

class err_log:public log
{
 BOOST_DEFINE_LOG_FILTER(g_err_filter, filter::no_ts )
   BOOST_DEFINE_LOG(g_log_err, logger_type)
   public:
  err_log()
  {
      
    // Err log
    g_log_err()->writer().add_formatter( formatter::idx(), "[%] "  );
    g_log_err()->writer().add_formatter( formatter::time("$hh:$mm.$ss ") );
    g_log_err()->writer().add_formatter( formatter::append_newline() );
    g_log_err()->writer().add_destination( destination::file(ERR_LOG_NAME) );

#ifdef ERR2CONSOLE// put error info into the console
    g_log_err()->writer().add_destination( destination::cout() );
#endif

    g_log_err()->mark_as_initialized();
    
#ifndef ERR_DISABLE // disable the error info
    g_err_filter()->set_enabled(true);
#else
    g_err_filter()->set_enabled(false);
#endif

  }


  virtual ~err_log()
  {
  }
  
  virtual err_log& operator << (const char* in){LERR__<<in;return *this;}
  virtual err_log& operator << (const std::string in){LERR__<<in.c_str();return *this;}
  virtual err_log& operator << (int in){LERR__<<in;return *this;}
  virtual err_log& operator << (double in){LERR__<<in;return *this;}
  virtual err_log& operator << (long double in){LERR__<<in;return *this;}
  virtual err_log& operator << (unsigned long in){LERR__<<in;return *this;}
  virtual err_log& operator << (float in){LERR__<<in;return *this;}
  virtual err_log& operator << (char in){LERR__<<in;return *this;}
  virtual err_log& operator << (unsigned short in){LERR__<<in;return *this;}
  virtual err_log& operator << (short in){LERR__<<in;return *this;}
  virtual err_log& operator << (unsigned int in){LERR__<<in;return *this;}
  virtual err_log& operator << (bool in){LERR__<<in;return *this;}
  
};

extern dbg_log dbg;
extern no_log nlog;
extern app_log app;
extern err_log err;

class if_log
{
 public:
  static log& if_dlog(int f)
  {
    if (f)
      return dbg;

    return nlog;
  }

  static log& if_elog(int f)
  {
    if (f)
      return err;

    return nlog;
  
  }

  static log& if_alog(int f)
  {
    if (f)
      return app;

    return nlog;
  }
}
;

#define IF_DLOG(F) if_log::if_dlog(F)
  
#define IF_ELOG(F) if_log::if_elog(F)
#define IF_ALOG(F) if_log::if_alog(F)


#define LDBG_ dbg
#define LERR_ err
#define LAPP_ app

//#define USING_IZENE_LOG() initiate_log::initiateLog()
#define USING_IZENE_LOG() dbg_log dbg;no_log nlog;app_log app;err_log err;

#else

#define IF_DLOG(F) if(F)cout
  
#define IF_ELOG(F) if(F)cout
#define IF_ALOG(F) if(F)cout


#define LDBG_ cout
#define LERR_ cout
#define LAPP_ cout

#endif
                               //#define LDBG_ cout
#endif //End of IZENE_UTIL_LOG_H
