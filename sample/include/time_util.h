/// @brief 获取当前时间相对于1970.01.01:00:00时刻的时间跨度，单位是us
/// @return 返回时间跨度

#ifndef TIME_UTIL_H_
#define TIME_UTIL_H_

#include <chrono>
#include <cstdint>
#include <string>

class TimeUtil {
 public:
  /// @brief get now time (us)
  /// @return now time
  static std::int64_t Now() {
    auto now{std::chrono::system_clock::now()};
    auto tmp{std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch())};
    return tmp.count();
  }
};

#endif