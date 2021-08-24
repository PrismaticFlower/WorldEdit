#pragma once

#include "types.hpp"

#include <optional>
#include <string>
#include <type_traits>

#include <boost/container/small_vector.hpp>
#include <boost/variant2/variant.hpp>
#include <d3d12.h>

namespace we::graphics::gpu {

constexpr static int render_latency = 2;

using virtual_address = D3D12_GPU_VIRTUAL_ADDRESS;
using gpu_virtual_address = virtual_address;

struct buffer_desc {
   uint32 alignment = 0;
   uint32 size = 0;
   D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

   operator D3D12_RESOURCE_DESC() const noexcept
   {
      return {.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
              .Alignment = alignment,
              .Width = size,
              .Height = 1,
              .DepthOrArraySize = 1,
              .MipLevels = 1,
              .SampleDesc = {1, 0},
              .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
              .Flags = flags};
   };
};

struct texture_desc {
   D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
   D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
   D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uint32 width = 1;
   uint32 height = 1;
   uint16 depth = 1;
   uint16 mip_levels = 1;
   uint16 array_size = 1;

   std::optional<D3D12_CLEAR_VALUE> optimized_clear_value = std::nullopt;

   operator D3D12_RESOURCE_DESC() const noexcept
   {
      return {.Dimension = dimension,
              .Alignment = 0,
              .Width = width,
              .Height = height,
              .DepthOrArraySize =
                 dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? depth : array_size,
              .MipLevels = mip_levels,
              .Format = format,
              .SampleDesc = {1, 0},
              .Layout = layout,
              .Flags = flags};
   };
};

constexpr uint32 texture_srv_max_mip_levels = static_cast<uint32>(-1);

struct buffer_srv {
   uint32 first_element = 0;
   uint32 number_elements = 0;
   uint32 structure_byte_stride = 0;
   D3D12_BUFFER_SRV_FLAGS flags = D3D12_BUFFER_SRV_FLAG_NONE;

   operator D3D12_BUFFER_SRV() const noexcept
   {
      return {.FirstElement = first_element,
              .NumElements = number_elements,
              .StructureByteStride = structure_byte_stride,
              .Flags = flags};
   }
};

struct texture1d_srv {
   uint32 most_detailed_mip = 0;
   uint32 mip_levels = texture_srv_max_mip_levels;
   float resource_min_lod_clamp = 0.0f;

   operator D3D12_TEX1D_SRV() const noexcept
   {
      return {.MostDetailedMip = most_detailed_mip,
              .MipLevels = mip_levels,
              .ResourceMinLODClamp = resource_min_lod_clamp};
   }
};

struct texture1d_array_srv {
   uint32 most_detailed_mip = 0;
   uint32 mip_levels = texture_srv_max_mip_levels;
   uint32 first_array_slice = 0;
   uint32 array_size = 1;
   float resource_min_lod_clamp = 0.0f;

   operator D3D12_TEX1D_ARRAY_SRV() const noexcept
   {
      return {.MostDetailedMip = most_detailed_mip,
              .MipLevels = mip_levels,
              .FirstArraySlice = first_array_slice,
              .ArraySize = array_size,
              .ResourceMinLODClamp = resource_min_lod_clamp};
   }
};

struct texture2d_srv {
   uint32 most_detailed_mip = 0;
   uint32 mip_levels = texture_srv_max_mip_levels;
   uint32 plane_slice = 0;
   float resource_min_lod_clamp = 0.0f;

   operator D3D12_TEX2D_SRV() const noexcept
   {
      return {.MostDetailedMip = most_detailed_mip,
              .MipLevels = mip_levels,
              .PlaneSlice = plane_slice,
              .ResourceMinLODClamp = resource_min_lod_clamp};
   }
};

struct texture2d_array_srv {
   uint32 most_detailed_mip = 0;
   uint32 mip_levels = texture_srv_max_mip_levels;
   uint32 first_array_slice = 0;
   uint32 array_size = 1;
   uint32 plane_slice = 0;
   float resource_min_lod_clamp = 0.0f;

   operator D3D12_TEX2D_ARRAY_SRV() const noexcept
   {
      return {.MostDetailedMip = most_detailed_mip,
              .MipLevels = mip_levels,
              .FirstArraySlice = first_array_slice,
              .ArraySize = array_size,
              .PlaneSlice = plane_slice,
              .ResourceMinLODClamp = resource_min_lod_clamp};
   }
};

struct texture2d_ms_srv {
   operator D3D12_TEX2DMS_SRV() const noexcept
   {
      return {};
   }
};

struct texture2d_ms_array_srv {
   uint32 first_array_slice = 0;
   uint32 array_size = 0;

