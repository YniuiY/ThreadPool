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
        threadQueue.push(thread(&ThreadPool::threadFunction, this, [](){cout<<"core Thread pushed\n";}));
        runingThread++;
        livingThread++;
    }

}

void ThreadPool::threadFunction(function<void()> func)
{
    if(func != nullptr)
    {
        cout<<"func not nullptr\n";
        func();
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
                cout<<"thread wait for "<<liveTime<<" secend"<<endl;
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
            cout<<"exit switch\n";
            cout<<"living threads:"<<livingThread<<endl;
            cout<<"task queue size:"<<getTaskQueueSize()<<endl;
            //如果线程阻塞超时，当前运行的线程数大于核心线程数，任务队列不满，则让非核心线程结束
            if(status == cv_status::timeout && (livingThread > coreThreadCount) && (getTaskQueueSize() < taskQueueLenght))
            {
                cout<<"\n*** thread exit ***"<<endl;
                livingThread--;
                break;
            }
            else if(status == cv_status::timeout && livingThread <= coreThreadCount)
            {
                cv.wait(tasksLock); //如果只剩下核心线程，则直接阻塞等待唤醒
            }

        }

    }

}

void ThreadPool::joinAllThreads()
{
    //queue没有迭代器
    /**
     * 由于核心线程与非核心线程不固定的原因，不能保证线程队列中的对象在调用本函数时依然存在
     * 调用本函数可能会join已经不存在thread
     */
    isShutdown = true;
    for(int i = 0; i < threadQueue.size(); i++)
    {
        threadQueue.front().join();
        threadQueue.pop();
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