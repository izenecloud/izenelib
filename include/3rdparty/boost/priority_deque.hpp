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
    typedef typename Container::const_reference const_reference;
    typedef typename Container::size_type size_type;

    /// O(1)  Returns number of elements in deque.
    size_type size(void) const;
    /// O(1)  Returns true if deque has 0 elements. Returns false otherwise.
    bool empty(void) const;
    /// O(n)  Removes all elements from the deque.
    void clear(void);

    /// O(log n)  Adds a copy of an element to the deque.
    void push(const value_type&);

    /// O(1)  Returns a reference to the element with highest priority.
    const_reference top(void) const;
    /// O(log n)  Removes element with highest priority.
    void pop_top(void);
    // Alias of pop_top to maintain full compatibility with std::priority_queue
    inline void pop(void) { pop_top(); };

    /// O(1)  Returns a reference to the element with lowest priority.
    const_reference bottom(void) const;
    /// O(log n)  Removes element with lowest priority.
    void pop_bottom(void);

    /** Random-access.
     *  index must be in [0, size).
     *  Element order may change when any non-const function is called.
     */
    /// O(1)  Returns a const reference to the element at position [index].
    const_reference get(size_type) const;
    /// O(log n)  Copies input into position [index].
    void set(size_type, const value_type&);
    /// O(log n)  Removes element at position [index] from deque.
    void pop(size_type);

    /// O(1)  Swaps the contents of this deque with the contents of the input deque.
    void swap(priority_deque<value_type, Container, Compare>&);
    /// O(n)  Copies all elements from the source into this deque.
    void merge(const priority_deque<value_type, Container, Compare>&);

    /** O(n)  Creates a new priority deque.
     **       Inputs: Comparision object (Optional), Initial container (Optional)
     */
    explicit priority_deque(const Compare& = Compare(), const Container& = Container());
    /** O(n)  Creates a new priority deque from the elements in [first, last).
     *        Inputs: Start iterator, End iterator,
     *                Comparision object (Optional), Initial container (Optional)
     *        Iterators must support increment (++), dereference (*), and comparison
     *        (==) operators.
     */
    template <typename InputIterator>
    priority_deque(InputIterator first, InputIterator last, const Compare& = Compare(), const Container& = Container());

    /// O(n)  Replaces all elements in deque with the elements from the input.
    priority_deque<value_type, Container, Compare>& operator=(const Container&);

private:
    Compare compare_;
    Container container_;
    // Theoretical heap starts at 1. heap_offset is 1 - first_index.
    static const size_type heap_offset = 1;

    /// O(1)  Returns the index immediately above the current index.
    inline static size_type get_parent(size_type n) throw() { return (n - heap_offset) >> 1; };
    /// O(1)  Returns the first index immediately belod the current index.
    inline static size_type get_child(size_type n) throw() { return (n << 1) + heap_offset; };
    /// O(log log max_size) Returns whether the index is an odd (Min) layer.
    static bool check_min_layer(size_type) throw();

    /// O(log n)  Restores the heap property by moving an element up the heap.
    void bubble(size_type);
    void bubble_max(size_type);
    void bubble_min(size_type);
    /// O(log n)  Restores the heap property by moving an element down the heap.
    inline void trickle(size_type);
    void trickle_max(size_type);
    void trickle_min(size_type);
    /// O(n)  Restores the heap property in the entire heap.
    void heapify(void);
};

// Nothing but implementation from here on down.

//----------------------------Swap Specialization------------------------------|
template <class T, class Container, class Compare>
inline void priority_deque<T, Container, Compare>::swap(priority_deque<T, Container, Compare>& pq2)
{
    std::swap(container_, pq2.container_);
}

template <class T, class Container, class Compare>
inline void swap(priority_deque<T, Container, Compare>& pq1, priority_deque<T, Container, Compare>& pq2)
{
    pq1.swap(pq2);
}

//----------------------------Default Constructor------------------------------|
template <class T, class Container, class Compare>
priority_deque<T, Container, Compare>::priority_deque(const Compare& comp, const Container& cont) : compare_(comp), container_(cont)
{
    heapify();
}

//---------------------------Create from Iterators-----------------------------|
template <class T, class Container, class Compare>
template <class InputIterator>
priority_deque<T, Container, Compare>::priority_deque(InputIterator first, InputIterator last, const Compare& comp, const Container& cont) : compare_(comp), container_(cont)
{
    while (first != last)
        container_.push_back(*(first++));
    heapify();
}

//----------------------------Copy from Container------------------------------|
template <class T, class Container, class Compare>
priority_deque<T, Container, Compare>& priority_deque<T, Container, Compare>::operator=(const Container& cont)
{
    container_ = cont;
    heapify();
    return *this;
}

