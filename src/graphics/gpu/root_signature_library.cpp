
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
      throw std::runtime_error{
         std::string{static_cast<const char*>(root_signature_blob->GetBufferPointer()),
                     root_signature_blob->GetBufferSize()}};
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
   constexpr D3D12_ROOT_PARAMETER1 global_cb_root_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
      .Descriptor = {.ShaderRegister = 0,
                     .RegisterSpace = 0,
                     .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE},
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX};

   constexpr D3D12_ROOT_PARAMETER1 object_cb_root_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
      .Descriptor = {.ShaderRegister = 1,
                     .RegisterSpace = 0,
                     .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE},
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX};

   constexpr std::array basic_root_params{global_cb_root_param, object_cb_root_param};

   basic_object_mesh = create_root_signature(
      device,
      {.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
       .Desc_1_1 = {.NumParameters = static_cast<UINT>(basic_root_params.size()),
                    .pParameters = basic_root_params.data(),
                    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT}});

   constexpr D3D12_ROOT_PARAMETER1 meta_object_color_cb_root_param{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
      .Descriptor = {.ShaderRegister = 0,
                     .RegisterSpace = 0,
                     .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE},
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL};

   constexpr std::array meta_object_mesh_params{global_cb_root_param,
                                                object_cb_root_param,
                                                meta_object_color_cb_root_param};

   meta_object_mesh = create_root_signature(
      device,
      {.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
       .Desc_1_1 = {.NumParameters = static_cast<UINT>(meta_object_mesh_params.size()),
                    .pParameters = meta_object_mesh_params.data(),
                    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT}});

   constexpr std::array meta_line_params{global_cb_root_param,
                                         meta_object_color_cb_root_param};

   meta_line = create_root_signature(
      device,
      {.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
       .Desc_1_1 = {.NumParameters = static_cast<UINT>(meta_line_params.size()),
                    .pParameters = meta_line_params.data(),
                    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT}});
}

}