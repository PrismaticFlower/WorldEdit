#pragma once

#include "types.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <span>

#include <dxgiformat.h>

namespace we::graphics::gpu {

/// Constants ///

constexpr uint32 append_aligned_input_element = 0xffffffffu;
constexpr uint32 resource_barrier_all_subresources = 0xffffffffu;

/// Enums & Typedefs ///

using gpu_virtual_address = uint64;

enum class heap_type {
   default_ = 1,
   upload = 2,
   readback = 3,
};

enum class resource_state {
   common = 0,
   vertex_and_constant_buffer = 0x1,
   index_buffer = 0x2,
   render_target = 0x4,
   unordered_access = 0x8,
   depth_write = 0x10,
   depth_read = 0x20,
   non_pixel_shader_resource = 0x40,
   pixel_shader_resource = 0x80,
   indirect_argument = 0x200,
   copy_dest = 0x400,
   copy_source = 0x800,
   resolve_dest = 0x1000,
   resolve_source = 0x2000,
   raytracing_acceleration_structure = 0x400000,
   shading_rate_source = 0x1000000,
   generic_read = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
   all_shader_resource = (0x40 | 0x80),
   present = 0,
   predication = 0x200
};

enum class texture_dimension { t_1d, t_2d, t_3d, t_cube };

enum class uav_dimension {
   buffer = 1,
   texture1d = 2,
   texture1d_array = 3,
   texture2d = 4,
   texture2d_array = 5,
   texture3d = 8
};

enum class rtv_dimension {
   texture1d = 2,
   texture1d_array = 3,
   texture2d = 4,
   texture2d_array = 5,
   texture2d_ms = 6,
   texture2d_ms_array = 7,
   texture3d = 8
};

enum class dsv_dimension {
   texture1d = 1,
   texture1d_array = 2,
   texture2d = 3,
   texture2d_array = 4,
   texture2d_ms = 5,
   texture2d_ms_array = 6,
};

enum class root_shader_visibility {
   all = 0,
   vertex = 1,
   pixel = 5,
   amplification = 6,
   mesh = 7
};

enum class root_parameter_type {
   constant_buffer_view,
   _32bit_constants,
   constant_buffer_view_address,
   shader_resource_view_address,
   unordered_access_view_address,

   paramters_end
};

enum class render_target_blend {
   disabled,            // overwrite with src
   premult_alpha_blend, // src + (dest * (1.0 - src_alpha))
   additive_blend,      // src + dest
   alpha_belnd          // (src * src_alpha) + (dest * (1.0 - src_alpha))
};

enum class cull_mode {
   none = 1,
   front = 2, // Counter Clockwise
   back = 3,  // Clockwise
};

enum class depth_test_func {
   equal = 3,
   not_equal = 6,
   less = 2,
   less_equal = 4,
   greater = 5,
   greater_equal = 7,
};

enum class primitive_type {
   point = 1,
   line = 2,
   triangle = 3,
};

enum class primitive_topology {
   undefined = 0,
   pointlist = 1,
   linelist = 2,
   trianglelist = 4
};

enum class barrier_split { none, begin, end };

enum class barrier_type { transition = 0, aliasing = 1, uav = 2, uav_any = -1 };

enum class filter {
   point = 0,
   bilinear = 0x14,
   trilinear = 0x15,
   anisotropic = 0x55,

   comparison_point = 0x80,
   comparison_bilinear_mip = 0x94,
   comparison_trilinear = 0x95,
   comparison_anisotropic = 0xd5,

   minimum_point = 0x100,
   minimum_bilinear = 0x114,
   minimum_trilinear = 0x115,
   minimum_anisotropic = 0x155,

   maximum_point = 0x180,
   maximum_bilinear = 0x194,
   maximum_trilinear = 0x195,
   maximum_anisotropic = 0x1d5
};

enum class texture_address_mode {
   wrap = 1,
   mirror = 2,
   clamp = 3,
   border = 4,
   mirror_once = 5
};

enum class comparison_mode {
   none = 8,
   equal = 3,
   not_equal = 6,
   less = 2,
   less_equal = 4,
   greater = 5,
   greater_equal = 7,
};

struct memory_range {
   std::size_t begin = 0;
   std::size_t end = 0;
};

struct rect {
   uint32 left;
   uint32 top;
   uint32 right;
   uint32 bottom;
};

struct box {
   uint32 left;
   uint32 top;
   uint32 front;
   uint32 right;
   uint32 bottom;
   uint32 back;
};

struct debug_name {
   debug_name() = default;

