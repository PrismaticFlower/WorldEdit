
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
   basic_test = create_root_signature(
      device,
      {.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
       .Desc_1_1 = {.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT}});
}

}