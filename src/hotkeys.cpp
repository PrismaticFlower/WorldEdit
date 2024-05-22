
#include "hotkeys.hpp"
#include "container/enum_array.hpp"
#include "hotkeys_io.hpp"
#include "utility/overload.hpp"
#include "utility/string_ops.hpp"

#include <span>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <imgui.h>

namespace we {

namespace {

const std::wstring_view save_path = L".bindings";

constexpr bool is_mouse_key(const key key) noexcept
{
   switch (key) {
   case key::mouse1:
   case key::mouse2:
   case key::mouse3:
   case key::mouse4:
   case key::mouse5:
   case key::mouse_wheel_forward:
   case key::mouse_wheel_back:
      return true;
   default:
      return false;
   }
}

constexpr bool is_keyboard_key(const key key) noexcept
{
   return not is_mouse_key(key);
}

enum class sizing_desc : uint8 {
   normal,
   fn_spacing,
   backspace,
   tab,
   caps_lock,
   enter,
   lshift,
   rshift,
   ctrl_alt,
   menu,
   spacebar,
   section_split,
   numpad_0,
};

struct keyboard_entry {
   key key = key::void_key;
   sizing_desc sizing = sizing_desc::normal;
};

constexpr keyboard_entry keyboard_row0[] = {
   {key::escape},
   {key::void_key},
   {key::f1},
   {key::f2},
   {key::f3},
   {key::f4},
   {key::void_key, sizing_desc::fn_spacing},
   {key::f5},
   {key::f6},
   {key::f7},
   {key::f8},
   {key::void_key, sizing_desc::fn_spacing},
   {key::f9},
   {key::f10},
   {key::f11},
   {key::f12},
   {key::void_key, sizing_desc::section_split},
   {key::print_screen},
   {key::scroll_lock},
   {key::pause},
};

constexpr keyboard_entry keyboard_row1[] = {
   {key::grave_accent},
   {key::_1},
   {key::_2},
   {key::_3},
   {key::_4},
   {key::_5},
   {key::_6},
   {key::_7},
   {key::_8},
   {key::_9},
   {key::_0},
   {key::minus},
   {key::equal},
   {key::backspace, sizing_desc::backspace},
   {key::void_key, sizing_desc::section_split},
   {key::insert},
   {key::home},
   {key::page_up},
   {key::void_key, sizing_desc::section_split},
   {key::num_lock},
   {key::numpad_divide},
   {key::numpad_multiply},
   {key::numpad_subtract},
};

constexpr keyboard_entry keyboard_row2[] = {
   {key::tab, sizing_desc::tab},
   {key::q},
   {key::w},
   {key::e},
   {key::r},
   {key::t},
   {key::y},
   {key::u},
   {key::i},
   {key::o},
   {key::p},
   {key::left_bracket},
   {key::right_bracket},
   {key::backslash, sizing_desc::tab},
   {key::void_key, sizing_desc::section_split},
   {key::del},
   {key::end},
   {key::page_down},
   {key::void_key, sizing_desc::section_split},
   {key::numpad_7},
   {key::numpad_8},
   {key::numpad_9},
   {key::numpad_add},
};

constexpr keyboard_entry keyboard_row3[] = {
   {key::caps_lock, sizing_desc::caps_lock}, //
   {key::a},
   {key::s},
   {key::d},
   {key::f},
   {key::g},
   {key::h},
   {key::j},
   {key::k},
   {key::l},
   {key::semicolon},
   {key::apostrophe},
   {key::enter, sizing_desc::enter},
   {key::void_key, sizing_desc::section_split},
   {key::void_key},
   {key::void_key},
   {key::void_key},
   {key::void_key, sizing_desc::section_split},
   {key::numpad_4},
   {key::numpad_5},
   {key::numpad_6},
};

constexpr keyboard_entry keyboard_row4[] = {
   {key::shift, sizing_desc::lshift},
   {key::z},
   {key::x},
   {key::c},
   {key::v},
   {key::b},
   {key::n},
   {key::m},
   {key::comma},
   {key::period},
   {key::slash},
   {key::shift, sizing_desc::rshift},
   {key::void_key, sizing_desc::section_split},
   {key::void_key},
   {key::up_arrow},
   {key::void_key},
   {key::void_key, sizing_desc::section_split},
   {key::numpad_1},
   {key::numpad_2},
   {key::numpad_3},
};

constexpr keyboard_entry keyboard_row5[] = {
   {key::ctrl, sizing_desc::ctrl_alt},
   {key::alt, sizing_desc::ctrl_alt},
   {key::space, sizing_desc::spacebar},
   {key::alt, sizing_desc::ctrl_alt},
   {key::menu, sizing_desc::menu},
   {key::ctrl, sizing_desc::ctrl_alt},
   {key::void_key, sizing_desc::section_split},
   {key::left_arrow},
   {key::down_arrow},
   {key::right_arrow},
   {key::void_key, sizing_desc::section_split},
   {key::numpad_0, sizing_desc::numpad_0},
   {key::numpad_decimal},
};

constexpr std::array<std::span<const keyboard_entry>, 6> keyboard_rows =
   {keyboard_row0, keyboard_row1, keyboard_row2,
    keyboard_row3, keyboard_row4, keyboard_row5};

constexpr key mouse_row_0[] = {key::void_key, key::mouse_wheel_forward,
                               key::void_key, key::mouse4};
constexpr key mouse_row_1[] = {key::mouse1, key::mouse3, key::mouse2};
constexpr key mouse_row_2[] = {key::void_key, key::mouse_wheel_back,
                               key::void_key, key::mouse5};

constexpr std::array<std::span<const key>, 3> mouse_rows = {mouse_row_0, mouse_row_1,
                                                            mouse_row_2};

constexpr std::array<hotkey_modifiers, 8> modifier_variations = {
   hotkey_modifiers{.ctrl = false, .shift = false, .alt = false},
   hotkey_modifiers{.ctrl = true, .shift = false, .alt = false},
   hotkey_modifiers{.ctrl = false, .shift = true, .alt = false},
   hotkey_modifiers{.ctrl = true, .shift = true, .alt = false},
   hotkey_modifiers{.ctrl = false, .shift = false, .alt = true},
   hotkey_modifiers{.ctrl = true, .shift = false, .alt = true},
   hotkey_modifiers{.ctrl = false, .shift = true, .alt = true},
   hotkey_modifiers{.ctrl = true, .shift = true, .alt = true},
};

}

struct hotkeys::impl {
   impl(commands& commands, output_stream& error_output_stream) noexcept
      : _commands{commands},
        _error_output_stream{error_output_stream},
        _saved_bindings{load_bindings(save_path)}
   {
      _hotkey_sets.reserve(16);
      _key_events.reserve(256);
   }

