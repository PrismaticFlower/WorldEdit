
#include "copy_command_list_pool.hpp"

#include <fmt/core.h>

namespace we::graphics {

pooled_copy_command_list::pooled_copy_command_list(gpu::copy_command_list command_list,
                                                   copy_command_list_pool& pool)
   : _command_list{std::move(command_list)}, _pool{pool}
{
}

pooled_copy_command_list ::~pooled_copy_command_list()
{
   _pool.add(std::move(_command_list));
}

auto pooled_copy_command_list::get() noexcept -> gpu::copy_command_list&
{
   return _command_list;
}

auto pooled_copy_command_list::operator->() noexcept -> gpu::copy_command_list*
{
   return &_command_list;
}

auto copy_command_list_pool::aquire_and_reset() -> pooled_copy_command_list
{
   return {aquire_and_reset_manual_management(), *this};
}

auto copy_command_list_pool::aquire_and_reset_manual_management() -> gpu::copy_command_list
{
   {
      std::scoped_lock lock{_mutex};

      if (not _command_lists.empty()) {
         gpu::copy_command_list command_list = std::move(_command_lists.back());

         _command_lists.pop_back();

         command_list.reset();

         return command_list;
      }
   }

   std::string debug_name =
      fmt::format("Pooled Copy Command List #{}",
                  _debug_count.fetch_add(1, std::memory_order_relaxed));

   gpu::copy_command_list command_list = _device.create_copy_command_list({
      .allocator_name = "Pooled Copy Command Lists Allocator",
      .debug_name = debug_name,
   });

   command_list.reset();

   return command_list;
}

void copy_command_list_pool::add(gpu::copy_command_list command_list)
{
   std::scoped_lock lock{_mutex};

   _command_lists.push_back(std::move(command_list));
}

}