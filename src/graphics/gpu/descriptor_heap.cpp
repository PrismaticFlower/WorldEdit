
#include "descriptor_heap.hpp"
#include "hresult_error.hpp"

#include <cassert>
#include <stdexcept>

namespace sk::graphics::gpu {

namespace {

template<typename Type>
auto index_descriptor_ptr(const Type ptr, const std::type_identity_t<Type> index,
                          const std::type_identity_t<Type> size)
{
   return ptr + (index * size);
}

}

descriptor_heap_cpu::descriptor_heap_cpu(D3D12_DESCRIPTOR_HEAP_TYPE type,
                                         UINT descriptor_count, ID3D12Device& device)

   : _descriptor_size{device.GetDescriptorHandleIncrementSize(type)}, _descriptor_count{descriptor_count}
{
   const D3D12_DESCRIPTOR_HEAP_DESC desc{.Type = type,
                                         .NumDescriptors = descriptor_count};

   throw_if_failed(
      device.CreateDescriptorHeap(&desc,
                                  IID_PPV_ARGS(_descriptor_heap.clear_and_assign())));

   _root_handle = _descriptor_heap->GetCPUDescriptorHandleForHeapStart();
}

auto descriptor_heap_cpu::allocate() -> D3D12_CPU_DESCRIPTOR_HANDLE
{
   std::lock_guard lock{_mutex};

   if (not _freed_stack.empty()) {
      const auto handle = _freed_stack.top();

      _freed_stack.pop();

      return handle;
   }

   if (_used_count == _descriptor_count) {
      throw std::runtime_error{"out of space in descriptor heap!"};
   }

   const UINT index = _used_count;
   ++_used_count;

   return {.ptr = index_descriptor_ptr(_root_handle.ptr, index, _descriptor_size)};
}

void descriptor_heap_cpu::free(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
   std::lock_guard lock{_mutex};

   assert(handle.ptr >= _root_handle.ptr and
          handle.ptr < index_descriptor_ptr(_root_handle.ptr, _descriptor_count,
                                            _descriptor_size));

   _freed_stack.push(handle);
}

}