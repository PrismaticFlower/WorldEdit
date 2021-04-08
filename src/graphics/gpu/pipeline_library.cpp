
#include "pipeline_library.hpp"
#include "hresult_error.hpp"
#include "root_signature_library.hpp"
#include "shader_library.hpp"

#include <array>
#include <stdexcept>

#include <fmt/format.h>

using namespace std::literals;

namespace sk::graphics::gpu {

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

constexpr D3D12_RENDER_TARGET_BLEND_DESC render_target_premult_alpha_belnd =
   {.BlendEnable = true,
    .LogicOpEnable = false,
    .SrcBlend = D3D12_BLEND_ONE,
    .DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
    .BlendOp = D3D12_BLEND_OP_ADD,
    .SrcBlendAlpha = D3D12_BLEND_ONE,
    .DestBlendAlpha = D3D12_BLEND_ZERO,
    .BlendOpAlpha = D3D12_BLEND_OP_ADD,
    .LogicOp = D3D12_LOGIC_OP_NOOP,
    .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL};

constexpr D3D12_RENDER_TARGET_BLEND_DESC render_target_additive_belnd =
   {.BlendEnable = true,
    .LogicOpEnable = false,
    .SrcBlend = D3D12_BLEND_ONE,
    .DestBlend = D3D12_BLEND_ONE,
    .BlendOp = D3D12_BLEND_OP_ADD,
    .SrcBlendAlpha = D3D12_BLEND_ONE,
    .DestBlendAlpha = D3D12_BLEND_ZERO,
    .BlendOpAlpha = D3D12_BLEND_OP_ADD,
    .LogicOp = D3D12_LOGIC_OP_NOOP,
    .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL};

constexpr D3D12_RENDER_TARGET_BLEND_DESC render_target_alpha_belnd =
   {.BlendEnable = true,
    .LogicOpEnable = false,
    .SrcBlend = D3D12_BLEND_SRC_ALPHA,
    .DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
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

constexpr D3D12_BLEND_DESC blend_premult_alpha =
   {.AlphaToCoverageEnable = false,
    .IndependentBlendEnable = false,
    .RenderTarget = {render_target_premult_alpha_belnd, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled}};

constexpr D3D12_BLEND_DESC blend_alpha =
   {.AlphaToCoverageEnable = false,
    .IndependentBlendEnable = false,
    .RenderTarget = {render_target_alpha_belnd, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled}};

constexpr D3D12_BLEND_DESC blend_additive =
   {.AlphaToCoverageEnable = false,
    .IndependentBlendEnable = false,
    .RenderTarget = {render_target_additive_belnd, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled,
                     render_target_blend_disabled, render_target_blend_disabled}};

constexpr UINT sample_mask_default = 0xffffffff;

constexpr D3D12_RASTERIZER_DESC rasterizer_cull_none =
   {.FillMode = D3D12_FILL_MODE_SOLID,
    .CullMode = D3D12_CULL_MODE_NONE,
    .FrontCounterClockwise = true,
    .DepthBias = 0,
    .DepthBiasClamp = 0.0f,
    .SlopeScaledDepthBias = 0.0f,
    .DepthClipEnable = true,
    .MultisampleEnable = false,
    .AntialiasedLineEnable = false,
    .ForcedSampleCount = 0,
    .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};

constexpr D3D12_RASTERIZER_DESC rasterizer_cull_backfacing =
   {.FillMode = D3D12_FILL_MODE_SOLID,
    .CullMode = D3D12_CULL_MODE_BACK,
    .FrontCounterClockwise = true,
    .DepthBias = 0,
    .DepthBiasClamp = 0.0f,
    .SlopeScaledDepthBias = 0.0f,
    .DepthClipEnable = true,
    .MultisampleEnable = false,
    .AntialiasedLineEnable = false,
    .ForcedSampleCount = 0,
    .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};

constexpr D3D12_RASTERIZER_DESC rasterizer_line_antialiased =
   {.FillMode = D3D12_FILL_MODE_SOLID,
    .CullMode = D3D12_CULL_MODE_NONE,
    .FrontCounterClockwise = true,
    .DepthBias = 0,
    .DepthBiasClamp = 0.0f,
    .SlopeScaledDepthBias = 0.0f,
    .DepthClipEnable = true,
    .MultisampleEnable = false,
    .AntialiasedLineEnable = true,
    .ForcedSampleCount = 0,
    .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};

constexpr D3D12_DEPTH_STENCILOP_DESC stencilop_disabled =
   {.StencilFailOp = D3D12_STENCIL_OP_KEEP,
    .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
    .StencilPassOp = D3D12_STENCIL_OP_KEEP,
    .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS};

constexpr D3D12_DEPTH_STENCIL_DESC depth_stencil_enabled =
   {.DepthEnable = true,
    .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
    .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
    .StencilEnable = false,
    .StencilReadMask = 0,
    .StencilWriteMask = 0,
    .FrontFace = stencilop_disabled,
    .BackFace = stencilop_disabled};

constexpr D3D12_DEPTH_STENCIL_DESC depth_stencil_readonly_enabled =
   {.DepthEnable = true,
    .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
    .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
    .StencilEnable = false,
    .StencilReadMask = 0,
    .StencilWriteMask = 0,
    .FrontFace = stencilop_disabled,
    .BackFace = stencilop_disabled};

constexpr std::array mesh_input_layout_elements =
   {D3D12_INPUT_ELEMENT_DESC{.SemanticName = "POSITION",
                             .SemanticIndex = 0,
                             .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                             .InputSlot = 0,
                             .AlignedByteOffset = 0,
                             .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
    D3D12_INPUT_ELEMENT_DESC{.SemanticName = "NORMAL",
                             .SemanticIndex = 0,
                             .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                             .InputSlot = 1,
                             .AlignedByteOffset = 0,
                             .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
    D3D12_INPUT_ELEMENT_DESC{.SemanticName = "TEXCOORD",
                             .SemanticIndex = 0,
                             .Format = DXGI_FORMAT_R32G32_FLOAT,
                             .InputSlot = 2,
                             .AlignedByteOffset = 0,
                             .InputSlotClass =
                                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA}};

constexpr D3D12_INPUT_LAYOUT_DESC mesh_input_layout =
   {.pInputElementDescs = mesh_input_layout_elements.data(),
    .NumElements = static_cast<UINT>(mesh_input_layout_elements.size())};

constexpr D3D12_INPUT_LAYOUT_DESC depth_only_mesh_input_layout =
   {.pInputElementDescs = mesh_input_layout_elements.data(),
    .NumElements = static_cast<UINT>(1)};

constexpr std::array meta_mesh_input_layout_elements = {
   D3D12_INPUT_ELEMENT_DESC{.SemanticName = "POSITION",
                            .SemanticIndex = 0,
                            .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                            .InputSlot = 0,
                            .AlignedByteOffset = 0,
                            .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA}};

constexpr D3D12_INPUT_LAYOUT_DESC meta_mesh_input_layout =
   {.pInputElementDescs = meta_mesh_input_layout_elements.data(),
    .NumElements = static_cast<UINT>(meta_mesh_input_layout_elements.size())};

auto create_material_pipelines(const std::string_view name, ID3D12Device& device,
                               const shader_library& shader_library,
                               const root_signature_library& root_signature_library)
   -> material_pipelines
{
   material_pipelines pipelines;

   for (auto i = 0u; i < pipelines.size(); ++i) {
      const auto flags = static_cast<material_pipeline_flags>(i);

      const auto ps_name = fmt::format(name, [&] {
         if (are_flags_set(flags, material_pipeline_flags::alpha_cutout)) {
            return "_cutout";
         }
         else if (are_flags_set(flags, material_pipeline_flags::transparent)) {
            return "_transparent";
         }

         return "";
      }());

      const auto blend_state = [&] {
         if (are_flags_set(flags, material_pipeline_flags::additive)) {
            return blend_additive;
         }
         else if (are_flags_set(flags, material_pipeline_flags::transparent)) {
            return blend_premult_alpha;
         }

         return blend_disabled;
      }();

      const auto rasterizer_state =
         are_flags_set(flags, material_pipeline_flags::doublesided)
            ? rasterizer_cull_none
            : rasterizer_cull_backfacing;

      const auto depth_stencil_state =
         are_flags_set(flags, material_pipeline_flags::transparent)
            ? depth_stencil_readonly_enabled
            : depth_stencil_enabled;

      pipelines[i] = create_graphics_pipeline(
         device, {.pRootSignature = root_signature_library.object_mesh.get(),
                  .VS = shader_library["basic_object_meshVS"sv],
                  .PS = shader_library[ps_name],
                  .StreamOutput = stream_output_disabled,
                  .BlendState = blend_state,
                  .SampleMask = sample_mask_default,
                  .RasterizerState = rasterizer_state,
                  .DepthStencilState = depth_stencil_state,
                  .InputLayout = mesh_input_layout,
                  .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                  .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                  .NumRenderTargets = 1,
                  .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
                  .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                  .SampleDesc = {1, 0}});
   }

   return pipelines;
}

}

pipeline_library::pipeline_library(ID3D12Device& device,
                                   const shader_library& shader_library,
                                   const root_signature_library& root_signature_library)
{
   depth_only_mesh = create_graphics_pipeline(
      device, {.pRootSignature = root_signature_library.depth_only_mesh.get(),
               .VS = shader_library["depth_only_meshVS"sv],
               .StreamOutput = stream_output_disabled,
               .BlendState = blend_disabled,
               .SampleMask = sample_mask_default,
               .RasterizerState = rasterizer_cull_none,
               .DepthStencilState = depth_stencil_enabled,
               .InputLayout = depth_only_mesh_input_layout,
               .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
               .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
               .NumRenderTargets = 0,
               .RTVFormats = {},
               .DSVFormat = DXGI_FORMAT_D32_FLOAT,
               .SampleDesc = {1, 0}});

   basic_object_mesh = create_graphics_pipeline(
      device, {.pRootSignature = root_signature_library.object_mesh.get(),
               .VS = shader_library["basic_object_meshVS"sv],
               .PS = shader_library["basic_object_meshPS"sv],
               .StreamOutput = stream_output_disabled,
               .BlendState = blend_disabled,
               .SampleMask = sample_mask_default,
               .RasterizerState = rasterizer_cull_backfacing,
               .DepthStencilState = depth_stencil_enabled,
               .InputLayout = mesh_input_layout,
               .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
               .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
               .NumRenderTargets = 1,
               .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
               .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
               .SampleDesc = {1, 0}});

   basic_mesh_lighting = create_graphics_pipeline(
      device, {.pRootSignature = root_signature_library.object_mesh.get(),
               .VS = shader_library["basic_object_meshVS"sv],
               .PS = shader_library["basic_mesh_lightingPS"sv],
               .StreamOutput = stream_output_disabled,
               .BlendState = blend_disabled,
               .SampleMask = sample_mask_default,
               .RasterizerState = rasterizer_cull_backfacing,
               .DepthStencilState = depth_stencil_enabled,
               .InputLayout = mesh_input_layout,
               .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
               .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
               .NumRenderTargets = 1,
               .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
               .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
               .SampleDesc = {1, 0}});

   terrain_basic = create_graphics_pipeline(
      device, {.pRootSignature = root_signature_library.terrain.get(),
               .VS = shader_library["terrain_patchVS"sv],
               .PS = shader_library["terrain_basicPS"sv],
               .StreamOutput = stream_output_disabled,
               .BlendState = blend_disabled,
               .SampleMask = sample_mask_default,
               .RasterizerState = rasterizer_cull_backfacing,
               .DepthStencilState = depth_stencil_enabled,
               .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
               .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
               .NumRenderTargets = 1,
               .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
               .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
               .SampleDesc = {1, 0}});

   terrain_lighting = create_graphics_pipeline(
      device, {.pRootSignature = root_signature_library.terrain.get(),
               .VS = shader_library["terrain_patchVS"sv],
               .PS = shader_library["terrain_lightingPS"sv],
               .StreamOutput = stream_output_disabled,
               .BlendState = blend_disabled,
               .SampleMask = sample_mask_default,
               .RasterizerState = rasterizer_cull_backfacing,
               .DepthStencilState = depth_stencil_enabled,
               .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
               .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
               .NumRenderTargets = 1,
               .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
               .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
               .SampleDesc = {1, 0}});

   terrain_normal = create_graphics_pipeline(
      device, {.pRootSignature = root_signature_library.terrain.get(),
               .VS = shader_library["terrain_patchVS"sv],
               .PS = shader_library["terrain_normalPS"sv],
               .StreamOutput = stream_output_disabled,
               .BlendState = blend_disabled,
               .SampleMask = sample_mask_default,
               .RasterizerState = rasterizer_cull_backfacing,
               .DepthStencilState = depth_stencil_enabled,
               .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
               .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
               .NumRenderTargets = 1,
               .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
               .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
               .SampleDesc = {1, 0}});

   meta_object_transparent_mesh = create_graphics_pipeline(
      device, {.pRootSignature = root_signature_library.meta_object_mesh.get(),
               .VS = shader_library["meta_object_meshVS"sv],
               .PS = shader_library["meta_object_meshPS"sv],
               .StreamOutput = stream_output_disabled,
               .BlendState = blend_additive,
               .SampleMask = sample_mask_default,
               .RasterizerState = rasterizer_cull_backfacing,
               .DepthStencilState = depth_stencil_readonly_enabled,
               .InputLayout = meta_mesh_input_layout,
               .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
               .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
               .NumRenderTargets = 1,
               .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
               .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
               .SampleDesc = {1, 0}});

   meta_object_mesh_outlined = create_graphics_pipeline(
      device, {.pRootSignature = root_signature_library.meta_object_mesh.get(),
               .VS = shader_library["meta_object_meshVS"sv],
               .PS = shader_library["meta_object_mesh_outlinedPS"sv],
               .GS = shader_library["meta_object_mesh_outlinedGS"sv],
               .StreamOutput = stream_output_disabled,
               .BlendState = blend_disabled,
               .SampleMask = sample_mask_default,
               .RasterizerState = rasterizer_cull_backfacing,
               .DepthStencilState = depth_stencil_enabled,
               .InputLayout = meta_mesh_input_layout,
               .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
               .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
               .NumRenderTargets = 1,
               .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
               .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
               .SampleDesc = {1, 0}});

   meta_line = create_graphics_pipeline(
      device, {.pRootSignature = root_signature_library.meta_line.get(),
               .VS = shader_library["meta_lineVS"sv],
               .PS = shader_library["meta_object_meshPS"sv],
               .StreamOutput = stream_output_disabled,
               .BlendState = blend_alpha,
               .SampleMask = sample_mask_default,
               .RasterizerState = rasterizer_line_antialiased,
               .DepthStencilState = depth_stencil_readonly_enabled,
               .InputLayout = meta_mesh_input_layout,
               .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
               .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
               .NumRenderTargets = 1,
               .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
               .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
               .SampleDesc = {1, 0}});

   normal_mesh = create_material_pipelines("normal{}_meshPS"sv, device,
                                           shader_library, root_signature_library);
}

}
