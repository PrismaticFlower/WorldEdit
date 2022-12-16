#pragma once

#include "gpu/rhi.hpp"

#include <atomic>
#include <shared_mutex>
#include <vector>

namespace we::graphics {

struct copy_command_list_pool;

struct pooled_copy_command_list {
   pooled_copy_command_list(gpu::copy_command_list command_list,
                            copy_command_list_pool& pool);

   ~pooled_copy_command_list();

   pooled_copy_command_list(const pooled_copy_command_list&) = delete;
   pooled_copy_command_list(pooled_copy_command_list&&) = delete;

   auto get() noexcept -> gpu::copy_command_list&;

   auto operator->() noexcept -> gpu::copy_command_list*;

private:
   gpu::copy_command_list _command_list;
   copy_command_list_pool& _pool;
};

struct copy_command_list_pool {
   copy_command_list_pool(gpu::device& device) : _device{device} {}

   auto aquire_and_reset() -> pooled_copy_command_list;

   auto aquire_and_reset_manual_management() -> gpu::copy_command_list;

   void add(gpu::copy_command_list command_list);

private:
   std::shared_mutex _mutex;
   std::vector<gpu::copy_command_list> _command_lists;
   std::atomic_int _debug_count = 0;

   gpu::device& _device;
};

}