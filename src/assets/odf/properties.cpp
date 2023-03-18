
#include "properties.hpp"
#include "utility/string_icompare.hpp"

#include <cctype>
#include <span>
#include <stdexcept>

#include <fmt/core.h>

using namespace std::literals;

namespace we::assets::odf {

namespace {

auto find_key(const std::string_view key, std::span<const property> props)
   -> const std::string_view
{
   for (auto& prop : props) {
      if (string::iequals(prop.key, key)) return prop.value;
   }

   throw std::invalid_argument{fmt::format("Unable to find property '{}'!", key)};
}

auto index_prop(const std::size_t index, std::span<const property> props)
   -> const property&
{
   if (index >= props.size()) {
      throw std::out_of_range{
         fmt::format("Unable property at index '{}' as it does not exist!", index)};
   }

   return props[index];
}

}

properties::properties(std::initializer_list<std::array<std::string_view, 2>> properties_list) noexcept
{
   _props_vec.reserve(properties_list.size());

   for (const auto& key_value : properties_list) {
      _props_vec.push_back(property{.key = key_value[0], .value = key_value[1]});
   }
}

auto properties::operator[](const std::string_view key) const -> std::string_view
{
   return find_key(key, _props_vec);
}

auto properties::operator[](const std::size_t index) const -> const property&
{
   return index_prop(index, _props_vec);
}

auto properties::at(const std::string_view key) const -> std::string_view
{
   return find_key(key, _props_vec);
}

auto properties::at(const std::size_t index) const -> const property&
{
   return index_prop(index, _props_vec);
}

bool properties::contains(const std::string_view key) const noexcept
{
   for (auto& prop : _props_vec) {
      if (string::iequals(prop.key, key)) return true;
   }
   return false;
}

auto properties::begin() noexcept -> iterator
{
   return _props_vec.begin();
}

auto properties::end() noexcept -> iterator
{
   return _props_vec.end();
}

auto properties::begin() const noexcept -> const_iterator
{
   return _props_vec.begin();
}

auto properties::end() const noexcept -> const_iterator
{
   return _props_vec.end();
}

auto properties::cbegin() const noexcept -> const_iterator
{
   return _props_vec.cbegin();
}

auto properties::cend() const noexcept -> const_iterator
{
   return _props_vec.cend();
}

auto properties::capacity() const noexcept -> size_type
{
   return _props_vec.capacity();
}

auto properties::size() const noexcept -> size_type
{
   return _props_vec.size();
}

bool properties::empty() const noexcept
{
   return _props_vec.empty();
}

auto properties::max_size() const noexcept -> size_type
{
   return _props_vec.max_size();
}

void properties::reserve(const std::size_t size) noexcept
{
   _props_vec.reserve(size);
}

void properties::push_back(property&& prop) noexcept
{
   _props_vec.push_back(std::move(prop));
}

}
