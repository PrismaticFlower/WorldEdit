
#include "hotkeys.hpp"
#include "hotkeys_io.hpp"
#include "utility/overload.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>
#include <span>

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

hotkeys::hotkeys(commands& commands, output_stream& error_output_stream) noexcept
   : _commands{commands},
     _error_output_stream{error_output_stream},
     _saved_bindings{load_bindings(save_path)},
     _saved_used_bindings{[this] {
        absl::flat_hash_map<std::string, absl::flat_hash_set<hotkey_bind>> saved_used_bindings;
        saved_used_bindings.reserve(_saved_bindings.size());

        for (const auto& [set_name, saved] : _saved_bindings) {
           absl::flat_hash_set<hotkey_bind>& used = saved_used_bindings[set_name];

           used.reserve(saved.size());

           for (const auto& [name, binding] : saved) {
              used.insert(binding);
           }
        }

        return saved_used_bindings;
     }()}
{
   _hotkey_sets.reserve(16);
   _key_events.reserve(256);
}

void hotkeys::add_set(hotkey_set_desc desc)
{
   absl::flat_hash_set<std::string> hotkey_set;
   absl::flat_hash_map<hotkey_bind, hotkey> bindings;
   absl::flat_hash_map<std::string, hotkey_bind> query_bindings;
   std::vector<hotkey> unbound_hotkeys;

   hotkey_set.reserve(desc.default_hotkeys.size());
   bindings.reserve(desc.default_hotkeys.size());
   query_bindings.reserve(desc.default_hotkeys.size());
   unbound_hotkeys.reserve(desc.default_hotkeys.size());

   for (auto& default_hotkey : desc.default_hotkeys) {
      validate_command(default_hotkey.command);

      if (not hotkey_set.emplace(default_hotkey.name).second) {
         std::terminate(); // Duplicate hotkey name!
      }

      const bool has_saved_binding =
         _saved_bindings[desc.name].contains(default_hotkey.name);
      const bool has_saved_used_set = _saved_used_bindings.contains(desc.name);

      const hotkey_bind binding =
         has_saved_binding ? _saved_bindings[desc.name].at(default_hotkey.name)
                           : default_hotkey.binding;

      const bool binding_free_for_use =
         not has_saved_used_set or
         not _saved_used_bindings.at(desc.name).contains(binding);

      if (has_saved_binding or binding_free_for_use) {
         bindings[binding] =
            hotkey{.command = std::string{default_hotkey.command},
                   .toggle = default_hotkey.bind_config.toggle,
                   .ignore_imgui_focus = default_hotkey.bind_config.ignore_imgui_focus,
                   .name = std::string{default_hotkey.name}};
         query_bindings[default_hotkey.name] = binding;
      }
      else {
         unbound_hotkeys.push_back(
            hotkey{.command = std::string{default_hotkey.command},
                   .toggle = default_hotkey.bind_config.toggle,
                   .ignore_imgui_focus = default_hotkey.bind_config.ignore_imgui_focus,
                   .name = std::string{default_hotkey.name}});
      }
   }

   _hotkey_sets.emplace_back(desc.name, std::move(desc.activated),
                             std::move(bindings), std::move(query_bindings),
                             std::move(unbound_hotkeys),
                             std::move(desc.description), desc.hidden);
}

void hotkeys::notify_key_down(const key key) noexcept
{
   _key_events.push_back({.key = key, .new_state = key_state::down});
}

void hotkeys::notify_key_up(const key key) noexcept
{
   _key_events.push_back({.key = key, .new_state = key_state::up});
}

void hotkeys::clear_state() noexcept
{
   release_all_toggles();

   _keys = {};
}

void hotkeys::update(const bool imgui_has_mouse, const bool imgui_has_keyboard) noexcept
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

auto hotkeys::query_binding(std::string_view set_name, std::string_view command) const noexcept
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

void hotkeys::process_new_key_state(const key key, const key_state new_state,
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

      absl::flat_hash_map<hotkey_bind, hotkey>& bindings = set.bindings;

      auto bind_hotkey =
         bindings.find({.key = key,
                        .modifiers = {.ctrl = is_key_down(key::ctrl),
                                      .shift = is_key_down(key::shift),
                                      .alt = is_key_down(key::alt)}});

      if (bind_hotkey == bindings.end()) continue;

      handle_hotkey(bind_hotkey->first, bind_hotkey->second);
      break;
   }
}

void hotkeys::validate_command(const std::string_view command)
{
   auto [command_name, command_args] = string::split_first_of_exclusive(command, " ");

   if (not _commands.has_command(command_name)) {
      throw unknown_command{fmt::format("Unknown command '{}'!", command_name)};
   }
}

void hotkeys::release_all_toggles() noexcept
{
   for (auto& active : _active_toggles) {
      hotkey& hotkey = _hotkey_sets[active.set_index].bindings[active.bind];

      if (std::exchange(hotkey.toggle_active, false)) {
         _commands.execute(hotkey.command);
      }
   }

   _active_toggles.clear();
}