   debug_name(const char* name) noexcept : name{name} {}

   debug_name(std::nullptr_t) noexcept = delete;

   [[nodiscard]] auto c_str() const noexcept -> const char*
   {
      return name;
   }

private:
   const char* name = "";
};

/// GPU Data Handles ///

enum class resource_handle : std::uintptr_t {};
enum class cbv_handle : std::uintptr_t {};
enum class rtv_handle : std::uintptr_t {};
enum class dsv_handle : std::uintptr_t {};
enum class resource_view_handle : std::uint32_t {};

struct resource_view {
   uint32 index = 0xffffffffu;
};

enum class root_signature_handle : std::uintptr_t {};
enum class pipeline_handle : std::uintptr_t {};

enum class sampler_heap_handle : std::uintptr_t {};

/// Pipeline Desc Structures ///

struct root_parameter {
   root_parameter_type type = root_parameter_type::paramters_end;
   uint32 shader_register = 0;
   uint32 register_space = 0;

   /// Number of 32-bit constants for root_parameter_type::_32bit_constants
   uint32 values_count = 0;

   root_shader_visibility visibility = root_shader_visibility::all;
};

struct root_signature_flags {
   bool allow_input_assembler_input_layout : 1 = false;
   bool deny_vertex_shader_root_access : 1 = false;
   bool deny_pixel_shader_root_access : 1 = false;
   bool deny_amplification_shader_root_access : 1 = false;
   bool deny_mesh_shader_root_access : 1 = false;
};

struct root_signature_desc {
   std::array<root_parameter, 8> parameters;

   root_signature_flags flags;

   debug_name debug_name;
};

struct blend_state_desc {
   bool alpha_to_coverage_enabled = false;
   bool independent_blend_enabled = false;

   std::array<render_target_blend, 8> render_target{};
};

struct rasterizer_state_desc {
   cull_mode cull_mode = cull_mode::back;

   bool antialiased_lines = false;
   bool conservative_raster = false;
};

struct depth_stencil_state_desc {
   bool depth_test_enabled = false;
   depth_test_func depth_test_func = depth_test_func::less;
   bool write_depth = true;
};

struct input_element_desc {
   const char* semantic_name = "";
   uint32 semantic_index = 0;
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uint32 input_slot = 0;
   uint32 aligned_byte_offset = append_aligned_input_element;
};

struct graphics_pipeline_desc {
   root_signature_handle root_signature;
   std::span<const std::byte> vs_bytecode;
   std::span<const std::byte> ps_bytecode;
   std::span<const std::byte> gs_bytecode;

   blend_state_desc blend_state{};
   rasterizer_state_desc rasterizer_state{};
   depth_stencil_state_desc depth_test_state{};
   std::span<const input_element_desc> input_layout;
   primitive_type primitive_type = primitive_type::triangle;

   uint32 render_target_count = 0;
   std::array<DXGI_FORMAT, 8> rtv_formats{};
   DXGI_FORMAT dsv_format = DXGI_FORMAT_UNKNOWN;

   uint32 sample_count = 1;

   debug_name debug_name;
};

struct compute_pipeline_desc {
   root_signature_handle root_signature;
   std::span<std::byte> cs_bytecode;

   debug_name debug_name;
};

/// Resource Desc Structures ///

struct texture_flags {
   bool allow_render_target : 1 = false;
   bool allow_depth_stencil : 1 = false;
   bool allow_unordered_access : 1 = false;
   bool deny_shader_resource : 1 = false;
};

struct texture_clear_value {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

   union {
      float4 color = {0.0f, 0.0f, 0.0f, 0.0f};

      struct depth_stencil {
         float depth = 1.0f;
         uint8 stencil = 0x0;
      } depth_stencil;
   };
};

struct buffer_desc {
   uint32 alignment = 0;
   uint64 size = 0;

   debug_name debug_name;
};

struct texture_desc {
   texture_dimension dimension = texture_dimension::t_1d;
   texture_flags flags = {};

   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uint32 width = 1;
   uint32 height = 1;
   uint32 depth = 1;
   uint32 mip_levels = 1;
   uint32 array_size = 1;
   uint32 sample_count = 1;

