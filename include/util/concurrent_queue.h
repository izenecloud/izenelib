#ifndef UTIL_CONCURRENT_QUEUE_H
#define UTIL_CONCURRENT_QUEUE_H
/**
 * @file util/concurrent_queue.h
 * @author Ian Yang
 * @date Created <2009-11-01 21:50:36>
 * @date Updated <2009-11-04 17:14:51>
 */

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/function.hpp>

#include <queue>
namespace izenelib {
namespace util {

template<typename T>
class concurrent_queue
{
public:
    explicit concurrent_queue(std::size_t capacity = -1)
        : capacity_(capacity)
    {
    }

    /**
     * @brief gets one element from queue
     * @param[out] t
     * Wait if queue is empty
     */
    void pop(T& t);
    void pop(T& t, boost::function<void()> after_cb);

    /**
     * @brief appends one element into queue
     */
    void push(const T& t);

    void clear()
    {
        {
            boost::unique_lock<boost::mutex> lock(mutex_);
            //std::deque<T>().swap(queue_);
            queue_.clear();
        }
        write_cond_.notify_all();
    }

    bool empty()
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        return queue_.empty();
    }

    template<typename Predicate>
    void remove_if(Predicate pred)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        std::remove_if(queue_.begin(), queue_.end(), pred);
    }

    template<typename IfPredicate, typename WhenPredicate>
    bool remove_if_when(IfPredicate ifPred, WhenPredicate whenPred)
    {
        boost::unique_lock<boost::mutex> lock(mutex_);

        if (whenPred())
        {
            std::remove_if(queue_.begin(), queue_.end(), ifPred);
            return true;
        }

        return false;
    }

    std::size_t size()
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        return queue_.size();
    }

    void resize(std::size_t capacity = -1)
    {
        capacity_ = capacity;
    }

private:
    std::deque<T> queue_;
    std::size_t capacity_;

    boost::mutex mutex_;
    boost::condition_variable read_cond_;
    boost::condition_variable write_cond_;
};

template<typename T>
void concurrent_queue<T>::pop(T& t)
{
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        while (queue_.empty())
        {
            read_cond_.wait(lock);
        }
        t = queue_.front();
        queue_.pop_front();
    }
    write_cond_.notify_one();
}

template<typename T>
void concurrent_queue<T>::pop(T& t, boost::function<void()> after_cb)
{
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        while (queue_.empty())
        {
            read_cond_.wait(lock);
        }
        t = queue_.front();
        queue_.pop_front();
        if (after_cb)
            after_cb();
    }
    write_cond_.notify_one();
}

template<typename T>
void concurrent_queue<T>::push(const T& t)
{
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        while (queue_.size() >= capacity_)
        {
            write_cond_.wait(lock);
        }
        queue_.push_back(t);
    }
    read_cond_.notify_one();
}

}} // namespace izenelib::util

#endif // UTIL_CONCURRENT_QUEUE_H
