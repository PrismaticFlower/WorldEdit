#pragma once

#include "thread_pool.hpp"

#include <ranges>
#include <tuple>
#include <vector>

namespace we::async {

/// @brief Gets all the results of a group of tasks. Can throw an exception stored inside the tasks.
/// @param ...tasks The tasks to get the results of.
/// @return The results of the tasks, as a tuple.
template<typename... Ts>
inline auto get_all(task<Ts>&... tasks) -> std::tuple<Ts...>
{
   return {tasks.get()...};
}

template<typename T>
concept result_producing_task_range = std::ranges::forward_range<T> and
   requires(std::ranges::range_reference_t<T> task)
{
   {task.get()};
};

/// @brief Gets the results of a range of tasks. Can throw an exception stored inside the tasks.
/// @param tasks The tasks to get the results of.
/// @return The results of the tasks, as a vector.
template<result_producing_task_range task_range>
inline auto get_all(task_range& tasks)
   -> std::vector<typename std::ranges::range_value_t<task_range>::result_type>
{
   std::vector<typename std::ranges::range_value_t<task_range>::result_type> results;

   results.reserve(std::ranges::size(tasks));

   for (auto& task : tasks) results.emplace_back(task.get());

   return results;
}

}
