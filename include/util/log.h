#ifndef IZENE_UTIL_LOG_H
#define IZENE_UTIL_LOG_H


#include <ostream>
#include <types.h>

#include <boost/logging/format_fwd.hpp>
BOOST_LOG_FORMAT_MSG(optimize::cache_string_one_str<> )

#include <boost/logging/format.hpp>
using namespace boost::logging;

typedef logger_format_write< > logger_type;

BOOST_DEFINE_LOG_FILTER(g_dbg_filter, filter::no_ts )
BOOST_DEFINE_LOG_FILTER(g_app_filter, filter::no_ts )
BOOST_DEFINE_LOG_FILTER(g_err_filter, filter::no_ts )
BOOST_DEFINE_LOG_FILTER(g_block, filter::no_ts )

BOOST_DEFINE_LOG(g_log_err, logger_type)
BOOST_DEFINE_LOG(g_log_app, logger_type)
BOOST_DEFINE_LOG(g_log_dbg, logger_type)

BOOST_DEFINE_LOG(vacancy_log, logger_type)
 
#ifndef ERR_TAG
#define ERR_TAG "[ERR] "//defult error log tag
#endif

#ifndef APP_TAG
#define APP_TAG "[APP] "//defult app log tag
#endif

#ifndef DBG_TAG
#define DBG_TAG "[DBG] "//defult debug log tag
#endif

#define LDBG_ BOOST_LOG_USE_LOG_IF_FILTER(g_log_dbg(), g_dbg_filter()->is_enabled() ) << DBG_TAG
#define LERR_ BOOST_LOG_USE_LOG_IF_FILTER(g_log_err(), g_err_filter()->is_enabled() ) << ERR_TAG
#define LAPP_ BOOST_LOG_USE_LOG_IF_FILTER(g_log_app(), g_app_filter()->is_enabled() ) << APP_TAG
#define VACANCY_ BOOST_LOG_USE_LOG_IF_FILTER(vacancy_log(), g_block()->is_enabled() )
 
#ifndef ERR_LOG_NAME
#define ERR_LOG_NAME "err.txt"//defult error log file name
#endif

void initiateLog()
{
// Err log
  g_log_err()->writer().add_formatter( formatter::idx(), "[%] "  );
  g_log_err()->writer().add_formatter( formatter::time("$hh:$mm.$ss ") );
  g_log_err()->writer().add_formatter( formatter::append_newline() );
  g_log_err()->writer().add_destination( destination::file(ERR_LOG_NAME) );

#ifdef ERR2CONSOLE// put error info into the console
  g_log_err()->writer().add_destination( destination::cout() );
#endif

#ifndef APP_LOG_NAME
#define APP_LOG_NAME "out.txt"//defult application output log file name
#endif
  // App log
  g_log_app()->writer().add_formatter( formatter::time("$hh:$mm.$ss ") );
  g_log_app()->writer().add_formatter( formatter::append_newline() );
  g_log_app()->writer().add_destination( destination::file(APP_LOG_NAME) );
#ifdef APP2CONSOLE // put application info to console
  g_log_app()->writer().add_destination( destination::cout() );
#endif

#ifndef DBG_LOG_NAME
#define DBG_LOG_NAME "dbg.txt"
#endif
  // Debug log
  g_log_dbg()->writer().add_formatter( formatter::time("$hh:$mm.$ss ") );
  g_log_dbg()->writer().add_formatter( formatter::append_newline() );
  g_log_dbg()->writer().add_destination( destination::dbg_window() );
  g_log_dbg()->writer().add_destination( destination::file(ERR_LOG_NAME) );
#ifdef DBG2CONSOLE // put debug info into console
  g_log_dbg()->writer().add_destination( destination::cout() );
#endif
 
  g_log_app()->mark_as_initialized();
  g_log_err()->mark_as_initialized();
  g_log_dbg()->mark_as_initialized();

#ifndef DBG_DISABLE //disable the debug info
  g_dbg_filter()->set_enabled(true);
#else
  g_dbg_filter()->set_enabled(false);
#endif

#ifndef APP_DISABLE // disable the app info
  g_app_filter()->set_enabled(true);
#else
  g_app_filter()->set_enabled(false);
#endif

#ifndef ERR_DISABLE // disable the error info
  g_err_filter()->set_enabled(true);
#else
  g_err_filter()->set_enabled(false);
#endif

  g_block()->set_enabled(false);
}

enum LogLevel
  {
    ERR,
    DBG,
    APP
  };


/* std::ostream LOG_IF( bool flag, int level=APP) */
/* { */
/*   if (!flag) */
/*     ;//return VACANCY_; */
  
/*   if (level == ERR) */
/*   { */
/*     return LERR_; */
/*   } */

/*   if (level== DBG) */
/*   { */
/*     return LDBG_; */
/*   } */
  
/*   if (level== APP) */
/*   { */
/*     return LAPP_; */
/*   } */
  
/* } */

NS_IZENELIB_UTIL_BEGIN


NS_IZENELIB_UTIL_END

#endif //End of IZENE_UTIL_LOG_H
