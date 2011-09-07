#include "reader.h"

#include <stdio.h>
#include <string.h>

#include <util/stringprintf.h>
#include <net/mrlite/common.h>

const int kDefaultMaxInputLineLength = 16 * 1024;    // 16 KB

namespace net{namespace mrlite{

CLASS_REGISTER_IMPLEMENT_REGISTRY(mapreduce_lite_reader_registry, Reader);
REGISTER_READER("text", TextReader);

//-----------------------------------------------------------------------------
// Implementation of Reader
//-----------------------------------------------------------------------------
void Reader::Open(const std::string& source_name)
{
    Close();  // Ensure to close pre-opened file.
    input_filename_ = source_name;
    input_stream_ = fopen(source_name.c_str(), "r");
    if (input_stream_ == NULL)
    {
        LOG(FATAL) << "Cannot open file: " << source_name ;
    }
}

void Reader::Close()
{
    if (input_stream_ != NULL)
    {
        fclose(input_stream_);
        input_stream_ = NULL;
    }
}

//-----------------------------------------------------------------------------
// Implementation of TextReader
//-----------------------------------------------------------------------------
TextReader::TextReader()
        : line_num_(0),
        reading_a_long_line_(false)
{
    try
    {
        line_.reset(new char[kDefaultMaxInputLineLength]);
    }
    catch (std::bad_alloc&)
    {
        LOG(FATAL) << "Cannot allocate line input buffer.";
    }
}

bool TextReader::Read(std::string* key, std::string* value)
{
    if (input_stream_ == NULL)
    {
        return false;
    }

    SStringPrintf(key, "%s-%010lld",
                  input_filename_.c_str(), ftell(input_stream_));
    value->clear();

    if (fgets(line_.get(), kDefaultMaxInputLineLength, input_stream_)
            == NULL)
    {
        return false;  // Either ferror or feof. Anyway, returns false to
        // notify the caller no further reading operations.
    }

    int read_size = strlen(line_.get());
    if (line_[read_size - 1] != '\n')
    {
        LOG(ERROR) << "Encountered a too-long line (line_num = " << line_num_
        << ").  May return one or more empty values while skipping "
        << " this long line.";
        reading_a_long_line_ = true;
        return true;  // Skip the current part of a long line.
    }
    else
    {
        ++line_num_;
        if (reading_a_long_line_)
        {
            reading_a_long_line_ = false;
            return true;  // Skip the last part of a long line.
        }
    }

    if (line_[read_size - 1] == '\n')
    {
        line_[read_size - 1] = '\0';
        if (read_size > 1 && line_[read_size - 2] == '\r')    // Handle DOS
        {
            // text format.
            line_[read_size - 2] = '\0';
        }
    }
    value->assign(line_.get());
    return true;
}

}}  // namespace mrlite
