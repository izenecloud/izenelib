/*----------------------------------------------------------------------------*\
|   Copyright (C) 2012 Nathaniel McClatchey                                    |
|   Released under the Boost Software License Version 1.0, which may be found  |
| at http://www.boost.org/LICENSE_1_0.txt                                      |
\*----------------------------------------------------------------------------*/

#ifndef BOOST_CONTAINER_PRIORITY_DEQUE_HPP_
#define BOOST_CONTAINER_PRIORITY_DEQUE_HPP_ 1

#include <assert.h>     // Give useful debugging output.
#include <algorithm>    // Default swap function (std::swap)
#include <functional>   // Default comparison class (std::less)
#include <vector>       // Default container (std::vector)


namespace boost
{
namespace container
{

namespace detail
{

template <class T, class Container, class Compare>
class min_max_heap
{
public:
    typedef T value_type;
    typedef typename Container::const_reference const_reference;
    typedef typename Container::size_type size_type;
    typedef min_max_heap<T, Container, Compare> self_type;

    min_max_heap(const Compare& comp, const Container& cont) : compare_(comp), container_(cont)
    {
        heapify();
    }

    template <class InputIterator>
    min_max_heap(InputIterator first, InputIterator last, const Compare& comp, const Container& cont) : compare_(comp), container_(cont)
    {
        container_.insert(container_.end(), first, last);
        heapify();
    }

    self_type& operator=(const Container& cont)
    {
        container_ = cont;
        heapify();
        return *this;
    }

    inline void clear(void)
    {
        container_.clear();
    }
    inline bool empty(void) const
    {
        return container_.empty();
    }
    inline size_type size(void) const
    {
        return container_.size();
    }

    inline void swap(self_type& other)
    {
        std::swap(container_, other.container_);
    }

    void merge(const self_type& other)
    {
        container_.insert(container_.end(), other.container_.begin(), other.container_.end());
        heapify();
    }

    inline const_reference get(size_type n) const
    {
        assert(n < size());
        return container_[n];
    }

    void set(size_type n, const value_type& value)
    {
        assert(n < size());
        container_[n] = value;
        bubble(n);
        trickle(n);
    }

    void pop(size_type n)
    {
        assert(n < size());
        container_[n] = container_.back();
        container_.pop_back();
        bubble(n);
        trickle(n);
    }

    void push(const value_type& value)
    {
        container_.push_back(value);
        bubble(container_.size() - 1);
    }

    inline const_reference top(void) const
    {
        assert(size() > 0);
        return container_[0];
    }
    void pop_top(void)
    {
        assert(size() > 0);
        container_[0] = container_.back();
        container_.pop_back();
        trickle_max(0);
    }

    inline const_reference bottom(void) const
    {
        assert(size() > 0);
        if (size() <= 2)
            return container_.back();
        return std::min(container_[1], container_[2], compare_);
    }
    void pop_bottom(void)
    {
        assert(size() > 0);
        if (size() <= 2)
        {
            container_.pop_back();
            return;
        }
        const size_type n = compare_(container_[1], container_[2]) ? 1 : 2;
        container_[n] = container_.back();
        container_.pop_back();
        trickle_min(n);
    }

private:
    Compare compare_;
    Container container_;

    static const size_type heap_offset = 1;

    inline void swap_elements(size_type x, size_type y) { std::swap(container_[x], container_[y]); }
    inline static size_type get_parent(size_type n) throw() { return (n - heap_offset) >> 1; }
    inline static size_type get_child(size_type n) throw() { return (n << 1) + heap_offset; }

    static bool check_min_layer(size_type index) throw()
    {
        // Layer is floor(log2(m + heap_offset)). Return true if it is odd.
        size_type mask = (~static_cast<size_type>(0)) << (sizeof(size_type) << 2);
        for (size_t shift = sizeof(size_type) << 1; shift; shift >>= 1)
        {
            if (index & mask)
                index &= mask;
            mask ^= (~mask) << shift;
        }
        return index & mask;
    }

