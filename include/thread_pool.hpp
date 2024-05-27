/*
 *  本线程池基本思路：
 *    1.用封装的有本地队列的CoreThread作为常驻核心线程，这种CoreThread优先从自己的任务队列中获取任务，
 *      其次从线程池任务队列，最后可以从其他核心线程队列窃取任务。
 *    2.用普通线程作为暂时辅助线程，在任务队列满时临时拉起一个线程，将用户提交的任务直接作为自己的任务执行。
 *      而后从线程池任务队列获取任务执行，在池任务队列空后，空等超时3次销毁自己。
 *    3.用户提交任务超出线程池接收上限后触发拒绝策略。
 *    4.核心线程可以批量获取线程。
 */

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <thread>
#include <future>
#include <atomic>
#include <chrono>

#include "global_define.h"
#include "reject_policy/reject_policy.hpp"
#include "thread/core_thread.h"

class ThreadPool {
 public:
  enum Unit {
    Hours,
    Minutes,
    Second,
    Millisecond,
    Microsecond,
    Nanosecond
  };

  ThreadPool();
  ThreadPool(int maxCount, int coreCount, int tQueueLength);
  ThreadPool(int maxCount, int coreCount, int tQueueLength, Policy p, int lTime, Unit u);
  ThreadPool(int maxCount, int coreCount, int tQueueLength, Policy p, int lTime, Unit u, bool is_batch_io);
  ThreadPool(int maxCount, int coreCount, int tQueueLength, Policy p, int lTime, Unit u, bool is_batch_io, bool is_bind_cpu);
  ~ThreadPool();

  // Task 移动构造的版本
  template<class Func, class... Args>
  bool Commit(Func&& task, Args&&... args) {
    bool ret = true;
    int index{get_real_index()};

    if (index < coreThreadCount) {
      // 进核心线程本地队列
      std::cout << "Push to core thread[" << index << "] local task queue\n";
      core_thread_queue_[index]->PushTask(std::move(std::bind(task, args...)));
    } else if (pool_task_queue_.Size() < taskQueueLength) {
      // 池任务队列不满，进池任务队列
      std::cout << "Push to pool task queue\n";
      pool_task_queue_.Push(std::move(std::bind(task, args...)));
      cv.notify_one();
    } else if (livingThread < maxThreadCount) {
      // 队列满，开非核心线程
      std::cout<<"Create none core thread\n";
      none_core_thread_queue_.push_back(std::thread(&ThreadPool::none_core_thread_function, this, std::bind(task, args...)));
      livingThread++;
    } else {
      // 队列满，线程数也到达最大线程数，调用拒绝策略
      RejectPolicyFactory::getInstance().getRejectPolicy(policy)->reject(std::bind(task,args...));
      ret = false;
    }

    return ret;
  }

  // Task 拷贝构造的版本
  template<class Func, class... Args>
  bool Commit(Func const& task, Args const& ...args) {
    bool ret = true;
    int index{get_real_index()};

    if (index < coreThreadCount) {
      // 进核心线程本地队列
      std::cout << "Push to core thread[" << index << "] local task queue\n";
      core_thread_queue_[index]->PushTask(std::move(std::bind(task, args...)));
    } else if (pool_task_queue_.Size() < taskQueueLength) {
      // 池任务队列不满，进池任务队列
      std::cout << "Push to pool task queue\n";
      pool_task_queue_.Push(std::move(std::bind(task, args...)));
      cv.notify_one();
    } else if (livingThread < maxThreadCount) {
      // 队列满，开非核心线程
      std::cout<<"Create none core thread \n";
      none_core_thread_queue_.push_back(std::thread(&ThreadPool::none_core_thread_function, this, std::bind(task, args...)));
      livingThread++;
    } else {
      // 队列满，线程数也到达最大线程数，调用拒绝策略
      RejectPolicyFactory::getInstance().getRejectPolicy(policy)->reject(std::bind(task,args...));
      ret = false;
    }

    return ret;
  }

 #if 0
  // 返回线程执行结果的版本，由于任务队列类型不一致这个重载版本暂时不可用
  template<class Func, class... Args>
  auto Commit(Func&& func, Args&&... args) -> std::future<decltype(std::declval<Func>()())> {
    using ResultType = decltype(std::declval<Func>()());
    std::packaged_task<ResultType()> task(std::bind(func, args...));
    std::future<ResultType> ret = task.get_future();

    int index{get_real_index()};
    if (index < coreThreadCount) {
      // 进核心线程本地队列
      std::cout << "Push to core thread[" << index << "] local task queue\n";
      core_thread_queue_[index]->PushTask(std::move(task));
    } else if (pool_task_queue_.Size() < taskQueueLength) {
      // 池任务队列不满，进池任务队列
      std::cout << "Push to pool task queue\n";
      pool_task_queue_.Push(std::move(task));
      cv.notify_one();
    } else if (livingThread < maxThreadCount) {
      // 队列满，开非核心线程
      std::cout << "Create none core thread\n";
      none_core_thread_queue_.push_back(std::thread(&ThreadPool::none_core_thread_function, this, std::move(task)));
      livingThread++;
    } else {
      // 队列满，线程数也到达最大线程数，调用拒绝策略
      RejectPolicyFactory::getInstance().getRejectPolicy(policy)->reject(std::move(task));
    }

    return ret;
  }
 #endif

  int  getRunningThreadCount();
  int  getLivingThreadCount();
  int  getCurrentTaskQueueSize();

 private:
    std::vector<std::shared_ptr<CoreThread>>  core_thread_queue_; // 核心本地队列线程
    std::vector<std::thread>                  none_core_thread_queue_; //非核心线程队列
    StealQueue<Task>                          pool_task_queue_;
    std::atomic<int>                          runningThread;
    std::atomic<int>                          runningTasks;
    std::atomic<int>                          livingThread;
    std::mutex                                mutex_;
    std::condition_variable                   cv;
    int                                       liveTime;
    Unit                                      unit;
    Policy                                    policy;
    bool                                      isShutdown;
    int                                       maxThreadCount;
    int                                       coreThreadCount;
    int                                       taskQueueLength;
    int                                       task_index_;
    int                                       none_core_thread_index_;
    bool                                      is_batch_io_;
    bool                                      is_bind_cpu_;

    /* Functions  */ 
    void  none_core_thread_function(std::function<void()>&&); //线程函数，起来以后是独立线程，任务函数执行在这个函数中
    int   getTaskQueueSize();
    void  joinAllThreads();
    void  init();
    void  getTid();
    int   get_real_index();
};

#endif