   void add_set(hotkey_set_desc desc);

   void notify_key_down(const key key) noexcept;

   void notify_key_up(const key key) noexcept;

   void update(const bool imgui_has_mouse, const bool imgui_has_keyboard) noexcept;

   void clear_state() noexcept;

   auto query_binding(std::string_view set_name, std::string_view name) const noexcept

      -> std::optional<hotkey_bind>;

   void show_imgui(bool& window_open, const scale_factor display_scale) noexcept;

private:
   enum class key_state : bool { up, down };

   void validate_command(const std::string_view command);

   void release_all_toggles() noexcept;

   void release_stale_toggles(const bool imgui_has_mouse,
                              const bool imgui_has_keyboard) noexcept;

   void release_toggles_using(const key key) noexcept;

   void process_new_key_state(const key key, const key_state new_state,
                              const bool imgui_has_mouse,
                              const bool imgui_has_keyboard);

   bool is_key_down(const key key) const noexcept;

   void try_execute_command(const std::string_view command) const noexcept;

   commands& _commands;
   output_stream& _error_output_stream;

   struct key_event {
      key key;
      key_state new_state;
   };

   std::vector<key_event> _key_events;

   struct hotkey {
      std::string command;

      bool toggle = false;
      bool toggle_active = false;
      bool ignore_imgui_focus = false;

      std::string name;

      bool operator==(const hotkey&) const noexcept = default;
   };

