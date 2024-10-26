#pragma once

#include "types.hpp"

namespace we::utility {

struct stopwatch {
   /// @brief Time elapsed on the stopwatch in seconds.
   /// @return The time in seconds.
   auto elapsed() const noexcept -> float;

   /// @brief Time elapsed on the stopwatch in milliseconds.
   /// @return  The time in milliseconds.
   auto elapsed_ms() const noexcept -> float;

   /// @brief Time elapsed on the stopwatch in seconds.
   /// @return The time in seconds.
   auto elapsed_f64() const noexcept -> double;

   /// @brief Restart the stopwatch.
   void restart() noexcept;

private:
   static auto init_timepoint() -> uint64;

   uint64 _timepoint = init_timepoint();
};

}