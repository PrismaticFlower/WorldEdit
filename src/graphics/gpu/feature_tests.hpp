#pragma once

#include <d3d12.h>

namespace we::graphics::gpu {

inline bool root_signature_version_support(ID3D12Device8& device,
                                           D3D_ROOT_SIGNATURE_VERSION version)
{
   D3D12_FEATURE_DATA_ROOT_SIGNATURE data{.HighestVersion = version};

   if (SUCCEEDED(device.CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &data,
                                            sizeof data))) {
      return data.HighestVersion >= version;
   }

   return false;
}

inline bool command_queue_priority_supported(ID3D12Device8& device,
                                             D3D12_COMMAND_LIST_TYPE type,
                                             D3D12_COMMAND_QUEUE_PRIORITY priority) noexcept
{
   D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY data{.CommandListType = type,
                                                  .Priority =
                                                     static_cast<UINT>(priority)};

   if (SUCCEEDED(device.CheckFeatureSupport(D3D12_FEATURE_COMMAND_QUEUE_PRIORITY,
                                            &data, sizeof data))) {
      return data.PriorityForTypeIsSupported != 0;
   }

   return false;
}

}
