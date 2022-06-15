#include "ThreadPool.hpp"

ThreadPool::ThreadPool(){}

ThreadPool::ThreadPool(int maxCount, int coreCount, int tQueueLenght)
    : maxThreadCount(maxCount), coreThreadCount(coreCount), taskQueueLenght(tQueueLenght),
    isShutdown(false)
{
    init();
}

ThreadPool::ThreadPool(int maxCount, int coreCount, int tQueueLenght, Policy p, int ltime, Unit u)
    : maxThreadCount(maxCount), coreThreadCount(coreCount), taskQueueLenght(tQueueLenght),
    isShutdown(false), policy(p), liveTime(ltime), unit(u),
    runingThread(0), runningTasks(0), livingThread(0)
{
    init();
}

ThreadPool::~ThreadPool()
{
    joinAllThreads();
}

void ThreadPool::gettid()
{
    cout<<"thread id:"<<this_thread::get_id()<<endl;
}

void ThreadPool::init()
{
    if(coreThreadCount <= 0 || maxThreadCount <= 0 || taskQueueLenght < 0)
    {
        throw invalid_argument("coreThreadCount, maxThreadCount and taskQueueLenght mast bigger than 0");
        return;
    }
    if(maxThreadCount < coreThreadCount)
    {
        throw out_of_range("max thread Count mast biggrt than core thread count");
        return;
    }

    for(int i = 0; i < coreThreadCount; i++)
    {
        threadQueue.push(thread(&ThreadPool::threadFunction, this, nullptr));
        runingThread++;
        livingThread++;
    }

}

void ThreadPool::threadFunction(function<void()> func)
{
    bool coreThread = false;
    if(func != nullptr)
    {
        cout<<"func not nullptr\n";
        func();
    }
    else
    {
        coreThread = true;
    }

    unique_lock<mutex> tasksLock(mutex_);
    for(;;)
    {
        if(isShutdown)
        {
            runingThread--;
            livingThread--;
            break;
        }
        else if(!taskQueue.empty())
        {
            /**
             * move返回一个原本左值对象的右值引用。
             * 将右值引用赋值给一个左值，这个过程会调用移动赋值运算符，移动构造对象避免了内存拷贝从而带来性能提高
             */
            function<void()> currentFunction = std::move(taskQueue.front());

            /**
             * 直接从队列中取出可调用对象也是可以的。
             * 只不过这个过程会调用拷贝赋值运算符，拷贝构造对象因为需要内存拷贝所以性能一般
             */
            // function<void()> currentFunction = taskQueue.front(); 
            taskQueue.pop();

            tasksLock.unlock(); //解锁才能并行执行任务
            currentFunction();
            tasksLock.lock(); //重新上锁保证下一次从任务队列取任务是互斥的
        }
        else if(taskQueue.empty())
        {
            runingThread--;
            cv_status status = cv_status::no_timeout;
            switch (unit)
            {
            case Hours:
                status = cv.wait_for(tasksLock, chrono::hours(liveTime));
                break;
            case Minutes:
                status = cv.wait_for(tasksLock, chrono::minutes(liveTime));
                break;
            case Secend:
                // cout<<"thread wait for "<<liveTime<<" secend"<<endl;
                status = cv.wait_for(tasksLock, chrono::seconds(liveTime));
                break;
            case Millisecend:
                status = cv.wait_for(tasksLock, chrono::milliseconds(liveTime));
                break;
            case Microsecend:
                status = cv.wait_for(tasksLock, chrono::microseconds(liveTime));
                break;
            case Nanosecend:
                status = cv.wait_for(tasksLock, chrono::nanoseconds(liveTime));
                break;
            
            default:
                status = cv.wait_for(tasksLock, chrono::seconds(liveTime));
                break;
            }

            //如果线程阻塞超时，任务队列不满，则让非核心线程结束
            if(!coreThread && status == cv_status::timeout && (getTaskQueueSize() < taskQueueLenght))
            {
                cout<<"\n*** none core thread exit ***"<<endl;
                livingThread--;
                break;
            }
            else if(coreThread && status == cv_status::timeout && getTaskQueueSize() == 0)
            {
                cv.wait(tasksLock); //核心线程，任务队列为空则阻塞
            }

        }

    }

}

void ThreadPool::joinAllThreads()
{
    //queue没有迭代器
    isShutdown = true;
    cv.notify_all();

    // 非核心线程join
    for(int i = 0; i < noneCoreThreadQueue.size(); i++)
    {
        if(noneCoreThreadQueue[i].joinable())
        {
            cout<<"None core thread join\n";
            noneCoreThreadQueue[i].join();
        }
    }

    // 核心线程join
    for(int i = 0; i < coreThreadCount; i++)
    {
        if(threadQueue.front().joinable())
        {
            cout<<"Core thread join\n";
            threadQueue.front().join();
            threadQueue.pop();
        }
    }
}

int ThreadPool::getRunningThreadCount()
{
    return runingThread;
}

int ThreadPool::getLivingThreadCount()
{
    return livingThread;
}

int ThreadPool::getCurrentTaskQueueSize()
{
    unique_lock<mutex> tasksLock(mutex_);
    return taskQueue.size();
}

int ThreadPool::getTaskQueueSize()
{
    return taskQueue.size();
}