   struct hotkey_set {
      std::string name;
      std::move_only_function<bool()> activated_predicate;
      absl::flat_hash_map<hotkey_bind, int32> bindings;
      absl::flat_hash_map<std::string, hotkey_bind> query_bindings;
      std::vector<hotkey> hotkeys;

      std::string description;
      bool hidden = false;
   };

   std::vector<hotkey_set> _hotkey_sets;

   struct active_toggle {
      std::ptrdiff_t set_index;
      hotkey_bind bind;

      template<typename H>
      friend H AbslHashValue(H h, const active_toggle& active_toggle)
      {
         return H::combine(std::move(h), active_toggle.set_index, active_toggle.bind);
      }

      constexpr bool operator==(const active_toggle&) const noexcept = default;
   };

   using activated_hotkey = active_toggle;

   absl::flat_hash_set<active_toggle> _active_toggles;

   bool _user_inputting_new_binding = false;
   bool _new_binding_popup_open = false;
   bool _swap_binding_popup_open = false;
   std::optional<key_event> _user_editing_last_key_event;
   std::ptrdiff_t _user_editing_bind_set = 0;
   std::optional<hotkey_bind> _user_editing_bind;
   int32 _user_editing_hotkey_index = 0;
   key _user_swapping_key = key::void_key;

   container::enum_array<key_state, key> _keys{};

   absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, hotkey_bind>> _saved_bindings;
};

void hotkeys::impl::add_set(hotkey_set_desc desc)
{
   absl::flat_hash_set<std::string> hotkey_set;
   absl::flat_hash_map<hotkey_bind, int32> bindings;
   absl::flat_hash_map<std::string, hotkey_bind> query_bindings;
   absl::flat_hash_set<hotkey_bind> saved_used_bindings;
   std::vector<hotkey> hotkeys;

   hotkey_set.reserve(desc.default_hotkeys.size());
   bindings.reserve(desc.default_hotkeys.size());
   query_bindings.reserve(desc.default_hotkeys.size());
   hotkeys.reserve(desc.default_hotkeys.size());

   if (auto saved_bindings_it = _saved_bindings.find(desc.name);
       saved_bindings_it != _saved_bindings.end()) {
      const auto& [set_name, saved] = *saved_bindings_it;

      saved_used_bindings.reserve(saved.size());

      for (const auto& default_hotkey : desc.default_hotkeys) {
         if (auto it = saved.find(default_hotkey.name); it != saved.end()) {
            const auto& [name, binding] = *it;

            saved_used_bindings.insert(binding);
         }
      }
   }

   int32 hotkey_index = 0;

   for (auto& default_hotkey : desc.default_hotkeys) {
      validate_command(default_hotkey.command);

      if (not hotkey_set.emplace(default_hotkey.name).second) {
         std::terminate(); // Duplicate hotkey name!
      }

      const bool has_saved_binding =
         _saved_bindings[desc.name].contains(default_hotkey.name);

      const hotkey_bind binding =
         has_saved_binding ? _saved_bindings[desc.name].at(default_hotkey.name)
                           : default_hotkey.binding;

      const bool binding_free_for_use = not saved_used_bindings.contains(binding);

      if (has_saved_binding or binding_free_for_use) {
         bindings[binding] = hotkey_index;
         query_bindings[default_hotkey.name] = binding;
      }

      hotkeys.push_back(hotkey{.command = std::string{default_hotkey.command},
                               .toggle = default_hotkey.bind_config.toggle,
                               .ignore_imgui_focus = default_hotkey.bind_config.ignore_imgui_focus,
                               .name = std::string{default_hotkey.name}});

      hotkey_index += 1;
   }

   _hotkey_sets.emplace_back(desc.name, std::move(desc.activated), std::move(bindings),
                             std::move(query_bindings), std::move(hotkeys),
                             std::move(desc.description), desc.hidden);
}

void hotkeys::impl::notify_key_down(const key key) noexcept
{
   _key_events.push_back({.key = key, .new_state = key_state::down});
}

void hotkeys::impl::notify_key_up(const key key) noexcept
{
   _key_events.push_back({.key = key, .new_state = key_state::up});
}

void hotkeys::impl::clear_state() noexcept
{
   release_all_toggles();

   _keys = {};
}

void hotkeys::impl::update(const bool imgui_has_mouse, const bool imgui_has_keyboard) noexcept
{
   release_stale_toggles(imgui_has_mouse, imgui_has_keyboard);

   for (const auto& event : _key_events) {
      process_new_key_state(event.key, event.new_state, imgui_has_mouse,
                            imgui_has_keyboard);
   }

   if (_user_inputting_new_binding and not _key_events.empty()) {
      _user_editing_last_key_event = _key_events.back();
   }

   _key_events.clear();
}

auto hotkeys::impl::query_binding(std::string_view set_name,
                                  std::string_view command) const noexcept
   -> std::optional<hotkey_bind>
{
   for (const auto& set : _hotkey_sets) {
      if (set.name != set_name) continue;

      if (auto it = set.query_bindings.find(command); it != set.query_bindings.end()) {
         const auto& [hotkey_command, hotkey_bind] = *it;

         if (hotkey_command == command) return hotkey_bind;
      }
   }

   return std::nullopt;
}

void hotkeys::impl::process_new_key_state(const key key, const key_state new_state,
                                          const bool imgui_has_mouse,
                                          const bool imgui_has_keyboard)
{
   const key_state old_state = std::exchange(_keys[key], new_state);

   if (old_state == new_state) return;

   release_toggles_using(key);

   if (new_state == key_state::up) return;

   for (std::ptrdiff_t i = std::ssize(_hotkey_sets) - 1; i >= 0; --i) {
      hotkey_set& set = _hotkey_sets[i];

      if (not set.activated_predicate()) continue;

      const auto handle_hotkey = [&](const hotkey_bind bind, hotkey& hotkey) {
         if (imgui_has_mouse and is_mouse_key(bind.key) and
             not hotkey.ignore_imgui_focus) {
            return;
         }

         if (imgui_has_keyboard and is_keyboard_key(bind.key) and
             not hotkey.ignore_imgui_focus) {
            return;
         }

         if (new_state == key_state::down) {
            try_execute_command(hotkey.command);

            if (hotkey.toggle) {
               hotkey.toggle_active = true;

               _active_toggles.emplace(i, bind);
            }
         }
      };

      absl::flat_hash_map<hotkey_bind, int32>& bindings = set.bindings;

      auto bind_hotkey =
         bindings.find({.key = key,
                        .modifiers = {.ctrl = is_key_down(key::ctrl),
                                      .shift = is_key_down(key::shift),
                                      .alt = is_key_down(key::alt)}});

      if (bind_hotkey == bindings.end()) continue;

      handle_hotkey(bind_hotkey->first, set.hotkeys[bind_hotkey->second]);
      break;
   }
}

void hotkeys::impl::validate_command(const std::string_view command)
{
   auto [command_name, command_args] = string::split_first_of_exclusive(command, " ");

   if (not _commands.has_command(command_name)) {
      throw unknown_command{fmt::format("Unknown command '{}'!", command_name)};
   }
}

void hotkeys::impl::release_all_toggles() noexcept
{
   for (auto& active : _active_toggles) {
      hotkey_set& set = _hotkey_sets[active.set_index];
      hotkey& hotkey = set.hotkeys[set.bindings[active.bind]];

      if (std::exchange(hotkey.toggle_active, false)) {
         _commands.execute(hotkey.command);
      }
   }

   _active_toggles.clear();
}

void hotkeys::impl::release_stale_toggles(const bool imgui_has_mouse,
                                          const bool imgui_has_keyboard) noexcept
{
   for (auto it = _active_toggles.begin(); it != _active_toggles.end();) {
      auto active = it++;

      hotkey_set& set = _hotkey_sets[active->set_index];
      hotkey& hotkey = set.hotkeys[set.bindings[active->bind]];

      bool release_toggle = false;

      if (imgui_has_mouse and not hotkey.ignore_imgui_focus and
          is_mouse_key(active->bind.key)) {
         release_toggle = true;
      }
      else if (imgui_has_keyboard and not hotkey.ignore_imgui_focus and
               is_keyboard_key(active->bind.key)) {
         release_toggle = true;
      }
      else if (not set.activated_predicate()) {
         release_toggle = true;
      }

      if (not release_toggle) continue;

      if (std::exchange(hotkey.toggle_active, false)) {
         _commands.execute(hotkey.command);
         _active_toggles.erase(active);
      }
   }
}

void hotkeys::impl::release_toggles_using(const key key) noexcept
{
   const bool is_ctrl = key == we::key::ctrl;
   const bool is_shift = key == we::key::shift;
   const bool is_alt = key == we::key::alt;

   for (auto it = _active_toggles.begin(); it != _active_toggles.end();) {
      auto active_toggle = it++;

      const bool release = active_toggle->bind.key == key or
                           (active_toggle->bind.modifiers.ctrl and is_ctrl) or
                           (active_toggle->bind.modifiers.shift and is_shift) or
                           (active_toggle->bind.modifiers.alt and is_alt);

      if (not release) continue;

      hotkey_set& set = _hotkey_sets[active_toggle->set_index];
      hotkey& hotkey = set.hotkeys[set.bindings[active_toggle->bind]];

      if (std::exchange(hotkey.toggle_active, false)) {
         _commands.execute(hotkey.command);
      }

      _active_toggles.erase(active_toggle);
   }
}

bool hotkeys::impl::is_key_down(const key key) const noexcept
{
   return _keys[key] == key_state::down;
}

void hotkeys::impl::try_execute_command(const std::string_view command) const noexcept
{
   try {
      _commands.execute(command);
   }
   catch (invalid_command_argument& e) {
      _error_output_stream
         .write("Failed to execute command '{}'\n   Error: {}\n", command, e.what());
   }
}

void hotkeys::impl::show_imgui(bool& window_open, const scale_factor display_scale) noexcept
{
   ImGui::SetNextWindowSize({960.0f * display_scale, 720.0f * display_scale},
                            ImGuiCond_Once);
   const ImVec2 imgui_center = ImGui::GetMainViewport()->GetCenter();
   ImGui::SetNextWindowPos(imgui_center, ImGuiCond_FirstUseEver, {0.5f, 0.5f});

   if (ImGui::Begin("Hotkeys Editor", &window_open)) {
      bool open_swap_bindings_popup = false;
      bool open_new_binding_popup = false;

      if (ImGui::CollapsingHeader("Keys Overview", ImGuiTreeNodeFlags_DefaultOpen)) {
         ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

         ImGui::SeparatorText("Keyboard");

         ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});

         for (int i = 0; i < keyboard_rows.size(); ++i) {
            if (i == 1) ImGui::Dummy({1.0f, 1.0f});

            const float key_height = 32.0f * display_scale;

            for (const auto& [key, sizing] : keyboard_rows[i]) {
               float width = 32.0f * display_scale;

               if (sizing == sizing_desc::fn_spacing) {
                  width = width / 2.0f - ImGui::GetStyle().ItemSpacing.x * 0.5f;
               }
               else if (sizing == sizing_desc::backspace) {
                  width = width * 2.0f + ImGui::GetStyle().ItemSpacing.x;
               }
               else if (sizing == sizing_desc::tab) {
                  width = width + (width / 2.0f) +
                          ImGui::GetStyle().ItemSpacing.x * 0.5f;
               }
               else if (sizing == sizing_desc::caps_lock) {
                  width = width + (width / 2.0f) +
                          ImGui::GetStyle().ItemSpacing.x * 1.5f;
               }
               else if (sizing == sizing_desc::enter) {
                  width = width * 2.5f + ImGui::GetStyle().ItemSpacing.x * 0.5f;
               }
               else if (sizing == sizing_desc::lshift) {
                  width = width * 2.0f + ImGui::GetStyle().ItemSpacing.x * 2.0f;
               }
               else if (sizing == sizing_desc::rshift) {
                  width = width * 3.0f + ImGui::GetStyle().ItemSpacing.x * 1.0f;
               }
               else if (sizing == sizing_desc::ctrl_alt) {
                  width = width * 2.0f;
               }
               else if (sizing == sizing_desc::menu) {
                  width = width * 1.5f;
               }
               else if (sizing == sizing_desc::spacebar) {

                  width = width * 5.5f + ImGui::GetStyle().ItemSpacing.x * 9.0f;
               }
               else if (sizing == sizing_desc::section_split) {
                  width = width / 4.0f;
               }
               else if (sizing == sizing_desc::numpad_0) {
                  width = width * 2.0f + ImGui::GetStyle().ItemSpacing.x;
               }

               width = std::floor(width);

               if (key == key::void_key) {
                  ImGui::Dummy({width, key_height});
               }
               else {
                  const bool has_bindings = [&] {
                     for (const hotkey_set& set : _hotkey_sets) {
                        for (const hotkey_modifiers& modifiers : modifier_variations) {
                           if (set.hidden) continue;

                           if (set.bindings.contains(hotkey_bind{key, modifiers})) {
                              return true;
                           }
                        }
                     }

                     return false;
                  }();

                  if (not has_bindings) ImGui::BeginDisabled();

                  ImGui::PushID(static_cast<int>(key));

                  if (ImGui::Button(get_key_name(key), {width, key_height})) {
                     _user_swapping_key = key;
                     open_swap_bindings_popup = true;
                  }

                  ImGui::PopID();

                  if (not has_bindings) ImGui::EndDisabled();

                  if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                     ImGui::SeparatorText(get_display_string(key, false, false, false));

                     for (const hotkey_set& set : _hotkey_sets) {
                        if (set.hidden) continue;

                        for (const hotkey_modifiers& modifiers : modifier_variations) {
                           if (auto it = set.bindings.find(hotkey_bind{key, modifiers});
                               it != set.bindings.end()) {
                              const auto& [bind, hotkey_index] = *it;

                              ImGui::Text("%s: %s", set.name.c_str(),
                                          set.hotkeys[hotkey_index].name.c_str());
                              ImGui::BulletText(get_display_string(bind));
                           }
                        }
                     }

                     ImGui::EndTooltip();
                  }
               }

               ImGui::SameLine();
            }

            ImGui::NewLine();
         }

