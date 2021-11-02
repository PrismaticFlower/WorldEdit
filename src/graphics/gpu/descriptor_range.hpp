#pragma once

#include "types.hpp"

#include <d3d12.h>

namespace we::graphics::gpu {

struct descriptor_handle {
   D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
   D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
};

template<typename Type>
constexpr auto index_descriptor_handle(const descriptor_handle handle, Type index,
                                       const std::type_identity_t<Type> size)
   -> descriptor_handle
{
   const Type offset = (index * size);

   return {.cpu = {handle.cpu.ptr + offset}, .gpu = {handle.gpu.ptr + offset}};
}

class descriptor_range {
public:
   descriptor_range() = default;

   descriptor_range(const uint32 base_index, const descriptor_handle handle,
                    const uint32 count, const uint32 descriptor_size) noexcept
      : _handle{handle}, _count{count}, _descriptor_size{descriptor_size}, _base_index{base_index} {};

   auto base_index() const noexcept -> uint32
   {
      return _base_index;
   }

   auto start() const noexcept -> descriptor_handle
   {
      return _handle;
   }

   auto size() const noexcept -> uint32
   {
      return _count;
   }

   auto operator[](const std::size_t i) const noexcept -> descriptor_handle
   {
      return index_descriptor_handle(_handle, i, _descriptor_size);
   }

private:
   descriptor_handle _handle{};
   uint32 _count = 0;
   uint32 _descriptor_size = 0;
   uint32 _base_index = 0;
};

}