    void bubble(size_type n)
    {
        // The top element in the heap has nowhere to go.
        if (n <= 1 - heap_offset)
            return;

        const size_type parent = get_parent(n);
        if (check_min_layer(n + heap_offset))
        {
            if (compare_(container_[parent], container_[n]))
            {
                // Parent is a max layer, so heap property is violated.
                swap_elements(n, parent);
                bubble_max(parent);
            }
            else
                bubble_min(n);
        }
        else
        {
            if (compare_(container_[n], container_[parent]))
            {
                // Parent is a min layer, so heap property is violated.
                swap_elements(n, parent);
                bubble_min(parent);
            }
            else
                bubble_max(n);
        }
    }
    void bubble_min(size_type n)
    {
        // Once it reaches the first 2 layers, quit.
        while (n > 3 - heap_offset)
        {
            const size_type grandparent = get_parent(get_parent(n));
            if (compare_(container_[n], container_[grandparent]))
            {
                swap_elements(n, grandparent);
                n = grandparent;
            }
            else
                break;
        }
    }
    void bubble_max(size_type n)
    {
        // Once it reaches the first 2 layers, quit.
        while (n > 3 - heap_offset)
        {
            const size_type grandparent = get_parent(get_parent(n));
            if (compare_(container_[grandparent], container_[n]))
            {
                swap_elements(n, grandparent);
                n = grandparent;
            }
            else
                break;
        }
    }

    inline void trickle(size_type n)
    {
        if (check_min_layer(n + heap_offset))
            trickle_min(n);
        else
            trickle_max(n);
    }
    void trickle_min(size_type n)
    {
        const size_type elements = size();
        for (size_type smallest = n;;)
        {
            size_type child = get_child(n);
            size_type grandchild = get_child(child);

            for (int i = 5; --i; ++grandchild)
            {
                if (grandchild >= elements)
                {
                    // If a child node has no children of its own, it may be the smallest element.
                    if ((i == 4) && (child < elements) && compare_(container_[child], container_[smallest]))
                        smallest = child;
                    if ((++child < elements) && compare_(container_[child], container_[smallest]))
                        smallest = child;
                    break;
                }
                if (compare_(container_[grandchild], container_[smallest]))
                    smallest = grandchild;
            }
            // Reached bottom of heap or heap property not violated.
            if (smallest == n)
                break;
            swap_elements(n, smallest);
            // Reached bottom of heap.
            if (smallest <= child)
                break;
            n = smallest;
            child = get_parent(n);
            if (compare_(container_[child], container_[n]))
                swap_elements(n, child);
        }
    }
    void trickle_max(size_type n)
    {
        const size_type elements = size();
        for (size_type largest = n;;)
        {
            size_type child = get_child(n);
            size_type grandchild = get_child(child);

            for (int i = 5; --i; ++grandchild)
            {
                if (grandchild >= elements)
                {
                    // If a child node has no children of its own, it may be the largest element.
                    if ((i == 4) && (child < elements) && compare_(container_[largest], container_[child]))
                        largest = child;
                    if ((++child < elements) && compare_(container_[largest], container_[child]))
                        largest = child;
                    break;
                }
                if (compare_(container_[largest], container_[grandchild]))
                    largest = grandchild;
            }
            // Reached bottom of heap or heap property not violated.
            if (largest == n)
                break;
            swap_elements(n, largest);
            // Reached bottom of heap.
            if (largest <= child)
                break;
            n = largest;
            child = get_parent(n);
            if (compare_(container_[n], container_[child]))
                swap_elements(n, child);
        }
    }

