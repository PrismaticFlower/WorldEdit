#pragma once

#include <functional>
#include <shared_mutex>
#include <vector>

namespace we::utility {

class synchronous_task_queue {
public:
   void enqueue(std::function<void()> task)
   {
      std::scoped_lock lock{_tasks_mutex};

      _tasks.emplace_back(std::move(task));
   }

   void execute() noexcept
   {
      std::scoped_lock lock{_tasks_mutex};

      for (const auto& task : _tasks) task();

      _tasks.clear();
   }

private:
   std::shared_mutex _tasks_mutex;
   std::vector<std::function<void()>> _tasks;
};

}
