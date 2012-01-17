/*****
* This is a C++ implementation of the paper "A Simple Optimistic skip-list Algorithm" 
*   written by Maurice Herlihy, Yossi Lev, Victor Luchangco, Nir Shavit.
*   http://www.cs.bgu.ac.il/~mpam092/wiki.files/LazySkipList.pdf
*   Author: Kumagi
*/
#ifndef IZENELIB_AM_CONCURRENT_SKIPLIST_HPP
#define IZENELIB_AM_CONCURRENT_SKIPLIST_HPP
#include "node.hpp"
#include <boost/foreach.hpp>
#include <boost/random.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/optional.hpp>
#include <utility>

namespace izenelib{ namespace am { namespace concurrent {
template <typename T>
void nothing_deleter(const T*)
{
    //std::cerr << "good morning all!" << std::endl;
} // do nothing

template <typename key,typename value, int height = 8>
class SkipList
{
    typedef SkipList<key,value,height> skipList_t;
    typedef node<key,value> node_t;
    typedef boost::shared_ptr<node_t> shared_node;
    typedef boost::weak_ptr<node_t> weak_node;
    typedef boost::array<node_t*, height> ptr_node_array;
    typedef boost::array<weak_node, height> weak_node_array;
    typedef boost::array<shared_node, height> locked_node_array;

    typedef std::pair<weak_node_array,weak_node_array> nodelists;
    typedef typename node_t::scoped_lock scoped_lock;

    typedef boost::shared_ptr<scoped_lock> scoped_lock_ptr;
    typedef boost::optional<typename node_t::scoped_lock> optional_lock;

public:
    class iterator
    {
        shared_node node;
    public:
        iterator() {}
        explicit iterator(shared_node& s):node(s) {}
        node_t& operator*()
        {
            return *node;
        }
        node_t* operator->()
        {
            return &operator*();
        }
        const node_t& operator*() const
        {
            return *node;
        }
        const node_t* operator->() const
        {
            return &operator*();
        }
        bool operator==(const iterator& rhs)const
        {
            return node.get() == rhs.node.get();
        }
        bool operator!=(const iterator& rhs)const
        {
            return !operator==(rhs);
        }
        shared_node& get()
        {
            return node;
        }
        const shared_node& get()const
        {
            return node;
        }
        iterator& operator++()
        {
            while(true)
            {
                if(node->next[0].get() != NULL)
                {
                    node = node->next[0];
                }
                if(node->marked) continue;
                break;
            }
            return *this;
        }
    private:

//		iterator(const iterator&);
    };

private:
    node_t head;
    iterator shared_head, shared_tail;
    boost::mt19937 engine;
    boost::uniform_smallint<> range;
    mutable boost::variate_generator<boost::mt19937&, boost::uniform_smallint<> > rand;
public:
    SkipList(const key& min, const key& max, int randomseed = 0)
        :head(min ,value(), height)
        ,engine(static_cast<unsigned long>(randomseed))
        ,range(0,(1<<height) -1),rand(engine,range)
    {
        shared_node head_node(&head,nothing_deleter<node_t>);
        shared_node tail_node(new node_t(max,value(),0));
        iterator head_iter(head_node);
        iterator tail_iter(tail_node);
        shared_head = head_iter;
        shared_tail = tail_iter;
        shared_node& tail = shared_tail.get();
        for(int i=0; i<height; i++)
        {
            head.next[i] = tail;
        }
        head.fullylinked = true;
    }
    ~SkipList()
    {
    }
    bool contains(const key& k)
    {
        nodelists lists;
        const int lv = find(k, &lists);
        weak_node_array& succs = lists.second;
        if(lv == -1) return false;
        shared_node succ = succs[lv].lock();
        if(!succ) return false;
        return succ->fullylinked
               && !succ->marked;
    }


    iterator get(const key& k)
    {
        nodelists lists;
        const int lv = find(k, &lists);
        weak_node_array& succs = lists.second;
        if(lv == -1) return end();
        shared_node succ = succs[lv].lock();
        if(!succ) return end();
        if(succ->fullylinked && !succ->marked)
        {
            return iterator(succ);
        }
        else
        {
            return end();
        }
    }

    iterator lower_bound(const key& k)
    {
        nodelists lists;
        const int lv = find(k, &lists);
        weak_node_array& currs = lists.first;
        weak_node_array& succs = lists.second;
        shared_node curr = currs[0].lock();
        if(lv == -1) return iterator(curr);
        shared_node succ = succs[lv].lock();
        if(!succ) return end();
        if(succ->fullylinked && !succ->marked)
        {
            return iterator(succ);
        }
        else
        {
            shared_node curr = currs[0].lock();
            return iterator(curr);
        }
    }

