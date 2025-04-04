#include "blocks.hpp"

#include <cassert>

namespace we::world {

block_id::block_id(block_box_id id) noexcept
   : id_type{block_type::box}, id{.box = id}
{
}

bool block_id::is_box() const noexcept
{
   return id_type == block_type::box;
}

auto block_id::get_box() const noexcept -> block_box_id
{
   assert(id_type == block_type::box);

   return id.box;
}

auto block_id::type() const noexcept -> block_type
{
   return id_type;
}

block_id block_id::none = {};

}