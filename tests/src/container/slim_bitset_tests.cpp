#include "pch.h"

#include "container/slim_bitset.hpp"

namespace we::container::tests {

TEST_CASE("slim_bitset default construct", "[Container][SlimBitset]")
{
   slim_bitset<64> bits;

   for (std::size_t i = 0; i < 64; ++i) {
      CHECK(bits[i] == false);
   }
}

TEST_CASE("slim_bitset value construct", "[Container][SlimBitset]")
{
   slim_bitset<64> true_bits{true};

   for (std::size_t i = 0; i < 64; ++i) {
      CHECK(true_bits[i] == true);
   }

   slim_bitset<64> false_bits{false};

   for (std::size_t i = 0; i < 64; ++i) {
      CHECK(false_bits[i] == false);
   }
}

TEST_CASE("slim_bitset proxy bit assign", "[Container][SlimBitset]")
{
   slim_bitset<8> bits;

   bits[3] = true;

   CHECK(bits[0] == false);
   CHECK(bits[1] == false);
   CHECK(bits[2] == false);
   CHECK(bits[3] == true);
   CHECK(bits[4] == false);
   CHECK(bits[5] == false);
   CHECK(bits[6] == false);
   CHECK(bits[7] == false);
}

TEST_CASE("slim_bitset test", "[Container][SlimBitset]")
{
   slim_bitset<8> bits;

   bits[3] = true;

   CHECK(bits.test(0) == false);
   CHECK(bits.test(1) == false);
   CHECK(bits.test(2) == false);
   CHECK(bits.test(3) == true);
   CHECK(bits.test(4) == false);
   CHECK(bits.test(5) == false);
   CHECK(bits.test(6) == false);
   CHECK(bits.test(7) == false);
}

TEST_CASE("slim_bitset set", "[Container][SlimBitset]")
{
   slim_bitset<8> bits;

   bits.set(3);

   CHECK(bits.test(0) == false);
   CHECK(bits.test(1) == false);
   CHECK(bits.test(2) == false);
   CHECK(bits.test(3) == true);
   CHECK(bits.test(4) == false);
   CHECK(bits.test(5) == false);
   CHECK(bits.test(6) == false);
   CHECK(bits.test(7) == false);
}

}
