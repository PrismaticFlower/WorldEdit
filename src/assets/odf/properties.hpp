#pragma once

#include <array>
#include <initializer_list>
#include <string_view>
#include <vector>

namespace we::assets::odf {

struct property {
   std::string_view key;
   std::string_view value;

   bool operator==(const property& other) const noexcept = default;
};

struct properties {
   using key_type = std::string_view;
   using mapped_type = std::string_view;
   using value_type = property;
   using reference = value_type&;
   using const_reference = const value_type&;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;

   using container_type = std::vector<value_type>;
   using iterator = container_type::iterator;
   using const_iterator = container_type::const_iterator;

   properties() = default;

   properties(std::initializer_list<std::array<std::string_view, 2>> properties_list) noexcept;

   auto operator[](const std::string_view key) const -> std::string_view;

   auto operator[](const std::size_t index) const -> const property&;

   auto at(const std::string_view key) const -> std::string_view;

   auto at(const std::size_t index) const -> const property&;

   bool contains(const std::string_view key) const noexcept;

   auto begin() noexcept -> iterator;
   auto end() noexcept -> iterator;

   auto begin() const noexcept -> const_iterator;
   auto end() const noexcept -> const_iterator;

   auto cbegin() const noexcept -> const_iterator;
   auto cend() const noexcept -> const_iterator;

   auto capacity() const noexcept -> size_type;

   auto size() const noexcept -> size_type;

   bool empty() const noexcept;

   auto max_size() const noexcept -> size_type;

   void reserve(const std::size_t size) noexcept;

   void push_back(property&& prop) noexcept;

   bool operator==(const properties& other) const noexcept = default;

private:
   std::vector<property> _props_vec;
};

}