         ImGui::PopStyleVar();

         ImGui::SeparatorText("Mouse");

         for (const auto& row : mouse_rows) {
            const float key_width = 128.0f * display_scale;
            const float key_height = 32.0f * display_scale;

            for (const key key : row) {

               if (key == key::void_key) {
                  ImGui::Dummy({key_width, key_height});
               }
               else {
                  const bool has_bindings = [&] {
                     for (const hotkey_set& set : _hotkey_sets) {
                        for (const hotkey_modifiers& modifiers : modifier_variations) {
                           if (set.hidden) continue;

                           if (set.bindings.size() > modifier_variations.size()) {
                              if (set.bindings.contains(hotkey_bind{key, modifiers})) {
                                 return true;
                              }
                           }
                        }
                     }

                     return false;
                  }();

                  if (not has_bindings) ImGui::BeginDisabled();

                  ImGui::PushID(static_cast<int>(key));

                  if (ImGui::Button(get_key_name(key), {key_width, key_height})) {
                     _user_swapping_key = key;
                     open_swap_bindings_popup = true;
                  }

                  ImGui::PopID();

                  if (not has_bindings) ImGui::EndDisabled();

                  if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                     ImGui::SeparatorText(get_display_string(key, false, false, false));

                     for (const hotkey_set& set : _hotkey_sets) {
                        if (set.hidden) continue;

                        for (const hotkey_modifiers& modifiers : modifier_variations) {
                           if (auto it = set.bindings.find(hotkey_bind{key, modifiers});
                               it != set.bindings.end()) {
                              const auto& [bind, hotkey_index] = *it;

                              ImGui::Text("%s: %s", set.name.c_str(),
                                          set.hotkeys[hotkey_index].name.c_str());
                              ImGui::BulletText(get_display_string(bind));
                           }
                        }
                     }

                     ImGui::EndTooltip();
                  }
               }

               ImGui::SameLine();
            }

