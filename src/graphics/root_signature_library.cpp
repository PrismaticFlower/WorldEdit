
#include "root_signature_library.hpp"
#include "gpu/common.hpp"
#include "hresult_error.hpp"

#include <algorithm>

#include <boost/container/static_vector.hpp>

namespace we::graphics {

namespace {

auto create_root_signature(ID3D12Device& device,
                           const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& desc,
                           const std::string_view name)
   -> utility::com_ptr<ID3D12RootSignature>
{
   utility::com_ptr<ID3DBlob> root_signature_blob;

   if (utility::com_ptr<ID3DBlob> root_signature_error_blob;
       FAILED(D3D12SerializeVersionedRootSignature(
          &desc, root_signature_blob.clear_and_assign(),
          root_signature_error_blob.clear_and_assign()))) {
      std::string message{static_cast<const char*>(
                             root_signature_error_blob->GetBufferPointer()),
                          root_signature_error_blob->GetBufferSize()};

      throw std::runtime_error{std::move(message)};
   }

   utility::com_ptr<ID3D12RootSignature> root_sig;

   throw_if_failed(
      device.CreateRootSignature(0, root_signature_blob->GetBufferPointer(),
                                 root_signature_blob->GetBufferSize(),
                                 IID_PPV_ARGS(root_sig.clear_and_assign())));

   if (not name.empty()) {
      root_sig->SetPrivateData(WKPDID_D3DDebugObjectName,
                               to_uint32(name.size()), name.data());
   }

   return root_sig;
}

auto create_root_signature(ID3D12Device& device, const gpu::root_signature_desc& desc)
   -> utility::com_ptr<ID3D12RootSignature>
{
   boost::container::static_vector<D3D12_DESCRIPTOR_RANGE1, 256> descriptor_ranges_stack;
   boost::container::small_vector<D3D12_ROOT_PARAMETER1, 16> parameters;
   boost::container::small_vector<D3D12_STATIC_SAMPLER_DESC, 16> samplers;

   for (auto& param : desc.parameters) {
      auto d3d12_param = boost::variant2::visit(
         [&]<typename T>(const T& param) -> D3D12_ROOT_PARAMETER1 {
            if constexpr (std::is_same_v<T, gpu::root_parameter_descriptor_table>) {
               const auto ranges_stack_offset = descriptor_ranges_stack.size();

               descriptor_ranges_stack.resize(ranges_stack_offset +
                                              param.ranges.size());

               std::copy_n(param.ranges.begin(), param.ranges.size(),
                           descriptor_ranges_stack.begin() + ranges_stack_offset);

               return D3D12_ROOT_PARAMETER1{
                  .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                  .DescriptorTable =
                     {
                        .NumDescriptorRanges = to_uint32(param.ranges.size()),
                        .pDescriptorRanges = &descriptor_ranges_stack[ranges_stack_offset],
                     },
                  .ShaderVisibility = param.visibility};
            }
            else {
               return param;
            }
         },
         param);

      parameters.push_back(d3d12_param);
   }

   samplers.resize(desc.samplers.size());

   std::ranges::copy(desc.samplers, samplers.begin());

   const D3D12_VERSIONED_ROOT_SIGNATURE_DESC
      d3d12_desc{.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
                 .Desc_1_1 = {.NumParameters = to_uint32(parameters.size()),
                              .pParameters = parameters.data(),
                              .NumStaticSamplers = to_uint32(samplers.size()),
                              .pStaticSamplers = samplers.data(),
                              .Flags = desc.flags}};

   return create_root_signature(device, d3d12_desc, desc.name);
}

}

root_signature_library::root_signature_library(ID3D12Device& device)
{
   const gpu::static_sampler_desc
      trilinear_sampler{.filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                        .address_u = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                        .address_v = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                        .address_w = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                        .mip_lod_bias = 0.0f,
                        .max_anisotropy = 0,
                        .comparison_func = D3D12_COMPARISON_FUNC_ALWAYS,
                        .border_color = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                        .min_lod = 0.0f,
                        .max_lod = D3D12_FLOAT32_MAX};

   const gpu::static_sampler_desc
      bilinear_sampler{.filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
                       .address_u = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                       .address_v = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                       .address_w = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                       .mip_lod_bias = 0.0f,
                       .max_anisotropy = 0,
                       .comparison_func = D3D12_COMPARISON_FUNC_ALWAYS,
                       .border_color = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                       .min_lod = 0.0f,
                       .max_lod = D3D12_FLOAT32_MAX};

   const gpu::static_sampler_desc
      shadow_sampler{.filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
                     .address_u = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                     .address_v = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                     .address_w = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                     .mip_lod_bias = 0.0f,
                     .max_anisotropy = 0,
                     .comparison_func = D3D12_COMPARISON_FUNC_LESS_EQUAL,
                     .border_color = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                     .min_lod = 0.0f,
                     .max_lod = D3D12_FLOAT32_MAX};

   constexpr uint32 mesh_register_space = 0;
   constexpr uint32 material_register_space = 1;
   constexpr uint32 lights_register_space = 0;
   constexpr uint32 terrain_register_space = 2;

   const gpu::root_signature_desc object_mesh_desc{
      .name = "mesh_root_signature",

      .parameters =
         {
            // per-object constants
            gpu::root_parameter_cbv{
               .shader_register = 1,
               .register_space = mesh_register_space,
               .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
               .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
            },

            // per-object material descriptors
            gpu::root_parameter_descriptor_table{
               .ranges =
                  {
                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                      .count = 2,
                      .base_shader_register = 0,
                      .register_space = material_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 0},
                  },
               .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
            },

            // camera descriptors
            gpu::root_parameter_descriptor_table{
               .ranges =
                  {
                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                      .count = 1,
                      .base_shader_register = 0,
                      .register_space = mesh_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 0},
                  },
               .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
            },

            // lights descriptors
            gpu::root_parameter_descriptor_table{
               .ranges =
                  {
                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                      .count = 1,
                      .base_shader_register = 0,
                      .register_space = lights_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 0},

                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                      .count = 2,
                      .base_shader_register = 0,
                      .register_space = lights_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 1},
                  },
               .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
            },
         },

      .samplers =
         {
            {.sampler = trilinear_sampler,
             .shader_register = 0,
             .register_space = 0,
             .visibility = D3D12_SHADER_VISIBILITY_PIXEL},

            {.sampler = shadow_sampler,
             .shader_register = 2,
             .register_space = 0,
             .visibility = D3D12_SHADER_VISIBILITY_PIXEL},
         },

      .flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

   object_mesh = create_root_signature(device, object_mesh_desc);

   const gpu::root_signature_desc terrain_desc{
      .name = "terrain_root_signature",

      .parameters =
         {
            // camera descriptor
            gpu::root_parameter_descriptor_table{
               .ranges =
                  {
                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                      .count = 1,
                      .base_shader_register = 0,
                      .register_space = mesh_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 0},
                  },
               .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
            },

            // lights descriptors
            gpu::root_parameter_descriptor_table{
               .ranges =
                  {
                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                      .count = 1,
                      .base_shader_register = 0,
                      .register_space = lights_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 0},

                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                      .count = 2,
                      .base_shader_register = 0,
                      .register_space = lights_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 1},
                  },
               .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
            },

            // terrain descriptors
            gpu::root_parameter_descriptor_table{
               .ranges =
                  {
                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                      .count = 1,
                      .base_shader_register = 0,
                      .register_space = terrain_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 0},

                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                      .count = 2,
                      .base_shader_register = 0,
                      .register_space = terrain_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 1},
                  },
               .visibility = D3D12_SHADER_VISIBILITY_ALL,
            },

            // terrain patch data
            gpu::root_parameter_srv{
               .shader_register = 2,
               .register_space = terrain_register_space,
               .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
               .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
            },

            // material descriptors
            gpu::root_parameter_descriptor_table{
               .ranges =
                  {
                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                      .count = 16,
                      .base_shader_register = 0,
                      .register_space = material_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 0},
                  },
               .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
            },
         },

      .samplers =
         {
            {.sampler = bilinear_sampler,
             .shader_register = 0,
             .register_space = 0,
             .visibility = D3D12_SHADER_VISIBILITY_PIXEL},

            {.sampler = trilinear_sampler,
             .shader_register = 1,
             .register_space = 0,
             .visibility = D3D12_SHADER_VISIBILITY_PIXEL},

            {.sampler = shadow_sampler,
             .shader_register = 2,
             .register_space = 0,
             .visibility = D3D12_SHADER_VISIBILITY_PIXEL},
         },

      .flags = D3D12_ROOT_SIGNATURE_FLAG_NONE};

   terrain = create_root_signature(device, terrain_desc);

   const gpu::root_signature_desc meta_object_desc{
      .name = "meta_object_root_signature",

      .parameters =
         {
            // per-object constants
            gpu::root_parameter_cbv{
               .shader_register = 1,
               .register_space = mesh_register_space,
               .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
               .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
            },

            // color constant (should this be a root constant?)
            gpu::root_parameter_cbv{
               .shader_register = 0,
               .register_space = mesh_register_space,
               .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
               .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
            },

            // camera descriptors
            gpu::root_parameter_descriptor_table{
               .ranges =
                  {
                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                      .count = 1,
                      .base_shader_register = 0,
                      .register_space = mesh_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 0},
                  },
               .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
            },
         },

      .flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

   meta_object_mesh = create_root_signature(device, meta_object_desc);

   const gpu::root_signature_desc meta_line_desc{
      .name = "meta_line_root_signature",

      .parameters =
         {
            // color constant (should this be a root constant?)
            gpu::root_parameter_cbv{
               .shader_register = 0,
               .register_space = mesh_register_space,
               .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
               .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
            },

            // camera descriptors
            gpu::root_parameter_descriptor_table{
               .ranges =
                  {
                     {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                      .count = 1,
                      .base_shader_register = 0,
                      .register_space = mesh_register_space,
                      .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                      .offset_in_descriptors_from_table_start = 0},
                  },
               .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
            },
         },

      .flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

   meta_line = create_root_signature(device, meta_line_desc);

   const gpu::root_signature_desc shadow_mesh_desc{
      .name = "shadow_mesh_root_signature",

      .parameters =
         {
            // transform cbv
            gpu::root_parameter_cbv{
               .shader_register = 0,
               .register_space = mesh_register_space,
               .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
               .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
            },

            // camera cbv
            gpu::root_parameter_cbv{
               .shader_register = 1,
               .register_space = mesh_register_space,
               .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
               .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
            },
         },

      .flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

   depth_only_mesh = create_root_signature(device, shadow_mesh_desc);
}

}
