#include <unistd.h>
#include <random>
#include <ctime>

#include "thread_pool.hpp"

void task(int a, int random_time) {
  std::cout << "##### " << a << " thread id:" << std::this_thread::get_id() << "sleep time: " << random_time << " #####" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(random_time));
}

void steal_test_case(ThreadPool& pool) {
  std::default_random_engine engine;
  std::uniform_int_distribution<int> uniform(10,10000); // 产生[10-10000]的随机数
  engine.seed(time(0));

  for (int i = 0; i < 100; ++i) {
    pool.Commit(task, i, uniform(engine));
  }

  std::this_thread::sleep_for(std::chrono::seconds(120));
}

int main() {
  int maxCount = 10;
  int coreCount = 8;
  int taskQueueLength = 20;
  Policy policy = Discard;
  int liveTime = 2;
  ThreadPool::Unit unit = ThreadPool::Unit::Second;
  bool is_batch_io = true;
  ThreadPool pool(maxCount, coreCount, taskQueueLength, policy, liveTime, unit, is_batch_io);

  steal_test_case(pool);

  return 0;
}