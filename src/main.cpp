#include "ThreadPool.hpp"
#include <windows.h>
void task(int a)
{
    cout<<"thread id:"<<this_thread::get_id()<<endl;
    // cout<<"a:"<<a++<<endl;
}

int main()
{
    int maxCount = 8;
    int coreCount = 2;
    int taskQueueLength = 8;
    Policy policy = Discard;
    int liveTime = 10;
    Unit unit = Secend;
    // ThreadPool(int maxCount, int coreCount, int tQueuelenght, Policy p, int lTime, Unit u);
    ThreadPool* threadPoolPtr = new ThreadPool(maxCount, coreCount, taskQueueLength, policy, liveTime, unit);
    for(int i = 0; i < 10; i++)
        threadPoolPtr->execute(task, 0);   
}