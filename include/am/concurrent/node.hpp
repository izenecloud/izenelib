#ifndef IZENELIB_AM_CONCURRENT_NODE_HPP
#define IZENELIB_AM_CONCURRENT_NODE_HPP

#include <boost/thread.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>
#include <iostream>

namespace izenelib{ namespace am{ namespace concurrent{

template <typename key, typename value>
struct node : public boost::noncopyable
{
    typedef node<key,value> node_t;
    typedef boost::shared_ptr<node_t> shared_node;
    typedef std::vector<shared_node> next_array;


    typedef boost::mutex::scoped_lock scoped_lock;
    const key first;
    value second;
    const int top_layer;
    boost::mutex guard;
    next_array next;
    volatile bool marked;
    volatile bool fullylinked;
    node(const key& k_, const value& v_, size_t top)
        :first(k_),second(v_),top_layer(top),next(top+1),marked(false),fullylinked(false)
    {
    }
    ~node()
    {
        //std::cerr << "ptr:" << this << " key:" << k << " value:" << v << " removed\n" ;
    }
    bool is_valid()const
    {
        return !marked && fullylinked;
    }
    void dump()const
    {
        std::cerr << "key:" << first << " value:" << second << " lv:" << top_layer << " nexts[";
        {
            boost::io::ios_flags_saver dec(std::cerr);
            for(int i=0; i<=top_layer; i++)
            {
                if(next[i] != NULL)
                {
                    std::cerr << i << ":" << next[i]->first << " ";
                }
                else
                {
                    std::cerr << i << ":NULL ";
                }
            }
        }
        std::cerr << "] marked:" << marked << " fullylinked:" << fullylinked;
    }
} __attribute__((aligned (64)));

}}}
#endif
