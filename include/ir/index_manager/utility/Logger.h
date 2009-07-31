#ifndef LOGGER_H
#define LOGGER_H

#include <ir/index_manager/utility/system.h>

#include <ostream>
#include <fstream>
#include <iosfwd>
#include <sstream>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class log_delegater
{
public:
    log_delegater() {}
    virtual ~log_delegater() {}
public:
    virtual void log(const char* text)=0;
};


class basic_logbuf : public std::stringbuf
{
public:
    basic_logbuf():std::stringbuf(),delegater(NULL)
    {
    }
    basic_logbuf(log_delegater* delegater):delegater(delegater)
    {
    }
    virtual ~basic_logbuf()
    {
        delegater = NULL;
    }

public:
    void setDelegater(log_delegater* delegater_)
    {
        delegater = delegater_;
    }
    log_delegater* getDelegater()
    {
        return delegater;
    }
protected:
    int sync()
    {
        output_log_string(this->str().c_str());
        str(std::string());    // Clear the string buffer

        return 0;
    }
    void output_log_string(const char *text)
    {
        if (delegater) delegater->log(text);
    }
protected:
    log_delegater*	delegater;
};


class basic_logostream : public std::ostream
{
public:

    basic_logostream() : std::ostream(new basic_logbuf())
    {
    }
    basic_logostream(log_delegater* delegater) : std::ostream(new basic_logbuf(delegater))
    {
    }
    ~basic_logostream()
    {
        delete this->rdbuf();
    }
public:
    void setDelegater(log_delegater* delegater)
    {
        ((basic_logbuf*)this->rdbuf())->setDelegater(delegater);
    }
    log_delegater* getDelegater()
    {
        return ((basic_logbuf*)this->rdbuf())->getDelegater();
    }
};


typedef basic_logostream logostream;


class stream_delegater: public log_delegater
{
public:
    stream_delegater(std::ostream* os):ostream(os)
    {
    }
    virtual ~stream_delegater()
    {
        ostream = NULL;
    }
public:
    void log(const char* text)
    {
        if (ostream)
            (*ostream) << text;
    }
protected:
    std::ostream* ostream;
};


class file_delegater: public log_delegater
{
public:
    file_delegater(const char* logfile)
    {
        ostream.open(logfile,std::ios_base::out);
        if (!ostream.is_open())
        {
            SF1V5_THROW(ERROR_FILEIO,"open log file error.");
        }
    }
    virtual ~file_delegater()
    {
        if (ostream.is_open())
        {
            ostream.close();
        }
    }
public:
    void log(const char* text)
    {
        if (text)
            ostream << text;
    }
protected:
    std::ofstream ostream;
};

//////////////////////////////////////////////////////////////////////////
//basic_logger
typedef unsigned int level_type;

#define LOG_DEFINE_LEVEL(lvl, value) namespace level { const level_type lvl = (level_type)(value); }

LOG_DEFINE_LEVEL(disable_all, -1)
LOG_DEFINE_LEVEL(enable_all, 0)
LOG_DEFINE_LEVEL(default_level, 1000)
LOG_DEFINE_LEVEL(fatal, 2000)
LOG_DEFINE_LEVEL(err, 1600)
LOG_DEFINE_LEVEL(warn, 1200)
LOG_DEFINE_LEVEL(info, 1000)


#define SF1V5_LOG_(logger_,log_level)\
			{if(logger::isEnabled(log_level) == false) {;} else {logger_.level(log_level)
#define SF1V5_ENDL endl;}}
#define SF1V5_END "";}}

#define ENABLE_LOGS(log_level) basic_logger::enableLogs(log_level);

class basic_logger : public basic_logostream
{
public:
    basic_logger()
    {
        curLogLevel = level::info;
        delegater = new stream_delegater(&std::cout);
        setDelegater(delegater);
        ownDelegater = true;
    }
    basic_logger(std::ostream& os)
    {
        curLogLevel = level::info;
        delegater = new stream_delegater(&os);
        setDelegater(delegater);
    }
    ~basic_logger()
    {
        this->flush();
        if ((ownDelegater == true) && delegater)
        {
            delete delegater;
            delegater = NULL;
        }
    }
public:
    basic_logger& level(level_type level)
    {
        curLogLevel = level;
        return *this;
    }

    bool canLog()
    {
        return (curLogLevel >= basic_logger::logLevel);
    }

    static bool isEnabled(level_type lvl)
    {
        return (lvl >= logLevel);
    }
    static void enableLogs(level_type lvl)
    {
        logLevel = lvl;
    }
    static void enableLogs(const std::string& logstr)
    {
        if (logstr == "default_level")
        {
            enableLogs(level::default_level);
        }
        else if (logstr == "enable_all")
        {
            enableLogs(level::enable_all);
        }
        else if (logstr == "disable_all")
        {
            enableLogs(level::disable_all);
        }
        else if (logstr == "fatal")
        {
            enableLogs(level::fatal);
        }
        else if (logstr == "err")
        {
            enableLogs(level::err);
        }
        else if (logstr == "warn")
        {
            enableLogs(level::warn);
        }
        else if (logstr == "info")
        {
            enableLogs(level::info);
        }
        else
        {
            enableLogs(level::default_level);
        }
    }
public:

    void reset(std::ostream& os)
    {
        if ((ownDelegater == true) && delegater)
        {
            delete delegater;
        }
        delegater = new stream_delegater(&os);
        setDelegater(delegater);
        ownDelegater = true;
    }

    void	reset(const char* logfile)
    {
        if ((ownDelegater == true) && delegater)
        {
            delete delegater;
        }
        delegater = new file_delegater(logfile);
        setDelegater(delegater);
        ownDelegater = true;
    }

    void reset(log_delegater* deleg)
    {
        if ((ownDelegater == true) && delegater)
        {
            delete delegater;
        }
        delegater = deleg;
        setDelegater(delegater);
        ownDelegater = false;
    }
protected:
    log_delegater* delegater;
    level_type curLogLevel;
    bool ownDelegater;

    static level_type	logLevel;
};

typedef basic_logger logger;

extern logger Logger;

#define SF1V5_LOG(log_level)  SF1V5_LOG_(Logger,log_level)


}

NS_IZENELIB_IR_END

#endif