            ImGui::NewLine();
         }

         ImGui::SeparatorText("Key Swap");

         ImGui::Text("You can swap the bindings of two keys by clicking one.");

         ImGui::PopStyleVar();
      }

      if (ImGui::BeginChild("Set List", {200.0f * display_scale, 0.0f},
                            ImGuiChildFlags_ResizeX)) {
         ImGui::SeparatorText("Sets");

         for (int set_index = 0; set_index < std::ssize(_hotkey_sets); ++set_index) {
            hotkey_set& set = _hotkey_sets[set_index];

            if (set.hidden) continue;

            if (ImGui::Selectable(set.name.c_str(), _user_editing_bind_set == set_index)) {
               _user_editing_bind_set = set_index;
            }

            if (ImGui::BeginItemTooltip()) {
               ImGui::TextUnformatted(set.description.data(),
                                      set.description.data() + set.description.size());

               ImGui::EndTooltip();
            }
         }
      }

      ImGui::EndChild();

      ImGui::SameLine();

      if (ImGui::BeginChild("Set View")) {
         hotkey_set& set = _hotkey_sets[_user_editing_bind_set];

         ImGui::SeparatorText("Description");

         ImGui::TextUnformatted(set.description.data(),
                                set.description.data() + set.description.size());

         ImGui::SeparatorText("Bindings");

         ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});

         for (int hotkey_index = 0; hotkey_index < std::ssize(set.hotkeys);
              ++hotkey_index) {
            ImGui::PushID(hotkey_index);

            hotkey& hotkey = set.hotkeys[hotkey_index];

            ImGui::LabelText("", hotkey.name.c_str());
            ImGui::SameLine();

            if (auto it = set.query_bindings.find(hotkey.name);
                it != set.query_bindings.end()) {
               const auto& [hotkey_command, hotkey_bind] = *it;

               if (ImGui::Selectable(get_display_string(hotkey_bind))) {
                  open_new_binding_popup = true;
                  _user_editing_bind = hotkey_bind;
                  _user_editing_hotkey_index = hotkey_index;
               }
            }
            else {
               if (ImGui::Selectable("<unbound>")) {
                  open_new_binding_popup = true;
                  _user_editing_bind = std::nullopt;
                  _user_editing_hotkey_index = hotkey_index;
               }
            }

            ImGui::PopID();
         }

         ImGui::PopStyleVar();
      }

      ImGui::EndChild();

      if (open_new_binding_popup) {
         _new_binding_popup_open = true;

         ImGui::OpenPopup("Press New Binding");
      }

      ImGui::SetNextWindowPos(imgui_center, ImGuiCond_Appearing, {0.5f, 0.5f});

      if (ImGui::BeginPopupModal("Press New Binding", &_new_binding_popup_open,
                                 ImGuiWindowFlags_AlwaysAutoResize)) {

         ImGui::Text("Press Escape to cancel and go back.");

         const std::optional<key_event> last_key_event =
            std::exchange(_user_editing_last_key_event, std::nullopt);

         if (last_key_event) {
            if (last_key_event->key == key::escape and
                last_key_event->new_state == key_state::up) {
               _new_binding_popup_open = false;

               ImGui::CloseCurrentPopup();
            }
            else if (last_key_event->new_state == key_state::up and
                     last_key_event->key != key::ctrl and
                     last_key_event->key != key::shift and
                     last_key_event->key != key::alt) {
               hotkey_set& set = _hotkey_sets[_user_editing_bind_set];

               if (_user_editing_bind) {
                  set.bindings.erase(*_user_editing_bind);
               }

               const hotkey_bind new_bind{.key = last_key_event->key,
                                          .modifiers = {.ctrl = is_key_down(key::ctrl),
                                                        .shift = is_key_down(key::shift),
                                                        .alt = is_key_down(key::alt)}};

               if (set.bindings.contains(new_bind)) {
                  const std::string_view name_to_remove =
                     set.hotkeys[set.bindings.at(new_bind)].name;

                  set.query_bindings.erase(name_to_remove);

                  _saved_bindings[set.name].erase(name_to_remove);
               }

               hotkey& hotkey = set.hotkeys[_user_editing_hotkey_index];

               set.bindings[new_bind] = _user_editing_hotkey_index;
               set.query_bindings[hotkey.name] = new_bind;

               _new_binding_popup_open = false;

               ImGui::CloseCurrentPopup();

               _saved_bindings[set.name][hotkey.name] = new_bind;

               save_bindings(save_path, _saved_bindings);
            }
         }

         ImGui::EndPopup();
      }

      if (open_swap_bindings_popup) {
         _swap_binding_popup_open = true;

         ImGui::OpenPopup("Swap Bindings");
      }

      ImGui::SetNextWindowPos(imgui_center, ImGuiCond_Appearing, {0.5f, 0.5f});

      if (ImGui::BeginPopupModal("Swap Bindings", &_swap_binding_popup_open,
                                 ImGuiWindowFlags_AlwaysAutoResize)) {
         ImGui::Text("Press Escape to cancel and go back.");

         const std::optional<key_event> last_key_event =
            std::exchange(_user_editing_last_key_event, std::nullopt);

         if (last_key_event) {
            if (last_key_event->key == key::escape and
                last_key_event->new_state == key_state::up) {
               _swap_binding_popup_open = false;

               ImGui::CloseCurrentPopup();
            }
            else if (last_key_event->new_state == key_state::up and
                     last_key_event->key != key::ctrl and
                     last_key_event->key != key::shift and
                     last_key_event->key != key::alt) {
               absl::flat_hash_map<hotkey_bind, int32> bindings;
               bindings.reserve(32);

               const key new_key = last_key_event->key;
               const key old_key = _user_swapping_key;

               for (hotkey_set& set : _hotkey_sets) {
                  bindings.clear();

                  for (auto it = set.bindings.begin(); it != set.bindings.end();) {
                     auto binding_it = it++;
                     auto& [bind, hotkey_index] = *binding_it;
                     hotkey& hotkey = set.hotkeys[hotkey_index];

                     if (bind.key == new_key or bind.key == old_key) {
                        set.query_bindings.erase(hotkey.name);
                        _saved_bindings[set.name].erase(hotkey.name);

                        bindings.emplace(hotkey_bind{.key = bind.key == new_key ? old_key : new_key,
                                                     .modifiers = bind.modifiers},
                                         hotkey_index);
                        set.bindings.erase(binding_it);
                     }
                  }

                  for (const auto& [bind, hotkey_index] : bindings) {
                     hotkey& hotkey = set.hotkeys[hotkey_index];

                     if (set.bindings.try_emplace(bind, hotkey_index).second) {
                        set.query_bindings[hotkey.name] = bind;
                        _saved_bindings[set.name][hotkey.name] = bind;
                     }
                  }
               }

               save_bindings(save_path, _saved_bindings);

               _swap_binding_popup_open = false;

               ImGui::CloseCurrentPopup();
            }
         }

         ImGui::EndPopup();
      }

      _user_inputting_new_binding = _new_binding_popup_open or _swap_binding_popup_open;
   }

   ImGui::End();

   if (not window_open) {
      _user_inputting_new_binding = false;
   }
}

