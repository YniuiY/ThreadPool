#include <unistd.h>

#include "thread_pool.hpp"

void task(int a) {
  std::cout << "##### " << a << " thread id:" << std::this_thread::get_id() << " #####" << std::endl;
  sleep(1);
}

int main() {
  int maxCount = 12;
  int coreCount = 8;
  int taskQueueLength = 50;
  Policy policy = Discard;
  int liveTime = 2;
  ThreadPool::Unit unit = ThreadPool::Unit::Second;
  // ThreadPool(int maxCount, int coreCount, int tQueuelenght, Policy p, int
  // lTime, Unit u);
  ThreadPool* threadPoolPtr = new ThreadPool(
      maxCount, coreCount, taskQueueLength, policy, liveTime, unit);
  for (int i = 0; i < 100; i++) {
    threadPoolPtr->Commit([]() {
      std::cout << "lambda get thread id:" << std::this_thread::get_id() << std::endl;
    });
    threadPoolPtr->Commit(task, i);
  }

  // delete threadPoolPtr;
  sleep(30);
  std::cout << "living thread count: " << threadPoolPtr->getLivingThreadCount() << std::endl;
  delete threadPoolPtr;

  return 0;
}