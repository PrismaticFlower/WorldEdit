
#include "asset_ref.hpp"
#include "asset_state.hpp"
#include "asset_traits.hpp"

#include <utility>

namespace we::assets {

template<typename T>
asset_ref<T>::asset_ref(std::shared_ptr<asset_state<T>> state) : _state{state}
{
   increment_ref_count();
}

template<typename T>
asset_ref<T>::asset_ref(const asset_ref& other) : _state{other._state}
{
   increment_ref_count();
}

template<typename T>
auto asset_ref<T>::operator=(const asset_ref& other) -> asset_ref&
{
   decrement_ref_count();

   this->_state = other._state;

   increment_ref_count();

   return *this;
}
template<typename T>
asset_ref<T>::asset_ref(asset_ref&& other) noexcept
{
   this->swap(other);
}

template<typename T>
auto asset_ref<T>::operator=(asset_ref&& other) noexcept -> asset_ref&
{
   asset_ref discarded;

   other.swap(discarded);
   this->swap(discarded);

   return *this;
}

template<typename T>
asset_ref<T>::~asset_ref()
{
   decrement_ref_count();
}

template<typename T>
auto asset_ref<T>::get_if() noexcept -> asset_data<T>
{
   std::shared_lock lock{_state->mutex};

   if (auto data = _state->data.lock(); data) {
      return data;
   }

   _state->start_load(); // DEADLOCK ='(

   return nullptr;
}

template<typename T>
bool asset_ref<T>::exists() const noexcept
{
   std::shared_lock lock{_state->mutex};

   return _state->exists;
}

template<typename T>
auto asset_ref<T>::use_count() const noexcept -> std::size_t
{
   return _state ? _state->ref_count.load(std::memory_order_relaxed) : 0;
}

template<typename T>
void asset_ref<T>::swap(asset_ref& other) noexcept
{
   using std::swap;

   swap(this->_state, other._state);
}

template<typename T>
auto asset_ref<T>::hash() const noexcept -> std::size_t
{
   return std::hash<decltype(_state)>{}(_state);
}

template struct asset_ref<odf::definition>;
template struct asset_ref<msh::flat_model>;
template struct asset_ref<texture::texture>;
template struct asset_ref<sky::config>;

}
