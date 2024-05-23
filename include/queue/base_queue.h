#ifndef BASE_QUEUE_H_
#define BASE_QUEUE_H_

#include <iostream>
#include <queue>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

class BaseQueue {
 public:
  BaseQueue() = default;
  virtual ~BaseQueue() = default;
 private:
  // ...
};

#endif // BASE_QUEUE_H_