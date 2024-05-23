#ifndef STEAL_QUEUE_H_
#define STEAL_QUEUE_H_

#include "base_queue.h"

template<class T>
class StealQueue : public BaseQueue {
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
      task = std::move(queue_.front());
      queue_.pop_front();
      lock_.unlock();
      ret = true;
    } else if (queue_.empty()) {
      // std::cout << "TryPop task but task queue is empty\n";
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
    }

    return ret;
  }

  bool TrySteal(T& task) {
    bool ret{false};

    if (!queue_.empty() && lock_.try_lock()) {
      task = std::move(queue_.back()); // 窃取从队列后端
      queue_.pop_back();
      lock_.unlock();
      ret = true;
      std::cout << "Steal task success\n";
    } else if (queue_.empty()) {
      // std::cout << "TrySteal task but task queue is empty\n";
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
    }

    return ret;
  }

  int Size() {
    int size{0};
    while (true) {
      if (lock_.try_lock()) {
        size = queue_.size();
        lock_.unlock();
        break;
      } else {
        std::this_thread::yield();
      }
    }

    return size;
  }

  bool Empty() {
    bool ret{false};

    while (true) {
      if (lock_.try_lock()) {
        ret = queue_.empty();
        lock_.unlock();
        break;
      } else {
        std::this_thread::yield();
      }
    }

    return ret;
  }

 private:
  /// @brief 双端队列，用于存储任务，方便任务从队尾窃取
  std::deque<T> queue_;
  std::mutex lock_;
};

#endif // STEAL_QUEUE_H_