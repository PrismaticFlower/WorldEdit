
#include "pipeline_library.hpp"
#include "hresult_error.hpp"
#include "root_signature_library.hpp"
#include "shader_library.hpp"

#include <array>
#include <stdexcept>

namespace sk::graphics {

namespace {

auto create_graphics_pipeline(ID3D12Device& device,
                              const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
   -> utility::com_ptr<ID3D12PipelineState>
{
   utility::com_ptr<ID3D12PipelineState> pso;

   throw_if_failed(
      device.CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pso.clear_and_assign())));

   return pso;
}

constexpr D3D12_STREAM_OUTPUT_DESC stream_output_disabled{.pSODeclaration = nullptr,
                                                          .NumEntries = 0,
                                                          .pBufferStrides = nullptr,
                                                          .NumStrides = 0,
                                                          .RasterizedStream = 0};

constexpr D3D12_RENDER_TARGET_BLEND_DESC render_target_blend_disabled =
   {.BlendEnable = false,
    .LogicOpEnable = false,
    .SrcBlend = D3D12_BLEND_ONE,
    .DestBlend = D3D12_BLEND_ZERO,
    .BlendOp = D3D12_BLEND_OP_ADD,
    .SrcBlendAlpha = D3D12_BLEND_ONE,
    .DestBlendAlpha = D3D12_BLEND_ZERO,
    .BlendOpAlpha = D3D12_BLEND_OP_ADD,
    .LogicOp = D3D12_LOGIC_OP_NOOP,
    .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL};

constexpr D3D12_BLEND_DESC blend_disabled =
   {.AlphaToCoverageEnable = false,
    .IndependentBlendEnable = false,
    .RenderTarget = {render_target_blend_disabled, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled}};

constexpr UINT sample_mask_default = 0xffffffff;

constexpr D3D12_RASTERIZER_DESC rasterizer_cull_backfacing =
   {.FillMode = D3D12_FILL_MODE_SOLID,
    .CullMode = D3D12_CULL_MODE_BACK,
    .FrontCounterClockwise = false,
    .DepthBias = 0,
    .DepthBiasClamp = 0.0f,
    .SlopeScaledDepthBias = 0.0f,
    .DepthClipEnable = true,
    .MultisampleEnable = false,
    .AntialiasedLineEnable = false,
    .ForcedSampleCount = 0,
    .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};

constexpr D3D12_DEPTH_STENCILOP_DESC stencilop_disabled =
   {.StencilFailOp = D3D12_STENCIL_OP_KEEP,
    .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
    .StencilPassOp = D3D12_STENCIL_OP_KEEP,
    .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS};

constexpr D3D12_DEPTH_STENCIL_DESC depth_stencil_disabled =
   {.DepthEnable = false,
    .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
    .DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS,
    .StencilEnable = false,
    .StencilReadMask = 0,
    .StencilWriteMask = 0,
    .FrontFace = stencilop_disabled,
    .BackFace = stencilop_disabled};

constexpr std::array mesh_input_layout_elements = {
   D3D12_INPUT_ELEMENT_DESC{.SemanticName = "POSITION",
                            .SemanticIndex = 0,
                            .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                            .InputSlot = 0,
                            .AlignedByteOffset = 0,
                            .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA}};

constexpr D3D12_INPUT_LAYOUT_DESC mesh_input_layout =
   {.pInputElementDescs = mesh_input_layout_elements.data(),
    .NumElements = static_cast<UINT>(mesh_input_layout_elements.size())};

}

pipeline_library::pipeline_library(ID3D12Device& device,
                                   const root_signature_library& root_signature_library)
{
   basic_test = create_graphics_pipeline(
      device, {.pRootSignature = root_signature_library.basic_test.get(),
               .VS = shader_library::basic_test_vs,
               .PS = shader_library::basic_test_ps,
               .StreamOutput = stream_output_disabled,
               .BlendState = blend_disabled,
               .SampleMask = sample_mask_default,
               .RasterizerState = rasterizer_cull_backfacing,
               .DepthStencilState = depth_stencil_disabled,
               .InputLayout = mesh_input_layout,
               .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
               .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
               .NumRenderTargets = 1,
               .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
               .DSVFormat = DXGI_FORMAT_UNKNOWN,
               .SampleDesc = {1, 0}});
}

}
