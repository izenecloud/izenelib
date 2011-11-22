#ifndef IZENELIB_IR_COMPARE_H_
#define IZENELIB_IR_COMPARE_H_

#include <types.h>
#include <boost/function.hpp>
#include <boost/type_traits.hpp>
#include <3rdparty/am/stx/btree_map.h>

NS_IZENELIB_IR_BEGIN
namespace indexmanager {

template<typename T>
class Compare
{
    public:
        
        static int compare(const T& t1, const T& t2)
        {
            return compare_impl_(t1, t2, boost::is_arithmetic<T>());
        }
        
        bool operator()(const T& t1, const T& t2) 
        { 
            return compare(t1,t2)<0; 
        }
        
        static bool start_with(const T& t1, const T& t2)
        {
            if(t1.find(t2)!=0) return false;
            return true;
        }
        
        static bool end_with(const T& t1, const T& t2)
        {
            if( t1.length()<t2.length() ) return false;
            return t1.substr(t1.length() - t2.length(), t2.length()).compare(t2) == 0;
        }
        
        static bool contains(const T& t1, const T& t2)
        {
            if( t1.length()<t2.length() ) return false;
            if(t1.find(t2)==T::npos) return false;
            return true;
        }
        
    private:

        static int compare_impl_(const T& t1, const T& t2, const boost::true_type& tp)
        {
            if ( t1>t2)
            {
                return 1;
            }
            else if (t1<t2)
            {
                return -1;
            }
            else
                return 0;
        }
        
        static int compare_impl_(const T& t1, const T& t2, const boost::false_type& tp)
        {
            return t1.compare(t2);
        }
};

}

NS_IZENELIB_IR_END

#endif
