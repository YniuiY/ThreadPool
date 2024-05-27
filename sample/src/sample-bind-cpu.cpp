#include <unistd.h>
#include <random>
#include <ctime>

#include "thread_pool.hpp"
#include "time_util.h"

void task(int a, int random_time) {
  std::this_thread::sleep_for(std::chrono::milliseconds(random_time));
    std::cout << "##### task id:" << a
              << ", thread id:" << std::this_thread::get_id()
              << ", sleep time: " << random_time
              << ", over timestamp: " << TimeUtil::Now()
              << " #####" << std::endl;
}

void batch_test_case(ThreadPool& pool) {
  std::default_random_engine engine;
  std::uniform_int_distribution<int> uniform(10,1000); // 产生[10-10000]的随机数
  engine.seed(time(0));

  for (int i = 0; i < 3000; ++i) {
    pool.Commit(task, i, uniform(engine));
    // pool.Commit(task, i, 1000);
  }

  std::this_thread::sleep_for(std::chrono::seconds(200));
}

int main() {
  int maxCount = 8;
  int coreCount = 8;
  int taskQueueLength = 3000;
  Policy policy = Discard;
  int liveTime = 2;
  ThreadPool::Unit unit = ThreadPool::Unit::Second;
  bool is_batch_io = true;
  bool is_bind_cpu = false;
  ThreadPool pool(maxCount, coreCount, taskQueueLength, policy, liveTime, unit, is_batch_io, is_bind_cpu);

  batch_test_case(pool);

  return 0;
}