    bool add(const key& k, const value& v)
    {
        const int top_layer = random_level();
        assert(top_layer < height);
        nodelists lists;
        std::auto_ptr<node_t> newnode(new node_t(k, v, top_layer));
        while(true)
        {
            const int lv = find(k, &lists);
            weak_node_array& preds = lists.first;
            weak_node_array& succs = lists.second;

            if(lv != -1)
            {
                const node_t* found = succs[lv].lock().get();
                if(!found) continue;
                if(!found->marked)
                {
                    while(!found->fullylinked)
                    {
                        ;
                    }
                    return false;
                }
                usleep(1);
                continue;
            }
            bool valid = true;


            // get shared_ptr from weak_ptr array
            locked_node_array locked_preds;
            locked_node_array locked_succs;
            for(int i=0; i<=top_layer; ++i)
            {
                locked_preds[i] = preds[i].lock();
                locked_succs[i] = succs[i].lock();
                if(!locked_preds[i] || !locked_succs[i])
                {
                    valid = false;
                    break;
                }
            }
            if(!valid)
            {
                continue;
            }

            node_t *prev_pred = NULL;
            std::vector<scoped_lock_ptr> pred_locks(top_layer+1);
            for(int layer = 0; valid && (layer <= top_layer); ++layer)
            {
                node_t* pred = locked_preds[layer].get();
                const node_t* succ = locked_succs[layer].get();

                if(pred != prev_pred)
                {
                    pred_locks[layer] = scoped_lock_ptr(new scoped_lock(pred->guard));
                    prev_pred = pred;
                }
                assert(pred);
                assert(succ);
                valid = !pred->marked
                        && !succ->marked
                        && pred->next[layer].get() == succ;

            }
            if(!valid)
            {
//				std::cerr << "unlock \n";
                continue; // unlock all
            }

            // start to insert
            for(int i = 0; i<=top_layer; ++i)
            {
                newnode->next[i] = shared_node(succs[i]);
                //std::cerr << "[" << newnode->next[i] << "]f";
            }

            shared_node newnode_insert(newnode.get());
            newnode.release(); // it's all reason why I use auto_ptr
            for(int layer = 0; layer <= top_layer; ++layer)
            {
                locked_preds[layer]->next[layer] = newnode_insert;
            }
            newnode_insert->fullylinked = true;
            return true;
        }
        assert(!"never reach");
    }

    iterator& begin()
    {
        return shared_head;
    }
    const iterator& begin()const
    {
        return shared_head;
    }
    iterator& end()
    {
        return shared_tail;
    }
    const iterator& end()const
    {
        return shared_tail;
    }

    bool remove(const key& k)
    {
        nodelists lists;
        bool is_marked = false;
        int top_layer = -1;
        shared_node victim;
        assert(victim.get() == NULL);
        while(true)
        {

            int lv = find(k, &lists);
            weak_node_array& preds = lists.first;
            weak_node_array& succs = lists.second;

            if(lv != -1)
            {
                victim = succs[lv].lock();
                if(!victim) return false;
            }
            if(is_marked ||
                    (lv != -1 && (victim->fullylinked
                                  && victim->top_layer == lv
                                  && !victim->marked)))
            {
                if(is_marked == false)  // only one time done
                {
                    top_layer = victim->top_layer;
                    victim->guard.lock(); // lock
                    if(victim->marked)
                    {
                        victim->guard.unlock();
                        return false;
                    }
                    victim->marked = true;
                    is_marked = true;
                }

                bool valid = true;
                // get shared_ptr from weak_ptr array
                locked_node_array locked_preds;
                locked_node_array locked_succs;
                for(int i=0; i<=top_layer; ++i)
                {
                    locked_preds[i] = preds[i].lock();
                    locked_succs[i] = succs[i].lock();
                    if(!locked_preds[i] || !locked_succs[i])
                    {
                        valid = false;
                        break;
                    }
                }
                if(!valid)
                {
                    continue;
                }

                node_t *prev_pred = NULL;
                std::vector<scoped_lock_ptr> pred_locks(top_layer+1);
                for(int layer = 0; valid && (layer <= top_layer); ++layer)
                {
                    node_t *pred = locked_preds[layer].get();
                    const node_t *succ = locked_succs[layer].get();
                    if(pred != prev_pred)
                    {
                        pred_locks[layer] = scoped_lock_ptr(new scoped_lock(pred->guard));
                        prev_pred = pred;
                    }
                    valid = !pred->marked && pred->next[layer].get() == succ;
                }
                if(!valid)
                {
                    continue;
                }

                for(int layer = top_layer; layer>=0; --layer)
                {
                    locked_preds[layer]->next[layer] = victim->next[layer];
                }
                victim->guard.unlock();
                return true;
            }
            else
                return false;
        }
    }

    int find(const key& target, nodelists* lists)
    {
        int found = -1;

        shared_node pred = shared_head.get();
        shared_node curr;
        *lists = nodelists();
        //lists->second.resize(height);
        for(int lv = height-1; lv >= 0; --lv)
        {
            curr = pred->next[lv];
            while(curr->first < target)
            {
                pred = curr;
                curr = pred->next[lv];
            }
            if(found == -1 && target == curr->first)
            {
                found = lv;
            }
            lists->first[lv] = weak_node(pred);
            lists->second[lv] = weak_node(curr);
        }

        return found;
    }

    void dump()const
    {
        const node_t* p = &head;
        while(p != NULL)
        {
            p->dump();
            p = p->next[0].get();
            std::cerr << std::endl;
        }
    }

    bool is_empty()const
    {
        return head.next[0].get() == end().get().get();
    }

    void clear()
    {
        while(shared_head->next[0] != shared_tail.get())
        {
            for(int i=0; i<height; i++)
            {
                shared_tail->next[i] = shared_tail->next[0]->next[0];
            }
        }
    }
public:
    uint32_t random_level()const
    {
        const uint32_t gen = rand();
        int bit = 1;
        int cnt=0;
        while(cnt < height-1)
        {
            if(gen & bit)
            {
                return cnt;
            }
            bit <<= 1;
            ++cnt;
        }
        return cnt;
    }
private:
    SkipList();
    SkipList(const skipList_t&);
    skipList_t& operator=(const skipList_t&);
};

}}}

#endif
