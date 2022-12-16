#include "pch.h"

#include "utility/implementation_storage.hpp"

#include <utility>

namespace we::tests {

namespace {

struct status {
   bool constructed = false;
   bool destroyed = false;

   bool copy_constructed = false;
   bool copy_assigned = false;

   bool move_constructed = false;
   bool move_assigned = false;
};

struct test_struct {
   test_struct(status& status) : status{status}
   {
      status.constructed = true;
   };

   ~test_struct()
   {
      status.destroyed = true;
   };

   test_struct(const test_struct& other) : status{other.status}
   {
      status.copy_constructed = true;
   };

   auto operator=(const test_struct&) -> test_struct&
   {
      status.copy_assigned = true;

      return *this;
   }

   test_struct(test_struct&& other) : status{other.status}
   {
      status.move_constructed = true;
   };

   auto operator=(test_struct&&) -> test_struct&
   {
      status.move_assigned = true;

      return *this;
   }

   status& status;
};

}

TEST_CASE("implementation_storage construction",
          "[Utility][ImplementationStorage]")
{
   status status;

   {
      implementation_storage<test_struct, sizeof(test_struct), alignof(test_struct)> storage{
         status};

      REQUIRE(status.constructed);
   }

   REQUIRE(status.destroyed);

   {
      implementation_storage<test_struct, sizeof(test_struct), alignof(test_struct)> other_storage{
         status};
      implementation_storage<test_struct, sizeof(test_struct), alignof(test_struct)> storage{
         other_storage};

      REQUIRE(status.copy_constructed);
   }

   {
      implementation_storage<test_struct, sizeof(test_struct), alignof(test_struct)> other_storage{
         status};
      implementation_storage<test_struct, sizeof(test_struct), alignof(test_struct)> storage{
         status};

      storage = other_storage;

      REQUIRE(status.copy_assigned);
   }

   {
      implementation_storage<test_struct, sizeof(test_struct), alignof(test_struct)> other_storage{
         status};
      implementation_storage<test_struct, sizeof(test_struct), alignof(test_struct)> storage{
         std::move(other_storage)};

      REQUIRE(status.move_constructed);
   }

   {
      implementation_storage<test_struct, sizeof(test_struct), alignof(test_struct)> other_storage{
         status};
      implementation_storage<test_struct, sizeof(test_struct), alignof(test_struct)> storage{
         status};

      storage = std::move(other_storage);

      REQUIRE(status.move_assigned);
   }
}

}