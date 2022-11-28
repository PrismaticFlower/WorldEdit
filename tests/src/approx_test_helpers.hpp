#pragma once

#include "catch.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#include <type_traits>

namespace we {

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

template<typename GLM_type>
inline bool approx_equals(const GLM_type& l, const std::type_identity_t<GLM_type>& r,
                          const typename GLM_type::value_type epsilon)
{
   return glm::all(glm::epsilonEqual(l, r, epsilon));
}

}