   texture_clear_value optimized_clear_value = {.format = DXGI_FORMAT_UNKNOWN};

   debug_name debug_name;
};

/// Resource View Desc Structures ///

struct constant_buffer_view_desc {
   gpu_virtual_address location = 0;
   uint64 size = 0;
};

struct shader_resource_view_desc {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

   struct {
      uint32 first_element = 0;
      uint32 number_elements = 0;
      uint32 structure_byte_stride = 0;
      bool raw_buffer = true;
   } buffer;
};

struct unordered_access_view_desc {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uav_dimension dimension = uav_dimension::buffer;

   union {
      struct buffer {
         uint32 first_element = 0;
         uint32 number_elements = 0;
         uint32 structure_byte_stride = 0;
         bool raw_buffer = true;

         const resource_handle* const counter_resource = nullptr;
         uint64 counter_offset_in_bytes = 0;
      } buffer = {};

      struct texture1d_uav {
         uint32 mip_slice = 0;
      } texture1d;

      struct texture1d_array_uav {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture1d_array;

      struct texture2d_uav {
         uint32 mip_slice = 0;
         uint32 plane_slice = 0;
      } texture2d;

      struct texture2d_array_uav {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
         uint32 plane_slice = 0;
      } texture2d_array;

      struct texture3d_uav {
         uint32 mip_slice = 0;
         uint32 first_w_slice = 0;
         uint32 w_size = 0;
      } texture3d;
   };
};

struct render_target_view_desc {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   rtv_dimension dimension = rtv_dimension::texture2d;

   union {
      struct texture1d_rtv {
         uint32 mip_slice = 0;
      } texture1d;

      struct texture1d_array_rtv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture1d_array;

      struct texture2d_rtv {
         uint32 mip_slice = 0;
      } texture2d;

      struct texture2d_array_rtv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture2d_array;

      struct texture2d_ms_rtv {
         uint32 mip_slice = 0;
      } texture2d_ms;

      struct texture2d_ms_array_rtv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture2d_ms_array;

      struct texture3d_rtv {
         uint32 mip_slice = 0;
         uint32 first_w_slice = 0;
         uint32 w_size = 0;
      } texture3d;
   };
};

struct depth_stencil_view_desc {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   dsv_dimension dimension = dsv_dimension::texture2d;

   union {
      struct texture1d_dsv {
         uint32 mip_slice = 0;
      } texture1d;

      struct texture1d_array_dsv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture1d_array;

      struct texture2d_dsv {
         uint32 mip_slice = 0;
      } texture2d;

      struct texture2d_array_dsv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture2d_array;

      struct texture2d_ms_dsv {
         uint32 mip_slice = 0;
      } texture2d_ms;

      struct texture2d_ms_array_dsv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture2d_ms_array;
   };
};

/// Sampler Desc Structures ///

struct sampler_desc {
   filter filter;
   texture_address_mode address_u = texture_address_mode::wrap;
   texture_address_mode address_v = texture_address_mode::wrap;
   texture_address_mode address_w = texture_address_mode::wrap;
   float mip_lod_bias = 0.0f;
   uint32 max_anisotropy = 1;
   comparison_mode comparison = comparison_mode::none;
   float4 BorderColor = {0.0f, 0.0f, 0.0f, 0.0f};
};

/// Swapchain Desc Structure ///

struct swap_chain_desc {
   using window_handle = void*;

   /// @brief HWND for the swap chain. Can not be left nullptr. The size of the swap chain will be derived from the window size.
   window_handle window = nullptr;

   /// @brief The format to use for the swap chain buffers.
   DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;

   /// @brief The format to use for the swap chain RTVs.
   DXGI_FORMAT format_rtv = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

   /// @brief Number of buffers in the swap chain.
   uint32 buffer_count = 2;

   /// @brief Maximum frame latency of the swap chain. Only has effect if frame_latency_waitable is true. https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_3/nf-dxgi1_3-idxgiswapchain2-setmaximumframelatency
   uint32 maximum_frame_latency = 1;

   /// @brief When true allows callling swap_chain::wait_for_ready to wait on the
   /// swap chain's frame latency waitable object. https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_3/nf-dxgi1_3-idxgiswapchain2-getframelatencywaitableobject
   bool frame_latency_waitable = true;