    void heapify(void)
    {
        if (size() <= 1)
            return;
        size_type m = get_parent(container_.size() - 1);
        size_type n = m + heap_offset;
        // Calculate 1 << floor(log2(m + heap_offset))
        size_type mask = (~static_cast<size_type>(0)) << (sizeof(size_type) << 2);
        for (size_t shift = sizeof(mask) << 1; shift; shift >>= 1)
        {
            if (n & mask)
                n &= mask;
            mask ^= (~mask) << shift;
        }
        if (n & mask)
            n &= mask;
        /** Assume that only the peak of each subheap violates heap property; now fix.
         *  Assumption is true if we have already fixed the subheap's child heaps or the
         *  subheap has depth <= 2
         */
        do
        {
            if (n & mask) // Odd (Min) layer
                trickle_min(m);
            else
                trickle_max(m);
            if (m + heap_offset == n)
                n >>= 1;
        } while (m--);
    }
};

template <class T, class Container, class Compare>
class interval_heap
{
public:
    typedef T value_type;
    typedef typename Container::const_reference const_reference;
    typedef typename Container::size_type size_type;
    typedef interval_heap<T, Container, Compare> self_type;

    interval_heap(const Compare& comp, const Container& cont) : compare_(comp), container_(cont)
    {
        heapify();
    }

    template <class InputIterator>
    interval_heap(InputIterator first, InputIterator last, const Compare& comp, const Container& cont) : compare_(comp), container_(cont)
    {
        container_.insert(container_.end(), first, last);
        heapify();
    }

    self_type& operator=(const Container& cont)
    {
        container_ = cont;
        heapify();
        return *this;
    }

    inline void clear(void)
    {
        container_.clear();
    }
    inline bool empty(void) const
    {
        return container_.empty();
    }
    inline size_type size(void) const
    {
        return container_.size();
    }

    inline void swap(self_type& other)
    {
        std::swap(container_, other.container_);
    }

    void merge(const self_type& other)
    {
        container_.insert(container_.end(), other.container_.begin(), other.container_.end());
        heapify();
    }

    inline const_reference get(size_type n) const
    {
        assert(n < size());
        return container_[n];
    }

    void set(size_type n, const value_type& value)
    {
        assert(n < size());
        container_[n] = value;
        if (n & 0x1)
        {
            if (compare_(container_[n - 1], container_[n]))
            {
                swap_elements(n - 1, n);
                bubble_max(n - 1);
                trickle_max(n - 1);
            }
            else
            {
                bubble_min(n);
                trickle_min(n);
            }
        }
        else
        {
            if (size() > n + 1 && compare_(container_[n], container_[n + 1]))
            {
                swap_elements(n, n + 1);
                bubble_min(n + 1);
                trickle_min(n + 1);
            }
            else
            {
                bubble_max(n);
                trickle_max(n);
            }
        }
    }

    void pop(size_type n)
    {
        assert(n < size());
        container_[n] = container_.back();
        container_.pop_back();
        if (n & 0x1)
        {
            if (compare_(container_[n - 1], container_[n]))
            {
                swap_elements(n - 1, n);
                bubble_max(n - 1);
                trickle_max(n - 1);
            }
            else
            {
                bubble_min(n);
                trickle_min(n);
            }
        }
        else
        {
            if (size() > n + 1 && compare_(container_[n], container_[n + 1]))
            {
                swap_elements(n, n + 1);
                bubble_min(n + 1);
                trickle_min(n + 1);
            }
            else
            {
                bubble_max(n);
                trickle_max(n);
            }
        }
    }

    void push(const value_type& value)
    {
        size_type n = size();
        container_.push_back(value);
        if (n & 0x1)
        {
            if (compare_(container_[n - 1], container_[n]))
            {
                swap_elements(n - 1, n);
                bubble_max(n - 1);
            }
            else
            {
                bubble_min(n);
            }
        }
        else
        {
            if (size() > n + 1 && compare_(container_[n], container_[n + 1]))
            {
                swap_elements(n, n + 1);
                bubble_min(n + 1);
            }
            else
            {
                bubble_max(n);
            }
        }
    }

