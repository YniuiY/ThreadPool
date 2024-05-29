#ifndef STEAL_QUEUE_H_
#define STEAL_QUEUE_H_

#include <iostream>
#include <queue>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

template<class T>
class StealQueue {
 public:
  StealQueue() = default;
  ~StealQueue() = default;

  void Push(T&& task) {
    while (true) {
      if (lock_.try_lock()) {
        queue_.emplace_back(std::forward<T>(task));
        lock_.unlock();
        break;
      } else {
        std::cout <<"Push task failed, yield\n";
        std::this_thread::yield(); // 主动让出cpu资源
      }
    }
  }

  void Push(std::vector<T>& tasks) {
    while (true) {
      if (lock_.try_lock()) {
        for (auto && task : tasks) {
          queue_.emplace_back(std::forward<T>(task));
        }
        lock_.unlock();
        break;
      } else {
        std::this_thread::yield(); // 主动让出cpu资源
      }
    }
  }

  bool TryPush(T&& task) {
    bool ret{false};

    if (lock_.try_lock()) {
      queue_.emplace_back(std::forward<T>(task));
      lock_.unlock();
      ret = true;
    }

    return ret;
  }

  bool TryPush(std::vector<T>& tasks) {
    bool ret{false};

    if (lock_.try_lock()) {
      for (auto && task : tasks) {
        queue_.emplace_back(std::forward<T>(task));
      }
      lock_.unlock();
      ret = true;
    }

    return ret;
  }

  bool TryPop(T& task) {
    bool ret{false};

    if (!queue_.empty() && lock_.try_lock()) {
      if (!queue_.empty()) {
        task = std::move(queue_.front());
        queue_.pop_front();
        lock_.unlock();
        if (task == nullptr) {
          std::cout << "TryPop task but task is nullptr\n";
          std::runtime_error("TryPop task but task is nullptr");
        }
        ret = true;
      } else {
        lock_.unlock();
      }
    }

    return ret;
  }

  bool TryPop(std::vector<T>& tasks, int max_batch_size) {
    bool ret{false};

    if (!queue_.empty() && lock_.try_lock()) {
      while (!queue_.empty() && max_batch_size-- > 0) {
        tasks.emplace_back(std::move(queue_.front()));
        queue_.pop_front();
      }
      lock_.unlock();
      ret = true;
      std::cout << "TryPop tasks num: " << tasks.size() << ", bacth size: " << max_batch_size << std::endl;
    }

    return ret;
  }

  bool TrySteal(T& task) {
    bool ret{false};

    if (!queue_.empty() && lock_.try_lock()) {
      if (!queue_.empty()) {
        task = std::move(queue_.back()); // 窃取从队列后端
        queue_.pop_back();
        lock_.unlock();
        if (task == nullptr) {
          std::cout << "TrySteal task but task is nullptr\n";
          std::runtime_error("TrySteal task but task is nullptr");
        }
        ret = true;
        std::cout << "Steal task success\n";
      } else {
        lock_.unlock();
      }
    }

    return ret;
  }

  bool TrySteal(std::vector<T>& tasks, int max_batch_size) {
    bool ret{false};

    if (!queue_.empty() && lock_.try_lock()) {
      while (!queue_.empty() && max_batch_size-- > 0) {
        tasks.emplace_back(std::move(queue_.back()));
        queue_.pop_back();
      }
      lock_.unlock();
      ret = true;
      std::cout << "Steal tasks success\n";
    }

    return ret;
  }

  int Size() {
    std::lock_guard<std::mutex> lg(lock_);
    return queue_.size();
  }

  bool Empty() {
    std::lock_guard<std::mutex> lg(lock_);
    return queue_.empty();
  }

 private:
  /// @brief 双端队列，用于存储任务，方便任务从队尾窃取
  std::deque<T> queue_;
  std::mutex lock_;
};

#endif // STEAL_QUEUE_H_