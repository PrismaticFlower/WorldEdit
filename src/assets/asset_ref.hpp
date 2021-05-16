#pragma once

#include "asset_state.hpp"

#include <memory>

namespace sk::assets {

template<typename T>
class asset_ref {
public:
   auto get() noexcept -> std::shared_ptr<T>;

private:
   std::shared_ptr<asset_state<T>> _state;
};

}

namespace sk {

template<typename T>
using asset_ref = assets::asset_ref<T>;

}
