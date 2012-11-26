#ifndef _IZENELIB_IR_COMMONSET_INDEX_BASE_HPP_
#define _IZENELIB_IR_COMMONSET_INDEX_BASE_HPP_

#include <string>

namespace izenelib
{
namespace ir
{
namespace commonset
{

template< typename DocID = unsigned int, typename TokenID = unsigned int >
class IndexBase
{
public:

    IndexBase( std::string directory = "./" ) :
        directory_(directory)
    {
        if( directory_.size() > 1 && directory_[directory_.size()-1] != '/' ) directory_ += "/";
    }

    virtual ~IndexBase() {}

    std::string getDirectory() const
    {
        return directory_;
    }

private:

    std::string directory_;

};

}
}
}

#endif
