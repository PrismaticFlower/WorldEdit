## Background
In the renderer Direct3D 12 is hidden away behind a small abstraction layer. The goal of the layer is to make things "simpler" without introducing any overhead while recording command lists. It is by no means an attempt to abstract away D3D12 such that it could be swapped out with Vulkan, Metal, etc with ease. It may be possible to do that but it is not a goal.

The entire public API is found in `gpu/rhi.hpp`. Additional public helpers can be found in the headers in `gpu/` but not in the subfolders.

### Resource Binding
Shader resource view binding is mostly focused around Shader Model 6.6 bindless support. With APIs for creating resource views returning a struct with a single member containing the index of the view's descriptor in the descriptor heap. The descriptor heap itself for resource views is abstracted away and hidden from the user code. This gives a simple and powerful interface.

```c++
struct resource_view {
   uint32 index;
};
```

Root parameters are limited to root SRV/UAV/CBV parameters and UAV resource view parameters. For better or worse descriptor tables are not exposed for constant buffer views or shader resource views.

In order to support Shader Model 6.1 shaders for older GPUs access to `ResourceDescriptorHeap` is abstracted by `resource_heaps.hlsli`. When SM 6.6 is not supported `gpu::device` will insert a hidden descriptor table at the start of each root signature. This hidden parameter will bind the descriptor heap to the first register of the [10000, 10015] binding spaces. And when a root signature is set on a command list the hidden paremter (again only when SM 6.6 is not supported) will be set with the base of the device's descriptor heap.

All of this allows writing and maintaining the same shaders for both SM 6.1 and SM 6.6 with the lowest possible overhead for the SM 6.6 case. (And no more overhead than you'd normally have for bindless on pre 6.6 HW.)

### Samplers
Samplers are now only exposed as static samplers. They were initial exposed as an entire heap that would be bound to a command list. For simplicity during development of the SM 6.1 support this was changed. This maybe revisited in the future if the need to change the anisotropy level at runtime arises.

### Lifetime Management
GPU objects can broken down into two categories. RAII Objects and Handle Objects. 

RAII Objects are structs created from `gpu::device` that are not trivial and (typically) have member functions of their own. Currently this is only `gpu::swap_chain`, `gpu::copy_command_list`, `gpu::compute_command_list` and `gpu::graphics_command_list`. It is not expected this list will grow. These objects can be freely destroyed without concern for if the GPU is currently using their contents. If synchronization is needed it will be performed implicitly. For `gpu::swapchain` in particular this cost could be expensive. But swap chains are rarely destroyed and as such it is not expected to be an issue.

Handle Objects on the other hand encompass both the various `gpu::*_handle` handles returned from `gpu::device` create functions and also the `resource_view` struct (which is just a wrapper around a uint32 indexing into the descriptor heap). These objects must be explicitly released. Unlike raw D3D12 where release is called on the object itself the release is queued on a `gpu::command_queue`. The exact time when the object will be freed is unspecified but it is **guaranteed to be after all preceeding work on the queue has completed**.

There is no restriction on what kind of Handle Objects a queue can release. i.e the background copy queue may release pipeline handles.


```c++
   // Release functions from gpu::command_queue

   void release_root_signature(root_signature_handle root_signature);

   void release_pipeline(pipeline_handle pipeline);

   void release_resource(resource_handle resource);

   void release_resource_view(resource_view resource_view);

   void release_render_target_view(rtv_handle render_target_view);

   void release_depth_stencil_view(dsv_handle depth_stencil_view);

   void release_query_heap(query_heap_handle query_heap);
```

Included in `gpu/resource.hpp` is `gpu::unique_handle`. This provides a smart pointer like interface around a handle and manages it's lifetime using the provided queue at construction time. Managing the lifetime of a `gpu::resource_view` is also supported. In most cases this is what you'll want to use over manually managing object lifetime.

#### Unsynced Lifetime Management
During optimizing the background texture creation it became apparent that the ability to release a resource immediately could speed things up for cases where you have a natural sync point (Like the `wait_for_idle` at the end of a texture upload.). With that in mind `gpu::device` now has functions to directly release GPU resources and objects. These perform no synchronization and as such must be treated with more care than the queued release functions.

```c++
   // Release functions from gpu::device

   void release_root_signature(root_signature_handle root_signature);

   void release_pipeline(pipeline_handle pipeline);

   void release_resource(resource_handle resource);

   void release_resource_view(resource_view resource_view);

   void release_render_target_view(rtv_handle render_target_view);

   void release_depth_stencil_view(dsv_handle depth_stencil_view);

   void release_query_heap(query_heap_handle query_heap);
```

```c++
// List of gpu::unique_handle instantiations

using unique_resource_handle  = /* ... */;
using unique_rtv_handle  = /* ... */;
using unique_dsv_handle = /* ... */;
using unique_resource_view = /* ... */;
using unique_root_signature_handle = /* ... */;
using unique_pipeline_handle = /* ... */;
using unique_sampler_heap_handle = /* ... */;
```

### Fences
Fences are not directly exposed. Instead command queues contain functions for syncing with another queue or having the CPU wait for all currently submitted queue work to complete. These cover the main needed uses of fences for the codebase with simple APIs.

`gpu::command_queue::wait_for_idle` only syncs with work submitted before the call. It is possible for another thread to submit work at the same time that does not end up being waited on.

```c++
   // Synchronization functions from gpu::command_queue

   /// @brief Insert a fence on this queue to synchronize with another queue.
   /// @param other The queue to synchronize with.
   void sync_with(command_queue& other);

   /// @brief Wait for all currently submited work to this queue to finish before returning.
   void wait_for_idle();
```

`gpu::device` also includes a function for efficiently waiting on all currently submitted work to complete. As with `gpu::command_queue::wait_for_idle` it only syncs with work submitted before the call.

```c++
   // from gpu::device

   /// @brief Wait for all currently submited work to the command queues to finish before returning
   void wait_for_idle();
```

### Frame Index
Buffering CPU visible resources across frames is not abstracted away but there is functionality to assist with it. First `frame_pipeline_length` defines the number of resources needed for all frames in flight and `gpu::device::frame_index` returns the index of the resource to use on the current frame.

All this depends on `gpu::device::end_frame`. Which must always be called at the end of a frame to update `frame_index` and perform end-of-frame tasks. It is important to note `frame_index` is synchronized for the `direct_queue` only.

```c++
constexpr inline uint32 frame_pipeline_length = 2;
```

```c++
   // Frame functions from gpu::device

   /// @brief Return the index to use for buffered resources (exluding swap chains) this frame.
   /// @return The index in the range [0, frame_pipeline_length - 1].
   auto frame_index() -> uint64;

   /// @brief Perform necessary end-of-frame tasks and synchronization.
   void end_frame();
```

### Thread Safety

`gpu::device::frame_index` - Thread safe as long as end_frame is not currently being called.
`gpu::device::end_frame` - Not thread safe.


### Null Resource Handles
For convenience handles have null values defined. However in all but one case this is just for user code. Handles passed back into the RHI must not be null. The only exception to this is `unordered_access_view_desc::buffer::counter_resource` which may be null (and is by default). All other handles must be valid when passed into the RHI.

