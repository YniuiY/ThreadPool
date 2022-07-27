#include "ThreadPool.hpp"
#include <unistd.h>
void task(int a)
{
    cout<<a<<" thread id:"<<this_thread::get_id()<<endl;
    sleep(1);
}

int main()
{
    int maxCount = 8;
    int coreCount = 2;
    int taskQueueLength = 8;
    Policy policy = Discard;
    int liveTime = 2;
    Unit unit = Second;
    // ThreadPool(int maxCount, int coreCount, int tQueuelenght, Policy p, int lTime, Unit u);
    ThreadPool* threadPoolPtr = new ThreadPool(maxCount, coreCount, taskQueueLength, policy, liveTime, unit);
    for(int i = 0; i < 100; i++)
    {
        threadPoolPtr->execute([](){cout<<"lambda get thread id:"<<this_thread::get_id()<<endl;});
        threadPoolPtr->execute(task, i);
    }

    // delete threadPoolPtr;
    sleep(10);
    cout<<"living thread count: "<<threadPoolPtr->getLivingThreadCount()<<endl;
    delete threadPoolPtr;

    return 0;
}