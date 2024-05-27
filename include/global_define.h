#ifndef GLOBAL_DEFINE_H_
#define GLOBAL_DEFINE_H_

#include <functional>
#include <future>

using Task = std::function<void(void)>;

using PackagedTask = std::packaged_task<void(void)>;

constexpr int NONE_CORE_THREAD_TTL{3};

#endif // GLOBAL_DEFINE_H_