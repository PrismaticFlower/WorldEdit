#include "add_world_req_list.hpp"

namespace we::edits {

namespace {

struct add_world_req_list final : edit<world::edit_context> {
   add_world_req_list(std::string file_type) : _file_type{std::move(file_type)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      context.world.requirements.push_back({.file_type = _file_type});
   }

   void revert(world::edit_context& context) const noexcept override
   {
      context.world.requirements.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const std::string _file_type;
};

}

auto make_add_world_req_list(std::string file_type)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_world_req_list>(std::move(file_type));
}

}