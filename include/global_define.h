#ifndef GLOBAL_DEFINE_H_
#define GLOBAL_DEFINE_H_

#include <functional>

using Task = std::function<void(void)>;

constexpr int NONE_CORE_THREAD_TTL{3};

#endif // GLOBAL_DEFINE_H_