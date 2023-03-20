#include "hotkeys_io.hpp"
#include "io/output_file.hpp"

namespace we {

auto load_bindings(const std::wstring_view path)
   -> absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, hotkey_bind>>;

void save_bindings(
   const std::wstring_view path,
   const absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, hotkey_bind>>& bindings)
{
   io::output_file file{path};

   for (const auto& [set_name, set] : bindings) {
      file.write_ln("set {}", set_name);

      for (const auto& [name, binding] : set) {
         file.write_ln("{}={}", name, get_display_string(binding));
      }
   }
}

}