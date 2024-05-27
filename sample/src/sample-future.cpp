#include <future>
#include <thread>
#include <functional>
#include <iostream>

int sum(int a, int b) {
  std::this_thread::sleep_for(std::chrono::seconds(10));
  return a+b;
}

int main() {
  std::packaged_task<int(int,int)> task(sum);
  std::future<int> ret = task.get_future();
  std::thread(std::bind(std::move(task),1,2)).join();

  std::cout << "sum ret:" << ret.get() << std::endl;
}