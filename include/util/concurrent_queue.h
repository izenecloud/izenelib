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

#include <queue>
namespace izenelib {
namespace util {

template<typename T>
class concurrent_queue
{
public:
    /**
     * @brief gets one element from queue
     * @param[out] t
     * Wait if queue is empty
     */
    void pop(T& t);

    /**
     * @brief appends one element into queue
     */
    void push(const T& t);

    void clear()
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        queue_.clear();
    }

    bool empty()
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        return queue_.empty();
    }

    std::size_t size()
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    std::deque<T> queue_;

    boost::mutex mutex_;
    boost::condition_variable cond_;
};

template<typename T>
void concurrent_queue<T>::pop(T& t)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    while(queue_.empty())
    {
        cond_.wait(lock);
    }
    t = queue_.front();
    queue_.pop_front();
}

template<typename T>
void concurrent_queue<T>::push(const T& t)
{
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        queue_.push_back(t);
    }
    cond_.notify_one();
}

}} // namespace izenelib::util

#endif // UTIL_CONCURRENT_QUEUE_H
