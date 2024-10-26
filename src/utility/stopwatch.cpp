#include "stopwatch.hpp"

#include <chrono>

namespace we::utility {

namespace {

template<typename R>
auto elapsed_impl(uint64 timepoint) noexcept -> R
{
   const std::chrono::steady_clock::time_point start{
      std::chrono::steady_clock::duration(timepoint)};
   const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
   const std::chrono::steady_clock::duration elapsed = now - start;

   return std::chrono::duration_cast<R>(elapsed);
}

}

auto stopwatch::elapsed() const noexcept -> float
{
   return elapsed_impl<std::chrono::duration<float>>(_timepoint).count();
}

auto stopwatch::elapsed_ms() const noexcept -> float
{
   return elapsed_impl<std::chrono::duration<float, std::milli>>(_timepoint).count();
}

auto stopwatch::init_timepoint() -> uint64
{
   return std::chrono::steady_clock::now().time_since_epoch().count();
}

auto stopwatch::elapsed_f64() const noexcept -> double
{
   return elapsed_impl<std::chrono::duration<double>>(_timepoint).count();
}

void stopwatch::restart() noexcept
{
   *this = {};
}

}