   operator D3D12_TEX2DMS_ARRAY_SRV() const noexcept
   {
      return {.FirstArraySlice = first_array_slice, .ArraySize = array_size};
   }
};

struct texture3d_srv {
   uint32 most_detailed_mip = 0;
   uint32 mip_levels = texture_srv_max_mip_levels;
   float resource_min_lod_clamp = 0.0f;

   operator D3D12_TEX3D_SRV() const noexcept
   {
      return {.MostDetailedMip = most_detailed_mip,
              .MipLevels = mip_levels,
              .ResourceMinLODClamp = resource_min_lod_clamp};
   }
};

struct texture_cube_srv {
   uint32 most_detailed_mip = 0;
   uint32 mip_levels = texture_srv_max_mip_levels;
   float resource_min_lod_clamp = 0.0f;

   operator D3D12_TEXCUBE_SRV() const noexcept
   {
      return {.MostDetailedMip = most_detailed_mip,
              .MipLevels = mip_levels,
              .ResourceMinLODClamp = resource_min_lod_clamp};
   }
};

struct texture_cube_array_srv {
   uint32 most_detailed_mip = 0;
   uint32 mip_levels = texture_srv_max_mip_levels;
   uint32 first_2d_array_face = 0;
   uint32 num_cubes = 0;
   float resource_min_lod_clamp = 0.0f;

   operator D3D12_TEXCUBE_ARRAY_SRV() const noexcept
   {
      return {.MostDetailedMip = most_detailed_mip,
              .MipLevels = mip_levels,
              .First2DArrayFace = first_2d_array_face,
              .NumCubes = num_cubes,
              .ResourceMinLODClamp = resource_min_lod_clamp};
   }
};

struct raytracing_acceleration_structure_srv {
   gpu_virtual_address location;

   operator D3D12_RAYTRACING_ACCELERATION_STRUCTURE_SRV() const noexcept
   {
      return {.Location = location};
   }
};

struct shader_resource_view_desc {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uint32 shader_4_component_mapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

   boost::variant2::variant<buffer_srv, texture1d_srv, texture1d_array_srv, texture2d_srv,
                            texture2d_array_srv, texture2d_ms_srv, texture2d_ms_array_srv,
                            texture3d_srv, texture_cube_srv, texture_cube_array_srv, raytracing_acceleration_structure_srv>
      type_description;

   operator D3D12_SHADER_RESOURCE_VIEW_DESC() const noexcept
   {
      D3D12_SHADER_RESOURCE_VIEW_DESC desc{.Format = format,
                                           .Shader4ComponentMapping =
                                              shader_4_component_mapping};

      boost::variant2::visit(
         [&](const auto& type_desc) {
            using Type = std::remove_cvref_t<decltype(type_desc)>;

            if constexpr (std::is_same_v<buffer_srv, Type>) {
               desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
               desc.Buffer = type_desc;
            }
            else if constexpr (std::is_same_v<texture1d_srv, Type>) {
               desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
               desc.Texture1D = type_desc;
            }
            else if constexpr (std::is_same_v<texture1d_array_srv, Type>) {
               desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
               desc.Texture1DArray = type_desc;
            }
            else if constexpr (std::is_same_v<texture2d_srv, Type>) {
               desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
               desc.Texture2D = type_desc;
            }
            else if constexpr (std::is_same_v<texture2d_array_srv, Type>) {
               desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
               desc.Texture2DArray = type_desc;
            }
            else if constexpr (std::is_same_v<texture2d_ms_srv, Type>) {
               desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
               desc.Texture2DMS = type_desc;
            }
            else if constexpr (std::is_same_v<texture2d_ms_array_srv, Type>) {
               desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
               desc.Texture2DMSArray = type_desc;
            }
            else if constexpr (std::is_same_v<texture3d_srv, Type>) {
               desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
               desc.Texture3D = type_desc;
            }
            else if constexpr (std::is_same_v<texture_cube_srv, Type>) {
               desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
               desc.TextureCube = type_desc;
            }
            else if constexpr (std::is_same_v<texture_cube_array_srv, Type>) {
               desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
               desc.TextureCubeArray = type_desc;
            }
            else if constexpr (std::is_same_v<raytracing_acceleration_structure_srv, Type>) {
               desc.ViewDimension =
                  D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
               desc.RaytracingAccelerationStructure = type_desc;
            }
         },
         type_description);

      return desc;
   }
};

struct constant_buffer_view {
   gpu_virtual_address buffer_location;
   uint32 size;

   operator D3D12_CONSTANT_BUFFER_VIEW_DESC() const noexcept
   {
      return {.BufferLocation = buffer_location, .SizeInBytes = size};
   }
};

struct buffer_uav {
   uint32 first_element = 0;
   uint32 number_elements = 0;
   uint32 structure_byte_stride = 0;
   uint64 counter_offset_in_bytes = 0;
   D3D12_BUFFER_UAV_FLAGS flags = D3D12_BUFFER_UAV_FLAG_NONE;

