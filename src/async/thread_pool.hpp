#pragma once

#include "utility/implementation_storage.hpp"

#include <concepts>
#include <exception>
#include <functional>
#include <latch>
#include <memory>
#include <vector>

namespace we::async {

class thread_pool;

namespace detail {

struct task_context_base {
   /// @brief Flag for if the task has started being executed or not.
   std::atomic_bool execution_started = false;

   /// @brief Contains the function that executes the task.
   std::move_only_function<void()> execute_function;

   /// @brief Latch that will be counted down upon completion of the task. Once at 0 task_exception_ptr and result can be safely used.
   std::latch executed_latch{1};

   /// @brief execute_function will catch and store any exception from the task into this.
   std::exception_ptr task_exception_ptr = nullptr;

   /// @brief The thread_pool that owns the task.
   std::weak_ptr<thread_pool> owning_thread_pool;

   /// @brief Use owning_thread_pool to cancel the task.
   void cancel() noexcept;

   /// @brief Use owning_thread_pool to cancel the task without waiting for it to complete if it's execution has started.
   void cancel_no_wait() noexcept;

   /// @brief Check if the task's result is ready.
   /// @return True if the task's result is ready, false otherwise.
   [[nodiscard]] bool ready() const noexcept
   {
      return executed_latch.try_wait();
   }

   /// @brief Wait for a task to be ready, executing it directly if needed.
   void wait() noexcept
   {
      if (ready()) return;

      if (try_direct_execute()) return;

      executed_latch.wait();
   }

   /// @brief Wait for a task to be ready, will not execute the task if it's still pending.
   void wait_no_execute() const noexcept
   {
      executed_latch.wait();
   }

   /// @brief Tries to execute a task directly if it's not already being executed by the thread_pool.
   /// @return True if the task was executed directly, false if it is being executed by the thread_pool.
   bool try_direct_execute() noexcept
   {
      if (execution_started.exchange(true)) return false;

      execute_function();
      cancel();

      return true;
   }
};

template<typename T>
struct task_context : task_context_base {
   /// @brief Flag for use by task to check for double calling of task::get.
   bool result_obtained = false;
   /// @brief Result of the task.
   T result;
};

template<>
struct task_context<void> : task_context_base {
   /// @brief Flag for use by task to check for double calling of task::get.
   bool result_obtained = false;
};

}

/// @brief A simple async task class.
/// @tparam T The type returned by the task.
///
/// Abandoning a task causes cancel() to be called for that task in it's destructor.
template<typename T>
class task {
public:
   static_assert(not std::is_reference_v<T>, "T can not be a reference type.");
   static_assert(std::is_default_constructible_v<T> or std::is_void_v<T>,
                 "T must be default constructible or void.");
   static_assert(std::is_move_assignable_v<T> or std::is_void_v<T>,
                 "T must be move assignable or void.");

   /// @brief The type of the result of the task. Can be void.
   using result_type = T;

   /// @brief Construct an empty task. Calling any method but valid() on an empty task results in std::terminate being called.
   task() noexcept = default;

   ~task()
   {
      if (_context and not ready()) {
         cancel();
      }
   }

   /// @brief Construct a task from a context. Intended to be called by thread_pool.
   /// @param context The task context.
   task(std::shared_ptr<detail::task_context<T>> context) noexcept
      : _context{std::move(context)}
   {
   }

   task(task&& other) noexcept = default;
   auto operator=(task&& other) noexcept -> task& = default;

   task(const task& other) noexcept = delete;
   auto operator=(const task& other) noexcept -> task& = delete;

   /// @brief Checks if the task's result is ready.
   /// @return True if the result is ready, false otherwise.
   [[nodiscard]] bool ready() const noexcept
   {
      if (!_context) std::terminate();

      return _context->executed_latch.try_wait();
   }

   /// @brief Cancels a task, removing it from the owning thread_pool's queue.
   /// After this calls to methods other than valid on this task object will result in std::terminate being called.
   void cancel() noexcept
   {
      if (!_context) std::terminate();

      _context->cancel();
      _context = nullptr;
   }

