#ifndef IZENELIB_DRIVER_RENDERER_H
#define IZENELIB_DRIVER_RENDERER_H
/**
 * @file izenelib/driver/Renderer.h
 * @author Ian Yang
 * @date Created <2010-06-11 13:07:26>
 */
#include "Value.h"

namespace izenelib {
namespace driver {

class Renderer
{
public:
    /// @brief default render method just assign data to underlying value.
    template<typename T>
    void render(const T& data, Value& value)
    {
        value = data;
    }
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_RENDERER_H
