#pragma once

#include <memory>

namespace sk::assets {

template<typename T>
class asset_state {
public:
   std::weak_ptr<T> loaded;

private:
};

}
