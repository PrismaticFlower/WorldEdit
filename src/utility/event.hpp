#pragma once

#include "event_listener.hpp"
#include "types.hpp"

#include <concepts>
#include <functional>
#include <limits>
#include <memory>
#include <shared_mutex>
#include <utility>

#include <absl/container/flat_hash_map.h>

namespace we::utility {

namespace impl {

template<typename T>
struct event_state : event_listener_control {
   using listeners_type = absl::flat_hash_map<int64, T>;

   auto add_listener(T callback) noexcept -> int64
   {
      std::scoped_lock lock{_mutex};

      const auto id = _next_id++;

      _listeners.emplace(id, std::move(callback));

      return id;
   }

   void erase_listener(int64 id) noexcept override
   {
      std::scoped_lock lock{_mutex};

      _listeners.erase(id);
   }

   void invoke_listeners(auto&... args) const noexcept
   {
      std::shared_lock lock{_mutex};

      for (const auto& [id, listener] : _listeners) {
         listener(args...);
      }
   }

private:
   mutable std::shared_mutex _mutex;
   listeners_type _listeners;
   int64 _next_id = std::numeric_limits<int64>::min();
};

}

template<typename T>
class event;

template<typename... Args>
class event<void(Args...)> {
public:
   using listener_type = event_listener<void(Args...)>;
   using callback_type = std::function<void(Args...)>;

   event() = default;

   event(const event&) = delete;
   auto operator=(const event&) -> event& = delete;

   event(event&&) = default;
   auto operator=(event&&) -> event& = default;

   ~event() = default;

   auto listen(callback_type callback) noexcept -> listener_type
   {
      const auto id = _state->add_listener(callback);

      return {id, _state};
   }

   void broadcast(Args... args) const noexcept
   {
      _state->invoke_listeners(args...);
   }

private:
   using state_type = impl::event_state<callback_type>;

   std::shared_ptr<state_type> _state = std::make_shared<state_type>();
};

}
