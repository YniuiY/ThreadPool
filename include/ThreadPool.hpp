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
    Second,
    Millisecond,
    Microsecond,
    Nanosecond
};

class ThreadPool
{
private:
    std::queue<std::thread>             threadQueue; //核心线程队列
    std::vector<std::thread>            noneCoreThreadQueue; //非核心线程队列
    std::queue<std::function<void()>>   taskQueue; //function模板可以将函数、指针、lambda等多种可调用对象，描述成一种可调用对象
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

    /* Functions  */ 
    void    threadFunction(std::function<void()>&&); //线程函数，起来以后是独立线程，任务函数执行在这个函数中
    int     getTaskQueueSize();
    void    joinAllThreads();
    void    init();
    void    getTid();

public:
    ThreadPool();
    ThreadPool(int maxCount, int coreCount, int tQueueLength);
    ThreadPool(int maxCount, int coreCount, int tQueueLength, Policy p, int lTime, Unit u);
    ~ThreadPool();
    
    /**
     * 形参为右值引用，形参将从实参中“窃取”（移动赋值/移动构造）数据，避免了数据拷贝过程提高了性能。
     * 但是这个版本的函数只能接受 非const的右值类型的实参。
     * 当实参是右值时优先匹配这个函数。
     */
    template<class Func, class... Args>
    bool execute(Func&& task, Args&&... args)
    {
        bool ret = true;
        if(livingThread >= coreThreadCount && getCurrentTaskQueueSize() < taskQueueLength)
        {
            cout<<"task queue push\n";
            taskQueue.push(bind(task, args...));//任务队列没满先让任务进队列。
            cv.notify_one();
            runningThread++;
        }
        else if(livingThread < maxThreadCount && getCurrentTaskQueueSize() >= taskQueueLength)
        {
            //任务队列满了，但是活跃的线程数小于最大线程数，则开新的线程并将任务直接交给新线程。
            cout<<"new thread \n";
            noneCoreThreadQueue.push_back(thread(&ThreadPool::threadFunction, this, bind(task, args...)));
            livingThread++;
        }
        else if(livingThread >= maxThreadCount && getCurrentTaskQueueSize() >= taskQueueLength)
        {
            //reject，队列满，线程数也到达最大线程数，这个时候调用拒绝策略
            cout<<"reject task \n";
            RejectPolicyFactory::getInstance()->getRejectPolicy(policy)->reject(bind(task,args...));
            ret = false;
        }
        return ret;
    }

    /**
     * 形参为const的左值引用，形参将从实参中“拷贝”（拷贝赋值/拷贝构造）数据
     * 这个版本的函数能接受任意类型实参对象（左右值都可以），但是由于数据需要从实参拷贝到形参，所以性能不如使用右值引用的函数
     * 当不存在形参为右值引用的函数时，这个const 左值引用为形参的函数也可以使用
     */
    template<class Func, class... Args>
    bool execute(const Func& task, const Args&... args)
    {
        bool ret = true;
        if(livingThread >= coreThreadCount && getCurrentTaskQueueSize() < taskQueueLength)
        {
            taskQueue.push(bind(task, args...));//任务队列没满先让任务进队列。
            cv.notify_one();
            runningThread++;
        }
        else if(livingThread < maxThreadCount && getCurrentTaskQueueSize() >= taskQueueLength)
        {
            //任务队列满了，但是活跃的线程数小于最大线程数，则开新的线程并将任务直接交给新线程。
            noneCoreThreadQueue.push_back(thread(&ThreadPool::threadFunction, this, bind(task, args...)));
            livingThread++;
        }
        else if(livingThread >= maxThreadCount && getCurrentTaskQueueSize() >= taskQueueLength)
        {
            //reject，队列满，线程数也到达最大线程数，这个时候调用拒绝策略
            RejectPolicyFactory::getInstance()->getRejectPolicy(policy)->reject(bind(task,args...));
            ret = false;
        }
        return ret;
    }

    int     getRunningThreadCount();
    int     getLivingThreadCount();
    int     getCurrentTaskQueueSize();
};

#endif