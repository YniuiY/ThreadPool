#include <unistd.h>
#include <random>
#include <ctime>

#include "thread_pool.hpp"

void task(int a, int random_time) {
  std::cout << "##### " << a << " thread id:" << std::this_thread::get_id() << " sleep time: " << random_time << " #####" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(random_time));
}

void long_time_run_case(ThreadPool& pool) {
  std::default_random_engine engine;
  std::uniform_int_distribution<int> uniform(1,30); // 产生[10-10000]的随机数
  engine.seed(time(0));

  int i = 0;
  while (true) {
    for (int j = 0; j < 10; j++) {
      pool.Commit(task, i++, uniform(engine));
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(33));
  }
}

int main() {
  int maxCount = 10;
  int coreCount = 8;
  int taskQueueLength = 2000;
  Policy policy = Discard;
  int liveTime = 2;
  ThreadPool::Unit unit = ThreadPool::Unit::Second;
  ThreadPool pool(maxCount, coreCount, taskQueueLength, policy, liveTime, unit);

  long_time_run_case(pool);

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}