#ifndef IZENELIB_DRIVER_WRITERS_PRETTY_JSON_WRITER_H
#define IZENELIB_DRIVER_WRITERS_PRETTY_JSON_WRITER_H
/**
 * @file izenelib/driver/writers/PrettyJsonWriter.h
 * @author Ian Yang
 * @date Created <2010-07-02 10:40:45>
 * @brief Write the Value in JSON format in a human friendly way.
 */
#include "../Writer.h"

namespace izenelib {
namespace driver {

class PrettyJsonWriter : public Writer
{
public:
    void write(const Value& value, std::string& document);
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_WRITERS_PRETTY_JSON_WRITER_H