    inline const_reference top(void) const
    {
        assert(size() > 0);
        return container_[0];
    }
    void pop_top(void)
    {
        assert(size() > 0);
        size_type n = size() - 1;
        if (n == 1)
        {
            container_[0] = container_[n];
            container_.pop_back();
            return;
        }
        if (n & 0x1)
        {
            container_[0] = container_[n - 1];
            container_[n - 1] = container_[n];
            container_.pop_back();
            trickle_max(0);
        }
        else
        {
            container_[0] = container_[n];
            container_.pop_back();
            trickle_max(0);
        }
    }

    inline const_reference bottom(void) const
    {
        assert(size() > 0);
        if (size() == 1)
            return container_[0];
        return container_[1];
    }
    void pop_bottom(void)
    {
        assert(size() > 0);
        if (size() <= 2)
        {
            container_.pop_back();
            return;
        }
        container_[1] = container_.back();
        container_.pop_back();
        trickle_min(1);
    }

private:
    Compare compare_;
    Container container_;

    inline void swap_elements(size_type x, size_type y) { std::swap(container_[x], container_[y]); }
    inline static size_type get_parent(size_type n) throw() { return (n + (n & 0x3) - 4) >> 1; }
    inline static size_type get_min_child(size_type n) throw() { return (n << 1) + 1; }
    inline static size_type get_max_child(size_type n) throw() { return (n + 1) << 1; }

    void bubble_min(size_type n)
    {
        if (n == 1) return;
        size_type parent = get_parent(n);
        while (compare_(container_[n], container_[parent]))
        {
            swap_elements(n, parent);
            if (parent == 1) break;
            n = parent;
            parent = get_parent(n);
        }
    }
    void bubble_max(size_type n)
    {
        if (n == 0) return;
        size_type parent = get_parent(n);
        while (compare_(container_[parent], container_[n]))
        {
            swap_elements(n, parent);
            if (parent == 0) break;
            n = parent;
            parent = get_parent(n);
        }
    }

    void trickle_min(size_type n)
    {
        size_type child = get_min_child(n);
        if (size() <= child) return;
        size_type smallest = n;
        if (compare_(container_[child], container_[smallest]))
        {
            smallest = child;
        }
        if (size() > (child += 2) && compare_(container_[child], container_[smallest]))
        {
            smallest = child;
        }
        while (n != smallest)
        {
            swap_elements(n, smallest);
            if (compare_(container_[smallest - 1], container_[smallest]))
            {
                swap_elements(smallest - 1, smallest);
                trickle_max(smallest - 1);
                return;
            }
            n = smallest;
            if (size() <= (child = get_min_child(n))) return;
            if (compare_(container_[child], container_[smallest]))
            {
                smallest = child;
            }
            if (size() > (child += 2) && compare_(container_[child], container_[smallest]))
            {
                smallest = child;
            }
        }
    }
    void trickle_max(size_type n)
    {
        size_type child = get_max_child(n);
        if (size() <= child) return;
        size_type largest = n;
        if (compare_(container_[largest], container_[child]))
        {
            largest = child;
        }
        if (size() > (child += 2) && compare_(container_[largest], container_[child]))
        {
            largest = child;
        }
        while (n != largest)
        {
            swap_elements(n, largest);
            if (size() <= largest + 1) break;
            if (compare_(container_[largest], container_[largest + 1]))
            {
                swap_elements(largest, largest + 1);
                trickle_min(largest + 1);
                return;
            }
            n = largest;
            if (size() <= (child = get_max_child(n))) return;
            if (compare_(container_[largest], container_[child]))
            {
                largest = child;
            }
            if (size() > (child += 2) && compare_(container_[largest], container_[child]))
            {
                largest = child;
            }
        }
    }

