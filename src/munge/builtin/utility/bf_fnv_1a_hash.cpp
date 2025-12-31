#include "bf_fnv_1a_hash.hpp"

namespace we::munge {

auto bf_fnv_1a_hash(const std::string_view str) noexcept -> uint32
{
   const uint32 fnv_prime = 16777619;
   const uint32 offset_basis = 2166136261;

   uint32 hash = offset_basis;

   for (auto c : str) {
      c |= 0x20;

      hash ^= c;
      hash *= fnv_prime;
   }

   return hash;
}

}