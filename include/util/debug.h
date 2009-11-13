#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H
/**
 * @file util/debug.h
 * @author Ian Yang
 * @date Created <2009-11-10 15:13:25>
 * @date Updated <2009-11-10 15:16:28>
 */

#ifndef NDEBUG
#  define IZS_VAR_DUMP(var)                         \
std::cout << __FILE__ << "(" << __LINE__ << "): "   \
          << #var"=|" << (var) << "|" << std::endl

#else

#  define IZS_VAR_DUMP(var)

#endif

#endif // UTIL_DEBUG_H
