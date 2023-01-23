#pragma once

#include "types.hpp"

namespace we::world {

/// @brief Helper struct to generate unique enum types for IDs.
template<typename T>
struct id_type_holder {
   enum class entity_id : uint32 {};
};

/// @brief Alias to get a unique ID enum type for a type.
/// @tparam T The input type.
template<typename T>
using id = id_type_holder<T>::entity_id;

/// @brief Helper struct to assign UINT32_MAX to an ID.
struct max_id_t {
   template<typename T>
   consteval operator T() const noexcept
   {
      return T{0xffffffffu};
   }
};

/// @brief Can be used to assign UINT32_MAX to an ID. Useful when a sentinel value is needed.
constexpr static max_id_t max_id;

/// @brief Simple class to generate IDs in sequence (0, 1, 2, 3, etc)
/// @tparam T The input type, the class will IDs of the type id<T>.
template<typename T>
class id_generator {
public:
   using id_type = id<T>;

   /// @brief Aquire a new unique ID.
   /// @return The ID. It is only unique relative to other IDs returned from this id_generator instance.
   [[nodiscard]] auto aquire() noexcept -> id_type
   {
      return id_type{_next_id++};
   }

private:
   uint32 _next_id = 0;
};

}