   /// @brief When true allows calling swap_chain::present with allow_tearing set to true.
   bool allow_tearing = false;
};

/// Barriers Structures ///

struct resource_transition_barrier {
   resource_handle resource;
   uint32 subresource = resource_barrier_all_subresources;
   resource_state state_before;
   resource_state state_after;
};

struct resource_aliasing_barrier {
   resource_handle resource_before;
   resource_handle resource_after;
};

struct resource_uav_barrier {
   resource_handle resource;
};

struct resource_uav_any_barrier {};

struct resource_barrier {
   barrier_type type = barrier_type::transition;
   barrier_split split = barrier_split::none; /// See Split Barriers in D3D12 docs

   union {
      resource_transition_barrier transition = {};
      resource_aliasing_barrier aliasing;
      resource_uav_barrier uav;
      resource_uav_any_barrier uav_any;
   };
};

/// Command List Structures and Definitions ///

struct texture_copy_buffer_footprint {
   uint64 offset = 0;
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uint32 width = 1;
   uint32 height = 1;
   uint32 depth = 1;
   uint32 row_pitch = 0;
};

struct index_buffer_view {
   gpu_virtual_address buffer_location = 0;
   uint32 size_in_bytes = 0;
   constexpr static DXGI_FORMAT format = DXGI_FORMAT_R16_UINT;
};

struct vertex_buffer_view {
   gpu_virtual_address buffer_location = 0;
   uint32 size_in_bytes = 0;
   uint32 stride_in_bytes = 0;
};

struct viewport {
   float top_left_x;
   float top_left_y;
   float width;
   float height;
   float min_depth;
   float max_depth;
};

struct depth_stencil_clear_flags {
   bool clear_depth : 1 = true;
   bool clear_stencil : 1 = true;
};

struct command_list {
   void close();

   void reset();

   void clear_state();
};

struct copy_command_list : command_list {
   void deferred_resource_barrier(const std::span<const resource_barrier> barriers);

   void deferred_resource_barrier(const resource_barrier& barrier);

   void flush_resource_barriers();

   void copy_buffer_region(const resource_handle dst_buffer,
                           const uint64 dst_offset, const resource_handle src_buffer,
                           const uint64 src_offset, const uint64 num_bytes);

   void copy_buffer_to_texture(resource_handle dst_texture, const uint32 dst_subresource,
                               const uint32 dst_x, const uint32 dst_y,
                               const uint32 dst_z, resource_handle src_buffer,
                               const texture_copy_buffer_footprint& src_footprint,
                               const box* const src_box = nullptr);

   void copy_texture_to_buffer(resource_handle dst_buffer,
                               const texture_copy_buffer_footprint& dst_footprint,
                               const uint32 dst_x, const uint32 dst_y,
                               const uint32 dst_z, resource_handle src_texture,
                               const uint32 src_subresource,
                               const box* const src_box = nullptr);

   void copy_texture_region(const resource_handle dst_texture,
                            const uint32 dst_subresource, const uint32 dst_x,
                            const uint32 dst_y, const uint32 dst_z,
                            const resource_handle src_texture,
                            const uint32 src_subresource,
                            const box* const src_box = nullptr);

   void copy_resource(resource_handle dst_resource, resource_handle src_resource);

   void write_buffer_immediate(const gpu_virtual_address address, const uint32 value);
};

struct compute_command_list : copy_command_list {
   /// @brief Reset the command list and bind the supplied sampler heap.
   /// @param sampler_heap This will be set on the command list as part of it being reset.
   void reset(sampler_heap_handle sampler_heap);

   void dispatch(const uint32 thread_group_count_x, const uint32 thread_group_count_y,
                 const uint32 thread_group_count_z);

   void set_pipeline_state(const pipeline_handle pipeline);

   void set_compute_root_signature(const root_signature_handle root_signature);

   void set_compute_root_cbv(const uint32 root_parameter_index, cbv_handle cbv);

   void set_compute_root_32bit_constant(const uint32 root_parameter_index,
                                        const uint32 constant,
                                        const uint32 dest_offset_in_32bit_values);

   void set_compute_root_32bit_constants(const uint32 root_parameter_index,
                                         const std::span<const std::byte> constants,
                                         const uint32 dest_offset_in_32bit_values);

