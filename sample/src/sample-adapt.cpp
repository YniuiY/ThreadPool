#include <unistd.h>

#include "thread_pool.hpp"

void task(int a) {
  std::cout << "##### " << a << " thread id:" << std::this_thread::get_id() << " #####" << std::endl;
  sleep(1);
}

void thread_num_adapt_case(ThreadPool& pool) {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 100; j++) {
      pool.Commit(task, j);
    }

    std::this_thread::sleep_for(std::chrono::seconds(20));
  }
}

int main() {
  int maxCount = 18;
  int coreCount = 8;
  int taskQueueLength = 20;
  Policy policy = Discard;
  int liveTime = 1;
  ThreadPool::Unit unit = ThreadPool::Unit::Second;
  ThreadPool pool(maxCount, coreCount, taskQueueLength, policy, liveTime, unit);

  thread_num_adapt_case(pool);

  return 0;
}