   operator D3D12_BUFFER_UAV() const noexcept
   {
      return {.FirstElement = first_element,
              .NumElements = number_elements,
              .StructureByteStride = structure_byte_stride,
              .CounterOffsetInBytes = counter_offset_in_bytes,
              .Flags = flags};
   }
};

struct texture1d_uav {
   uint32 mip_slice = 0;

   operator D3D12_TEX1D_UAV() const noexcept
   {
      return {.MipSlice = mip_slice};
   };
};

struct texture1d_array_uav {
   uint32 mip_slice = 0;
   uint32 first_array_slice = 0;
   uint32 array_size = 0;

   operator D3D12_TEX1D_ARRAY_UAV() const noexcept
   {
      return {.MipSlice = mip_slice,
              .FirstArraySlice = first_array_slice,
              .ArraySize = array_size};
   }
};

struct texture2d_uav {
   uint32 mip_slice = 0;
   uint32 plane_slice = 0;

   operator D3D12_TEX2D_UAV() const noexcept
   {
      return {.MipSlice = mip_slice, .PlaneSlice = plane_slice};
   };
};

struct texture2d_array_uav {
   uint32 mip_slice = 0;
   uint32 first_array_slice = 0;
   uint32 array_size = 0;
   uint32 plane_slice = 0;

   operator D3D12_TEX2D_ARRAY_UAV() const noexcept
   {
      return {.MipSlice = mip_slice,
              .FirstArraySlice = first_array_slice,
              .ArraySize = array_size,
              .PlaneSlice = plane_slice};
   }
};

struct texture3d_uav {
   uint32 mip_slice = 0;
   uint32 first_w_slice = 0;
   uint32 w_size = 0;

   operator D3D12_TEX3D_UAV() const noexcept
   {
      return {.MipSlice = mip_slice, .FirstWSlice = first_w_slice, .WSize = w_size};
   };
};

struct unordered_access_view_desc {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

   boost::variant2::variant<buffer_uav, texture1d_uav, texture1d_array_uav, texture2d_uav,
                            texture2d_uav, texture2d_array_uav, texture3d_uav>
      type_description;

   operator D3D12_UNORDERED_ACCESS_VIEW_DESC() const noexcept
   {
      D3D12_UNORDERED_ACCESS_VIEW_DESC desc{.Format = format};

      boost::variant2::visit(
         [&](const auto& type_desc) {
            using Type = std::remove_cvref_t<decltype(type_desc)>;

            if constexpr (std::is_same_v<buffer_uav, Type>) {
               desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
               desc.Buffer = type_desc;
            }
            else if constexpr (std::is_same_v<texture1d_uav, Type>) {
               desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
               desc.Texture1D = type_desc;
            }
            else if constexpr (std::is_same_v<texture1d_array_uav, Type>) {
               desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
               desc.Texture1DArray = type_desc;
            }
            else if constexpr (std::is_same_v<texture2d_uav, Type>) {
               desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
               desc.Texture2D = type_desc;
            }
            else if constexpr (std::is_same_v<texture2d_array_uav, Type>) {
               desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
               desc.Texture2DArray = type_desc;
            }
            else if constexpr (std::is_same_v<texture3d_uav, Type>) {
               desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
               desc.Texture3D = type_desc;
            }
         },
         type_description);

      return desc;
   }
};

struct resource_view_desc {
   std::shared_ptr<ID3D12Resource> resource;
   std::shared_ptr<ID3D12Resource> counter_resource = nullptr;

   boost::variant2::variant<shader_resource_view_desc, constant_buffer_view, unordered_access_view_desc> view_desc;
};

struct static_sampler_desc {
   D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
   D3D12_TEXTURE_ADDRESS_MODE address_u = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
   D3D12_TEXTURE_ADDRESS_MODE address_v = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
   D3D12_TEXTURE_ADDRESS_MODE address_w = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
   float mip_lod_bias = 0.0f;
   uint32 max_anisotropy = 0;
   D3D12_COMPARISON_FUNC comparison_func = D3D12_COMPARISON_FUNC_ALWAYS;
   D3D12_STATIC_BORDER_COLOR border_color = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
   FLOAT min_lod = 0.0;
   FLOAT max_lod = D3D12_FLOAT32_MAX;
};

struct root_parameter_descriptor_range {
   D3D12_DESCRIPTOR_RANGE_TYPE type;
   uint32 count = 0;
   uint32 base_shader_register = 0;
   uint32 register_space = 0;
   D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
   uint32 offset_in_descriptors_from_table_start = 0;

