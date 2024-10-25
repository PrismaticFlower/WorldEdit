#include "hotkeys.hpp"

#include <absl/container/flat_hash_map.h>

namespace we {

auto load_bindings(const std::string_view path)
   -> absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, hotkey_bind>>;

void save_bindings(
   const std::string_view path,
   const absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, hotkey_bind>>& bindings);

}