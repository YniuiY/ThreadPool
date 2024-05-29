#include "thread_pool.hpp"

#include <queue/steal_queue.h>

ThreadPool::ThreadPool() {}

ThreadPool::ThreadPool(int maxCount, int coreCount, int tQueueLength)
    : maxThreadCount(maxCount),
      coreThreadCount(coreCount),
      taskQueueLength(tQueueLength),
      isShutdown(false),
      none_core_thread_index_(0),
      is_batch_io_(false),
      is_bind_cpu_{false} {
  init();
}

ThreadPool::ThreadPool(int maxCount, int coreCount, int tQueueLength, Policy p,
                       int ltime, Unit u)
    : maxThreadCount(maxCount),
      coreThreadCount(coreCount),
      taskQueueLength(tQueueLength),
      isShutdown(false),
      policy(p),
      liveTime(ltime),
      unit(u),
      runningThread(0),
      runningTasks(0),
      livingThread(0),
      none_core_thread_index_(0),
      is_batch_io_(false),
      is_bind_cpu_{false} {
  init();
}

ThreadPool::ThreadPool(int maxCount, int coreCount, int tQueueLength, Policy p,
                       int ltime, Unit u, bool is_batch_io)
    : maxThreadCount(maxCount),
      coreThreadCount(coreCount),
      taskQueueLength(tQueueLength),
      isShutdown(false),
      policy(p),
      liveTime(ltime),
      unit(u),
      runningThread(0),
      runningTasks(0),
      livingThread(0),
      none_core_thread_index_(0),
      is_batch_io_(is_batch_io),
      is_bind_cpu_{false} {
  init();
}

ThreadPool::ThreadPool(int maxCount, int coreCount, int tQueueLength, Policy p,
                       int ltime, Unit u, bool is_batch_io, bool is_bind_cpu)
    : maxThreadCount(maxCount),
      coreThreadCount(coreCount),
      taskQueueLength(tQueueLength),
      isShutdown(false),
      policy(p),
      liveTime(ltime),
      unit(u),
      runningThread(0),
      runningTasks(0),
      livingThread(0),
      none_core_thread_index_(0),
      is_batch_io_(is_batch_io),
      is_bind_cpu_{is_bind_cpu} {
  init();
}

ThreadPool::~ThreadPool() { join_all_threads(); }

int ThreadPool::GetRunningThreadCount() { return runningThread; }

int ThreadPool::GetLivingThreadCount() { return livingThread; }

int ThreadPool::GetCurrentTaskQueueSize() { return pool_task_queue_.Size(); }

void ThreadPool::get_tid() {
  std::cout << "thread id:" << std::this_thread::get_id() << std::endl;
}

void ThreadPool::init() {
  if (coreThreadCount <= 0 || maxThreadCount <= 0 || taskQueueLength < 0) {
    throw std::invalid_argument(
        "coreThreadCount, maxThreadCount and taskQueueLength mast bigger than "
        "0");
    return;
  }
  if (maxThreadCount < coreThreadCount) {
    throw std::out_of_range(
        "max thread Count mast biggrt than core thread count");
    return;
  }

  core_thread_queue_.reserve(coreThreadCount);
  for (int i = 0; i < coreThreadCount; i++) {
    core_thread_queue_.emplace_back(std::make_shared<CoreThread>());
    runningThread++;
    livingThread++;
    std::cout << "Make core thread: " << i << ", thread id: " << core_thread_queue_[i] << std::endl;
  }

  std::cout << "ThreadPool Open Batch IO: " << is_batch_io_
            << ", Open Bind CPU: " << is_bind_cpu_ << std::endl;

  for (int thread_index = 0; thread_index < coreThreadCount; thread_index++) {
    core_thread_queue_[thread_index]->SetThreadPoolParam(
        core_thread_queue_, &pool_task_queue_, thread_index, coreThreadCount, is_batch_io_, is_bind_cpu_);
    core_thread_queue_[thread_index]->Init();
  }
}

void ThreadPool::none_core_thread_function(std::function<void()>&& func) {
  int ttl = NONE_CORE_THREAD_TTL; // ttl归零线程退出
  std::mutex wait_mutex_;
  int index = ++none_core_thread_index_;

  if (func != nullptr) {
    func();
  }

  while (!isShutdown) {
    if (isShutdown) {
      runningThread--;
      livingThread--;
      break;
    }
    Task task;
    if (pool_task_queue_.TryPop(task)) {
      // std::cout << "none core thread run task\n";
      if (task ==  nullptr) {
        std::cout << "none core thread poped task is nullptr\n";
        std::runtime_error("none core thread poped task is nullptr");
      }
      task();
      // std::cout << "none core thread: " << index << " run task over\n";
    } else if (pool_task_queue_.Empty()) {
      runningThread--;
      std::unique_lock<std::mutex> wait_lock(wait_mutex_);
      std::cv_status status = std::cv_status::no_timeout;
      switch (unit) {
        case Hours:
          status = cv.wait_for(wait_lock, std::chrono::hours(liveTime));
          break;
        case Minutes:
          status = cv.wait_for(wait_lock, std::chrono::minutes(liveTime));
          break;
        case Second:
          status = cv.wait_for(wait_lock, std::chrono::seconds(liveTime));
          break;
        case Millisecond:
          status = cv.wait_for(wait_lock, std::chrono::milliseconds(liveTime));
          break;
        case Microsecond:
          status = cv.wait_for(wait_lock, std::chrono::microseconds(liveTime));
          break;
        case Nanosecond:
          status = cv.wait_for(wait_lock, std::chrono::nanoseconds(liveTime));
          break;

        default:
          status = cv.wait_for(wait_lock, std::chrono::seconds(liveTime));
          break;
      }
      // 如果线程阻塞超时 && 池任务队列为空，非核心线程的 ttl-1，ttl归零后退出
      if (status == std::cv_status::timeout && pool_task_queue_.Empty()) {
        --ttl;
        std::cout << " none core thread ttl: " << ttl << std::endl;
        if (ttl <= 0) {
          std::cout << "\n*** none core thread exit ***" << std::endl;
          livingThread--;
          break;
        }
      }
    }
  }
}

void ThreadPool::join_all_threads() {
  isShutdown = true;
  cv.notify_all();

  // 非核心线程join
  for (auto& none_core_thread : none_core_thread_queue_) {
    if (none_core_thread.joinable()) {
      std::cout << "None core thread join\n";
      none_core_thread.join();
    }
  }

  // 核心线程join
  for (auto& core_thread: core_thread_queue_) {
    core_thread->Stop();
  }
}

int ThreadPool::get_task_queue_size() { return pool_task_queue_.Size(); }

int ThreadPool::get_real_index() {
  int real_index = task_index_++;
  if (task_index_ > maxThreadCount || task_index_ < 0) {
    task_index_ = 0;
  }
  return real_index;
}