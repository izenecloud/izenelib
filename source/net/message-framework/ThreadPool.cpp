#include <net/message-framework/ThreadPool.h>
#include <net/message-framework/MessageFrameworkException.h>

#include <boost/bind.hpp>

namespace messageframework
{

  ThreadPool::ThreadPool(int workerNumber) {
    workerNumber_ = workerNumber;
    stop_all_threads = false;

    for( int i=0; i < workerNumber_; i++ ) {
      workerThreads_.create_thread(boost::bind(&ThreadPool::thread_body, this) );
    }
  }

  ThreadPool::~ThreadPool() {
    stop_all();
  }

  void ThreadPool::stop_all() {
    stop_all_threads = true;
    newTaskArrivalEvent_.notify_all();
    workerThreads_.join_all();
  }

  void ThreadPool::thread_body() {
    while(!stop_all_threads) {
      boost::mutex::scoped_lock lock(taskQueueMutex_);
      while( taskQueue_.empty() && !stop_all_threads )
        newTaskArrivalEvent_.wait(lock);
      if(stop_all_threads)
        return;
      // get a task
      Task task = taskQueue_.front();
      taskQueue_.pop();
      lock.unlock();

      // execute the task
      // should caught any exceptions and errors
      try {
        task();
      }
      catch(MessageFrameworkException& e) {
        e.output(std::cout);
      }
    }
  }

  void ThreadPool::executeTask(Task task) {
    {
      boost::mutex::scoped_lock lock(taskQueueMutex_);
      taskQueue_.push(task);
    }
    newTaskArrivalEvent_.notify_all();
  }

}
