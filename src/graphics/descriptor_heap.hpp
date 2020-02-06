#pragma once

#include "utility/com_ptr.hpp"

#include <mutex>
#include <stack>
#include <vector>

#include <d3d12.h>

namespace sk::graphics {

class descriptor_heap_cpu {
public:
   descriptor_heap_cpu(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptor_count,
                       ID3D12Device& device);

   auto allocate() -> D3D12_CPU_DESCRIPTOR_HANDLE;

   void free(D3D12_CPU_DESCRIPTOR_HANDLE handle);

private:
   std::mutex _mutex;

   UINT _descriptor_size{};
   utility::com_ptr<ID3D12DescriptorHeap> _descriptor_heap;

   UINT _descriptor_count = 0;
   UINT _used_count = 0;

   D3D12_CPU_DESCRIPTOR_HANDLE _root_handle;

   std::stack<D3D12_CPU_DESCRIPTOR_HANDLE, std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> _freed_stack;
};

}