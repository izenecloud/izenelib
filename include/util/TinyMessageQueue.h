#ifndef UTIL_TINY_MESSAGE_QUEUE_H
#define UTIL_TINY_MESSAGE_QUEUE_H
/**
 * @file util/TinyMessageQueue.h
 * @author Ian Yang
 * @date Created <2009-11-01 21:50:36>
 * @date Updated <2009-11-01 23:00:42>
 */

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include <queue>
namespace izenelib {
namespace util {

template<typename T>
class TinyMessageQueue
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

private:
    std::deque<T> queue_;

    boost::mutex mutex_;
    boost::condition_variable cond_;
};

template<typename T>
void TinyMessageQueue<T>::pop(T& t)
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
void TinyMessageQueue<T>::push(const T& t)
{
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        queue_.push_back(t);
    }
    cond_.notify_all();
}

}} // namespace izenelib::util

#endif // UTIL_TINY_MESSAGE_QUEUE_H
