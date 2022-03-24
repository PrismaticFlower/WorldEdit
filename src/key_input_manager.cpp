#include "key_input_manager.hpp"

#include <utility>

namespace we {

void key_input_manager::notify_key_down(key key) noexcept
{
   _key_events.push_back({.key = key, .new_state = key_state::down});
}

void key_input_manager::notify_key_up(key key) noexcept
{
   _key_events.push_back({.key = key, .new_state = key_state::up});
}

void key_input_manager::bind(const bind_key binding, const bind_config bind_config,
                             std::function<void()> callback) noexcept
{
   select_key_binding(binding) =
      key_binding{.toggle = bind_config.toggle, .callback = std::move(callback)};
}

void key_input_manager::unbind(const bind_key binding) noexcept
{
   select_key_binding(binding) = std::nullopt;
}

void key_input_manager::update(const bool ignore_unmodified_bindings) noexcept
{
   for (auto [key, new_state] : _key_events) {
      process_new_key_state(key, new_state, ignore_unmodified_bindings);
   }

   _key_events.clear();
}

void key_input_manager::release_toggles() noexcept
{
   for (auto& key : _keys) {
      const auto process_binding = [](key_binding& binding) noexcept {
         if (binding.toggle and std::exchange(binding.toggle_active, false)) {
            binding.callback();
         }
      };

      if (key.binding) process_binding(*key.binding);
      if (key.ctrl_binding) process_binding(*key.ctrl_binding);
      if (key.ctrl_shift_binding) process_binding(*key.ctrl_shift_binding);
      if (key.shift_binding) process_binding(*key.shift_binding);
   }
}

void key_input_manager::release_unmodified_toggles() noexcept
{
   for (auto& key : _keys) {
      const auto process_binding = [](key_binding& binding) noexcept {
         if (binding.toggle and std::exchange(binding.toggle_active, false)) {
            binding.callback();
         }
      };

      if (key.binding) process_binding(*key.binding);
   }
}

void key_input_manager::process_new_key_state(const key key, const key_state new_state,
                                              const bool ignore_unmodified_bindings) noexcept
{
   auto& bindings = _keys[key];

   const key_state old_state = std::exchange(bindings.state, new_state);

   if (old_state == new_state) return;

   const auto process_binding = [&](key_binding& binding) noexcept {
      if (binding.toggle and (new_state == key_state::down or binding.toggle_active)) {
         binding.toggle_active = not binding.toggle_active;
         binding.callback();
      }
      else if (new_state == key_state::down) {
         // If this was true then we know this is a transition from up to down
         // as we've already ruled out a transition to the same state above.

         binding.callback();
      }
   };

   if (bindings.binding and not ignore_unmodified_bindings) {
      process_binding(*bindings.binding);
   }

   if (bindings.ctrl_binding and is_key_down(key::control)) {
      process_binding(*bindings.ctrl_binding);
   }

   if (bindings.ctrl_shift_binding and is_key_down(key::control) and
       is_key_down(key::shift)) {
      process_binding(*bindings.ctrl_shift_binding);
   }

   if (bindings.shift_binding and is_key_down(key::shift)) {
      process_binding(*bindings.shift_binding);
   }
}

bool key_input_manager::is_key_down(const key key) const noexcept
{
   return _keys[key].state == key_state::down;
}

auto key_input_manager::select_key_binding(const bind_key binding) noexcept
   -> std::optional<key_binding>&
{
   if (auto& key = _keys[binding.key];
       binding.modifiers.ctrl and binding.modifiers.shift) {
      return key.ctrl_shift_binding;
   }
   else if (binding.modifiers.ctrl) {
      return key.ctrl_binding;
   }
   else if (binding.modifiers.shift) {
      return key.shift_binding;
   }
   else {
      return key.binding;
   }
}

auto translate_virtual_key(const std::uintptr_t vk) noexcept -> key
{
   // This ain't pretty but it should never have to be edited so there's that.

   switch (vk) {
   case 0x30:
      return key::_0;
   case 0x31:
      return key::_1;
   case 0x32:
      return key::_2;
   case 0x33:
      return key::_3;
   case 0x34:
      return key::_4;
   case 0x35:
      return key::_5;
   case 0x36:
      return key::_6;
   case 0x37:
      return key::_7;
   case 0x38:
      return key::_8;
   case 0x39:
      return key::_9;
   case 0x41:
      return key::a;
   case 0x1e:
      return key::accept;
   case 0x6b:
      return key::add;
   case 0x42:
      return key::b;
   case 0x08:
      return key::back;
   case 0xa6:
      return key::browser_back;
   case 0xab:
      return key::browser_favorites;
   case 0xa7:
      return key::browser_forward;
   case 0xac:
      return key::browser_home;
   case 0xa8:
      return key::browser_refresh;
   case 0xaa:
      return key::browser_search;
   case 0xa9:
      return key::browser_stop;
   case 0x43:
      return key::c;
   case 0x14:
      return key::capital;
   case 0x0c:
      return key::clear;
   case 0x11:
      return key::control;
   case 0x1c:
      return key::convert;
   case 0x44:
      return key::d;
   case 0x6e:
      return key::decimal;
   case 0x2e:
      return key::delete_;
   case 0x6f:
      return key::divide;
   case 0x28:
      return key::down;
   case 0x45:
      return key::e;
   case 0x23:
      return key::end;
   case 0x1b:
      return key::escape;
   case 0x2b:
      return key::execute;
   case 0x46:
      return key::f;
   case 0x70:
      return key::f1;
   case 0x79:
      return key::f10;
   case 0x7a:
      return key::f11;
   case 0x7b:
      return key::f12;
   case 0x7c:
      return key::f13;
   case 0x7d:
      return key::f14;
   case 0x7e:
      return key::f15;
   case 0x7f:
      return key::f16;
   case 0x80:
      return key::f17;
   case 0x81:
      return key::f18;
   case 0x82:
      return key::f19;
   case 0x71:
      return key::f2;
   case 0x83:
      return key::f20;
   case 0x84:
      return key::f21;
   case 0x85:
      return key::f22;
   case 0x86:
      return key::f23;
   case 0x87:
      return key::f24;
   case 0x72:
      return key::f3;
   case 0x73:
      return key::f4;
   case 0x74:
      return key::f5;
   case 0x75:
      return key::f6;
   case 0x76:
      return key::f7;
   case 0x77:
      return key::f8;
   case 0x78:
      return key::f9;
   case 0x47:
      return key::g;
   case 0x48:
      return key::h;
   case 0x2f:
      return key::help;
   case 0x24:
      return key::home;
   case 0x49:
      return key::i;
   case 0x2d:
      return key::insert;
   case 0x4a:
      return key::j;
   case 0x4b:
      return key::k;
   case 0x4c:
      return key::l;
   case 0x25:
      return key::left;
   case 0x4d:
      return key::m;
   case 0x12:
      return key::menu;
   case 0x1f:
      return key::modechange;
   case 0x6a:
      return key::multiply;
   case 0x4e:
      return key::n;
   case 0x22:
      return key::next;
   case 0x1d:
      return key::nonconvert;
   case 0x90:
      return key::numlock;
   case 0x60:
      return key::numpad0;
   case 0x61:
      return key::numpad1;
   case 0x62:
      return key::numpad2;
   case 0x63:
      return key::numpad3;
   case 0x64:
      return key::numpad4;
   case 0x65:
      return key::numpad5;
   case 0x66:
      return key::numpad6;
   case 0x67:
      return key::numpad7;
   case 0x68:
      return key::numpad8;
   case 0x69:
      return key::numpad9;
   case 0x4f:
      return key::o;
   case 0xba:
      return key::oem_1;
   case 0xe2:
      return key::oem_102;
   case 0xbf:
      return key::oem_2;
   case 0xc0:
      return key::oem_3;
   case 0xdb:
      return key::oem_4;
   case 0xdc:
      return key::oem_5;
   case 0xdd:
      return key::oem_6;
   case 0xde:
      return key::oem_7;
   case 0xdf:
      return key::oem_8;
   case 0xe1:
      return key::oem_ax;
   case 0xbc:
      return key::oem_comma;
   case 0x92:
      return key::oem_fj_jisho;
   case 0x95:
      return key::oem_fj_loya;
   case 0x93:
      return key::oem_fj_masshou;
   case 0x96:
      return key::oem_fj_roya;
   case 0x94:
      return key::oem_fj_touroku;
   case 0xbd:
      return key::oem_minus;
   case 0xbe:
      return key::oem_period;
   case 0xbb:
      return key::oem_plus;
   case 0x50:
      return key::p;
   case 0x13:
      return key::pause;
   case 0x2a:
      return key::print;
   case 0x21:
      return key::prior;
   case 0x51:
      return key::q;
   case 0x52:
      return key::r;
   case 0x0d:
      return key::return_;
   case 0x27:
      return key::right;
   case 0x53:
      return key::s;
   case 0x91:
      return key::scroll;
   case 0x29:
      return key::select;
   case 0x6c:
      return key::separator;
   case 0x10:
      return key::shift;
   case 0x5f:
      return key::sleep;
   case 0x2c:
      return key::snapshot;
   case 0x20:
      return key::space;
   case 0x6d:
      return key::subtract;
   case 0x54:
      return key::t;
   case 0x09:
      return key::tab;
   case 0x55:
      return key::u;
   case 0x26:
      return key::up;
   case 0x56:
      return key::v;
   case 0x57:
      return key::w;
   case 0x58:
      return key::x;
   case 0x59:
      return key::y;
   case 0x5a:
      return key::z;
   case 0x01:
      return key::mouse1;
   case 0x02:
      return key::mouse2;
   case 0x04:
      return key::mouse3;
   case 0x05:
      return key::mouse4;
   case 0x06:
      return key::mouse5;
   default:
      return key::void_key;
   };
}

}
