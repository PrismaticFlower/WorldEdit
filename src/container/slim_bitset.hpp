#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace we::container {

/// @brief Very similiar to std::bitset but "slimmer" in API richness and (sometimes) size. For simplicity a max of 64 bits is supported.
template<std::size_t length>
class slim_bitset {
private:
   using storage_type = std::invoke_result_t<decltype([] {
      // clang-format off
      if constexpr (length <= 8) return std::uint8_t{};
      else if constexpr (length <= 16) return std::uint16_t{};
      else if constexpr (length <= 32) return std::uint32_t{};
      else if constexpr (length <= 64) return std::uint64_t{};
      // clang-format on
   })>;

public:
   static_assert(length <= 64, "slim_bitset length can only be up to 64 bits");

   struct reference_proxy {
      reference_proxy() = delete;
      reference_proxy(const reference_proxy&) = delete;
      reference_proxy(reference_proxy&&) = delete;

      constexpr auto operator=(const bool value) noexcept -> reference_proxy&
      {
         const storage_type mask = storage_type{1} << i;

         bits &= ~mask;
         bits |= (value << i);

         return *this;
      }

      constexpr operator bool() const noexcept
      {
         return ((bits >> i) & storage_type{1}) != 0;
      }

   private:
      constexpr reference_proxy(storage_type& bits, const std::size_t i) noexcept
         : bits{bits}, i{i}
      {
      }

      friend slim_bitset;

      storage_type& bits;
      const std::size_t i;
   };

   constexpr slim_bitset() = default;

   constexpr slim_bitset(const bool initial_value) noexcept
   {
      _bits = initial_value ? std::numeric_limits<storage_type>::max()
                            : storage_type{0};
   }

   [[nodiscard]] constexpr bool operator[](const std::size_t i) const noexcept
   {
      return ((_bits >> i) & storage_type{1}) != 0;
   }

   [[nodiscard]] constexpr auto operator[](const std::size_t i) noexcept -> reference_proxy
   {
      return reference_proxy{_bits, i};
   }

   [[nodiscard]] constexpr bool test(const std::size_t i) const noexcept
   {
      return ((_bits >> i) & storage_type{1}) != 0;
   }

   constexpr void set(const std::size_t i) noexcept
   {
      _bits |= (true << i);
   }

private:
   storage_type _bits = {};
};

}