auto get_display_string(const std::optional<hotkey_bind> binding) -> const char*
{
   if (not binding) return "<UNBOUND>";

   return get_display_string(binding->key, binding->modifiers.ctrl,
                             binding->modifiers.shift, binding->modifiers.alt);
}

hotkeys::hotkeys(commands& commands, output_stream& error_output_stream) noexcept
   : _impl{commands, error_output_stream}
{
}

hotkeys::~hotkeys() = default;

void hotkeys::add_set(hotkey_set_desc desc)
{
   return _impl->add_set(std::move(desc));
}

void hotkeys::notify_key_down(const key key) noexcept
{
   return _impl->notify_key_down(key);
}

void hotkeys::notify_key_up(const key key) noexcept
{
   return _impl->notify_key_up(key);
}

void hotkeys::update(const bool imgui_has_mouse, const bool imgui_has_keyboard) noexcept
{
   return _impl->update(imgui_has_mouse, imgui_has_keyboard);
}

void hotkeys::clear_state() noexcept
{
   return _impl->clear_state();
}

auto hotkeys::query_binding(std::string_view set_name, std::string_view name) const noexcept
   -> std::optional<hotkey_bind>
{
   return _impl->query_binding(set_name, name);
}

void hotkeys::show_imgui(bool& window_open, const scale_factor display_scale) noexcept
{
   return _impl->show_imgui(window_open, display_scale);
}
}