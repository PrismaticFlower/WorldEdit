#pragma once

namespace we {

/// @brief Helper class for combing lambdas into an overload set.
template<typename... Fns>
struct overload : Fns... {
   using Fns::operator()...;
};

template<typename... Fns>
overload(Fns...) -> overload<Fns...>;

}
