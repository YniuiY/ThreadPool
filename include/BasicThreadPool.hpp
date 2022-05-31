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

#ifndef BASICTHREADPOOL_H_
#define BASICTHREADPOOL_H_

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <thread>
#include <iostream>
using namespace std;

class BasicThreadPool
{
private:
    queue<function<void()>> tasks_;                 //任务队列
    vector<thread>          threads_;               //线程队列
    mutex                   mutex_;                 //互斥锁
    condition_variable      conditionVariable_;     //条件变量
    bool                    isShutdown = false;     //线程池关闭标志
    int                     coreThreadNum_ = 0;     //核心线程数
    int                     maxThreadNum_;          //最大线程数
    int                     maxTaskQueueLength_;    //最大任务队列长度
    int                     liveTime_;              //空闲线程生存时间  
    atomic<int>             runningTaskCount = 0;   //原子类型，正在执行的任务数量
    mutex                   taskQueueLock;          //获取队列长度时用到的互斥锁

    void threadInitFunction();                      //线程函数，在这个函数内部取出任务执行（任务可以是函数或者lambda等可调用对象）
    void threadFuncton(function<void> func);        //可以在创建线程时传入可执行对象的线程函数
    void joinAllThread();
    int  getTaskQueueLength();                      //获取任务队列在当前时刻的长度
    int  getRunningTaskCount();                     //获取当前正在执行的任务数量
    

public:
    BasicThreadPool(int coreThreadNum);
    ~BasicThreadPool();

    /**
     *  execute是可变参数模板函数，模板函数必须在头文件中实现
     *  尾置返回值类型，返回类型与task相同都是void类型
     *  &&表示右值引用
     *  execute的参数可以是任意类型，任意数量
     *  这里的bind()函数，将可调用对象task和他的参数包args，
     *  打包成一个void()类型的可调用对象，以适配任务队列的元素类型function<void()>类型。
    **/
    template<class Func_, class... Args_>
    auto execute(Func_&& task, Args_&&... args) ->decltype(task)
    {
        unique_lock<mutex> uniqueLock(mutex_);
        tasks_.push(bind(task,args));
        conditionVariable_.notify_one();    //每次push进入一个任务，都要唤醒一个线程；
    }

};

BasicThreadPool::BasicThreadPool(int coreThreadNum)
{
    coreThreadNum_ = coreThreadNum;
    for(int i = 0; i < coreThreadNum_; i++)
    {
        //  使用成员函数初始化thread函数时，需要显式的 “&”才能得到函数指针，
        //  并且，需要将成员函数所属对象的指针传入thread的函数，这里就是this指针。
        threads_.push_back(thread(&BasicThreadPool::threadInitFunction, this));
    }

}

BasicThreadPool::~BasicThreadPool()
{
    lock_guard<mutex> lockGurad(mutex_);
    isShutdown = true;
    conditionVariable_.notify_all();    //唤醒所有线程，让他们检测isShutdown然后退出for(;;)
    joinAllThread();
}

void BasicThreadPool::threadInitFunction()
{
    for(;;)
    {
        unique_lock<mutex> uniqueLock(mutex_);
        if(isShutdown)
        {
            break;
        }
        else if (tasks_.empty())
        {
            conditionVariable_.wait(uniqueLock);
        }
        else if(!tasks_.empty())
        {
            auto currentTask = std::move(tasks_.front());
            tasks_.pop();

            uniqueLock.unlock();        //解锁是为了，让提交的任务并行执行
            currentTask();
        }

    }
}

void BasicThreadPool::joinAllThread()
{
    for(auto beg = threads_.begin(), end = threads_.end(); beg != end; beg++)
    {
        beg->join();
    }
}


#endif