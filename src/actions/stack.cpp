
#include "stack.hpp"

#include <ranges>

namespace we::actions {

stack::stack(const stack_init params) noexcept
{
   _applied.reserve(params.reserve_size);
   _reverted.reserve(params.reserve_size);
}

void stack::apply(std::unique_ptr<action> action, world::world& world) noexcept
{
   action->apply(world);

   _applied.push_back(std::move(action));
   _reverted.clear();
}

void stack::revert(world::world& world) noexcept
{
   revert(1, world);
}

void stack::reapply(world::world& world) noexcept
{
   reapply(1, world);
}

void stack::revert(const std::size_t count, world::world& world) noexcept
{
   const std::size_t clamped_count = std::min(count, _applied.size());

   using namespace std::ranges::views;

   for (auto& action : _applied | reverse | take(clamped_count)) {
      action->revert(world);

      _reverted.push_back(std::move(action));
   }

   _applied.erase(_applied.end() - clamped_count, _applied.end());
}

void stack::reapply(const std::size_t count, world::world& world) noexcept
{
   const std::size_t clamped_count = std::min(count, _reverted.size());

   using namespace std::ranges::views;

   for (auto& action : _reverted | reverse | take(clamped_count)) {
      action->apply(world);

      _applied.push_back(std::move(action));
   }

   _reverted.erase(_reverted.end() - clamped_count, _reverted.end());
}

void stack::revert_all(world::world& world) noexcept
{
   revert(_applied.size(), world);
}

void stack::reapply_all(world::world& world) noexcept
{
   reapply(_reverted.size(), world);
}

auto stack::applied_size() const noexcept -> std::size_t
{
   return _applied.size();
}

auto stack::reverted_size() const noexcept -> std::size_t
{
   return _reverted.size();
}

}