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
#include "RejectPolicy.hpp"
using namespace std;

enum Unit
{
    Hours,
    Minutes,
    Secend,
    Millisecend,
    Microsecend,
    Nanosecend
};

class ThreadPool
{
private:
    std::queue<std::thread>             threadQueue; //线程队列
    std::queue<std::function<void()>>   taskQueue; //function模板可以将函数、指针、lambda等多种可调用对象，描述成一个对象
    std::atomic<int>                    runingThread;
    std::atomic<int>                    runningTasks;
    std::atomic<int>                    livingThread;
    std::mutex                          mutex_;
    std::condition_variable             cv;
    int                                 liveTime;
    Unit                                unit;
    Policy                              policy;
    RejectPolicyFactory*                rejectFatory;
    bool                                isShutdown;
    int                                 maxThreadCount;
    int                                 coreThreadCount;
    int                                 taskQueueLenght;

    /* Functions  */ 
    void    threadFunction(std::function<void()>); //线程函数，起来以后是独立线程，任务函数执行在这个函数中
    // int getRunningTasksCount();
    int     getRunningThreadCount();
    int     getLivingThreadCount();
    int     getCurrentTaskQueueSize();
    void    joinAllThreads();
    void    init();
    void    gettid();

public:
    ThreadPool();
    ThreadPool(int maxCount, int coreCount, int tQueuelenght);
    ThreadPool(int maxCount, int coreCount, int tQueuelenght, Policy p, int lTime, Unit u);
    ~ThreadPool();
    
    template<class Func, class... Args>
    bool execute(Func&& task, Args&&... args)
    {
        bool ret = true;
        if(threadQueue.size() >= coreThreadCount && getCurrentTaskQueueSize() < taskQueueLenght)
        {
            taskQueue.push(bind(task, args...));//任务队列没满先让任务进队列。
            cv.notify_one();
            runingThread++;
        }
        else if(threadQueue.size() < maxThreadCount && getCurrentTaskQueueSize() >= taskQueueLenght)
        {
            //任务队列满了，但是活跃的线程数小于最大线程数，则开新的线程并将任务直接交给新线程。
            threadQueue.push(thread(&ThreadPool::threadFunction, this, bind(task, args...)));
            livingThread++;
        }
        else if(runingThread >= coreThreadCount && getCurrentTaskQueueSize() >= taskQueueLenght)
        {
            //reject，队列满，线程数也到达最大线程数，这个时候调用拒绝策略
            rejectFatory->getInstance()->getRejectPolicy(policy)->reject(bind(task, args...));
            ret = false;
        }
        return ret;
    }
};

#endif