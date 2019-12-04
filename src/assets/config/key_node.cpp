
#include <assets/config/key_node.hpp>

#include <algorithm>

#include <fmt/format.h>

using namespace std::literals;

namespace sk::assets::config {

auto node::count(const std::string_view child_key) const noexcept -> std::size_t
{
   return std::count_if(cbegin(), cend(), [child_key](const key_node& child) {
      return child.key == child_key;
   });
}

bool node::contains(const std::string_view child_key) const noexcept
{
   return find(child_key) != cend();
}

auto node::operator[](const std::string_view child_key) noexcept -> key_node&
{
   if (auto it = find(child_key); it != end()) {
      return *it;
   }

   return emplace_back(std::string{child_key}, config::values{});
}

auto node::at(const std::string_view child_key) -> key_node&
{
   if (auto it = find(child_key); it != end()) {
      return *it;
   }

   throw std::runtime_error{
      fmt::format("config node has no child key-node named {}"sv, child_key)};
}

auto node::at(const std::string_view child_key) const -> const key_node&
{
   if (auto it = find(child_key); it != cend()) {
      return *it;
   }

   throw std::runtime_error{
      fmt::format("config node has no child key-node named {}"sv, child_key)};
}

auto node::find(const std::string_view child_key) noexcept -> iterator
{
   return std::find_if(begin(), end(), [child_key](const key_node& child) {
      return child.key == child_key;
   });
}

auto node::find(const std::string_view child_key) const noexcept -> const_iterator
{
   return std::find_if(cbegin(), cend(), [child_key](const key_node& child) {
      return child.key == child_key;
   });
}

auto node::erase(const std::string_view child_key) noexcept -> std::size_t
{
   const auto begin_size = size();

   erase(std::remove_if(begin(), end(),
                        [child_key](const key_node& child) {
                           return child.key == child_key;
                        }),
         end());

   return begin_size - size();
}

auto key_node::at(const std::string_view child_key) -> key_node&
{
   try {
      return static_cast<node&>(*this).at(child_key);
   }
   catch (std::runtime_error&) {
      throw std::runtime_error{fmt::format("config key-node {} has no child key-node named {}"sv,
                                           key, child_key)};
   }
}

auto key_node::at(const std::string_view child_key) const -> const key_node&
{
   try {
      return static_cast<const node&>(*this).at(child_key);
   }
   catch (std::runtime_error&) {
      throw std::runtime_error{fmt::format("config key-node {} has no child key-node named {}"sv,
                                           key, child_key)};
   }
}

}