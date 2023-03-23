
#include "thread_pool.hpp"

#include <Windows.h>

#include <fmt/xchar.h>

namespace we::async {

namespace detail {

void task_context_base::cancel() noexcept
{
   if (auto thread_pool = owning_thread_pool.lock(); thread_pool) {
      thread_pool->cancel_task(*this);
   }

   // If execution has started on the task then we must wait for it to finish
   // before returning as a task may be being canceled because objects it's
   // callback references are about to be destroyed.
   if (execution_started.load()) wait();
}

}

thread_pool::~thread_pool()
{
   const auto stop_threads = [](priority_level_context& context) noexcept {
      context.pending_tasks.store(pending_tasks_end_value);
      context.pending_tasks.notify_all();
   };

   stop_threads(_lowp_context);
   stop_threads(_normalp_context);
}

thread_pool::thread_pool(const thread_pool_init init)
{
   const auto init_threads = [](priority_level_context& context, std::size_t count,
                                int priority, std::wstring_view description_suffix) {
      context.threads.reserve(count);

      for (std::size_t i = 0; i < count; ++i) {
         auto& thread =
            context.threads.emplace_back([&]() { worker_thread_main(context); });

         SetThreadPriority(thread.native_handle(), priority);
         SetThreadDescription(thread.native_handle(),
                              fmt::format(L"WorldEdit Thread Pool Worker #{}{}",
                                          i, description_suffix)
                                 .c_str());
      }
   };

   init_threads(_lowp_context,
                std::max(init.low_priority_thread_count, std::size_t{1}),
                THREAD_PRIORITY_BELOW_NORMAL, L" (Low Priority)");
   init_threads(_normalp_context, std::max(init.thread_count, std::size_t{1}),
                THREAD_PRIORITY_NORMAL, L"");
}

void thread_pool::cancel_task(detail::task_context_base& task_context) noexcept
{
   const auto cancel_task = [&](priority_level_context& level_context) noexcept {
      std::scoped_lock lock{level_context.tasks_mutex};

      const std::size_t erased =
         std::erase_if(level_context.tasks,
                       [&](std::shared_ptr<detail::task_context_base>& queued_context) noexcept {
                          return &task_context == queued_context.get();
                       });

      level_context.pending_tasks.fetch_sub(erased);
   };

   cancel_task(_lowp_context);
   cancel_task(_normalp_context);
}

void thread_pool::worker_thread_main(priority_level_context& context) noexcept
{
   while (true) {
      context.pending_tasks.wait(0);

      // <= is used here in case another thread is executing a task
      // and drops pending_tasks below pending_tasks_end_value
      if (context.pending_tasks.load() <= pending_tasks_end_value) break;

      std::shared_ptr<detail::task_context_base> task;

      // Try to grab a task off the queue.
      {
         std::scoped_lock lock{context.tasks_mutex};

         // No task for us? Skip back to the start of the loop.
         if (context.tasks.empty()) continue;

         // Yay! A task, grab it and then erase it from the queue.
         task = std::move(context.tasks.front());
         context.tasks.erase(context.tasks.begin());
      }

      // We got a task! Decrement pending_tasks.
      context.pending_tasks.fetch_sub(1);

      // Mark the task as beginning execution, if the task owning has already asked for the result and
      // directly executed the task themselves we skip calling execute_function.
      if (task->execution_started.exchange(true)) continue;

      task->execute_function();
   }
}

}