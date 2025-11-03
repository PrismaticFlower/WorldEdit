#pragma once

#include "../world.hpp"

namespace we::world {

/// @brief Test if two entities are "similar". (Same layer, class, etc)
/// @param a The first entity to compare.
/// @param b The second entity to compare.
/// @return True if the entities are similar, false otherwise.
[[nodiscard]] bool is_similar(const object& a, const object& b) noexcept;

/// @brief Test if two entities are "similar". (Same layer, class, etc)
/// @param a The first entity to compare.
/// @param b The second entity to compare.
/// @return True if the entities are similar, false otherwise.
[[nodiscard]] bool is_similar(const light& a, const light& b) noexcept;

/// @brief Test if two entities are "similar". (Same layer, class, etc)
/// @param a The first entity to compare.
/// @param b The second entity to compare.
/// @return True if the entities are similar, false otherwise.
[[nodiscard]] bool is_similar(const region& a, const region& b) noexcept;

/// @brief Test if two entities are "similar". (Same layer, class, etc)
/// @param a The first entity to compare.
/// @param b The second entity to compare.
/// @return True if the entities are similar, false otherwise.
[[nodiscard]] bool is_similar(const portal& a, const portal& b) noexcept;

/// @brief Test if two entities are "similar". (Same layer, class, etc)
/// @param a The first entity to compare.
/// @param b The second entity to compare.
/// @return True if the entities are similar, false otherwise.
[[nodiscard]] bool is_similar(const barrier& a, const barrier& b) noexcept;

/// @brief Test if two entities are "similar". (Same layer, class, etc)
/// @param a The first entity to compare.
/// @param b The second entity to compare.
/// @return True if the entities are similar, false otherwise.
[[nodiscard]] bool is_similar(const hintnode& a, const hintnode& b) noexcept;

/// @brief Test if two entities are "similar". (Same layer, class, etc)
/// @param a The first entity to compare.
/// @param b The second entity to compare.
/// @return True if the entities are similar, false otherwise.
[[nodiscard]] bool is_similar(const planning_connection& a,
                              const planning_connection& b) noexcept;

}