#pragma once

#include <memory>
#include <string_view>

namespace we::assets {

/// @brief String storage class that guarantees heap storage. Not for generic use.
struct stable_string {
   stable_string() = default;

   explicit stable_string(const std::string_view str)
      : _data{str.empty() ? nullptr
                          : std::make_unique_for_overwrite<char[]>(str.size())},
        _size{str.size()}
   {
      if (not str.empty()) std::memcpy(&_data[0], &str[0], _size);
   }

   stable_string(const stable_string&) = delete;
   auto operator=(const stable_string&) noexcept -> stable_string& = delete;

   stable_string(stable_string&& other) noexcept
   {
      stable_string discard;

      other.swap(discard);
      this->swap(discard);
   }

   auto operator=(stable_string&& other) noexcept -> stable_string&
   {
      stable_string discard;

      other.swap(discard);
      this->swap(discard);

      return *this;
   }

   void swap(stable_string& other)
   {
      using std::swap;

      swap(this->_data, other._data);
      swap(this->_size, other._size);
   }

   operator std::string_view() const noexcept
   {
      return {_data.get(), _size};
   }

   friend bool operator==(const stable_string& l, const stable_string& r) noexcept
   {
      return std::string_view{l} == std::string_view{r};
   }

   friend auto operator<=>(const stable_string& l, const stable_string& r) noexcept
   {
      return std::string_view{l} <=> std::string_view{r};
   }

   friend bool operator==(const stable_string& l, const std::string_view& r) noexcept
   {
      return std::string_view{l} == r;
   }

   friend auto operator<=>(const stable_string& l, const std::string_view& r) noexcept
   {
      return std::string_view{l} <=> r;
   }

private:
   std::unique_ptr<char[]> _data = nullptr;
   std::size_t _size = 0;
};

}