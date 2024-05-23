/*
 *  线程池的基本思路：
 *      两个队列（容器），一个线程队列，一个任务队列，
 *      从任务队列中取出任务，交给线程队列中的线程执行。
 *      循环往复就是线程池。
 *  具体实现：
 *      线程函数一直循环，取任务在自己内部执行，
 *      当取不到任务时阻塞自己等待任务。
 *
 *      任务队列每加入一个新的任务都需要唤醒一个线程。
 *
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
  ~ThreadPool();

  /**
   * 形参为右值引用，形参将从实参中“窃取”（移动赋值/移动构造）数据，避免了数据拷贝过程提高了性能。
   * 但是这个版本的函数只能接受 非const的右值类型的实参。
   * 当实参是右值时优先匹配这个函数。
   */
  template<class Func, class... Args>
  bool Commit(Func&& task, Args&&... args) {
    bool ret = true;
    int index{get_real_index()};

    if (index < coreThreadCount) {
      // 进核心线程本地队列
      std::cout << "push to core thread[" << index << "] local task queue\n";
      core_thread_queue_[index]->PushTask(std::move(std::bind(task, args...)));
    } else if (pool_task_queue_.Size() < taskQueueLength) {
      // 池任务队列不满，进池任务队列
      std::cout << "push to pool task queue\n";
      pool_task_queue_.Push(std::move(std::bind(task, args...)));
      cv.notify_one();
    } else if (livingThread < maxThreadCount) {
      // 队列满，开非核心线程
      std::cout<<"new std::thread \n";
      none_core_thread_queue_.push_back(std::thread(&ThreadPool::none_core_thread_function, this, std::bind(task, args...)));
      livingThread++;
    } else {
      // 队列满，线程数也到达最大线程数，调用拒绝策略
      // std::cout<<"reject task \n";
      RejectPolicyFactory::getInstance().getRejectPolicy(policy)->reject(std::bind(task,args...));
      ret = false;
    }

    return ret;
  }

  /**
   * 形参为const的左值引用，形参将从实参中“拷贝”（拷贝赋值/拷贝构造）数据
   * 这个版本的函数能接受任意类型实参对象（左右值都可以），但是由于数据需要从实参拷贝到形参，所以性能不如使用右值引用的函数
   * 当不存在形参为右值引用的函数时，这个const 左值引用为形参的函数也可以使用
   */
  // template<class Func, class... Args>
  // bool Commit(Func const& task, Args const& ...args) {
  //   bool ret = true;
  //   if(livingThread >= coreThreadCount && getCurrentTaskQueueSize() < taskQueueLength) {
  //     taskQueue.push(std::bind(task, args...));//任务队列没满先让任务进队列。
  //     cv.notify_one();
  //     runningThread++;
  //   }
  //   else if(livingThread < maxThreadCount && getCurrentTaskQueueSize() >= taskQueueLength) {
  //     //任务队列满了，但是活跃的线程数小于最大线程数，则开新的线程并将任务直接交给新线程。
  //     noneCoreThreadQueue.push_back(std::thread(&ThreadPool::threadFunction, this, std::bind(task, args...)));
  //     livingThread++;
  //   }
  //   else if(livingThread >= maxThreadCount && getCurrentTaskQueueSize() >= taskQueueLength) {
  //     //reject，队列满，线程数也到达最大线程数，这个时候调用拒绝策略
  //     RejectPolicyFactory::getInstance()->getRejectPolicy(policy)->reject(std::bind(task,args...));
  //     ret = false;
  //   }
  //   return ret;
  // }

  int  getRunningThreadCount();
  int  getLivingThreadCount();
  int  getCurrentTaskQueueSize();

 private:
    std::vector<std::shared_ptr<CoreThread>> core_thread_queue_; // 核心本地队列线程
    std::vector<std::thread>                 none_core_thread_queue_; //非核心线程队列
    StealQueue<Task>                    pool_task_queue_;
    std::atomic<int>                    runningThread;
    std::atomic<int>                    runningTasks;
    std::atomic<int>                    livingThread;
    std::mutex                          mutex_;
    std::condition_variable             cv;
    int                                 liveTime;
    Unit                                unit;
    Policy                              policy;
    bool                                isShutdown;
    int                                 maxThreadCount;
    int                                 coreThreadCount;
    int                                 taskQueueLength;
    int                                 task_index_;
    int                                 none_core_thread_index_;
    bool                                is_batch_io_;

    /* Functions  */ 
    void  none_core_thread_function(std::function<void()>&&); //线程函数，起来以后是独立线程，任务函数执行在这个函数中
    int   getTaskQueueSize();
    void  joinAllThreads();
    void  init();
    void  getTid();
    int   get_real_index();
};

#endif