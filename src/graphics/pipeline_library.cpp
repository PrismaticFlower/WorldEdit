
#include "pipeline_library.hpp"
#include "gpu/set_debug_name.hpp"
#include "hresult_error.hpp"
#include "io/error.hpp"
#include "io/output_file.hpp"
#include "io/read_file.hpp"
#include "root_signature_library.hpp"
#include "shader_library.hpp"

#include <array>
#include <stdexcept>

using namespace std::literals;

namespace we::graphics {

namespace {

constexpr auto library_disk_cache_path = L"pipeline library.bin"sv;

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

constexpr D3D12_RASTERIZER_DESC rasterizer_conservative_cull_frontfacing =
   {.FillMode = D3D12_FILL_MODE_SOLID,
    .CullMode = D3D12_CULL_MODE_FRONT,
    .FrontCounterClockwise = true,
    .DepthBias = 0,
    .DepthBiasClamp = 0.0f,
    .SlopeScaledDepthBias = 0.0f,
    .DepthClipEnable = true,
    .MultisampleEnable = false,
    .AntialiasedLineEnable = false,
    .ForcedSampleCount = 0,
    .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON};

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

constexpr D3D12_DEPTH_STENCIL_DESC depth_stencil_readonly_equal =
   {.DepthEnable = true,
    .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
    .DepthFunc = D3D12_COMPARISON_FUNC_EQUAL,
    .StencilEnable = false,
    .StencilReadMask = 0,
    .StencilWriteMask = 0,
    .FrontFace = stencilop_disabled,
    .BackFace = stencilop_disabled};

constexpr D3D12_DEPTH_STENCIL_DESC depth_stencil_readonly_less_equal =
   {.DepthEnable = true,
    .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
    .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
    .StencilEnable = false,
    .StencilReadMask = 0,
    .StencilWriteMask = 0,
    .FrontFace = stencilop_disabled,
    .BackFace = stencilop_disabled};

constexpr D3D12_DEPTH_STENCIL_DESC depth_stencil_disabled =
   {.DepthEnable = false,
    .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
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
                             .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                             .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
    D3D12_INPUT_ELEMENT_DESC{.SemanticName = "TANGENT",
                             .SemanticIndex = 0,
                             .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                             .InputSlot = 1,
                             .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                             .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
    D3D12_INPUT_ELEMENT_DESC{.SemanticName = "BITANGENT",
                             .SemanticIndex = 0,
                             .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                             .InputSlot = 1,
                             .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                             .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
    D3D12_INPUT_ELEMENT_DESC{.SemanticName = "TEXCOORD",
                             .SemanticIndex = 0,
                             .Format = DXGI_FORMAT_R32G32_FLOAT,
                             .InputSlot = 1,
                             .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                             .InputSlotClass =
                                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA}};

constexpr D3D12_INPUT_LAYOUT_DESC mesh_input_layout =
   {.pInputElementDescs = mesh_input_layout_elements.data(),
    .NumElements = static_cast<UINT>(mesh_input_layout_elements.size())};

constexpr D3D12_INPUT_LAYOUT_DESC position_only_mesh_input_layout =
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

auto create_material_pipelines(std::string_view pixel_shader_name_base,
                               std::wstring_view pipeline_name_base,
                               pipeline_library& library,
                               const shader_library& shader_library,
                               const root_signature_library& root_signature_library)
   -> material_pipelines
{
   material_pipelines pipelines;

   for (auto i = 0u; i < pipelines.size(); ++i) {
      const auto flags = static_cast<material_pipeline_flags>(i);

      if (are_flags_set(flags, material_pipeline_flags::alpha_cutout)) continue;

      std::string pixel_shader_name{pixel_shader_name_base};

      pixel_shader_name += are_flags_set(flags, material_pipeline_flags::transparent)
                              ? "_transparent"sv
                              : ""sv;
      pixel_shader_name += "PS"sv;

      std::wstring pipeline_name{pipeline_name_base};

      if (are_flags_set(flags, material_pipeline_flags::additive)) {
         pipeline_name += L"_additive"sv;
      }

      if (are_flags_set(flags, material_pipeline_flags::transparent)) {
         pipeline_name += L"_transparent"sv;
      }

      if (are_flags_set(flags, material_pipeline_flags::doublesided)) {
         pipeline_name += L"_doublesided"sv;
      }

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
            ? depth_stencil_readonly_less_equal
            : depth_stencil_readonly_equal;

      pipelines[i] = library.create_graphics_pipeline(
         pipeline_name.data(),
         {.pRootSignature = root_signature_library.mesh.get(),
          .VS = shader_library["meshVS"sv],
          .PS = shader_library[pixel_shader_name],
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

      // Duplicate to alpha cutout pipeline.
      pipelines[flags | material_pipeline_flags::alpha_cutout] = pipelines[i];
   }

   return pipelines;
}

auto read_disk_cache_data() -> std::vector<std::byte>
{
   try {
      return io::read_file_to_bytes(library_disk_cache_path);
   }
   catch (io::error&) {
      return {};
   }
}

void save_disk_cache_data(ID3D12PipelineLibrary1& library)
{
   std::vector<std::byte> library_bytes;
   library_bytes.resize(library.GetSerializedSize());

   library.Serialize(library_bytes.data(),
                     library_bytes.size()); // TODO: Performance? Write out directly to a memory mapped file.

   io::output_file{library_disk_cache_path}.write(library_bytes);
}

auto make_d3d12_pipeline_library(ID3D12Device10& device,
                                 std::span<const std::byte> disk_cache_data)
   -> utility::com_ptr<ID3D12PipelineLibrary1>
{
   utility::com_ptr<ID3D12PipelineLibrary1> library;

   if (not disk_cache_data.empty()) {
      if (SUCCEEDED(
             device.CreatePipelineLibrary(disk_cache_data.data(),
                                          disk_cache_data.size(),
                                          IID_PPV_ARGS(library.clear_and_assign())))) {
         return library;
      }
   }

   if (FAILED(device.CreatePipelineLibrary(nullptr, 0,
                                           IID_PPV_ARGS(library.clear_and_assign())))) {
      throw std::runtime_error{"Failed to make ID3D12PipelineLibrary1!"};
   }

   return library;
}

}

pipeline_library::pipeline_library(utility::com_ptr<ID3D12Device10> device,
                                   const shader_library& shader_library,
                                   const root_signature_library& root_signature_library)
   : device{device},
     library_disk_cache_data{read_disk_cache_data()},
     library{make_d3d12_pipeline_library(*device, library_disk_cache_data)}
{

   mesh_shadow =
      create_graphics_pipeline(L"mesh_shadow",
                               {.pRootSignature =
                                   root_signature_library.mesh_shadow.get(),
                                .VS = shader_library["mesh_shadowVS"sv],
                                .StreamOutput = stream_output_disabled,
                                .BlendState = blend_disabled,
                                .SampleMask = sample_mask_default,
                                .RasterizerState = rasterizer_cull_none,
                                .DepthStencilState = depth_stencil_enabled,
                                .InputLayout = position_only_mesh_input_layout,
                                .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                                .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                                .NumRenderTargets = 0,
                                .RTVFormats = {},
                                .DSVFormat = DXGI_FORMAT_D32_FLOAT,
                                .SampleDesc = {1, 0}});

   mesh_depth_prepass =
      create_graphics_pipeline(L"mesh_depth_prepass",
                               {.pRootSignature =
                                   root_signature_library.mesh_depth_prepass.get(),
                                .VS = shader_library["mesh_depth_prepassVS"sv],
                                .StreamOutput = stream_output_disabled,
                                .BlendState = blend_disabled,
                                .SampleMask = sample_mask_default,
                                .RasterizerState = rasterizer_cull_backfacing,
                                .DepthStencilState = depth_stencil_enabled,
                                .InputLayout = position_only_mesh_input_layout,
                                .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                                .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                                .NumRenderTargets = 0,
                                .RTVFormats = {},
                                .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                .SampleDesc = {1, 0}});

   mesh_depth_prepass_doublesided =
      create_graphics_pipeline(L"mesh_depth_prepass_doublesided",
                               {.pRootSignature =
                                   root_signature_library.mesh_depth_prepass.get(),
                                .VS = shader_library["mesh_depth_prepassVS"sv],
                                .StreamOutput = stream_output_disabled,
                                .BlendState = blend_disabled,
                                .SampleMask = sample_mask_default,
                                .RasterizerState = rasterizer_cull_none,
                                .DepthStencilState = depth_stencil_enabled,
                                .InputLayout = position_only_mesh_input_layout,
                                .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                                .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                                .NumRenderTargets = 0,
                                .RTVFormats = {},
                                .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                .SampleDesc = {1, 0}});

   mesh_depth_prepass_alpha_cutout =
      create_graphics_pipeline(L"mesh_depth_prepass_alpha_cutout",
                               {.pRootSignature =
                                   root_signature_library.mesh_depth_prepass.get(),
                                .VS = shader_library["meshVS"sv],
                                .PS = shader_library["mesh_depth_cutoutPS"sv],
                                .StreamOutput = stream_output_disabled,
                                .BlendState = blend_disabled,
                                .SampleMask = sample_mask_default,
                                .RasterizerState = rasterizer_cull_backfacing,
                                .DepthStencilState = depth_stencil_enabled,
                                .InputLayout = mesh_input_layout,
                                .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                                .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                                .NumRenderTargets = 0,
                                .RTVFormats = {},
                                .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                .SampleDesc = {1, 0}});

   mesh_depth_prepass_alpha_cutout_doublesided =
      create_graphics_pipeline(L"mesh_depth_prepass_alpha_cutout_doublesided",
                               {.pRootSignature =
                                   root_signature_library.mesh_depth_prepass.get(),
                                .VS = shader_library["meshVS"sv],
                                .PS = shader_library["mesh_depth_cutoutPS"sv],
                                .StreamOutput = stream_output_disabled,
                                .BlendState = blend_disabled,
                                .SampleMask = sample_mask_default,
                                .RasterizerState = rasterizer_cull_backfacing,
                                .DepthStencilState = depth_stencil_enabled,
                                .InputLayout = mesh_input_layout,
                                .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                                .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                                .NumRenderTargets = 0,
                                .RTVFormats = {},
                                .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                .SampleDesc = {1, 0}});

   mesh_basic =
      create_graphics_pipeline(L"mesh_basic",
                               {.pRootSignature = root_signature_library.mesh.get(),
                                .VS = shader_library["meshVS"sv],
                                .PS = shader_library["mesh_basicPS"sv],
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

   mesh_basic_lighting =
      create_graphics_pipeline(L"mesh_basic_lighting",
                               {.pRootSignature = root_signature_library.mesh.get(),
                                .VS = shader_library["meshVS"sv],
                                .PS = shader_library["mesh_basic_lightingPS"sv],
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

   mesh_normal = create_material_pipelines("mesh_normal", L"mesh_normal", *this,
                                           shader_library, root_signature_library);

   terrain_depth_prepass =
      create_graphics_pipeline(L"terrain_depth_prepass",
                               {.pRootSignature = root_signature_library.terrain.get(),
                                .VS = shader_library["terrain_patchVS"sv],
                                .StreamOutput = stream_output_disabled,
                                .BlendState = blend_disabled,
                                .SampleMask = sample_mask_default,
                                .RasterizerState = rasterizer_cull_backfacing,
                                .DepthStencilState = depth_stencil_enabled,
                                .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                                .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                                .NumRenderTargets = 0,
                                .RTVFormats = {},
                                .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                .SampleDesc = {1, 0}});

   terrain_basic =
      create_graphics_pipeline(L"terrain_basic",
                               {.pRootSignature = root_signature_library.terrain.get(),
                                .VS = shader_library["terrain_patchVS"sv],
                                .PS = shader_library["terrain_basicPS"sv],
                                .StreamOutput = stream_output_disabled,
                                .BlendState = blend_disabled,
                                .SampleMask = sample_mask_default,
                                .RasterizerState = rasterizer_cull_backfacing,
                                .DepthStencilState = depth_stencil_readonly_equal,
                                .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                                .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                                .NumRenderTargets = 1,
                                .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
                                .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                .SampleDesc = {1, 0}});

   terrain_lighting =
      create_graphics_pipeline(L"terrain_lighting",
                               {.pRootSignature = root_signature_library.terrain.get(),
                                .VS = shader_library["terrain_patchVS"sv],
                                .PS = shader_library["terrain_lightingPS"sv],
                                .StreamOutput = stream_output_disabled,
                                .BlendState = blend_disabled,
                                .SampleMask = sample_mask_default,
                                .RasterizerState = rasterizer_cull_backfacing,
                                .DepthStencilState = depth_stencil_readonly_equal,
                                .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                                .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                                .NumRenderTargets = 1,
                                .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
                                .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                .SampleDesc = {1, 0}});

   terrain_normal =
      create_graphics_pipeline(L"terrain_normal",
                               {.pRootSignature = root_signature_library.terrain.get(),
                                .VS = shader_library["terrain_patchVS"sv],
                                .PS = shader_library["terrain_normalPS"sv],
                                .StreamOutput = stream_output_disabled,
                                .BlendState = blend_disabled,
                                .SampleMask = sample_mask_default,
                                .RasterizerState = rasterizer_cull_backfacing,
                                .DepthStencilState = depth_stencil_readonly_equal,
                                .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                                .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                                .NumRenderTargets = 1,
                                .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
                                .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                .SampleDesc = {1, 0}});

   meta_mesh = create_graphics_pipeline(
      L"meta_mesh", {.pRootSignature = root_signature_library.meta_mesh.get(),
                     .VS = shader_library["meta_meshVS"sv],
                     .PS = shader_library["meta_meshPS"sv],
                     .StreamOutput = stream_output_disabled,
                     .BlendState = blend_additive,
                     .SampleMask = sample_mask_default,
                     .RasterizerState = rasterizer_cull_backfacing,
                     .DepthStencilState = depth_stencil_readonly_less_equal,
                     .InputLayout = meta_mesh_input_layout,
                     .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                     .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                     .NumRenderTargets = 1,
                     .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
                     .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                     .SampleDesc = {1, 0}});

   meta_mesh_outlined =
      create_graphics_pipeline(L"meta_mesh_outlined",
                               {.pRootSignature =
                                   root_signature_library.meta_mesh.get(),
                                .VS = shader_library["meta_meshVS"sv],
                                .PS = shader_library["meta_mesh_outlinedPS"sv],
                                .GS = shader_library["meta_mesh_outlinedGS"sv],
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
      L"meta_line", {.pRootSignature = root_signature_library.meta_line.get(),
                     .VS = shader_library["meta_lineVS"sv],
                     .PS = shader_library["meta_meshPS"sv],
                     .StreamOutput = stream_output_disabled,
                     .BlendState = blend_alpha,
                     .SampleMask = sample_mask_default,
                     .RasterizerState = rasterizer_line_antialiased,
                     .DepthStencilState = depth_stencil_readonly_less_equal,
                     .InputLayout = meta_mesh_input_layout,
                     .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                     .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
                     .NumRenderTargets = 1,
                     .RTVFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},
                     .DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                     .SampleDesc = {1, 0}});

   tile_lights_clear =
      create_compute_pipeline(L"tile_lights_clear",
                              {.pRootSignature =
                                  root_signature_library.tile_lights_clear.get(),
                               .CS = shader_library["tile_lights_clearCS"sv]});

   tile_lights_spheres = create_graphics_pipeline(
      L"tile_lights_spheres",
      {.pRootSignature = root_signature_library.tile_lights.get(),
       .VS = shader_library["tile_lightsVS"sv],
       .PS = shader_library["tile_lightsPS"sv],
       .StreamOutput = stream_output_disabled,
       .BlendState = blend_disabled,
       .SampleMask = sample_mask_default,
       .RasterizerState = rasterizer_conservative_cull_frontfacing,
       .DepthStencilState = depth_stencil_disabled,
       .InputLayout = meta_mesh_input_layout,
       .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
       .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
       .NumRenderTargets = 0,
       .RTVFormats = {},
       .DSVFormat = DXGI_FORMAT_UNKNOWN,
       .SampleDesc = {1, 0}});
}

pipeline_library::~pipeline_library()
{
   try {
      save_disk_cache_data(*library);
   }
   catch (std::exception&) {
      // Failure to save the pipeline library can be safely ignored.
   }
}

auto pipeline_library ::create_graphics_pipeline(
   const wchar_t* name, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
   -> utility::com_ptr<ID3D12PipelineState>
{
   utility::com_ptr<ID3D12PipelineState> pso;

   if (SUCCEEDED(library->LoadGraphicsPipeline(name, &desc,
                                               IID_PPV_ARGS(pso.clear_and_assign())))) {
      return pso;
   }

   throw_if_failed(
      device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pso.clear_and_assign())));

   library->StorePipeline(name, pso.get());

   return pso;
}

auto pipeline_library::create_compute_pipeline(const wchar_t* name,
                                               const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc)
   -> utility::com_ptr<ID3D12PipelineState>
{
   utility::com_ptr<ID3D12PipelineState> pso;

   if (SUCCEEDED(library->LoadComputePipeline(name, &desc,
                                              IID_PPV_ARGS(pso.clear_and_assign())))) {
      return pso;
   }

   throw_if_failed(
      device->CreateComputePipelineState(&desc, IID_PPV_ARGS(pso.clear_and_assign())));

   library->StorePipeline(name, pso.get());

   return pso;
}
}