   /// @brief Cancels a task, removing it from the owning thread_pool's queue. If the task had started execution this method will not
   /// wait for it to complete before returning.
   ///
   /// This makes this method unsafe if the task was referencing objects that will be destroyed after the cancel call.
   ///
   /// After this calls to methods other than valid on this task object will result in std::terminate being called.
   void cancel_no_wait() noexcept
   {
      if (!_context) std::terminate();

      _context->cancel_no_wait();
      _context = nullptr;
   }

   /// @brief Gets the result of the task, waiting or executing the task directly if needed. Throws any exception thrown by the task. Calling this twice results in std::terminate being called.
   /// @return The result of the task.
   [[nodiscard]] auto get() -> T
      requires(not std::same_as<T, void>)
   {
      if (not _context or std::exchange(_context->result_obtained, true)) {
         std::terminate();
      }

      wait();

      if (_context->task_exception_ptr != nullptr) {
         std::rethrow_exception(std::exchange(_context->task_exception_ptr, nullptr));
      }

      return std::move(_context->result);
   }

   /// @brief Waits for a task that returns nothing to be ready. Throws any exception thrown by the task. Calling this twice results in std::terminate being called.
   void get()
      requires(std::same_as<T, void>)
   {
      if (not _context or std::exchange(_context->result_obtained, true)) {
         std::terminate();
      }

      wait();

      if (_context->task_exception_ptr != nullptr) {
         std::rethrow_exception(std::exchange(_context->task_exception_ptr, nullptr));
      }
   }

   /// @brief Waits for the task's result to be ready. If the task is not already executing this function may execute it itself.
   void wait() noexcept
   {
      if (!_context) std::terminate();

      _context->wait();
   }

   /// @brief Waits for the task's result to be ready without starting it's execution.
   void wait_no_execute() const noexcept
   {
      if (!_context) std::terminate();

      _context->wait_no_execute();
   }

   /// @brief Checks if the task object refers to a task scheduled on a thread_pool.
   /// @return True if the task object is valid, false otherwise.
   [[nodiscard]] bool valid() const noexcept
   {
      return _context != nullptr;
   }

private:
   std::shared_ptr<detail::task_context<T>> _context;
};

/// @brief Priority for tasks scheduled on a thread_pool.
enum class task_priority { low, normal };

/// @brief Initialization parameters for the thread_pool.
struct thread_pool_init {
   /// @brief Number of threads to create for the thread_pool. Must be >= 1.
   const std::size_t thread_count;
   /// @brief Number of low priority threads to create for the thread_pool. Must be >= 1.
   const std::size_t low_priority_thread_count;
};

/// @brief thread_pool implementation focusing on simplicity, support for priorities and predictability.
/// This is not intended to have the most features or the best raw throughput, rather it is focused on
/// being "good enough" for WorldEdit's specific use case.
class thread_pool : public std::enable_shared_from_this<thread_pool> {
public:
   /// @brief Initialize the thread_pool with a default number of threads.
   [[nodiscard]] static auto make() noexcept -> std::shared_ptr<thread_pool>;

   /// @brief Initialize the thread_pool explicit settings.
   [[nodiscard]] static auto make(const thread_pool_init& init) noexcept
      -> std::shared_ptr<thread_pool>;

   ~thread_pool();

   thread_pool(const thread_pool&) = delete;
   thread_pool(thread_pool&&) = delete;
   auto operator=(const thread_pool&) -> thread_pool& = delete;
   auto operator=(thread_pool&&) -> thread_pool& = delete;

   /// @brief Adds a task to the thread_pool's work queue.
   /// @tparam Fn The function to invoke for the task.
   /// @tparam T The return type of the task.
   /// @param priority The priority of the task.
   /// @param func The task's function.
   /// @return The task.
   template<std::invocable Fn, typename T = std::invoke_result_t<Fn>>
   [[nodiscard]] auto exec(const task_priority priority, Fn func) noexcept
      -> task<T>
   {
      auto task_context = std::make_shared<detail::task_context<T>>();

      task_context->execute_function = [task_context = task_context.get(),
                                        func = std::move(func)]() noexcept {
         try {
            if constexpr (std::is_void_v<T>) {
               func();
            }
            else {
               task_context->result = func();
            }
         }
         catch (...) {
            task_context->task_exception_ptr = std::current_exception();
         }

         task_context->executed_latch.count_down();
      };
      task_context->owning_thread_pool = shared_from_this();

      submit_task(priority, task_context);

      return {task_context};
   }

