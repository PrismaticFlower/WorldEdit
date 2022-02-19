#pragma once

#include "thread_pool.hpp"

#include <ranges>

namespace we::async {

/// @brief Wait on a group of tasks, only returning once all the tasks are ready.
/// @param ...tasks The tasks to wait on.
template<typename... Ts>
inline void wait_all(task<Ts>&... tasks) noexcept
{
   (tasks.wait(), ...);
}

template<typename T>
concept waitable_task_range = std::ranges::input_range<T> and
   requires(std::ranges::range_reference_t<T> task)
{
   {task.wait()};
};

/// @brief Wait on a range of tasks, only returning once all the tasks are ready.
/// @param ...tasks The tasks to wait on.
inline void wait_all(waitable_task_range auto& tasks) noexcept
{
   for (auto& task : tasks) {
      task.wait();
   }
}

}
