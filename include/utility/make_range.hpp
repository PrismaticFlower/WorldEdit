#pragma once

namespace sk::utility {

template<typename Begin, typename End>
auto make_range(Begin begin, End end) noexcept
{
   struct adhoc_range {
      Begin begin_it;
      End end_it;

      auto begin() const noexcept
      {
         return begin_it;
      }

      auto end() const noexcept
      {
         return end_it;
      }
   };

   return adhoc_range{.begin_it = begin, .end_it = end};
}

}