   operator D3D12_DESCRIPTOR_RANGE() const noexcept
   {
      return {.RangeType = type,
              .NumDescriptors = count,
              .BaseShaderRegister = base_shader_register,
              .RegisterSpace = register_space,
              .OffsetInDescriptorsFromTableStart =
                 offset_in_descriptors_from_table_start};
   }

   operator D3D12_DESCRIPTOR_RANGE1() const noexcept
   {
      return {.RangeType = type,
              .NumDescriptors = count,
              .BaseShaderRegister = base_shader_register,
              .RegisterSpace = register_space,
              .Flags = flags,
              .OffsetInDescriptorsFromTableStart =
                 offset_in_descriptors_from_table_start};
   }
};

struct root_parameter_descriptor_table {
   using range = root_parameter_descriptor_range;

   boost::container::small_vector<range, 4> ranges;
   D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;
};

struct root_parameter_32bit_constants {
   uint32 shader_register = 0;
   uint32 register_space = 0;
   uint32 values_count = 0;
   D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;

   operator D3D12_ROOT_PARAMETER() const noexcept
   {
      return {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
              .Constants = {.ShaderRegister = shader_register,
                            .RegisterSpace = register_space,
                            .Num32BitValues = values_count},
              .ShaderVisibility = visibility};
   }

   operator D3D12_ROOT_PARAMETER1() const noexcept
   {
      return {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
              .Constants = {.ShaderRegister = shader_register,
                            .RegisterSpace = register_space,
                            .Num32BitValues = values_count},
              .ShaderVisibility = visibility};
   }
};

struct root_parameter_cbv {
   uint32 shader_register = 0;
   uint32 register_space = 0;
   D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
   D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;

   operator D3D12_ROOT_PARAMETER() const noexcept
   {
      return {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
              .Descriptor = {.ShaderRegister = shader_register,
                             .RegisterSpace = register_space},
              .ShaderVisibility = visibility};
   }

   operator D3D12_ROOT_PARAMETER1() const noexcept
   {
      return {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
              .Descriptor = {.ShaderRegister = shader_register,
                             .RegisterSpace = register_space,
                             .Flags = flags},
              .ShaderVisibility = visibility};
   }
};

struct root_parameter_srv {
   uint32 shader_register = 0;
   uint32 register_space = 0;
   D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
   D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;

   operator D3D12_ROOT_PARAMETER() const noexcept
   {
      return {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
              .Descriptor = {.ShaderRegister = shader_register,
                             .RegisterSpace = register_space},
              .ShaderVisibility = visibility};
   }

   operator D3D12_ROOT_PARAMETER1() const noexcept
   {
      return {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
              .Descriptor = {.ShaderRegister = shader_register,
                             .RegisterSpace = register_space,
                             .Flags = flags},
              .ShaderVisibility = visibility};
   }
};

struct root_parameter_uav {
   uint32 shader_register = 0;
   uint32 register_space = 0;
   D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
   D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;

   operator D3D12_ROOT_PARAMETER() const noexcept
   {
      return {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV,
              .Descriptor = {.ShaderRegister = shader_register,
                             .RegisterSpace = register_space},
              .ShaderVisibility = visibility};
   }

   operator D3D12_ROOT_PARAMETER1() const noexcept
   {
      return {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV,
              .Descriptor = {.ShaderRegister = shader_register,
                             .RegisterSpace = register_space,
                             .Flags = flags},
              .ShaderVisibility = visibility};
   }
};

struct root_static_sampler {
   static_sampler_desc sampler;
   uint32 shader_register = 0;
   uint32 register_space = 0;
   D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;

   operator D3D12_STATIC_SAMPLER_DESC() const noexcept
   {
      return D3D12_STATIC_SAMPLER_DESC{.Filter = sampler.filter,
                                       .AddressU = sampler.address_u,
                                       .AddressV = sampler.address_v,
                                       .AddressW = sampler.address_w,
                                       .MipLODBias = sampler.mip_lod_bias,
                                       .MaxAnisotropy = sampler.max_anisotropy,
                                       .ComparisonFunc = sampler.comparison_func,
                                       .BorderColor = sampler.border_color,
                                       .MinLOD = sampler.min_lod,
                                       .MaxLOD = sampler.max_lod,
                                       .ShaderRegister = shader_register,
                                       .RegisterSpace = register_space,
                                       .ShaderVisibility = visibility};
   }
};

struct root_signature_desc {
   using parameter =
      boost::variant2::variant<root_parameter_descriptor_table, root_parameter_32bit_constants,
                               root_parameter_cbv, root_parameter_srv, root_parameter_uav>;

   std::string name;

   boost::container::small_vector<parameter, 16> parameters;
   boost::container::small_vector<root_static_sampler, 16> samplers;
   D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
};

}
