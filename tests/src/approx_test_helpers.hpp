#pragma once

#include <catch2/catch.hpp>
#include <glm/glm.hpp>

#include <type_traits>

namespace sk {

template<typename GLM_type>
inline bool approx_equals(const GLM_type& l, const std::type_identity_t<GLM_type>& r)
{
   for (int i = 0; i < l.length(); ++i) {
      if (l[i] != Approx(r[i])) {
         return false;
      }
   }

   return true;
}

}
