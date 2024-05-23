#include "thread/core_thread.h"

void CoreThread::Init() {
  is_running_ = true;
  /// TODO: 暂时关闭，先将本地队列和任务窃取调试完成，再调试扇入扇出
  is_batch_io_ = false;
  thread_ = std::thread(std::bind(&CoreThread::run, this));
  std::cout << "CoreThread: " << index_ << "is running\n";
}

void CoreThread::Stop() {
  std::cout << "Core thread Stop\n";
  is_running_ = false;
  cv_.notify_all();
  if (thread_.joinable()) {
    std::cout << "Core thread join\n";
    thread_.join();
  }
}

void CoreThread::SetThreadPoolParam(std::vector<std::shared_ptr<CoreThread>>const& core_thread_vector,
                                    StealQueue<Task>* pool_task_queue_ptr,
                                    int index, int core_thread_num) {
  core_thread_queue_ = core_thread_vector;
  pool_task_queue_ptr_ = pool_task_queue_ptr;
  index_ = index;
  core_thread_num_ = core_thread_num;
  create_steal_range();
}

void CoreThread::PushTask(Task&& task) {
  while (!(frist_task_queue_.TryPush(std::forward<Task>(task)) ||
           second_task_queue_.TryPush(std::forward<Task>(task)))) {
    std::this_thread::yield();
  }
  cv_.notify_one();
}

void CoreThread::run() {
  while (is_running_) {
    if (is_batch_io_) {
      run_tasks();
    } else {
      run_task();
    }
  }
}

void CoreThread::run_task () {
  Task task;
  if (pop_task(task) || pop_pool_task(task) || steal_task(task)) {
    std::cout << "*** Core thread task run ***\n";
    task();
    std::cout << "*** Core thread task run over ***\n";
  } else {
    std::unique_lock<std::mutex> ul(lock_);
    cv_.wait_for(ul, std::chrono::milliseconds(2000));
  }
}

void CoreThread::run_tasks() {
  std::vector<Task> tasks;
  if (pop_tasks(tasks) || pop_pool_tasks(tasks) || steal_tasks(tasks)) {
    for (auto && task:  tasks) {
      task();
    }
  } else {
    std::unique_lock<std::mutex> ul(lock_);
    cv_.wait_for(ul, std::chrono::milliseconds(2000));
  }
}

bool CoreThread::pop_task(Task& task) {
  return frist_task_queue_.TryPop(task) || second_task_queue_.TryPop(task);
}

bool CoreThread::pop_tasks(std::vector<Task>& tasks) {
  return frist_task_queue_.TryPop(tasks, MAX_BATCH_SIZE) || second_task_queue_.TryPop(tasks, MAX_BATCH_SIZE - tasks.size());
}

bool CoreThread::pop_pool_task(Task& task) {
  return pool_task_queue_ptr_->TryPop(task);
}

bool CoreThread::pop_pool_tasks(std::vector<Task>& tasks) {
  return pool_task_queue_ptr_->TryPop(tasks, MAX_BATCH_SIZE);
}

bool CoreThread::steal_task(Task& task) {
  bool ret{false};

  for (auto index : steal_range_) {
    if (core_thread_queue_[index]->second_task_queue_.TrySteal(task) ||
        core_thread_queue_[index]->frist_task_queue_.TrySteal(task)) {
      ret = true;
      break;
    }
  }
  std::cout << "CoreThread Steal task, ret: " << ret << std::endl;
  return ret;
}

bool CoreThread::steal_tasks(std::vector<Task>& tasks) {
  bool ret{false};
  tasks.clear();
  int left_size = MAX_BATCH_SIZE;
  for (auto index : steal_range_) {
    if (left_size > 0 && core_thread_queue_[index]->second_task_queue_.TrySteal(tasks, MAX_BATCH_SIZE)) {
      ret = true;
      left_size = MAX_BATCH_SIZE - tasks.size();
      if (left_size > 0 && core_thread_queue_[index]->frist_task_queue_.TrySteal(tasks, left_size)) {
        ret = true;
      }
    } else  if (left_size <= 0) {
      break;
    }
  }

  return ret;
}

void CoreThread::create_steal_range() {
  std::cout << "core_thread_num_: " << core_thread_num_ << std::endl;
  if (core_thread_num_ <= 0) {
    throw std::runtime_error("core_thread_num_ <= 0");
  }

  for (int i = 0; i < MAX_STEAL_RANGE; i++) {
    steal_range_.emplace_back((index_ + i) % core_thread_num_);
    std::cout << "CoreThread: " << index_ << ", steal range: " << steal_range_[i] << " ";
  }
  std::cout << std::endl;
}