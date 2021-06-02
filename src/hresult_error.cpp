
#include "hresult_error.hpp"

#include <fmt/format.h>

namespace we {

hresult_exception::hresult_exception(const HRESULT hr) noexcept
   : std::runtime_error{fmt::format("HRESULT: {:#x}",
                                    static_cast<std::make_unsigned_t<HRESULT>>(hr))},
     _hr{hr} {};

}
