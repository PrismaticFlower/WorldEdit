#pragma once

#include <initializer_list>
#include <type_traits>

#include <boost/container/small_vector.hpp>
#include <boost/variant2/variant.hpp>

namespace sk::assets::config {

using values_container_type =
   boost::container::small_vector<boost::variant2::variant<int, long long, double, std::string>, 4>;

class values : values_container_type {
public:
   using container_type = values_container_type;
   using variant_type = boost::variant2::variant<int, long long, double, std::string>;

   static_assert(std::is_same_v<container_type::value_type, variant_type>);

   template<typename Type>
   auto get(const std::size_t index) const -> Type
   {
      namespace variant2 = boost::variant2;

      if constexpr (std::is_integral_v<Type>) {
         if (holds_alternative<int>(at(index))) {
            return static_cast<Type>(variant2::get<int>(at(index)));
         }
         else if (holds_alternative<double>(at(index))) {
            return static_cast<Type>(variant2::get<double>(at(index)));
         }
         else {
            return static_cast<Type>(variant2::get<long long>(at(index)));
         }
      }
      else if constexpr (std::is_floating_point_v<Type>) {
         if (holds_alternative<int>(at(index))) {
            return static_cast<Type>(variant2::get<int>(at(index)));
         }
         else if (holds_alternative<long long>(at(index))) {
            return static_cast<Type>(variant2::get<long long>(at(index)));
         }

         return static_cast<Type>(variant2::get<double>(at(index)));
      }
      else if constexpr (std::is_same_v<Type, std::string_view>) {
         return std::string_view{variant2::get<std::string>(at(index))};
      }
      else if constexpr (std::is_same_v<Type, std::string>) {
         return variant2::get<std::string>(at(index));
      }
      else {
         static_assert(
            false,
            "Values in config files can only be ints, floats or strings.");
      }
   }

   void set(const std::size_t index, variant_type value)
   {
      at(index) = std::move(value);
   }

   using container_type::const_iterator;
   using container_type::const_reverse_iterator;
   using container_type::difference_type;
   using container_type::iterator;
   using container_type::reverse_iterator;
   using container_type::size_type;

   using container_type::at;
   using container_type::container_type;
   using container_type::operator[];
   using container_type::back;
   using container_type::begin;
   using container_type::capacity;
   using container_type::cbegin;
   using container_type::cend;
   using container_type::clear;
   using container_type::crbegin;
   using container_type::crend;
   using container_type::data;
   using container_type::emplace;
   using container_type::emplace_back;
   using container_type::empty;
   using container_type::end;
   using container_type::erase;
   using container_type::front;
   using container_type::insert;
   using container_type::max_size;
   using container_type::pop_back;
   using container_type::push_back;
   using container_type::rbegin;
   using container_type::rend;
   using container_type::reserve;
   using container_type::resize;
   using container_type::shrink_to_fit;
   using container_type::size;
   using container_type::swap;
};

}