void hotkeys::release_stale_toggles(const bool imgui_has_mouse,
                                    const bool imgui_has_keyboard) noexcept
{
   for (auto it = _active_toggles.begin(); it != _active_toggles.end();) {
      auto active = it++;

      hotkey_set& set = _hotkey_sets[active->set_index];
      hotkey& hotkey = set.bindings[active->bind];

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

void hotkeys::release_toggles_using(const key key) noexcept
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

      hotkey& hotkey =
         _hotkey_sets[active_toggle->set_index].bindings[active_toggle->bind];

      if (std::exchange(hotkey.toggle_active, false)) {
         _commands.execute(hotkey.command);
      }

      _active_toggles.erase(active_toggle);
   }
}

bool hotkeys::is_key_down(const key key) const noexcept
{
   return _keys[key] == key_state::down;
}

void hotkeys::try_execute_command(const std::string_view command) const noexcept
{
   try {
      _commands.execute(command);
   }
   catch (invalid_command_argument& e) {
      _error_output_stream
         .write("Failed to execute command '{}'\n   Error: {}\n", command, e.what());
   }
}

void hotkeys::show_imgui(bool& window_open, const scale_factor display_scale) noexcept
{
   ImGui::SetNextWindowSize({960.0f * display_scale, 540.0f * display_scale},
                            ImGuiCond_Once);
   const ImVec2 imgui_center = ImGui::GetMainViewport()->GetCenter();
   ImGui::SetNextWindowPos(imgui_center, ImGuiCond_FirstUseEver, {0.5f, 0.5f});

   if (ImGui::Begin("Hotkeys Editor", &window_open)) {
      if (ImGui::CollapsingHeader("Keys Overview", ImGuiTreeNodeFlags_DefaultOpen)) {
         ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

         bool open_swap_bindings_popup = false;

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
                              const auto& [bind, hotkey] = *it;

                              ImGui::Text("%s: %s", set.name.c_str(),
                                          hotkey.name.c_str());
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
                              const auto& [bind, hotkey] = *it;

                              ImGui::Text("%s: %s", set.name.c_str(),
                                          hotkey.name.c_str());
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

         if (open_swap_bindings_popup) {
            _user_inputting_new_binding = true;

            ImGui::OpenPopup("Swap Bindings");
         }

         ImGui::PopStyleVar();
      }

      bool open_new_binding_popup = false;

      for (int set_index = 0; set_index < std::ssize(_hotkey_sets); ++set_index) {
         hotkey_set& set = _hotkey_sets[set_index];

         if (set.hidden) continue;

         ImGui::PushID(set_index);

         if (not ImGui::CollapsingHeader(set.name.c_str())) {
            ImGui::PopID();
            continue;
         }

         ImGui::SeparatorText("Description");

         ImGui::TextUnformatted(set.description.data(),
                                set.description.data() + set.description.size());

         ImGui::SeparatorText("Bindings");

         ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});

         fill_sorted_info(set, _sorted_hotkey_set);

         for (int bind_index = 0; bind_index < std::ssize(_sorted_hotkey_set);
              ++bind_index) {
            ImGui::PushID(bind_index);

            std::visit(overload{[&](const std::pair<const hotkey_bind, hotkey>* hotkey_bind_hotkey) {
                                   const auto& [hotkey_bind, hotkey] =
                                      *hotkey_bind_hotkey;

                                   ImGui::LabelText("", hotkey.name.c_str());
                                   ImGui::SameLine();

                                   if (ImGui::Selectable(get_display_string(hotkey_bind))) {
                                      open_new_binding_popup = true;
                                      _user_editing_bind_set = set_index;
                                      _user_editing_bind = hotkey_bind;
                                      _user_editing_hotkey = hotkey;
                                   }
                                },
                                [&](const hotkey* hotkey) {
                                   ImGui::LabelText("", hotkey->name.c_str());
                                   ImGui::SameLine();

                                   if (ImGui::Selectable("<unbound>")) {
                                      open_new_binding_popup = true;
                                      _user_editing_bind_set = set_index;
                                      _user_editing_bind = std::nullopt;
                                      _user_editing_hotkey = *hotkey;
                                   }
                                }},
                       _sorted_hotkey_set[bind_index]);

            ImGui::PopID();
         }

         ImGui::PopStyleVar();
         ImGui::PopID();
      }

      if (open_new_binding_popup) {
         _user_inputting_new_binding = true;

         ImGui::OpenPopup("Press New Binding");
      }

      ImGui::SetNextWindowPos(imgui_center, ImGuiCond_Appearing, {0.5f, 0.5f});

      if (ImGui::BeginPopupModal("Press New Binding", &_user_inputting_new_binding,
                                 ImGuiWindowFlags_AlwaysAutoResize)) {
         ImGui::Text("Press Escape to cancel and go back.");

         const std::optional<key_event> last_key_event =
            std::exchange(_user_editing_last_key_event, std::nullopt);

         if (last_key_event) {
            if (last_key_event->key == key::escape and
                last_key_event->new_state == key_state::up) {
               _user_inputting_new_binding = false;

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
                  set.unbound_hotkeys.push_back(set.bindings.at(new_bind));
                  set.query_bindings.erase(set.unbound_hotkeys.back().name);

                  _saved_bindings[set.name].erase(set.unbound_hotkeys.back().name);
               }

               set.bindings[new_bind] = _user_editing_hotkey;
               set.query_bindings[_user_editing_hotkey.name] = new_bind;

               std::erase_if(set.unbound_hotkeys, [&](const hotkey& hotkey) {
                  return hotkey == _user_editing_hotkey;
               });

               _user_inputting_new_binding = false;

               ImGui::CloseCurrentPopup();

               _saved_bindings[set.name][_user_editing_hotkey.name] = new_bind;

               save_bindings(save_path, _saved_bindings);
            }
         }

         ImGui::EndPopup();
      }

      ImGui::SetNextWindowPos(imgui_center, ImGuiCond_Appearing, {0.5f, 0.5f});

      if (ImGui::BeginPopupModal("Swap Bindings", &_user_inputting_new_binding,
                                 ImGuiWindowFlags_AlwaysAutoResize)) {
         ImGui::Text("Press Escape to cancel and go back.");

         const std::optional<key_event> last_key_event =
            std::exchange(_user_editing_last_key_event, std::nullopt);

         if (last_key_event) {
            if (last_key_event->key == key::escape and
                last_key_event->new_state == key_state::up) {
               _user_inputting_new_binding = false;

               ImGui::CloseCurrentPopup();
            }
            else if (last_key_event->new_state == key_state::up and
                     last_key_event->key != key::ctrl and
                     last_key_event->key != key::shift and
                     last_key_event->key != key::alt) {
               absl::flat_hash_map<hotkey_bind, hotkey> bindings;
               bindings.reserve(32);

               const key new_key = last_key_event->key;
               const key old_key = _user_swapping_key;

               for (hotkey_set& set : _hotkey_sets) {
                  bindings.clear();

                  for (auto it = set.bindings.begin(); it != set.bindings.end();) {
                     auto binding_it = it++;
                     auto& [bind, hotkey] = *binding_it;

                     if (bind.key == new_key or bind.key == old_key) {
                        set.query_bindings.erase(hotkey.name);
                        _saved_bindings[set.name].erase(hotkey.name);

                        bindings.emplace(hotkey_bind{.key = bind.key == new_key ? old_key : new_key,
                                                     .modifiers = bind.modifiers},
                                         std::move(hotkey));
                        set.bindings.erase(binding_it);
                     }
                  }

                  for (const auto& [bind, hotkey] : bindings) {
                     if (set.bindings.try_emplace(bind, hotkey).second) {
                        set.query_bindings[hotkey.name] = bind;
                        _saved_bindings[set.name][hotkey.name] = bind;
                     }
                     else {
                        set.unbound_hotkeys.push_back(hotkey);
                     }
                  }
               }

               save_bindings(save_path, _saved_bindings);

               _user_inputting_new_binding = false;

               ImGui::CloseCurrentPopup();
            }
         }

         ImGui::EndPopup();
      }
   }

   ImGui::End();

   if (not window_open) {
      _user_inputting_new_binding = false;
   }
}

