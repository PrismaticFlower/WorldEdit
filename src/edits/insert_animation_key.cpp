#include "insert_animation_key.hpp"

namespace we::edits {

namespace {

template<typename T>
struct insert_animation_key final : edit<world::edit_context> {
   insert_animation_key(std::vector<T>* keys, std::size_t insert_before_index, T key)
      : keys{keys}, insert_before_index{insert_before_index}, key{key}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(keys));
      assert(insert_before_index <= keys->size());

      keys->insert(keys->begin() + insert_before_index, key);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(keys));
      assert(insert_before_index < keys->size());

      keys->erase(keys->begin() + insert_before_index);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<T>* keys;
   std::size_t insert_before_index;
   T key;
};

}

auto make_insert_animation_key(std::vector<world::position_key>* keys,
                               std::size_t insert_before_index, world::position_key key)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_animation_key<world::position_key>>(keys, insert_before_index,
                                                                      key);
}

auto make_insert_animation_key(std::vector<world::rotation_key>* keys,
                               std::size_t insert_before_index, world::rotation_key key)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_animation_key<world::rotation_key>>(keys, insert_before_index,
                                                                      key);
}

}