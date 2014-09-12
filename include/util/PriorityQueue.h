/**
* @file        PriorityQueue.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief An implementation of priority queue
*/
#ifndef IZENE_UTIL_PRIORITYQUEUE_H
#define IZENE_UTIL_PRIORITYQUEUE_H

#include <cstddef>

namespace izenelib
{
namespace util
{

/**
* This priorityqueue is used temporarily because it does not need to allocate
* memory on heap.
 */
template <class Type>
class PriorityQueue
{
public:
    typedef Type elem_type;

    virtual ~PriorityQueue()
    {
        delete[] heap_;
    }

    /**
     * Adds an Object to a PriorityQueue in log(size) time.
     * If one tries to add more objects than capacity_ from initialize
     * a RuntimeException (ArrayIndexOutOfBound) is thrown.
     */
    void put(const Type& element)
    {
        heap_[++size_] = element;
        upHeap();
    }

    /**
     * Adds element to the PriorityQueue in log(size) time if either
     * the PriorityQueue is not full, or not lessThan(element, top()).
     * @param element
     * @return true if element is added, false otherwise.
     */
    bool insert(const Type& element)
    {
        if (size_ < capacity_)
        {
            put(element);
            return true;
        }
        else if (lessThan(heap_[1], element))
        {
            heap_[1] = element;
            adjustTop();
            return true;
        }
        else
            return false;
    }

    /**
     * Returns the least element of the PriorityQueue in constant time.
     */
    const Type& top()
    {
        static const Type empty = Type();
        if (size_ > 0)
            return heap_[1];
        else
            return empty;
    }

    /**
     * Removes and returns the least element of the PriorityQueue in log(size) time.
     */
    Type pop()
    {
        if (size_ > 0)
        {
            Type result = heap_[1];     // save first value
            heap_[1] = heap_[size_];    // move last to first

            heap_[size_] = Type();      // permit GC of objects
            --size_;
            downHeap();                 // adjust heap_
            return result;
        }
        else
            return Type();
    }

    /**
     * Should be called when the object at top changes values.  Still log(n)
     * worst case, but it's at least twice as fast to <pre>
     * { pq.top().change(); pq.adjustTop(); }
     * </pre> instead of <pre>
     * { o = pq.pop(); o.change(); pq.push(o); }
     * </pre>
     */
    void adjustTop()
    {
        downHeap();
    }


    /**
     * Returns the number of elements currently stored in the PriorityQueue.
     */
    size_t size() const
    {
        return size_;
    }

    size_t capacity() const
    {
        return capacity_;
    }

    /** return element by position */
    Type& operator [](size_t _pos)
    {
        return heap_[_pos + 1];
    }
    Type& getAt(size_t _pos)
    {
        return heap_[_pos + 1];
    }

protected:
    PriorityQueue()
        : size_(0)
        , capacity_(0)
        , heap_(NULL)
    {
    }

    /**
     * Determines the ordering of objects in this priority queue.
     * Subclasses must define this one method.
     */
    virtual bool lessThan(const Type& a, const Type& b) const = 0;

    /**
     * Subclass constructors must call this.
     */
    void initialize(const size_t maxSize)
    {
        size_ = 0;
        capacity_ = maxSize;
        size_t heapSize = capacity_ + 1;
        heap_ = new Type[heapSize];
    }

private:
    void upHeap()
    {
        size_t i = size_;
        Type node = heap_[i];                               // save bottom node (WAS object)
        size_t j = i >> 1;
        while (j > 0 && lessThan(node,heap_[j]))
        {
            heap_[i] = heap_[j];                            // shift parents down
            i = j;
            j = j >> 1;
        }
        heap_[i] = node;                                    // install saved node
    }

    void downHeap()
    {
        size_t i = 1;
        Type node = heap_[i];                               // save top node
        size_t j = i << 1;                                  // find smaller child
        size_t k = j + 1;
        if (k <= size_ && lessThan(heap_[k], heap_[j]))
        {
            j = k;
        }
        while (j <= size_ && lessThan(heap_[j],node))
        {
            heap_[i] = heap_[j];                            // shift up child
            i = j;
            j = i << 1;
            k = j + 1;
            if (k <= size_ && lessThan(heap_[k], heap_[j]))
            {
                j = k;
            }
        }
        heap_[i] = node;                                    // install saved node
    }

private:
    size_t  size_;
    size_t  capacity_;
    Type*   heap_;
};

}
}

#endif
