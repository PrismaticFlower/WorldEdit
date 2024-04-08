#include "pch.h"

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "output_stream.hpp"
#include "world/object_class_library.hpp"

using namespace std::literals;

namespace we::world::tests {

TEST_CASE("world object_class_library acquire-free", "[World]")
{
   null_output_stream output;
   std::shared_ptr<async::thread_pool> thread_pool =
      async::thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});
   assets::libraries_manager assets_libraries{output, thread_pool};

   object_class_library library{assets_libraries};

   const lowercase_string class0{"hello"sv};
   const lowercase_string class1{"goodbye"sv};

   object_class_handle handle0 = library.acquire(class0);

   CHECK(library.debug_ref_count(class0) == 1);

   object_class_handle handle1 = library.acquire(class1);

   CHECK(library.debug_ref_count(class1) == 1);

   object_class_handle handle2 = library.acquire(class0);

   CHECK(library.debug_ref_count(class0) == 2);

   library.free(handle0);

   CHECK(library.debug_ref_count(class0) == 1);

   library.free(handle1);

   CHECK(library.debug_ref_count(class1) == 0);

   library.free(handle2);

   CHECK(library.debug_ref_count(class0) == 0);
}

TEST_CASE("world object_class_library acquire-free empty class name", "[World]")
{
   null_output_stream output;
   std::shared_ptr<async::thread_pool> thread_pool =
      async::thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});
   assets::libraries_manager assets_libraries{output, thread_pool};

   object_class_library library{assets_libraries};

   object_class_handle handle0 = library.acquire(lowercase_string{""sv});
   object_class_handle handle1 = library.acquire(lowercase_string{""sv});
   object_class_handle handle2 = library.acquire(lowercase_string{""sv});

   CHECK(library.debug_ref_count(lowercase_string{""sv}) == 0);

   library.free(handle0);
   library.free(handle1);
   library.free(handle2);

   CHECK(library.debug_ref_count(lowercase_string{""sv}) == 0);
}

TEST_CASE("world object_class_library acquire-clear", "[World]")
{
   null_output_stream output;
   std::shared_ptr<async::thread_pool> thread_pool =
      async::thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});
   assets::libraries_manager assets_libraries{output, thread_pool};

   object_class_library library{assets_libraries};

   const lowercase_string class0{"hello"sv};
   const lowercase_string class1{"goodbye"sv};

   (void)library.acquire(class0);
   (void)library.acquire(class1);
   (void)library.acquire(class0);

   library.clear();

   CHECK(library.debug_ref_count(class0) == 0);
   CHECK(library.debug_ref_count(class1) == 0);
}

TEST_CASE("world object_class_library free default handle", "[World]")
{
   null_output_stream output;
   std::shared_ptr<async::thread_pool> thread_pool =
      async::thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});
   assets::libraries_manager assets_libraries{output, thread_pool};

   object_class_library library{assets_libraries};

   library.free(object_class_handle{});
   library.free(object_class_handle{0});

   CHECK(library.debug_ref_count(lowercase_string{""sv}) == 0);
}

#ifdef OBJECT_CLASS_LIBRARY_SLOW_TESTS

TEST_CASE("world object_class_library acquire-free max_ref clamp", "[World]")
{

   null_output_stream output;
   std::shared_ptr<async::thread_pool> thread_pool =
      async::thread_pool::make({.thread_count = 1, .low_priority_thread_count = 1});
   assets::libraries_manager assets_libraries{output, thread_pool};

   object_class_library library{assets_libraries};

   const lowercase_string class0{"hello"sv};

   for (uint32 i = 0; i < UINT32_MAX; ++i) {
      // This would leak handle in real code.
      (void)library.acquire(class0);
   }

   CHECK(library.debug_ref_count(class0) == UINT32_MAX);

   object_class_handle last_handle = library.acquire(class0);

   CHECK(library.debug_ref_count(class0) == UINT32_MAX);

   object_class_handle handle_null = library.acquire(lowercase_string{""sv});

   CHECK(last_handle == handle_null);

   library.clear();
}

#endif

}