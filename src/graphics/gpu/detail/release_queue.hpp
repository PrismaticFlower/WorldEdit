#pragma once

#include "types.hpp"

#include <atomic>
#include <shared_mutex>
#include <vector>

namespace we::graphics::gpu::detail {

template<typename T>
struct release_queue {
   void push(uint64 work_item, T value)
   {
      std::scoped_lock lock{_mutex};

      _objects.emplace_back(work_item, std::move(value));
      _object_count.fetch_add(1, std::memory_order_release);
   }

   void process(uint64 completed_work_item)
   {
      if (_object_count.load(std::memory_order_acquire) == 0) return;

      std::scoped_lock lock{_mutex};

      std::erase_if(_objects, [completed_work_item](const object& object) {
         return object.work_item <= completed_work_item;
      });

      _object_count.store(_objects.size(), std::memory_order_release);
   }

private:
   struct object {
      uint64 work_item;
      T value;
   };

   std::atomic_size_t _object_count = 0;
   std::shared_mutex _mutex;
   std::vector<object> _objects = [] {
      std::vector<object> objects;

      objects.reserve(256);

      return objects;
   }();
};

}