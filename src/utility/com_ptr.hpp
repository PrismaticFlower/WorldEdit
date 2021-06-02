#pragma once

#include <functional>
#include <iosfwd>
#include <memory>
#include <type_traits>
#include <utility>

#include <gsl/gsl>

namespace we::utility {

namespace detail {

template<typename Type, typename = void>
struct is_com_class : std::false_type {
};

template<typename Type>
struct is_com_class<Type, std::void_t<decltype(std::declval<Type>().AddRef()),
                                      decltype(std::declval<Type>().Release())>>
   : std::true_type {
};

template<typename Type>
constexpr bool is_com_class_v = is_com_class<Type>::value;
}

template<typename Class>
class com_ptr {
public:
   static_assert(detail::is_com_class_v<Class>,
                 "Class does not implement AddRef and Release.");

   using element_type = Class;

   com_ptr() = default;

   ~com_ptr()
   {
      if (_pointer) _pointer->Release();
   }

   explicit com_ptr(Class* from) noexcept
   {
      _pointer = from;
   }

   com_ptr(std::nullptr_t){};

   com_ptr(const com_ptr& other) noexcept
   {
      if (other) {
         other->AddRef();

         _pointer = other.get();
      }
   }

   com_ptr& operator=(const com_ptr& other) noexcept
   {
      if (other) {
         other->AddRef();
      }

      reset(other.get());

      return *this;
   }

   com_ptr(com_ptr&& other) noexcept
   {
      _pointer = other.release();
   }

   com_ptr& operator=(com_ptr&& other) noexcept
   {
      reset(other.release());

      return *this;
   }

   template<typename Other, typename = std::enable_if_t<std::is_convertible_v<Other*, Class*>>>
   com_ptr(const com_ptr<Other>& other)
   {
      auto other_copy = other;

      _pointer = static_cast<Class*>(other_copy.release());
   }

   template<typename Other, typename = std::enable_if_t<std::is_convertible_v<Other*, Class*>>>
   com_ptr& operator=(const com_ptr<Other>& other) noexcept
   {
      auto other_copy = other;

      reset(static_cast<Class*>(other_copy.release()));

      return *this;
   }

   template<typename Other, typename = std::enable_if_t<std::is_convertible_v<Other*, Class*>>>
   com_ptr(com_ptr<Other>&& other)
   {
      _pointer = static_cast<Class*>(other.release());
   }

   template<typename Other, typename = std::enable_if_t<std::is_convertible_v<Other*, Class*>>>
   com_ptr& operator=(com_ptr<Other>&& other) noexcept
   {
      reset(static_cast<Class*>(other.release()));

      return *this;
   }

   template<typename Other, typename = std::enable_if_t<std::is_convertible_v<Other*, Class*>>>
   explicit com_ptr(const std::shared_ptr<Other>& other)
   {
      _pointer = static_cast<Class*>(other.get());

      if (_pointer) _pointer->AddRef();
   }

   void reset(Class* with) noexcept
   {
      com_ptr discarded{_pointer};

      _pointer = with;
   }

   void swap(com_ptr& other) noexcept
   {
      std::swap(this->_pointer, other._pointer);
   }

   [[nodiscard]] auto release() noexcept -> gsl::owner<Class*>
   {
      return std::exchange(_pointer, nullptr);
   }

   auto get() const noexcept -> Class*
   {
      return _pointer;
   }

   auto get_ptr_ptr() const noexcept -> Class* const*
   {
      return &_pointer;
   }

   auto operator*() const noexcept -> Class&
   {
      return *_pointer;
   }

   auto operator->() const noexcept -> Class*
   {
      return _pointer;
   }

   [[nodiscard]] auto clear_and_assign() noexcept -> Class**
   {
      com_ptr discarded{};
      swap(discarded);

      return &_pointer;
   }

   [[nodiscard]] auto void_clear_and_assign() noexcept -> void**
   {
      com_ptr discarded{};
      swap(discarded);

      static_assert(sizeof(void**) == sizeof(Class**));

      return reinterpret_cast<void**>(&_pointer);
   }

   [[nodiscard]] auto unmanaged_copy() const noexcept -> gsl::owner<Class*>
   {
      Expects(this->get() != nullptr);

      _pointer->AddRef();

      return _pointer;
   }

   explicit operator bool() const noexcept
   {
      return (_pointer != nullptr);
   }

private:
   gsl::owner<Class*> _pointer = nullptr;
};

template<typename Class>
inline void swap(com_ptr<Class>& left, com_ptr<Class>& right) noexcept
{
   left.swap(right);
}

template<typename Class>
inline bool operator==(const com_ptr<Class>& left, const com_ptr<Class>& right) noexcept
{
   return (left.get() == right.get());
}

template<typename Class>
inline bool operator!=(const com_ptr<Class>& left, const com_ptr<Class>& right) noexcept
{
   return (left.get() != right.get());
}

template<typename Class>
inline bool operator==(const com_ptr<Class>& left, std::nullptr_t) noexcept
{
   return (left.get() == nullptr);
}

template<typename Class>
inline bool operator!=(const com_ptr<Class>& left, std::nullptr_t) noexcept
{
   return (left.get() != nullptr);
}

template<typename Class>
inline bool operator==(std::nullptr_t, const com_ptr<Class>& right) noexcept
{
   return (right == nullptr);
}

template<typename Class>
inline bool operator!=(std::nullptr_t, const com_ptr<Class>& right) noexcept
{
   return (right != nullptr);
}

template<typename Class>
inline bool operator==(const com_ptr<Class>& left, Class* const right) noexcept
{
   return (left.get() == right);
}

template<typename Class>
inline bool operator!=(const com_ptr<Class>& left, Class* const right) noexcept
{
   return (left.get() != right);
}

template<typename Class>
inline bool operator==(Class* const left, const com_ptr<Class>& right) noexcept
{
   return (right == left);
}

template<typename Class>
inline bool operator!=(Class* const left, const com_ptr<Class>& right) noexcept
{
   return (right != left);
}

template<typename Class, typename Char, typename Traits>
inline auto operator<<(std::basic_ostream<Char, Traits>& ostream,
                       const com_ptr<Class>& com_ptr)
   -> std::basic_ostream<Char, Traits>&
{
   return ostream << com_ptr.get();
}

template<typename Class>
inline auto make_shared_com_ptr(com_ptr<Class> com_ptr) -> std::shared_ptr<Class>
{
   return {com_ptr.release(), [](Class* ptr) {
              if (ptr) ptr->Release();
           }};
}
}

template<typename Class>
struct std::hash<we::utility::com_ptr<Class>> {
   auto operator()(const we::utility::com_ptr<Class>& ptr) const noexcept
   {
      return std::hash<Class*>{}(ptr.get());
   }
};
