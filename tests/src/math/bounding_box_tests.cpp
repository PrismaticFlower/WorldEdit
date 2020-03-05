#include "pch.h"

#include "approx_test_helpers.hpp"
#include "math/bounding_box.hpp"

namespace sk::math::tests {

TEST_CASE("bounding box combine tests", "[Math][BoundingBox]")
{
   const math::bounding_box a{.min = {-3.0f, 0.0f, 0.0f}, .max = {0.0f, 2.0f, 0.0f}};
   const math::bounding_box b{.min = {0.0f, -1.0f, -2.0f}, .max = {1.0f, 2.0f, -1.0f}};

   const math::bounding_box result = combine(a, b);

   REQUIRE(approx_equals(result.min, {-3.0f, -1.0f, -2.0f}));
   REQUIRE(approx_equals(result.max, {1.0f, 2.0f, 0.0f}));
}

TEST_CASE("bounding box integrate tests", "[Math][BoundingBox]")
{
   math::bounding_box box{.min = {-1.0f, -1.0f, -1.0f}, .max = {1.0f, 1.0f, 1.0f}};

   SECTION("outside point")
   {
      box = integrate(box, {-2.0f, 0.0f, 0.0f});

      REQUIRE(approx_equals(box.min, {-2.0f, -1.0f, -1.0f}));
      REQUIRE(approx_equals(box.max, {1.0f, 1.0f, 1.0f}));
   }

   SECTION("inside point")
   {
      box = integrate(box, {0.5f, 0.5f, 0.5f});

      REQUIRE(approx_equals(box.min, {-1.0f, -1.0f, -1.0f}));
      REQUIRE(approx_equals(box.max, {1.0f, 1.0f, 1.0f}));
   }
}

TEST_CASE("bounding box operator tests", "[Math][BoundingBox]")
{
   math::bounding_box box{.min = {-2.0f, -1.0f, -1.0f}, .max = {2.0f, 1.0f, 1.0f}};

   SECTION("rotation test")
   {
      box = quaternion{0.92388f, 0.382683f, 0.0f, 0.0f} * box;

      REQUIRE(approx_equals(box.min, {-2.0f, -1.41421378f, -1.41421378f}));
      REQUIRE(approx_equals(box.max, {2.0f, 1.41421378f, 1.41421378f}));
   }

   SECTION("translation test")
   {
      box = box + float3{1.0f, 1.0f, 1.0f};

      REQUIRE(approx_equals(box.min, {-1.0f, 0.0f, 0.0f}));
      REQUIRE(approx_equals(box.max, {3.0f, 2.0f, 2.0f}));
   }
}
}
