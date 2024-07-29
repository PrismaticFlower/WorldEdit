#include "pch.h"

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "output_stream.hpp"

namespace we::edits::tests {

auto null_asset_libraries() noexcept -> assets::libraries_manager&
{
   static null_output_stream output;
   static assets::libraries_manager asset_libraries{output,
                                                    async::thread_pool::make(
                                                       {.thread_count = 1,
                                                        .low_priority_thread_count = 1})};

   return asset_libraries;
}

}
