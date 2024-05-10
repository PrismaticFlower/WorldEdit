#include "delete_animation_key.hpp"

namespace we::edits {

namespace {

template<typename T>
struct delete_animation_key final : edit<world::edit_context> {
   delete_animation_key(std::vector<T>* keys, uint32 index)
      : keys{keys}, index{index}, key{(*keys)[index]}
   {
      assert(index < keys->size());
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(keys));
      assert(index < keys->size());

      keys->erase(keys->begin() + index);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(keys));
      assert(index <= keys->size());

      keys->insert(keys->begin() + index, key);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<T>* keys;
   uint32 index;
   T key;
};

}

auto make_delete_animation_key(std::vector<world::position_key>* keys, uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_animation_key<world::position_key>>(keys, index);
}

auto make_delete_animation_key(std::vector<world::rotation_key>* keys, uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_animation_key<world::rotation_key>>(keys, index);
}

}