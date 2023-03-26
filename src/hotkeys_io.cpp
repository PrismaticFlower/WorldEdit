#include "hotkeys_io.hpp"
#include "io/output_file.hpp"
#include "io/read_file.hpp"
#include "utility/string_ops.hpp"

namespace we {

auto load_bindings(const std::wstring_view path)
   -> absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, hotkey_bind>>
{
   const std::filesystem::path fs_path{path};

   if (not std::filesystem::exists(fs_path)) return {};

   try {
      absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, hotkey_bind>> loaded;

      auto string = io::read_file_to_string(path);

      std::string_view current_set = "";

      for (const auto [line_number, line] : string::lines_iterator{string}) {
         if (line.starts_with("set")) {
            current_set = string::split_first_of_exclusive(line, " ")[1];
         }
         else {
            auto [name, bind_string] = string::split_first_of_exclusive(line, "=");

            key key = key::void_key;
            bool ctrl = false;
            bool shift = false;
            bool alt = false;

            if (parse_display_string(bind_string, key, ctrl, shift, alt)) {
               loaded[current_set][name] =
                  hotkey_bind{.key = key,
                              .modifiers = {.ctrl = ctrl, .shift = shift, .alt = alt}};
            }
         }
      }

      return loaded;
   }
   catch (std::exception&) {
      return {};
   }
}

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