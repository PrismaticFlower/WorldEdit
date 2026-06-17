#include "object_optional_link.hpp"

#include "world.hpp"

namespace we::world {

object_optional_link::object_optional_link(const uint32 object_index)
   : _index{object_index}
{
}

object_optional_link::object_optional_link(std::string object_name)
   : _name{std::move(object_name)}
{
}

bool object_optional_link::has_index() const noexcept
{
   return _index >= 0;
}

bool object_optional_link::has_name() const noexcept
{
   return not has_index();
}

auto object_optional_link::index() const noexcept -> uint32
{
   if (_index < 0) std::terminate();

   return static_cast<uint32>(_index);
}

auto object_optional_link::name() noexcept -> std::string&
{
   if (_index >= 0) std::terminate();

   return _name;
}

auto object_optional_link::name() const noexcept -> const std::string&
{
   if (_index >= 0) std::terminate();

   return _name;
}

auto object_optional_link::name_lookup(const world& world) const noexcept
   -> const std::string&
{
   if (has_index()) {
      assert(index() < world.objects.size());

      return world.objects[index()].name;
   }
   else {
      return _name;
   }
}

bool operator==(const object_optional_link& link, const uint32 index) noexcept
{
   if (not link.has_index()) return false;

   return link.index() == index;
}

}