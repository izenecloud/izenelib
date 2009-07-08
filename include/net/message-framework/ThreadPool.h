///  @file ThreadPool.h
///  @date 2009-02-19
///  @author Wei Cao
///  @brief This file defines ThreadPool class

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <queue>

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

namespace messageframework
{

  class ThreadPool : private boost::noncopyable{

  public:

    typedef boost::function<void (void)> Task;

    ThreadPool(int workerNumber);

    ~ThreadPool();

    void executeTask(Task task);

    void stop_all();

  protected:

    void thread_body();

  private:

    int workerNumber_;

    boost::thread_group workerThreads_;

    bool stop_all_threads;

    /**
     * @brief Task queue, a task will be inserted to taskQueue.
     */
    boost::mutex taskQueueMutex_;
    std::queue<Task> taskQueue_;
    boost::condition_variable newTaskArrivalEvent_;
  };

}
#endif
