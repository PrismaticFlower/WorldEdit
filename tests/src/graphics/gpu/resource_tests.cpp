#include "pch.h"

#include "graphics/gpu/resource.hpp"

namespace we::graphics::gpu::tests {

#if 0

struct test_device {
   int called = 0;
};

constexpr int null_test_handle = 0;

using unique_test_handle =
   unique_handle<int, null_test_handle,
                 [](test_device& dev, int) { ++dev.called; }, test_device>;

TEST_CASE("unique_handle tests", "[Graphics][GPU]")
{
   test_device test_device;

   {
      unique_test_handle handle{1, test_device};
      unique_test_handle default_handle;

      REQUIRE(handle.get() == 1);
      REQUIRE(handle);
      REQUIRE(handle != null_test_handle);
      REQUIRE(default_handle == null_test_handle);

      unique_test_handle other_handle;

      other_handle.swap(handle);

      REQUIRE(handle == null_test_handle);
      REQUIRE(other_handle == 1);
   }

   REQUIRE(test_device.called == 1);
}

TEST_CASE("unique_handle reset tests", "[Graphics][GPU]")
{
   test_device test_device;

   {
      unique_test_handle handle{1, test_device};

      handle.reset();

      REQUIRE(not handle);
   }

   REQUIRE(test_device.called == 1);
}

TEST_CASE("unique_handle release tests", "[Graphics][GPU]")
{
   test_device test_device;

   {
      unique_test_handle handle{1, test_device};

      REQUIRE(handle.release() == 1);
      REQUIRE(not handle);
   }

   REQUIRE(test_device.called == 0);
}

#endif

}
