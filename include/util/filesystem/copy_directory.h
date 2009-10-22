#ifndef UTIL_FILESYSTEM_COPY_DIRECTORY_H
#define UTIL_FILESYSTEM_COPY_DIRECTORY_H
/**
 * @file util/filesystem/copy_directory.h
 * @author Ian Yang
 * @date Created <2009-10-22 11:46:10>
 * @date Updated <2009-10-22 14:06:14>
 */

#include <boost/filesystem.hpp>
#include <boost/utility/enable_if.hpp>

namespace izenelib {
namespace util {
namespace filesystem {

using namespace boost::filesystem;

/**
 * @brief copies regular files in \a from to \a to
 * @exception basic_filesystem_error<Path>
 *
 * Only copies regular files in directory \a from, with depth 1.
 */
template<typename Path>
typename boost::enable_if<is_basic_path<Path> >::type
copy_directory(const Path& from, const Path& to)
{
    remove_all(to);
    create_directory(to);

    static const basic_directory_iterator<Path> itEnd;
    for (basic_directory_iterator<Path> it(from);
         it != itEnd;
         ++it)
    {
        if (!is_directory(it->status()))
        {
            copy_file(it->path(), to / it->path().filename());
        }
    }
}

/**
 * @brief copies file recursively in \a from to \a to.
 * @exception basic_filesystem_error<Path>
 */
template<typename Path>
typename boost::enable_if<is_basic_path<Path> >::type
recursive_copy_directory(const Path& from, const Path& to)
{
    remove_all(to);
    create_directory(to);

    static const basic_directory_iterator<Path> itEnd;
    for (basic_directory_iterator<Path> it(from);
         it != itEnd;
         ++it)
    {
        if (is_directory(it->status()))
        {
            recursive_copy_directory(it->path(), to / it->path().filename());
        }
        else
        {
            copy_file(it->path(), to / it->path().filename());
        }
    }
}

inline void copy_directory(const path& from, const path& to)
{
    return copy_directory<path>(from, to);
}
inline void copy_directory(const wpath& from, const wpath& to)
{
    return copy_directory<wpath>(from, to);
}
inline void recursive_copy_directory(const path& from, const path& to)
{
    return recursive_copy_directory<path>(from, to);
}
inline void recursive_copy_directory(const wpath& from, const wpath& to)
{
    return recursive_copy_directory<wpath>(from, to);
}

}}} // namespace izenelib::util::filesystem

#endif // UTIL_FILESYSTEM_COPY_DIRECTORY_H
