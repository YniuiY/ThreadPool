#include "thread/core_thread.h"

CoreThread::CoreThread()
    : is_running_{false},
      is_batch_io_{false},
      is_bind_cpu_{false},
      wait_num_{0},
      wait_timeout_num_{0} {}

CoreThread::CoreThread(
    std::vector<std::shared_ptr<CoreThread>> const& core_thread_vector,
    StealQueue<Task>* pool_task_queue, int index, int core_thread_num,
    bool is_batch_io, bool is_bind_cpu)
    : core_thread_queue_{core_thread_vector},
      pool_task_queue_ptr_{pool_task_queue},
      index_{index},
      is_batch_io_{is_batch_io},
      is_bind_cpu_{is_bind_cpu},
      core_thread_num_{core_thread_num},
      is_running_{false},
      wait_num_{0},
      wait_timeout_num_{0} {
  create_steal_range();
}

void CoreThread::SetThreadPoolParam(std::vector<std::shared_ptr<CoreThread>>const& core_thread_vector,
                                    StealQueue<Task>* pool_task_queue_ptr,
                                    int index, int core_thread_num,
                                    bool is_batch_io) {
  core_thread_queue_ = core_thread_vector;
  pool_task_queue_ptr_ = pool_task_queue_ptr;
  index_ = index;
  core_thread_num_ = core_thread_num;
  is_batch_io_ = is_batch_io;
  create_steal_range();
}

void CoreThread::SetThreadPoolParam(std::vector<std::shared_ptr<CoreThread>>const& core_thread_vector,
                                    StealQueue<Task>* pool_task_queue_ptr,
                                    int index, int core_thread_num,
                                    bool is_batch_io,
                                    bool is_bind_cpu) {
  core_thread_queue_ = core_thread_vector;
  pool_task_queue_ptr_ = pool_task_queue_ptr;
  index_ = index;
  core_thread_num_ = core_thread_num;
  is_batch_io_ = is_batch_io;
  is_bind_cpu_ = is_bind_cpu;
  create_steal_range();
}

void CoreThread::Init() {
  is_running_ = true;
  thread_ = std::thread(std::bind(&CoreThread::run, this));
  std::cout << "CoreThread: " << index_ << ", thread id: " << thread_.get_id() << " is running\n";
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

void CoreThread::PushTask(Task&& task) {
  while (!(frist_task_queue_.TryPush(std::forward<Task>(task)) ||
           second_task_queue_.TryPush(std::forward<Task>(task)))) {
    std::this_thread::yield();
  }
  cv_.notify_one();
}

void CoreThread::run() {
  std::cout << "is_batch_io_: " << is_batch_io_ << std::endl;
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
    task();
  } else {
    std::cout << "Core thread: " << index_ << " wait num: " << wait_num_++ <<  std::endl;
    std::unique_lock<std::mutex> ul(lock_);
    auto status = cv_.wait_for(ul, std::chrono::milliseconds(2000));
    if (status == std::cv_status::timeout) {
      std::cout << "Core thread: " << index_ << " wait timeout num: " << wait_timeout_num_++ << std::endl;
    }
  }
}

void CoreThread::run_tasks() {
  std::vector<Task> tasks;
  if (pop_tasks(tasks) || pop_pool_tasks(tasks) || steal_tasks(tasks)) {
    std::cout << "Core thread: " << index_ << " get tasks num: " << tasks.size() << std::endl;
    for (auto && task:  tasks) {
      task();
    }
  } else {
    std::cout << "Core thread: " << index_ << " wait num: " << wait_num_++ <<  std::endl;
    std::unique_lock<std::mutex> ul(lock_);
    auto status = cv_.wait_for(ul, std::chrono::milliseconds(2000));
    if (status == std::cv_status::timeout) {
      std::cout << "Core thread: " << index_ << " wait timeout num: " << wait_timeout_num_++ << std::endl;
    }
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
      std::cout << "Core thread " << index_ << " Steal thread: " << index << " task\n";
      break;
    }
  }
  return ret;
}

bool CoreThread::steal_tasks(std::vector<Task>& tasks) {
  bool ret{false};
  tasks.clear();
  int left_size = MAX_BATCH_SIZE;
  for (auto index : steal_range_) {
    if (left_size > 0 && core_thread_queue_[index]->second_task_queue_.TrySteal(tasks, MAX_BATCH_SIZE)) {
      ret = true;
      std::cout << "Core thread: " << index_ << " steal thread: " << index << " task\n";
      left_size = MAX_BATCH_SIZE - tasks.size();
      if (left_size > 0 && core_thread_queue_[index]->frist_task_queue_.TrySteal(tasks, left_size)) {
        ret = true;
        std::cout << "Core thread: " << index_ << " steal thread: " << index << " task\n";
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