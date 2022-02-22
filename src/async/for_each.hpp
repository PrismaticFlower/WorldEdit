#pragma once

#include "thread_pool.hpp"

#include <concepts>
#include <ranges>

namespace we::async {

template<std::ranges::random_access_range random_access_range,
         std::invocable<std::ranges::range_reference_t<random_access_range>> callback_t>
inline void for_each(thread_pool& thread_pool, task_priority priority,
                     random_access_range& range, const callback_t& callback) noexcept
   requires(std::is_nothrow_invocable_v<callback_t, std::ranges::range_reference_t<random_access_range>>)
{
   thread_pool.for_each_n(priority, std::ranges::size(range),
                          [iter = std::ranges::begin(range), &callback](
                             const std::size_t i) noexcept { callback(iter[i]); });
}

}
