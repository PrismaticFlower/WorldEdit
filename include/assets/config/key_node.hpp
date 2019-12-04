#pragma once

#include "values.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace sk::assets::config {

class key_node;

class node : std::vector<key_node> {
public:
   using container_type = std::vector<key_node>;
   using iterator = std::vector<key_node>::iterator;
   using const_iterator = std::vector<key_node>::const_iterator;

   node() = default;

   node(values values, std::vector<key_node> children = {}) noexcept
      : values{std::move(values)}, container_type{std::move(children)}
   {
   }

   values values;

   auto count(const std::string_view child_key) const noexcept -> std::size_t;

   bool contains(const std::string_view child_key) const noexcept;

   auto operator[](const std::string_view child_key) noexcept -> key_node&;

   auto at(const std::string_view child_key) -> key_node&;

   auto at(const std::string_view child_key) const -> const key_node&;

   auto find(const std::string_view child_key) noexcept -> iterator;

   auto find(const std::string_view child_key) const noexcept -> const_iterator;

   auto erase(const std::string_view child_key) noexcept -> std::size_t;

   using container_type::begin;

   using container_type::end;

   using container_type::cbegin;

   using container_type::cend;

   using container_type::insert;

   using container_type::emplace;

   using container_type::push_back;

   using container_type::emplace_back;

   using container_type::reserve;

   using container_type::assign;

   using container_type::erase;

   using container_type::empty;

   using container_type::size;

   using container_type::capacity;
};

struct key_base {
   std::string key;
};

class key_node : public key_base, public node {
public:
   key_node() = default;

   key_node(std::string key, config::values values,
            std::vector<key_node> children = {}) noexcept
      : key_base{std::move(key)}, node{std::move(values), std::move(children)}
   {
   }

   auto at(const std::string_view child_key) -> key_node&;

   auto at(const std::string_view child_key) const -> const key_node&;
};
}