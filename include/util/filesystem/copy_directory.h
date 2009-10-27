#ifndef UTIL_FILESYSTEM_COPY_DIRECTORY_H
#define UTIL_FILESYSTEM_COPY_DIRECTORY_H
/**
 * @file util/filesystem/copy_directory.h
 * @author Ian Yang
 * @date Created <2009-10-22 11:46:10>
 * @date Updated <2009-10-27 17:09:22>
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
 * Only copies regular files in directory \a from, no sub-directory is
 * traversed.
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
 * @brief copies regular files which path satisfies \c Predicate \a shouldCopy
 * in \a from to \a to
 *
 * @tparam Path path type
 *
 * @tparam Predicate unary functor accepting an argument of type \c Path and
 * return \c true if the file should be copied.
 *
 * @exception basic_filesystem_error<Path>
 * Only copies regular files in directory \a from, no sub-directory is
 * traversed.
 */
template<typename Path, typename Predicate>
typename boost::enable_if<is_basic_path<Path> >::type
copy_directory_if(const Path& from, const Path& to, Predicate shouldCopy)
{
    remove_all(to);
    create_directory(to);

    static const basic_directory_iterator<Path> itEnd;
    for (basic_directory_iterator<Path> it(from);
         it != itEnd;
         ++it)
    {
        if (!is_directory(it->status()) && shouldCopy(it->path()))
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

/**
 * @brief copies files recursively in \a from to \a to.
 * @exception basic_filesystem_error<Path>
 */
/**
 * @brief copies files which path satisfies \c Predicate \a shouldCopy in \a
 * from to \a to recursively.
 *
 * @tparam Path path type
 *
 * @tparam Predicate unary functor accepting an argument of type \c Path and
 * return \c true if the file should be copied.
 *
 * @exception basic_filesystem_error<Path>
 */
template<typename Path, typename Predicate>
typename boost::enable_if<is_basic_path<Path> >::type
recursive_copy_directory_if(const Path& from, const Path& to, Predicate shouldCopy)
{
    remove_all(to);
    create_directory(to);

    static const basic_directory_iterator<Path> itEnd;
    for (basic_directory_iterator<Path> it(from);
         it != itEnd;
         ++it)
    {
        if (shouldCopy(it->path()))
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
}

inline void copy_directory(const path& from, const path& to)
{
    return copy_directory<path>(from, to);
}
inline void copy_directory(const wpath& from, const wpath& to)
{
    return copy_directory<wpath>(from, to);
}
template<typename Predicate>
inline void copy_directory_if(const path& from, const path& to, Predicate shouldCopy)
{
    return copy_directory_if<path>(from, to, shouldCopy);
}
template<typename Predicate>
inline void copy_directory_if(const wpath& from, const wpath& to, Predicate shouldCopy)
{
    return copy_directory_if<wpath>(from, to, shouldCopy);
}
inline void recursive_copy_directory(const path& from, const path& to)
{
    return recursive_copy_directory<path>(from, to);
}
inline void recursive_copy_directory(const wpath& from, const wpath& to)
{
    return recursive_copy_directory<wpath>(from, to);
}
template<typename Predicate>
inline void recursive_copy_directory_if(const path& from, const path& to, Predicate shouldCopy)
{
    return recursive_copy_directory_if<path>(from, to, shouldCopy);
}
template<typename Predicate>
inline void recursive_copy_directory_if(const wpath& from, const wpath& to, Predicate shouldCopy)
{
    return recursive_copy_directory_if<wpath>(from, to, shouldCopy);
}

}}} // namespace izenelib::util::filesystem

// expose to izenelib::util
namespace izenelib {
namespace util {
using filesystem::copy_directory;
using filesystem::recursive_copy_directory;
using filesystem::copy_directory_if;
using filesystem::recursive_copy_directory_if;
}} // namespace izenelib::util


#endif // UTIL_FILESYSTEM_COPY_DIRECTORY_H
