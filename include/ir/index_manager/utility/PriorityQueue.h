/**
* @file        PriorityQueue.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief An implementation of priority queue
*/
#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include <ir/index_manager/utility/system.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
 * A PriorityQueue maintains a partial ordering of its elements such that the
 * least element can always be found in constant time.  Put()'s and pop()'s
 *  require log(size) time.
 */
template <class Type>
class PriorityQueue
{

private:
    Type*	heap_;
    size_t	size_;
    bool		bDelete_;
    size_t	maxSize_;

    void upHeap()
    {
        size_t i = size_;
        Type node = heap_[i];			  // save bottom node (WAS object)
        int32_t j = ((uint32_t)i) >> 1;
        while (j > 0 && lessThan(node,heap_[j]))
        {
            heap_[i] = heap_[j];			  // shift parents down
            i = j;
            j = ((uint32_t)j) >> 1;
        }
        heap_[i] = node;				  // install saved node
    }
    void downHeap()
    {
        size_t i = 1;
        Type node = heap_[i];			  // save top node
        size_t j = i << 1;				  // find smaller child
        size_t k = j + 1;
        if (k <= size_ && lessThan(heap_[k], heap_[j]))
        {
            j = k;
        }
        while (j <= size_ && lessThan(heap_[j],node))
        {
            heap_[i] = heap_[j];			  // shift up child
            i = j;
            j = i << 1;
            k = j + 1;
            if (k <= size_ && lessThan(heap_[k], heap_[j]))
            {
                j = k;
            }
        }
        heap_[i] = node;				  // install saved node
    }

protected:
    PriorityQueue()
    {
        size_ = 0;
        bDelete_ = false;
        heap_ = NULL;
        maxSize_ = 0;
    }

    /**
     * Determines the ordering of objects in this priority queue.
     * Subclasses must define this one method.
     */
    virtual bool lessThan(Type a, Type b)=0;

    /**
     * Subclass constructors must call this.
     */
    void initialize(const size_t maxSize, bool deleteOnClear)
    {
        size_ = 0;
        bDelete_ = deleteOnClear;
        maxSize_ = maxSize;
        size_t heapSize = maxSize_ + 1;
        heap_ = new Type[heapSize];
    }

public:
    virtual ~PriorityQueue()
    {
        clear();
        delete[] heap_;
    }

    /**
     * Adds an Object to a PriorityQueue in log(size) time.
     * If one tries to add more objects than maxSize_ from initialize
     * a RuntimeException (ArrayIndexOutOfBound) is thrown.
     */
    void put(Type element)
    {
        if (size_ >= maxSize_)
            SF1V5_THROW(ERROR_GENERIC,"PriorityQueue::put():add is out of bounds");

        size_++;
        heap_[size_] = element;
        upHeap();
    }

    /**
     * Adds element to the PriorityQueue in log(size) time if either
     * the PriorityQueue is not full, or not lessThan(element, top()).
     * @param element
     * @return true if element is added, false otherwise.
     */
    bool insert(Type element)
    {
        if (size_ < maxSize_)
        {
            put(element);
            return true;
        }
        else if (size_ > 0 && !lessThan(element, top()))
        {
            if ( bDelete_ )
            {
                delete heap_[1];
            }
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
    Type top()
    {
        if (size_ > 0)
            return heap_[1];
        else
            return NULL;
    }

    /**
     * Removes and returns the least element of the PriorityQueue in log(size) time.
     */
    Type pop()
    {
        if (size_ > 0)
        {
            Type result = heap_[1];			  // save first value
            heap_[1] = heap_[size_];			  // move last to first

            heap_[size_] = (Type)0;			  // permit GC of objects
            size_--;
            downHeap();				  // adjust heap_
            return result;
        }
        else
            return (Type)NULL;
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
    size_t size()
    {
        return size_;
    }

    /**
     * Removes all entries from the PriorityQueue.
     */
    void clear()
    {
        for (size_t i = 1; i <= size_; i++)
        {
            if ( bDelete_ )
            {
                delete heap_[i];
            }
        }
        size_ = 0;
    }
    /** return element by position */
    Type operator [](size_t _pos)
    {
        return heap_[_pos+1];
    }
    Type getAt(size_t _pos)
    {
        return heap_[_pos+1];
    }


    void setDel(bool bDel)
    {
        bDelete_ = bDel;
    }

};


}

NS_IZENELIB_IR_END

#endif