   void set_compute_root_cbv_address(const uint32 root_parameter_index,
                                     const gpu_virtual_address buffer_location);

   void set_compute_root_srv_address(const uint32 root_parameter_index,
                                     const gpu_virtual_address buffer_location);

   void set_compute_root_uav_address(const uint32 root_parameter_index,
                                     const gpu_virtual_address buffer_location);

   void clear_uav_uint(const resource_view_handle uav_handle, resource_handle resource,
                       const std::array<const uint32, 4>& values,
                       const rect* const rect = nullptr);

   void clear_uav_float(const resource_view_handle uav_handle, resource_handle resource,
                        const float4& values, const rect* const rect = nullptr);

   void discard_resource(const resource_handle resource);
};

struct graphics_command_list : compute_command_list {
   void draw_instanced(const uint32 vertex_count_per_instance,
                       const uint32 instance_count, const uint32 start_vertex_location,
                       const uint32 start_instance_location);

   void draw_indexed_instanced(const uint32 index_count_per_instance,
                               const uint32 instance_count,
                               const uint32 start_index_location,
                               const int32 base_vertex_location,
                               const uint32 start_instance_location);

   void resolve_subresource(resource_handle& dst_resource,
                            const uint32 dst_subresource, resource_handle& src_resource,
                            const uint32 src_subresource, const DXGI_FORMAT format);

   void set_graphics_root_signature(const root_signature_handle root_signature);

   void set_graphics_root_cbv(const uint32 root_parameter_index, cbv_handle cbv);

   void set_graphics_root_32bit_constant(const uint32 root_parameter_index,
                                         const uint32 constant,
                                         const uint32 dest_offset_in_32bit_values);

   void set_graphics_root_32bit_constants(const uint32 root_parameter_index,
                                          const std::span<const std::byte> constants,
                                          const uint32 dest_offset_in_32bit_values);

   void set_graphics_root_cbv_address(const uint32 root_parameter_index,
                                      const gpu_virtual_address buffer_location);

   void set_graphics_root_srv_address(const uint32 root_parameter_index,
                                      const gpu_virtual_address buffer_location);

   void set_graphics_root_uav_address(const uint32 root_parameter_index,
                                      const gpu_virtual_address buffer_location);

   void ia_set_primitive_topology(const primitive_topology primitive_topology);

   void ia_set_index_buffer(const index_buffer_view& view);

   void ia_set_vertex_buffers(const uint32 start_slot,
                              const std::span<const vertex_buffer_view> views);

   void ia_set_vertex_buffers(const uint32 start_slot, const vertex_buffer_view& view);

   void rs_set_viewports(const std::span<const viewport> viewports);

   void rs_set_viewports(const viewport& viewport);

   void rs_set_scissor_rects(const std::span<const rect> scissor_rects);

   void rs_set_scissor_rects(const rect& scissor_rect);

   void om_set_blend_factor(const float4& blend_factor);

   void om_set_stencil_ref(const uint32 stencil_ref);

   void om_set_render_targets(const std::span<const rtv_handle> render_targets);

   void om_set_render_targets(const std::span<const rtv_handle> render_targets,
                              const dsv_handle depth_stencil);

   void om_set_render_targets(const rtv_handle render_target);

   void om_set_render_targets(const rtv_handle render_target,
                              const dsv_handle depth_stencil);

   void clear_depth_stencil_view(const dsv_handle depth_stencil,
                                 const depth_stencil_clear_flags flags,
                                 const float depth, const uint8 stencil,
                                 const rect* const rect = nullptr);

   void clear_render_target_view(const rtv_handle rendertarget, const float4& color,
                                 const rect* const rect = nullptr);
};

/// Command Queue Definitions ///

/// @brief Ecapsulates a ID3D12CommandQueue with a high(er) level interface.
struct command_queue {
   /// @brief Submits command lists for execution to the queue.
   /// @param command_lists The command lists to execute.
   void execute_command_lists(std::derived_from<command_list> auto&... command_lists)
   {
      std::array command_lists_array{static_cast<command_list&>(command_lists)...};

      execute_command_lists(command_lists_array);
   }

   /// @brief Submits command lists for execution to the queue.
   /// @param command_lists The command lists to execute.
   void execute_command_lists(std::span<command_list&> command_lists);

   /// @brief Insert a fence on this queue to synchronize with another queue.
   /// @param other The queue to synchronize with.
   void sync_with(command_queue& other);

