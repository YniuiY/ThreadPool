#ifndef CORE_THREAD_H_
#define CORE_THREAD_H_

#include "queue/steal_queue.h"
#include "global_define.h"

constexpr int MAX_STEAL_RANGE{3}; // 最大窃取范围，包含自己
constexpr int MAX_BATCH_SIZE{2};  // 获取任务的最大数量

class CoreThread {
 public:
  CoreThread() = default;
  ~CoreThread() = default;

  void Init();
  void Stop();
  void PushTask(Task&& task);

  bool steal_task(Task& task);
  bool steal_tasks(std::vector<Task>& tasks);

  void SetThreadPoolParam(std::vector<std::shared_ptr<CoreThread>>const& core_thread_vector,
                          StealQueue<Task>* pool_task_queue,
                          int index,
                          int core_thread_num);

 private:
  void run();
  void run_task();
  void run_tasks();

  
  bool pop_task(Task& task);
  bool pop_tasks(std::vector<Task>& tasks);
  bool pop_pool_task(Task& task);
  bool pop_pool_tasks(std::vector<Task>& tasks);
  
  void create_steal_range();

  std::vector<int> steal_range_;            // 任务窃取范围，元素是核心线程索引
  bool is_batch_io_;                        // 是否批量获取任务
  bool is_running_;                         // 线程运行状态
  int index_;                               // 核心线程索引
  int core_thread_num_;                     //
  std::mutex lock_;                         // 
  std::condition_variable cv_;              // 
  std::thread thread_;                      // 线程
  StealQueue<Task> frist_task_queue_;       // 第一任务队列
  StealQueue<Task> second_task_queue_;      // 第二任务队列
  StealQueue<Task>* pool_task_queue_ptr_;   // 线程池任务队列
  std::vector<std::shared_ptr<CoreThread>> core_thread_queue_; //核心线程队列，在这里用于任务窃取
};

#endif // CORE_THREAD_H_