#include "random.hpp"

#include <pcg_basic.h>

#include <array>
#include <chrono>

namespace we {

struct random_generator::state {
   pcg_state_setseq_64 pcg;
};

random_generator::random_generator()
{
   const uint64 init_state =
      std::chrono::system_clock::now().time_since_epoch().count();
   const uint64 init_seq = 0xda3e39cb94b95bdbull;

   pcg32_srandom_r(&state->pcg, init_state, init_seq);
}

random_generator::random_generator(uint64 init_state, uint64 init_seq)
{
   pcg32_srandom_r(&state->pcg, init_state, init_seq);
}

random_generator::random_generator(const random_generator&) = default;

auto random_generator::operator=(const random_generator&) -> random_generator& = default;

random_generator::~random_generator() = default;

auto random_generator::operator()() -> uint32
{
   return pcg32_random_r(&state->pcg);
}

auto random_generator::generate_bounded(uint32 bound) -> uint32
{
   return pcg32_boundedrand_r(&state->pcg, bound);
}

auto random_generator::generate_unorm_float() -> float
{
   return generate_bounded(16777216) / 16777216.0f;
}

}