   /// @brief Adds a task to the thread_pool's work queue with task_priority::normal.
   /// @tparam Fn The function to invoke for the task.
   /// @tparam T The return type of the task.
   /// @param func The task's function.
   /// @return The task.
   template<std::invocable Fn, typename T = std::invoke_result_t<Fn>>
   [[nodiscard]] auto exec(Fn&& func) noexcept -> task<T>
   {
      return exec(task_priority::normal, std::forward<Fn>(func));
   }

   /// @brief Executes a function over a range of indices.
   /// @tparam Fn The function to invoke for each index. Must be nothrow invocable.
   /// @param priority The return type of the task.
   /// @param size The size of the range, exclusive. Used as if for (std::size_t i = 0; i < size; ++i) { ... }.
   /// @param func The function processes the index.
   template<std::invocable<std::size_t> Fn>
   void for_each_n(const task_priority priority, const std::size_t size,
                   const Fn& func) noexcept
      requires(std::is_nothrow_invocable_v<Fn, std::size_t>)
   {
      // If we're being called from the "main" thread add an extra task for it to process.
      const std::size_t desired_task_count =
         std::max(is_main_thread() ? thread_count(priority) + 1 : thread_count(priority),
                  std::size_t{1});

      if (size <= desired_task_count) {
         auto tasks = std::make_shared<detail::task_context_base[]>(size);

         // Schedule the tasks!
         for (std::size_t i = 0; i < size; ++i) {
            auto& task = tasks[i];

            task.execute_function = [&task, &func, i]() noexcept {
               func(i);

               task.executed_latch.count_down();
            };
            task.owning_thread_pool = shared_from_this();

            submit_task(priority,
                        std::shared_ptr<detail::task_context_base>{tasks, &task});
         }

         for (std::ptrdiff_t i = static_cast<std::ptrdiff_t>(size) - 1; i >= 0; --i) {
            tasks[i].wait();
         }
      }
      else {
         const std::size_t task_work_size = size / desired_task_count;
         const std::size_t remainder_task_work_size =
            size - (desired_task_count * task_work_size);
         const std::size_t final_task_count = remainder_task_work_size != 0
                                                 ? desired_task_count + 1
                                                 : desired_task_count;

         auto tasks = std::make_shared<detail::task_context_base[]>(final_task_count);

         for (std::size_t i = 0; i < desired_task_count; ++i) {
            auto& task = tasks[i];

            task.execute_function = [&task, &func, start = i * task_work_size,
                                     end = (i + 1) * task_work_size]() noexcept {
               for (std::size_t i = start; i < end; ++i) {
                  func(i);
               }

               task.executed_latch.count_down();
            };
            task.owning_thread_pool = shared_from_this();

            submit_task(priority,
                        std::shared_ptr<detail::task_context_base>{tasks, &task});
         }

         if (remainder_task_work_size != 0) {
            auto& task = tasks[desired_task_count];

            task.execute_function = [&task, &func,
                                     start = desired_task_count * task_work_size,
                                     end = size]() noexcept {
               for (std::size_t i = start; i < end; ++i) {
                  func(i);
               }

               task.executed_latch.count_down();
            };
            task.owning_thread_pool = shared_from_this();

            submit_task(priority,
                        std::shared_ptr<detail::task_context_base>{tasks, &task});
         }

         for (std::ptrdiff_t i = static_cast<std::ptrdiff_t>(final_task_count) - 1;
              i >= 0; --i) {
            tasks[i].wait();
         }
      }
   }

   /// @brief Cancel a task. Directly calling is not needed (it is called from inside task).
   /// @param task_context The context of the task to cancel.
   void cancel_task(detail::task_context_base& task_context) noexcept;

   /// @brief Gets the thread count for a priority level.
   /// @param priority The priority level to get the thread count for.
   /// @return The thread count.
   [[nodiscard]] auto thread_count(const task_priority priority) const noexcept
      -> std::size_t;

private:
   thread_pool(const thread_pool_init& init);

   void submit_task(const task_priority priority,
                    std::shared_ptr<detail::task_context_base> task) noexcept;

   bool is_main_thread() const noexcept;

   struct impl;

   implementation_storage<impl, 168> impl;
};

}
