#pragma once

#include "types.hpp"

#include <shared_mutex>
#include <vector>

namespace we::graphics::gpu::detail {

template<typename T>
struct release_queue {
   void push(uint64 work_item, T value)
   {
      std::scoped_lock lock{_mutex};

      _objects.emplace_back(work_item, std::move(value));
   }

   void process(uint64 completed_work_item)
   {
      std::scoped_lock lock{_mutex};

      std::erase_if(_objects, [completed_work_item](const object& object) {
         return object.work_item <= completed_work_item;
      });
   }

private:
   struct object {
      uint64 work_item;
      T value;
   };

   std::shared_mutex _mutex;
   std::vector<object> _objects = [] {
      std::vector<object> objects;

      objects.reserve(256);

      return objects;
   }();
};

}