void hotkeys::fill_sorted_info(const hotkey_set& set,
                               std::vector<sorted_hotkey_variant>& sorted_hotkey_set)
{
   sorted_hotkey_set.clear();
   sorted_hotkey_set.reserve(set.bindings.size());

   for (const auto& bind_hotkey : set.bindings) {
      sorted_hotkey_set.emplace_back(&bind_hotkey);
   }

   for (const auto& unbound : set.unbound_hotkeys) {
      sorted_hotkey_set.emplace_back(&unbound);
   }

   std::sort(sorted_hotkey_set.begin(), sorted_hotkey_set.end(),
             [](const sorted_hotkey_variant& left, const sorted_hotkey_variant& right) {
                const std::string& left_name =
                   std::holds_alternative<const std::pair<const hotkey_bind, hotkey>*>(left)
                      ? std::get<const std::pair<const hotkey_bind, hotkey>*>(left)
                           ->second.name
                      : std::get<const hotkey*>(left)->name;
                const std::string& right_name =
                   std::holds_alternative<const std::pair<const hotkey_bind, hotkey>*>(right)
                      ? std::get<const std::pair<const hotkey_bind, hotkey>*>(right)
                           ->second.name
                      : std::get<const hotkey*>(right)->name;

                return left_name < right_name;
             });
}

auto get_display_string(const std::optional<hotkey_bind> binding) -> const char*
{
   if (not binding) return "<UNBOUND>";

   return get_display_string(binding->key, binding->modifiers.ctrl,
                             binding->modifiers.shift, binding->modifiers.alt);
}
}