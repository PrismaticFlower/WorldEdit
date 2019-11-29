
#include <assets/odf/properties.hpp>

#include <cctype>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

using namespace std::literals;

namespace sk::assets::odf {

namespace {

template<typename Vector>
auto find_key(const std::string_view key, Vector& props_vec) -> auto&
{
   for (auto& prop : props_vec) {
      if (boost::iequals(prop.key, key)) return prop.value;
   }

   throw std::invalid_argument{fmt::format("Unable to find property '{}'!"sv, key)};
}

template<typename Vector>
auto index_prop(const std::size_t index, Vector& props_vec) -> auto&
{
   if (index >= props_vec.size()) {
      throw std::out_of_range{
         fmt::format("Unable property at index '{}' as it does not exist!"sv, index)};
   }

   return props_vec[index];
}

}

properties::properties(std::initializer_list<std::array<std::string_view, 2>> properties_list) noexcept
{
   _props_vec.reserve(properties_list.size());

   for (const auto& key_value : properties_list) {
      _props_vec.push_back(property{.key = std::string{key_value[0]},
                                    .value = std::string{key_value[1]}});
   }
}

auto properties::operator[](const std::string_view key) -> std::string&
{
   return find_key(key, _props_vec);
}

auto properties::operator[](const std::string_view key) const -> const std::string&
{
   return find_key(key, _props_vec);
}

auto properties::operator[](const std::size_t index) -> property&
{
   return index_prop(index, _props_vec);
}

auto properties::operator[](const std::size_t index) const -> const property&
{
   return index_prop(index, _props_vec);
}

auto properties::at(const std::string_view key) -> std::string&
{
   return find_key(key, _props_vec);
}

auto properties::at(const std::string_view key) const -> const std::string&
{
   return find_key(key, _props_vec);
}

auto properties::at(const std::size_t index) -> property&
{
   return index_prop(index, _props_vec);
}

auto properties::at(const std::size_t index) const -> const property&
{
   return index_prop(index, _props_vec);
}

auto properties::front() noexcept -> property&
{
   return _props_vec.front();
}

auto properties::front() const noexcept -> const property&
{
   return _props_vec.front();
}

auto properties::back() noexcept -> property&
{
   return _props_vec.back();
}

auto properties::back() const noexcept -> const property&
{
   return _props_vec.back();
}

bool properties::contains(const std::string_view key) const noexcept
{
   for (auto& prop : _props_vec) {
      if (boost::iequals(prop.key, key)) return true;
   }

   return false;
}

auto properties::capacity() const noexcept -> size_type
{
   return _props_vec.capacity();
}

auto properties::size() const noexcept -> size_type
{
   return _props_vec.size();
}

auto properties::max_size() const noexcept -> size_type
{
   return _props_vec.max_size();
}

bool properties::empty() const noexcept
{
   return _props_vec.empty();
}

void properties::reserve(const std::size_t size) noexcept
{
   _props_vec.reserve(size);
}

auto properties::push_back(const property& prop) noexcept -> property&
{
   _props_vec.push_back(prop);

   return _props_vec.back();
}

auto properties::push_back(property&& prop) noexcept -> property&
{
   _props_vec.push_back(std::move(prop));

   return _props_vec.back();
}

void properties::pop_back() noexcept
{
   _props_vec.pop_back();
}

void properties::clear() noexcept
{
   _props_vec.clear();
}

}