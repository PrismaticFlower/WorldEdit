#pragma once

#include <array>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace we::assets::odf {

struct property {
   std::string key;
   std::string value;

   bool operator==(const property& other) const noexcept = default;
};

class properties {
public:
   using key_type = std::string;
   using mapped_type = std::string;
   using value_type = property;
   using reference = property&;
   using const_reference = const property&;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;

   properties() = default;

   properties(std::initializer_list<std::array<std::string_view, 2>> properties_list) noexcept;

   auto operator[](const std::string_view key) -> std::string&;

   auto operator[](const std::string_view key) const -> const std::string&;

   auto operator[](const std::size_t index) -> property&;

   auto operator[](const std::size_t index) const -> const property&;

   auto at(const std::string_view key) -> std::string&;

   auto at(const std::string_view key) const -> const std::string&;

   auto at(const std::size_t index) -> property&;

   auto at(const std::size_t index) const -> const property&;

   auto front() noexcept -> property&;

   auto front() const noexcept -> const property&;

   auto back() noexcept -> property&;

   auto back() const noexcept -> const property&;

   // TODO: Add iterators.
   // auto begin() noexcept -> iterator;
   // auto end() noexcept -> iterator;
   //
   // auto begin() const noexcept -> const_iterator;
   // auto end() const noexcept -> const_iterator;
   //
   // auto cbegin() const noexcept -> const_iterator;
   // auto cend() const noexcept -> const_iterator;

   bool contains(const std::string_view key) const noexcept;

   auto capacity() const noexcept -> size_type;

   auto size() const noexcept -> size_type;

   auto max_size() const noexcept -> size_type;

   bool empty() const noexcept;

   void reserve(const std::size_t size) noexcept;

   auto push_back(const property& prop) noexcept -> property&;

   auto push_back(property&& prop) noexcept -> property&;

   void pop_back() noexcept;

   void clear() noexcept;

   bool operator==(const properties& other) const noexcept = default;

private:
   std::vector<property> _props_vec;
};

}
