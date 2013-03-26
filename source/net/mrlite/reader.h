#ifndef MRLITE_READERS_H_
#define MRLITE_READERS_H_

#include <stdio.h>
#include <string>

#include <util/class_register.h>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

namespace net{namespace mrlite{

//-----------------------------------------------------------------------------
// The interface implemented by ``real'' readers.
//-----------------------------------------------------------------------------
class Reader
{
public:
    Reader()
    {
        input_stream_ = NULL;
    }
    virtual ~Reader()
    {
        Close();
    }

    virtual void Open(const std::string& source_name);
    virtual void Close();

    // Returns false to indicate that the current read failed and no
    // further reading operations should be performed.
    virtual bool Read(std::string* key, std::string* value) = 0;

protected:
    std::string input_filename_;
    FILE* input_stream_;
};


//-----------------------------------------------------------------------------
// Read each record as a line in a text file.
// - The key returned by Read() is "filename-offset", the value
//   returned by Read is the content of a line.
// - The value might be empty if it is reading a too long line.
// - The '\r' (if there is any) and '\n' at the end of a line are
//   removed.
//-----------------------------------------------------------------------------
class TextReader : public Reader
{
public:
    TextReader();
    virtual bool Read(std::string* key, std::string* value);
private:
    boost::scoped_array<char> line_;      // input line buffer
    int line_num_;                 // count line number
    bool reading_a_long_line_;     // is reading a lone line
};



CLASS_REGISTER_DEFINE_REGISTRY(mapreduce_lite_reader_registry, Reader);

#define REGISTER_READER(format_name, reader_name)       \
  CLASS_REGISTER_OBJECT_CREATOR(                        \
      mapreduce_lite_reader_registry,                   \
      Reader,                                           \
      format_name,                                      \
      reader_name)

#define CREATE_READER(format_name)              \
  CLASS_REGISTER_CREATE_OBJECT(                 \
      mapreduce_lite_reader_registry,           \
      format_name)

}}  // namespace mrlite

#endif  // MRLITE_READERS_H_
