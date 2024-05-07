#include "delete_animation.hpp"
#include "utility/string_icompare.hpp"

namespace we::edits {

namespace {

struct animation_group_ref {
   uint32 group_index = 0;
   uint32 entry_index = 0;
   world::animation_group::entry entry;
};

struct delete_animation final : edit<world::edit_context> {
   delete_animation(uint32 index, std::vector<animation_group_ref> group_refs)
      : index{index}, group_refs{std::move(group_refs)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      assert(index < context.world.animations.size());

      std::swap(animation, context.world.animations[index]);

      context.world.animations.erase(context.world.animations.begin() + index);

      for (std::ptrdiff_t i = 0; i < std::ssize(group_refs); ++i) {
         animation_group_ref& ref = group_refs[i];

         assert(ref.group_index < context.world.animation_groups.size());

         world::animation_group& group =
            context.world.animation_groups[ref.group_index];

         assert(ref.entry_index < group.entries.size());

         std::swap(ref.entry, group.entries[ref.entry_index]);

         group.entries.erase(group.entries.begin() + ref.entry_index);
      }
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(index <= context.world.animations.size());

      for (std::ptrdiff_t i = std::ssize(group_refs) - 1; i >= 0; --i) {
         animation_group_ref& ref = group_refs[i];

         assert(ref.group_index < context.world.animation_groups.size());

         world::animation_group& group =
            context.world.animation_groups[ref.group_index];

         assert(ref.entry_index <= group.entries.size());

         group.entries.emplace(group.entries.begin() + ref.entry_index,
                               std::move(ref.entry));
      }

      context.world.animations.insert(context.world.animations.begin() + index,
                                      std::move(animation));
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   uint32 index;
   world::animation animation;
   std::vector<animation_group_ref> group_refs;
};

}

auto make_delete_animation(uint32 index, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const world::animation& animation = world.animations[index];

   uint32 group_count = 0;

   for (const auto& group : world.animation_groups) {
      for (const auto& entry : group.entries) {
         if (string::iequals(animation.name, entry.animation)) {
            group_count += 1;
         }
      }
   }

   std::vector<animation_group_ref> group_refs;

   group_refs.reserve(group_count);

   for (const auto& group : world.animation_groups) {
      for (const auto& entry : group.entries) {
         if (string::iequals(animation.name, entry.animation)) {
            group_count += 1;
         }
      }
   }

   for (uint32 group_index = 0; group_index < world.animation_groups.size();
        ++group_index) {
      const world::animation_group& group = world.animation_groups[group_index];

      uint32 delete_offset = 0;

      for (uint32 entry_index = 0; entry_index < group.entries.size(); ++entry_index) {
         if (string::iequals(animation.name, group.entries[entry_index].animation)) {
            group_refs.emplace_back(group_index, entry_index - delete_offset);

            delete_offset += 1;
         }
      }
   }

   return std::make_unique<delete_animation>(index, std::move(group_refs));
}

}
