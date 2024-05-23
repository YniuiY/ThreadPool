#include <unistd.h>

#include "thread_pool.hpp"

void task(int a) {
  std::cout << "##### " << a << " thread id:" << std::this_thread::get_id() << " #####" << std::endl;
  sleep(1);
}

void batch_task_case(ThreadPool& pool) {

}

int main() {
  int maxCount = 8;
  int coreCount = 8;
  int taskQueueLength = 0;
  Policy policy = Discard;
  int liveTime = 2;
  ThreadPool::Unit unit = ThreadPool::Unit::Second;
  ThreadPool pool(maxCount, coreCount, taskQueueLength, policy, liveTime, unit);

  batch_task_case(pool);

  return 0;
}