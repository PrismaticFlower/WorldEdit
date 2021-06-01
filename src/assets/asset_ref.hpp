#pragma once

#include "asset_state.hpp"
#include "utility/event.hpp"

#include <algorithm>
#include <memory>
#include <utility>

namespace sk::assets {

/// @brief Convenience alias for std::shared_ptr that adds const to the type.
/// @tparam T The type of the asset.
template<typename T>
using asset_data = std::shared_ptr<const T>;

/// @brief Represents a reference to an asset. This class is thread safe but
/// must not outlive the asset library the reference came from.
/// @tparam T The type of the asset.
template<typename T>
class asset_ref {
public:
   /// @brief Constructs an asset_ref that references nothing.
   asset_ref() = default;

   /// @brief Constructs a new asset_ref.
   /// @param state The private asset state.
   asset_ref(std::shared_ptr<asset_state<T>> state) : _state{state}
   {
      increment_ref_count();
   }

   /// @brief Copy constructor.
   /// @param other The asset_ref to copy.
   asset_ref(const asset_ref& other) : _state{other._state}
   {
      increment_ref_count();
   }

   /// @brief Copy assignment.
   /// @param other The asset_ref to copy.
   auto operator=(const asset_ref& other) -> asset_ref&
   {
      decrement_ref_count();

      this->_state = other._state;

      return *this;
   }

   /// @brief Move constructor.
   /// @param other The asset_ref to move.
   asset_ref(asset_ref&& other) noexcept
   {
      this->swap(other);
   }

   /// @brief Move assignment.
   /// @param other The asset_ref to move.
   auto operator=(asset_ref&& other) noexcept -> asset_ref&
   {
      asset_ref discarded;

      other.swap(discarded);
      this->swap(discarded);

      return *this;
   }

   ~asset_ref()
   {
      decrement_ref_count();
   }

   /// @brief Gets the loaded asset data or begins loading it.
   /// @return The loaded asset data or nullptr if the asset is still loading,
   ///  failed loading or doesn't exist.
   auto get_if() noexcept -> asset_data<T>
   {
      std::shared_lock lock{_state->mutex};

      if (auto data = _state->data.lock(); data) {
         return data;
      }

      _state->start_load();

      return nullptr;
   }

   /// @brief Check if the asset exists (that is to say has a file on disk)
   /// @return true is the asset exists, false if otherwise.
   bool exists() const noexcept
   {
      std::shared_lock lock{_state->mutex};

      return _state->exists;
   }

   /// @brief Gets the number of references currently held to this asset.
   /// @return The use count.
   auto use_count() const noexcept -> std::size_t
   {
      return _state ? _state->ref_count.load(std::memory_order_relaxed) : 0;
   }

   /// @brief Swaps this with other.
   void swap(asset_ref& other) noexcept
   {
      using std::swap;

      swap(this->_state, other._state);
   }

   /// @brief Returns a hash of the asset state for use in hash maps.
   /// @return The hash.
   auto hash() const noexcept -> std::size_t
   {
      return std::hash<decltype(_state)>{}(_state);
   }

   bool operator==(const asset_ref&) const noexcept = default;

private:
   void increment_ref_count()
   {
      if (_state) _state->ref_count.fetch_add(1, std::memory_order_relaxed);
   }

   void decrement_ref_count()
   {
      if (_state) _state->ref_count.fetch_sub(1, std::memory_order_relaxed);
   }

   std::shared_ptr<asset_state<T>> _state;
};

}

namespace sk {

using assets::asset_data;
using assets::asset_ref;

}

namespace std {

template<typename T>
struct hash<sk::assets::asset_ref<T>> {
   auto operator()(const sk::assets::asset_ref::asset_ref<T>& ref) const noexcept
      -> std::size_t
   {
      return ref.hash();
   }
};

}
