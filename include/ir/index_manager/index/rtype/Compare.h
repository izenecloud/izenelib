#ifndef IZENELIB_IR_COMPARE_H_
#define IZENELIB_IR_COMPARE_H_

#include <types.h>
#include <boost/function.hpp>
#include <boost/type_traits.hpp>
#include <3rdparty/am/stx/btree_map.h>

NS_IZENELIB_IR_BEGIN
namespace indexmanager {
    
class Compare
{
    public:
        template<typename T>
        static int compare(const T& t1, const T& t2)
        {
            return compare_impl_(t1, t2, boost::is_arithmetic<T>());
        }
    private:
        template<typename T>
        static int compare_impl_(const T& t1, const T& t2, const boost::true_type& tp)
        {
            return t1<t2;
        }
        
        template<typename T>
        static int compare_impl_(const T& t1, const T& t2, const boost::false_type& tp)
        {
            return t1.compare(t2);
        }
};

}

NS_IZENELIB_IR_END

#endif
