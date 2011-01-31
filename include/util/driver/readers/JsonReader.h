#ifndef IZENELIB_DRIVER_READERS_JSON_READER_H
#define IZENELIB_DRIVER_READERS_JSON_READER_H
/**
 * @file core/driver/readers/JsonReader.h
 * @author Ian Yang
 * @date Created <2010-06-09 16:15:38>
 */
#include "../Reader.h"

namespace izenelib {
namespace driver {

class JsonReader : public Reader
{
public:
    bool read(const std::string& document, Value& value);
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_READERS_JSON_READER_H