   // @brief Wait for all currently submited work to this queue to finish before returning.
   void wait_for_idle();
};

/// Swapchain Definitions ///

struct current_backbuffer {
   resource_handle resource;
   rtv_handle rtv;
};

struct swap_chain {
   /// @brief Wait for the swap chain to be ready for rendering. Or does nothing if swap_chain_desc::frame_latency_waitable is false.
   void wait_for_ready();

   /// @brief Wait for the swap chain to be ready for rendering. Or does nothing if swap_chain_desc::frame_latency_waitable is false.
   /// @param allow_tearing Allow tearing/disable V-Sync.
   void present(bool allow_tearing);

   /// @brief Resize the swap chain buffers in response to a window size change.
   /// @param new_width The new width.
   /// @param new_height The new height.
   void resize(uint32 new_width, uint32 new_height);

   /// @brief Gets the back buffer to use for rendering the current frame.
   /// @return The back buffer to draw into for the current frame. The handles remain owned by the swap chain.
   auto current_back_buffer() -> current_backbuffer;

   /// @brief Gets the current width of the back buffer.
   auto width() -> uint32;

   /// @brief Gets the current height of the back buffer.
   auto height() -> uint32;
};

/// Device Definitions ///

struct device {
   command_queue direct_queue;
   command_queue compute_queue;
   command_queue background_copy_queue;

   /// @brief Wait for all currently submited work to the command queues to finish before returning
   void wait_for_idle();

   /// Command List Functions ///

   [[nodiscard]] auto create_copy_command_list(const debug_name debug_name = "")
      -> copy_command_list;

   [[nodiscard]] auto create_compute_command_list(const debug_name debug_name = "")
      -> compute_command_list;

   [[nodiscard]] auto create_graphics_command_list(const debug_name debug_name = "")
      -> graphics_command_list;

   /// Pipeline Functions ///

   [[nodiscard]] auto create_root_signature(const root_signature_desc& desc)
      -> root_signature_handle;

   void release_root_signature(root_signature_handle root_signature);

   [[nodiscard]] auto create_graphics_pipeline(const root_signature_desc& desc)
      -> root_signature_handle;

   void release_pipeline(pipeline_handle pipeline);

   /// Resource Functions ///

   [[nodiscard]] auto create_buffer(const buffer_desc& desc, const heap_type heap_type,
                                    const resource_state initial_resource_state)
      -> resource_handle;

   [[nodiscard]] auto create_texture(const texture_desc& desc,
                                     const resource_state initial_resource_state)
      -> resource_handle;

   void release_resource(resource_handle resource);

   [[nodiscard]] auto get_gpu_virtual_address(resource_handle resource)
      -> gpu_virtual_address;

   [[nodiscard]] auto map(resource_handle resource, memory_range read_range) -> void*;

   void unmap(resource_handle resource, memory_range write_range);

   /// Resource View Functions ///

   [[nodiscard]] auto create_constant_buffer_view(const constant_buffer_view_desc& desc)
      -> cbv_handle;

   void release_constant_buffer_view(cbv_handle view);

   [[nodiscard]] auto create_shader_resource_view(
      resource_handle resource,
      const shader_resource_view_desc* const desc = nullptr) -> resource_view;

   [[nodiscard]] auto create_unordered_access_view(
      resource_handle resource,
      const unordered_access_view_desc* const desc = nullptr) -> resource_view;

   void release_resource_view(resource_view resource_view);

   [[nodiscard]] auto create_render_target_view(resource_handle resource,
                                                const render_target_view_desc& desc)
      -> rtv_handle;

   void release_render_target_view(rtv_handle render_target_view);

   [[nodiscard]] auto create_depth_stencil_view(resource_handle resource,
                                                const depth_stencil_view_desc& desc)
      -> dsv_handle;

   void release_depth_stencil_view(dsv_handle depth_stencil_view);

   /// Sampler Functions ///

   [[nodiscard]] auto create_sampler_heap(std::span<const sampler_desc> sampler_descs)
      -> sampler_heap_handle;

   void release_sampler_heap(sampler_heap_handle sampler_heap);

   /// Swapchain Functions ///

   [[nodiscard]] auto create_swap_chain(const swap_chain_desc& desc) -> swap_chain;
};

}
