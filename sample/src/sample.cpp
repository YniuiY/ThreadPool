#include <unistd.h>

#include "thread_pool.hpp"

void task(int a) {
  std::cout << "##### " << a << " thread id:" << std::this_thread::get_id() << " #####" << std::endl;
  sleep(1);
}

void normal_test_case(ThreadPool& pool) {
  // 提交测试任务
  for (int i = 0; i < 100; i++) {
    pool.Commit([]() {
      std::cout << "lambda get thread id:" << std::this_thread::get_id() << std::endl;
    });
    pool.Commit(task, i);
  }

  sleep(30);
  std::cout << "living thread count: " << pool.getLivingThreadCount() << std::endl;
}

int main() {
  int maxCount = 10;
  int coreCount = 8;
  int taskQueueLength = 10;
  Policy policy = Discard;
  int liveTime = 2;
  ThreadPool::Unit unit = ThreadPool::Unit::Second;
  ThreadPool pool(maxCount, coreCount, taskQueueLength, policy, liveTime, unit);

  normal_test_case(pool);

  return 0;
}