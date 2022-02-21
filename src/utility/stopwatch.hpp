#pragma once

#include <chrono>

namespace we::utility {

template<typename T = std::chrono::high_resolution_clock>
class stopwatch {
public:
   using clock_type = T;

   template<typename R = clock_type::duration>
   auto elapsed() const noexcept -> R
   {
      const clock_type::time_point now = clock_type::now();
      const clock_type::duration elapsed = now - _start;

      return std::chrono::duration_cast<R>(elapsed);
   }

private:
   clock_type::time_point _start = clock_type::now();
};

}