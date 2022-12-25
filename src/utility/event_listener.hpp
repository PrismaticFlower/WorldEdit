#pragma once

#include "types.hpp"

#include <memory>

namespace we::utility {

namespace impl {

struct event_listener_control {
   virtual ~event_listener_control() = default;

   virtual void erase_listener(int64 id) noexcept = 0;
};

}

template<typename T>
struct event_listener;

template<typename... Args>
struct event_listener<void(Args...)> {
   event_listener() = default;

   event_listener(int64 id, std::weak_ptr<impl::event_listener_control> weak_control)
      : _weak_control{weak_control}, _id{id}
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
      if (auto state = _weak_control.lock(); state) {
         state->erase_listener(_id);
      }
   }

   void swap(event_listener& other) noexcept
   {
      using std::swap;

      swap(this->_weak_control, other._weak_control);
      swap(this->_id, other._id);
   }

private:
   std::weak_ptr<impl::event_listener_control> _weak_control;
   int64 _id = 0;
};

}

namespace we {

using utility::event_listener;

}