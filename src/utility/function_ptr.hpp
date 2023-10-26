#pragma once

#include <cassert>
#include <type_traits>
#include <utility>

namespace we {

template<typename...>
struct function_ptr;

/// @brief A non-owner wrapper around a callable, like a lambda with captures or function pointer.
/// Intended to be used as a function argument.
///
/// Will have dangerous and exciting behaviour if you do:
///
/// function_ptr<int(int, int) noexcept> func_ptr{[](int x, int y) noexcept -> int { return x + y; }};
///
///
/// Which creates a function_ptr pointing named func_ptr to a dead lambda. The following is the correct and safe way to to it.
///
/// auto add = [](int x, int y) noexcept -> int { return x + y; };
///
/// function_ptr<int(int, int) noexcept> func_ptr{add};
///
/// However when used as a function argument the following is perfectly safe as the lambda will live until after the function call.
///
/// use_callback(function_ptr<int(int, int) noexcept>{[](int x, int y) noexcept -> int { return x + y; }});
///
template<typename R, typename... Args>
struct function_ptr<R(Args...) noexcept> {
   /// @brief Construct an empty function_ptr. Trying to call this will be exciting.
   function_ptr() = default;

   function_ptr(const function_ptr&) = default;
   function_ptr(function_ptr&&) = delete;

   auto operator=(const function_ptr&) -> function_ptr& = delete;
   auto operator=(function_ptr&&) -> function_ptr& = delete;

   /// @brief Construct a function_ptr from a callable object, like a lambda
   /// @tparam T The type of the callable.
   /// @param function A reference to the callable object, it the object should outlive the function_ptr.
   template<typename T>
   function_ptr(const T& function)
   {
      static_assert(std::is_nothrow_invocable_r_v<R, T, Args...>,
                    "T is not valid for use with this function_ptr.");

      _invoker = [](const void* function, Args... args) -> R {
         return (*static_cast<std::add_pointer_t<const T>>(function))(
            std::forward<Args>(args)...);
      };
      _function = &function;
   }

   /// @brief Construction a function_ptr from an actual function_ptr.
   /// @param function The function pointer.
   function_ptr(const std::add_pointer_t<R(Args...)> function)
   {
      _invoker = [](const void* function, Args... args) -> R {
         return (static_cast<const std::add_pointer_t<R(Args...)>>(function))(
            std::forward<Args>(args)...);
      };
      _function = function;
   }

   /// @brief Construct an empty function_ptr.
   function_ptr(std::nullptr_t) : function_ptr(){};

   /// @brief Call the function_ptr.
   /// @param ...args The args for the function.
   /// @return The result of the function.
   auto operator()(Args... args) const noexcept -> R
   {
      assert(_invoker);

      return _invoker(_function, std::forward<Args>(args)...);
   }

   /// @brief Checks if the function_ptr holds a a pointer to a function.
   explicit operator bool() const noexcept
   {
      return _invoker;
   }

private:
   std::add_pointer_t<R(const void*, Args...)> _invoker;
   const void* _function = nullptr;
};

template<typename... Args>
inline bool operator==(const function_ptr<Args...>& l, std::nullptr_t) noexcept
{
   return not l;
}

}
