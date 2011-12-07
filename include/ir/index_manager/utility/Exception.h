/**
* @file        Exception.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief The definition of all IndexManagerException
*/
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <types.h>

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef unsigned short exception_type;

enum ExceptionCode
{
    ERROR_UNKNOWN,
    ERROR_GENERIC,
    ERROR_MISSING_PARAMETER,
    ERROR_BAD_PARAMETER,
    ERROR_FILEIO,
    ERROR_RUNTIME,
    ERROR_OUTOFMEM,
    ERROR_ILLEGALARGUMENT,
    ERROR_UNSUPPORTED,
    ERROR_OUTOFRANGE,
    ERROR_INDEX_COLLAPSE,
    ERROR_VERSION,
    ERROR_ASSERT,
    ERROR_EMPTY_BARREL,
    NUM_ERRORS
};

class IndexManagerException : public std::runtime_error
{
public:
    IndexManagerException(exception_type code)
    : std::runtime_error("")
    , code_(code)
    {
        try
        {
            imp_.reset(new Imp_);
        }
        catch (...)
        {
            imp_.reset();
        }
    }
    IndexManagerException(exception_type code, const std::string& msg)
    : std::runtime_error(msg)
    , code_(code)
    {
        try
        {
            imp_.reset(new Imp_);
        }
        catch (...)
        {
            imp_.reset();
        }
    }
    IndexManagerException(exception_type code,
                          const std::string& msg,
                          const std::string& file,
                          int line)
    : std::runtime_error(msg)
    , code_(code)
    {
        try
        {
            imp_.reset(new Imp_);
            imp_->file = file;
            imp_->line = line;
        }
        catch (...)
        {
            imp_.reset();
        }
    }

    ~IndexManagerException() throw() {}

    const char* what() const throw()
    // see http://www.boost.org/more/error_handling.html for lazy build rationale
    {
        if (imp_.get())
        {
            try
            {
                if (imp_->what.empty())
                {
                    imp_->what = std::runtime_error::what();
                    if (!imp_->what.empty())
                    {
                        imp_->what += ": ";
                    }
                    if (code_ < NUM_ERRORS)
                    {
                        imp_->what += s_errorStrings[code_];
                    }
                    else
                    {
                        imp_->what += s_errorStrings[ERROR_UNKNOWN];
                    }
                    if (!imp_->file.empty())
                    {
                        imp_->what += " at ";
                        imp_->what += imp_->file;
                        imp_->what += "(";
                        imp_->what +=
                            boost::lexical_cast<std::string>(imp_->line);
                        imp_->what += ")";
                    }
                }
                return imp_->what.c_str();
            }
            catch (...)
            {
                imp_->what.clear();
            }
        }

        return std::runtime_error::what();
    }
    virtual exception_type code() const throw()
    {
        return code_;
    }

private:
    exception_type code_;
    struct Imp_
    {
        std::string file;
        int line;
        std::string what;
    };
    boost::shared_ptr<Imp_> imp_;

    static std::string s_errorStrings[NUM_ERRORS];
};

class IllegalArgumentException : public IndexManagerException
{
public:
    IllegalArgumentException(const std::string& msg)
    : IndexManagerException(ERROR_ILLEGALARGUMENT, msg)
    {}
};
class UnsupportedOperationException : public IndexManagerException
{
public:
    UnsupportedOperationException(const std::string& msg)
    : IndexManagerException(ERROR_UNSUPPORTED, msg)
    {}
};
class FileIOException : public IndexManagerException
{
public:
    FileIOException(const std::string& msg)
    : IndexManagerException(ERROR_FILEIO, msg)
    {}
};
class OutOfMemoryException : public IndexManagerException
{
public:
    OutOfMemoryException(const std::string& msg)
    : IndexManagerException(ERROR_OUTOFMEM, msg)
    {}
};
class EmptyBarrelException : public IndexManagerException
{
public:
    EmptyBarrelException(const std::string& msg)
    : IndexManagerException(ERROR_EMPTY_BARREL, msg, __FILE__, __LINE__)
    {}
};

#define SF1V5_THROW(code,msg) throw ::izenelib::ir::indexmanager::IndexManagerException(code, msg, __FILE__, __LINE__)
#define SF1V5_RETHROW(e) throw ::izenelib::ir::indexmanager::IndexManagerException(e)

}

NS_IZENELIB_IR_END

#endif
