#pragma once

#include "assets/msh/default_missing_scene.hpp"
#include "assets/msh/flat_model.hpp"
#include "model.hpp"
#include "texture_manager.hpp"

#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include <gsl/gsl>
#include <tbb/task_group.h>

namespace sk::graphics {

class model_manager {
public:
   model_manager(gpu::device& gpu_device, texture_manager& texture_manager)
      : _gpu_device{&gpu_device}, _texture_manager{texture_manager}
   {
      _copy_fence_wait_value = _placeholder_model.init_gpu_buffer_async(gpu_device);
   };

   ~model_manager()
   {
      _creation_tasks.cancel();
      _creation_tasks.wait();
   }

   auto get(const std::shared_ptr<assets::msh::flat_model>& flat_model) -> model&
   {
      if (not flat_model) return _placeholder_model;

      if (auto model = get_if_loaded(flat_model); model) {
         return *model;
      }

      enqueue_create_model(flat_model);

      return _placeholder_model;
   }

   auto copy_fence_wait_value() -> UINT64
   {
      std::shared_lock lock{_mutex};

      return _copy_fence_wait_value;
   }

private:
   auto get_if_loaded(const std::shared_ptr<assets::msh::flat_model>& flat_model)
      -> model*
   {
      std::shared_lock lock{_mutex};

      if (auto it = _models.find(flat_model); it != _models.end()) {
         return &it->second;
      }

      return nullptr;
   }

   void enqueue_create_model(const std::shared_ptr<assets::msh::flat_model>& flat_model)
   {
      std::scoped_lock lock{_mutex};

      if (auto [it, inserted] = _pending_models.insert(flat_model); not inserted) {
         return;
      }

      _creation_tasks.run([&, flat_model] {
         model model{*flat_model, *_gpu_device, _texture_manager};

         const auto copy_fence_value = model.init_gpu_buffer_async(*_gpu_device);

         std::scoped_lock lock{_mutex};

         _copy_fence_wait_value = std::max(copy_fence_value, _copy_fence_wait_value);
         _models.emplace(flat_model, std::move(model));
         _pending_models.erase(flat_model);
      });
   }

   gsl::not_null<gpu::device*> _gpu_device; // Do NOT change while _creation_tasks has active tasks queued.
   texture_manager& _texture_manager;

   std::shared_mutex _mutex;
   std::unordered_map<std::shared_ptr<assets::msh::flat_model>, model> _models;
   std::unordered_set<std::shared_ptr<assets::msh::flat_model>> _pending_models;

   UINT64 _copy_fence_wait_value = 0;

   tbb::task_group _creation_tasks;

   model _placeholder_model{*assets::msh::default_missing_scene(), *_gpu_device,
                            _texture_manager};
};

}
