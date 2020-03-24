#pragma once

#include <type_traits>

namespace sk::math {

template<typename Type>
constexpr auto align_up(Type value, std::type_identity_t<Type> alignment) -> Type
{
   return (value + alignment - Type{1}) / alignment * alignment;
}

}