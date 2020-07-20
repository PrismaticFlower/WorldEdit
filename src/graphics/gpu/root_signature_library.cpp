
#include "root_signature_library.hpp"
#include "hresult_error.hpp"

namespace sk::graphics::gpu {

namespace {

auto create_root_signature(ID3D12Device& device,
                           const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& desc)
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

   return root_sig;
}

}

root_signature_library::root_signature_library(ID3D12Device& device)
{
   const D3D12_DESCRIPTOR_RANGE1 camera_descriptor_table_descriptor_range{
      .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
      .NumDescriptors = 1,
      .BaseShaderRegister = 0,
      .RegisterSpace = 0,
      .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
      .OffsetInDescriptorsFromTableStart = 0};

   const D3D12_ROOT_PARAMETER1 camera_cb_descriptor_table_root_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
      .DescriptorTable =
         {
            .NumDescriptorRanges = 1,
            .pDescriptorRanges = &camera_descriptor_table_descriptor_range,

         },
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX};

   const D3D12_DESCRIPTOR_RANGE1 material_descriptor_table_descriptor_range{
      .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
      .NumDescriptors = 2,
      .BaseShaderRegister = 0,
      .RegisterSpace = 1,
      .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
      .OffsetInDescriptorsFromTableStart = 0};

   const D3D12_ROOT_PARAMETER1 material_descriptor_table_root_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
      .DescriptorTable =
         {
            .NumDescriptorRanges = 1,
            .pDescriptorRanges = &material_descriptor_table_descriptor_range,

         },
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL};

   const D3D12_ROOT_PARAMETER1 object_cb_root_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
      .Descriptor = {.ShaderRegister = 1,
                     .RegisterSpace = 0,
                     .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC},
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX};

   const std::array<D3D12_DESCRIPTOR_RANGE1, 2> object_lights_descriptor_table_descriptor_ranges{
      {{.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
        .NumDescriptors = 1,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
        .OffsetInDescriptorsFromTableStart = 0},
       {.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
        .NumDescriptors = 2,
        .BaseShaderRegister = 0,
        .RegisterSpace = 0,
        .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
        .OffsetInDescriptorsFromTableStart = 1}}};

   const D3D12_ROOT_PARAMETER1 object_lights_descriptor_table_root_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
      .DescriptorTable =
         {
            .NumDescriptorRanges = static_cast<UINT>(
               object_lights_descriptor_table_descriptor_ranges.size()),
            .pDescriptorRanges =
               object_lights_descriptor_table_descriptor_ranges.data(),

         },
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL};

   const std::array basic_root_params{object_cb_root_param,
                                      material_descriptor_table_root_param,
                                      camera_cb_descriptor_table_root_param,
                                      object_lights_descriptor_table_root_param};

   const D3D12_STATIC_SAMPLER_DESC trilinear_static_sampler{
      .Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
      .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
      .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
      .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
      .MipLODBias = 0.0f,
      .MaxAnisotropy = 0,
      .ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS,
      .BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
      .MinLOD = 0.0f,
      .MaxLOD = D3D12_FLOAT32_MAX,
      .ShaderRegister = 0,
      .RegisterSpace = 0,
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL};

   const D3D12_STATIC_SAMPLER_DESC shadow_static_sampler{
      .Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
      .AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
      .AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
      .AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
      .MipLODBias = 0.0f,
      .MaxAnisotropy = 0,
      .ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL,
      .BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
      .MinLOD = 0.0f,
      .MaxLOD = D3D12_FLOAT32_MAX,
      .ShaderRegister = 2,
      .RegisterSpace = 0,
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL};

   const std::array object_static_sampler{trilinear_static_sampler,
                                          shadow_static_sampler};

   object_mesh = create_root_signature(
      device,
      {.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
       .Desc_1_1 = {.NumParameters = static_cast<UINT>(basic_root_params.size()),
                    .pParameters = basic_root_params.data(),
                    .NumStaticSamplers = static_cast<UINT>(object_static_sampler.size()),
                    .pStaticSamplers = object_static_sampler.data(),
                    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT}});

   const D3D12_STATIC_SAMPLER_DESC terrain_bilinear_static_sampler{
      .Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
      .AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
      .AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
      .AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
      .MipLODBias = 0.0f,
      .MaxAnisotropy = 0,
      .ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS,
      .BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
      .MinLOD = 0.0f,
      .MaxLOD = D3D12_FLOAT32_MAX,
      .ShaderRegister = 0,
      .RegisterSpace = 0,
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL};

   const D3D12_STATIC_SAMPLER_DESC terrain_trilinear_static_sampler{
      .Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
      .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
      .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
      .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
      .MipLODBias = 0.0f,
      .MaxAnisotropy = 0,
      .ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS,
      .BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
      .MinLOD = 0.0f,
      .MaxLOD = D3D12_FLOAT32_MAX,
      .ShaderRegister = 1,
      .RegisterSpace = 0,
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL};

   const std::array terrain_static_sampler{terrain_bilinear_static_sampler,
                                           terrain_trilinear_static_sampler,
                                           shadow_static_sampler};

   const D3D12_DESCRIPTOR_RANGE1 terrain_constants_descriptor_range{
      .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
      .NumDescriptors = 1,
      .BaseShaderRegister = 0,
      .RegisterSpace = 2,
      .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
      .OffsetInDescriptorsFromTableStart = 0};

   const D3D12_DESCRIPTOR_RANGE1
      terrain_maps_descriptor_range{.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                    .NumDescriptors = 2,
                                    .BaseShaderRegister = 0,
                                    .RegisterSpace = 2,
                                    .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                                    .OffsetInDescriptorsFromTableStart = 1};

   const std::array terrain_descriptor_table_ranges{terrain_constants_descriptor_range,
                                                    terrain_maps_descriptor_range};

   const D3D12_ROOT_PARAMETER1 terrain_descriptor_table_root_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
      .DescriptorTable =
         {
            .NumDescriptorRanges =
               static_cast<UINT>(terrain_descriptor_table_ranges.size()),
            .pDescriptorRanges = terrain_descriptor_table_ranges.data(),

         },
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL};

   const D3D12_ROOT_PARAMETER1 terrain_patch_srv_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
      .Descriptor = {.ShaderRegister = 2,
                     .RegisterSpace = 2,
                     .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC},
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX};

   const D3D12_DESCRIPTOR_RANGE1 terrain_material_descriptor_table_descriptor_range{
      .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
      .NumDescriptors = 16,
      .BaseShaderRegister = 0,
      .RegisterSpace = 1,
      .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
      .OffsetInDescriptorsFromTableStart = 0};

   const D3D12_ROOT_PARAMETER1 terrain_material_table_root_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
      .DescriptorTable =
         {
            .NumDescriptorRanges = 1,
            .pDescriptorRanges = &terrain_material_descriptor_table_descriptor_range,

         },
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL};

   const std::array terrain_root_params{camera_cb_descriptor_table_root_param,
                                        object_lights_descriptor_table_root_param,
                                        terrain_descriptor_table_root_param,
                                        terrain_patch_srv_param,
                                        terrain_material_table_root_param};

   terrain = create_root_signature(
      device,
      {.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
       .Desc_1_1 = {.NumParameters = static_cast<UINT>(terrain_root_params.size()),
                    .pParameters = terrain_root_params.data(),
                    .NumStaticSamplers =
                       static_cast<UINT>(terrain_static_sampler.size()),
                    .pStaticSamplers = terrain_static_sampler.data(),
                    .Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE}});

   const D3D12_ROOT_PARAMETER1 meta_object_color_cb_root_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
      .Descriptor = {.ShaderRegister = 0,
                     .RegisterSpace = 0,
                     .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE},
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL};

   const std::array meta_object_mesh_params{object_cb_root_param,
                                            meta_object_color_cb_root_param,
                                            camera_cb_descriptor_table_root_param};

   meta_object_mesh = create_root_signature(
      device,
      {.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
       .Desc_1_1 = {.NumParameters = static_cast<UINT>(meta_object_mesh_params.size()),
                    .pParameters = meta_object_mesh_params.data(),
                    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT}});

   const std::array meta_line_params{meta_object_color_cb_root_param,
                                     camera_cb_descriptor_table_root_param};

   meta_line = create_root_signature(
      device,
      {.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
       .Desc_1_1 = {.NumParameters = static_cast<UINT>(meta_line_params.size()),
                    .pParameters = meta_line_params.data(),
                    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT}});

   const std::array depth_only_mesh_root_params{
      D3D12_ROOT_PARAMETER1{.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                            .Descriptor = {.ShaderRegister = 0,
                                           .RegisterSpace = 0,
                                           .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC},
                            .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX},

      D3D12_ROOT_PARAMETER1{.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                            .Descriptor = {.ShaderRegister = 1,
                                           .RegisterSpace = 0,
                                           .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC},
                            .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX}};

   depth_only_mesh = create_root_signature(
      device,
      {.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
       .Desc_1_1 = {.NumParameters =
                       static_cast<UINT>(depth_only_mesh_root_params.size()),
                    .pParameters = depth_only_mesh_root_params.data(),
                    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT}});
}

}