    void heapify(void)
    {
        size_type n = size();
        if (n <= 1) return;
        for (size_t i = (n & 0x1) ? n - 3: n - 2;; i-= 2)
        {
            if (compare_(container_[i], container_[i + 1]))
            {
                swap_elements(i, i + 1);
            }
            trickle_max(i - 1);
            trickle_min(i);
            if (i == 0) break;
        }
    }
};

}

template <class T, class Container = std::vector<T>, class Compare = std::less<typename Container::value_type> >
class priority_deque;

/** O(1)  Swaps the contents of the input deques.
 *        Partial template specialization impossible.
 */
template <class T, class Container, class Compare>
void swap(priority_deque<T, Container, Compare>& pq1, priority_deque<T, Container, Compare>& pq2);

/// Double-ended priority queue.
template <class T, class Container, class Compare>
class priority_deque
{
public:
    // Some containers may change the type (eg. vector<bool>), so I need to change the type too.
    typedef T value_type;
    typedef detail::min_max_heap<T, Container, Compare> heap_type;
    typedef typename heap_type::const_reference const_reference;
    typedef typename heap_type::size_type size_type;
    typedef priority_deque<T, Container, Compare> self_type;

    /// O(1)  Returns number of elements in deque.
    inline size_type size(void) const { return heap_.size(); }
    /// O(1)  Returns true if deque has 0 elements. Returns false otherwise.
    inline bool empty(void) const { return heap_.empty(); }
    /// O(n)  Removes all elements from the deque.
    inline void clear(void) { heap_.clear(); }

    /// O(log n)  Adds a copy of an element to the deque.
    inline void push(const value_type& value) { heap_.push(value); }

    /// O(1)  Returns a reference to the element with highest priority.
    inline const_reference top(void) const { return heap_.top(); }
    /// O(log n)  Removes element with highest priority.
    inline void pop_top(void) { heap_.pop_top(); }
    // Alias of pop_top to maintain full compatibility with std::priority_queue
    inline void pop(void) { pop_top(); }

    /// O(1)  Returns a reference to the element with lowest priority.
    inline const_reference bottom(void) const { return heap_.bottom(); }
    /// O(log n)  Removes element with lowest priority.
    inline void pop_bottom(void) { heap_.pop_bottom(); }

    /** Random-access.
     *  index must be in [0, size).
     *  Element order may change when any non-const function is called.
     */
    /// O(1)  Returns a const reference to the element at position [index].
    inline const_reference get(size_type n) const { return heap_.get(n); }
    /// O(log n)  Copies input into position [index].
    inline void set(size_type n, const value_type& value) { heap_.set(n, value); }
    /// O(log n)  Removes element at position [index] from deque.
    inline void pop(size_type n) { heap_.pop(n); }

    /// O(1)  Swaps the contents of this deque with the contents of the input deque.
    inline void swap(self_type& other) { heap_.swap(other.heap_); }
    /// O(n)  Copies all elements from the source into this deque.
    inline void merge(const self_type& other) { heap_.merge(other.heap_); }

    /** O(n)  Creates a new priority deque.
     **       Inputs: Comparision object (Optional), Initial container (Optional)
     */
    explicit priority_deque(const Compare& comp = Compare(), const Container& cont = Container())
        : heap_(comp, cont)
    {
    }
    /** O(n)  Creates a new priority deque from the elements in [first, last).
     *        Inputs: Start iterator, End iterator,
     *                Comparision object (Optional), Initial container (Optional)
     *        Iterators must support increment (++), dereference (*), and comparison
     *        (==) operators.
     */
    template <typename InputIterator>
    priority_deque(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Container& cont = Container())
        : heap_(first, last, comp, cont)
    {
    }

    /// O(n)  Replaces all elements in deque with the elements from the input.
    self_type& operator=(const Container& cont)
    {
        heap_ = cont;
        return *this;
    }

private:
    heap_type heap_;
};

template <class T, class Container, class Compare>
inline void swap(priority_deque<T, Container, Compare>& pq1, priority_deque<T, Container, Compare>& pq2)
{
    pq1.swap(pq2);
}

}
}

#endif
