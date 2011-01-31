#ifndef IZENELIB_DRIVER_WRITERS_JSON_WRITER_H
#define IZENELIB_DRIVER_WRITERS_JSON_WRITER_H
/**
 * @file izenelib/driver/writers/JsonWriter.h
 * @author Ian Yang
 * @date Created <2010-06-09 16:38:15>
 */
#include "../Writer.h"

namespace izenelib {
namespace driver {

class JsonWriter : public Writer
{
public:
    void write(const Value& value, std::string& document);
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_WRITERS_JSON_WRITER_H