//-----------------------------------Merge-------------------------------------|
template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::merge(const priority_deque<T, Container, Compare>& other)
{
    for (size_type n = other.size(); n--;)
        container_.push_back(other.container_[n]);
    heapify();
}

template <class T, class Container, class Compare>
inline void priority_deque<T, Container, Compare>::clear(void)
{
    container_.clear();
};

//---------------------------------Accessors-----------------------------------|
template <class T, class Container, class Compare>
inline bool priority_deque<T, Container, Compare>::empty(void) const
{
    return container_.empty();
};

template <class T, class Container, class Compare>
inline typename priority_deque<T, Container, Compare>::size_type priority_deque<T, Container, Compare>::size(void) const
{
    return container_.size();
};

//-------------------------------Random Access---------------------------------|
template <class T, class Container, class Compare>
inline typename priority_deque<T, Container, Compare>::const_reference priority_deque<T, Container, Compare>::get(size_type n) const
{
    assert(n < size());
    return container_[n];
}

template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::set(size_type n, const value_type& value)
{
    assert(n < size());
    container_[n] = value;
    bubble(n);
    trickle(n);
}

template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::pop(size_type n)
{
    assert(n < size());
    container_[n] = container_.back();
    container_.pop_back();
    bubble(n);
    trickle(n);
}

//-----------------------------Restricted Access-------------------------------|
template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::push(const value_type& value)
{
    container_.push_back(value);
    bubble(container_.size() - 1);
}

//----------------------------------Maximum------------------------------------|
template <class T, class Container, class Compare>
inline typename priority_deque<T, Container, Compare>::const_reference priority_deque<T, Container, Compare>::top(void) const
{
    assert(size() > 0);
    return container_.front();
}

template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::pop_top(void)
{
    assert(size() > 0);
    container_[0] = container_.back();
    container_.pop_back();
    trickle_max(0);
}

//----------------------------------Minimum------------------------------------|
template <class T, class Container, class Compare>
typename priority_deque<T, Container, Compare>::const_reference priority_deque<T, Container, Compare>::bottom(void) const
{
    assert(size() > 0);
    if (size() <= 2)
        return container_.back();
    Compare const_compare = compare_;
    return container_[const_compare(container_[1], container_[2]) ? 1 : 2];
}

template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::pop_bottom(void)
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

//-----------------------------Private Functions-------------------------------|

//----------------------------------Heapify------------------------------------|
template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::heapify(void)
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

//----------------------------Determine Layer Type-----------------------------|
template <class T, class Container, class Compare>
bool priority_deque<T, Container, Compare>::check_min_layer(size_type index) throw()
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
//----------------------------------Bubble-------------------------------------|
template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::bubble(size_type n)
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
            std::swap(container_[n], container_[parent]);
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
            std::swap(container_[n], container_[parent]);
            bubble_min(parent);
        }
        else
            bubble_max(n);
    }
}

template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::bubble_max(size_type n)
{
    // Once it reaches the first 2 layers, quit.
    while (n > 3 - heap_offset)
    {
        const size_type grandparent = get_parent(get_parent(n));
        if (compare_(container_[grandparent], container_[n]))
        {
            std::swap(container_[n], container_[grandparent]);
            n = grandparent;
        }
        else
            break;
    }
}

template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::bubble_min(size_type n)
{
    // Once it reaches the first 2 layers, quit.
    while (n > 3 - heap_offset)
    {
        const size_type grandparent = get_parent(get_parent(n));
        if (compare_(container_[n], container_[grandparent]))
        {
            std::swap(container_[n], container_[grandparent]);
            n = grandparent;
        }
        else
            break;
    }
}

//----------------------------------Trickle------------------------------------|
template <class T, class Container, class Compare>
inline void priority_deque<T, Container, Compare>::trickle(size_type n)
{
    if (check_min_layer(n + heap_offset))
        trickle_min(n);
    else
        trickle_max(n);
}

template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::trickle_max(size_type n)
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
        std::swap(container_[n], container_[largest]);
        // Reached bottom of heap.
        if (largest <= child)
            break;
        n = largest;
        child = get_parent(n);
        if (compare_(container_[n], container_[child]))
            std::swap(container_[n], container_[child]);
    }
}

template <class T, class Container, class Compare>
void priority_deque<T, Container, Compare>::trickle_min(size_type n)
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
        std::swap(container_[n], container_[smallest]);
        // Reached bottom of heap.
        if (smallest <= child)
            break;
        n = smallest;
        child = get_parent(n);
        if (compare_(container_[child], container_[n]))
            std::swap(container_[n], container_[child]);
    }
}

}

#endif
