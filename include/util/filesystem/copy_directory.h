#ifndef UTIL_FILESYSTEM_COPY_DIRECTORY_H
#define UTIL_FILESYSTEM_COPY_DIRECTORY_H
/**
 * @file util/filesystem/copy_directory.h
 * @author Ian Yang
 * @date Created <2009-10-22 11:46:10>
 * @date Updated <2009-10-27 17:09:22>
 */

#include <boost/filesystem.hpp>

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
inline void copy_directory(const path& from, const path& to)
{
    remove_all(to);
    create_directory(to);

    static const directory_iterator itEnd;
    for (directory_iterator it(from);
         it != itEnd;
         ++it)
    {
        if (!is_directory(*it))
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
template<typename Predicate>
inline void copy_directory_if(const path& from, const path& to, Predicate shouldCopy)
{
    remove_all(to);
    create_directory(to);

    static const directory_iterator itEnd;
    for (directory_iterator it(from);
         it != itEnd;
         ++it)
    {
        if (!is_directory(*it) && shouldCopy(it->path()))
        {
            copy_file(it->path(), to / it->path().filename());
        }
    }
}

/**
 * @brief copies file recursively in \a from to \a to.
 * @exception basic_filesystem_error<Path>
 */
inline void recursive_copy_directory(const path& from, const path& to)
{
    remove_all(to);
    create_directory(to);

    static const directory_iterator itEnd;
    for (directory_iterator it(from);
         it != itEnd;
         ++it)
    {
        if (is_directory(*it))
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
template<typename Predicate>
inline void recursive_copy_directory_if(const path& from, const path& to, Predicate shouldCopy)
{
    remove_all(to);
    create_directory(to);

    static const directory_iterator itEnd;
    for (directory_iterator it(from);
         it != itEnd;
         ++it)
    {
        if (shouldCopy(it->path()))
        {
            if (is_directory(*it))
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
