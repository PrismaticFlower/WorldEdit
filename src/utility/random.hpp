#pragma once

#include "implementation_storage.hpp"
#include "types.hpp"

namespace we {

struct random_generator {
   /// @brief Initialize the random generator with seed from std::chrono::system_clock.
   random_generator();

   /// @brief Initialize with specific initial state.
   random_generator(uint64 init_state, uint64 init_seq);

   random_generator(const random_generator&);
   auto operator=(const random_generator&) -> random_generator&;

   ~random_generator();

   /// @brief Generate a random number.
   auto operator()() -> uint32;

   /// @brief Generate a uniformly distributed number in the range [0, bound).
   auto generate_bounded(uint32 bound) -> uint32;

   /// @brief Generate a uniformly distributed float in the range [0, 1).
   auto generate_unorm_float() -> float;

private:
   struct state;

   implementation_storage<state, 16> state;
};

}
