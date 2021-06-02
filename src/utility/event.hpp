#pragma once

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
class event_state {
public:
   using listeners_type = absl::flat_hash_map<int64, T>;

   auto add_listener(T callback) noexcept -> int64
   {
      std::scoped_lock lock{_mutex};

      const auto id = _next_id++;

      _listeners.emplace(id, std::move(callback));

      return id;
   }

   void erase_listener(int64 id)
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

template<typename T>
class event_listener;

template<typename... Args>
class event_listener<void(Args...)> {
private:
   using callback_type = std::function<void(Args...)>;
   using state_type = impl::event_state<callback_type>;

public:
   event_listener() = default;

   event_listener(int64 id, std::weak_ptr<state_type> weak_state)
      : _weak_state{weak_state}, _id{id}
   {
   }

   event_listener(const event_listener&) = delete;
   auto operator=(const event_listener&) -> event_listener& = delete;

   event_listener(event_listener&& other) noexcept
   {
      this->swap(other);
   }

   auto operator=(event_listener&& other) -> event_listener&
   {
      event_listener discarded;

      other.swap(discarded);
      this->swap(discarded);

      return *this;
   }

   ~event_listener()
   {
      if (auto state = _weak_state.lock(); state) {
         state->erase_listener(_id);
      }
   }

   void swap(event_listener& other) noexcept
   {
      using std::swap;

      swap(this->_weak_state, other._weak_state);
      swap(this->_id, other._id);
   }

private:
   std::weak_ptr<state_type> _weak_state;
   int64 _id = 0;
};

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

namespace we {

using utility::event_listener;

}
