#pragma once

#if  __SHADER_TARGET_MAJOR <= 6 && __SHADER_TARGET_MINOR < 6 

Texture2D Texture2DHeap[] : register(t0, space10000);
Texture2D<uint4> Texture2D_uint_Heap[] : register(t0, space10001);
Texture2D<int4> HeapTexture2D_int_Heap[] : register(t0, space10002);

Texture2DArray Texture2DArrayHeap[] : register(t0, space10003);
Texture2DArray<uint4> Texture2DArray_uint_Heap[] : register(t0, space10004);
Texture2DArray<int4> Texture2DArray_int_Heap[] : register(t0, space10005);

TextureCube TextureCubeHeap[] : register(t0, space10006);
TextureCubeArray TextureCubeArrayHeap[] : register(t0, space10007);

Texture3D Texture3DHeap[] : register(t0, space10008);

ByteAddressBuffer ByteAddressBufferHeap[] : register(t0, space10009);

#define STRUCTURED_BUFFER_HEAP_SPACE0 space10010
#define STRUCTURED_BUFFER_HEAP_SPACE1 space10011
#define STRUCTURED_BUFFER_HEAP_SPACE2 space10012
#define STRUCTURED_BUFFER_HEAP_SPACE3 space10013
#define STRUCTURED_BUFFER_HEAP_SPACE4 space10014
#define STRUCTURED_BUFFER_HEAP_SPACE5 space10015

// Define a custom structured buffer heap. This can be accessed using INDEX_STRUCTURED_BUFFER_HEAP to share code between SM 6.0 and SM 6.6.
#define STRUCTURED_BUFFER_HEAP(type, name, space) StructuredBuffer<type> name[] : register(t0, space)

// Index into a structured buffer heap defined by STRUCTURED_BUFFER_HEAP.
#define INDEX_STRUCTURED_BUFFER_HEAP(name, index) name[index]

#else // Code path for SM 6.6 and higher using ResourceDescriptorHeap.

#define Texture2DHeap ResourceDescriptorHeap
#define Texture2D_uint_Heap ResourceDescriptorHeap
#define HeapTexture2D_int_Heap ResourceDescriptorHeap

#define Texture2DArrayHeap ResourceDescriptorHeap
#define Texture2DArray_uint_Heap ResourceDescriptorHeap
#define Texture2DArray_int_Heap ResourceDescriptorHeap

#define TextureCubeHeap ResourceDescriptorHeap
#define TextureCubeArrayHeap ResourceDescriptorHeap

#define Texture3DHeap ResourceDescriptorHeap

#define ByteAddressBufferHeap ResourceDescriptorHeap

#define STRUCTURED_BUFFER_HEAP(type, name, space) 
#define INDEX_STRUCTURED_BUFFER_HEAP(name, index) ResourceDescriptorHeap[index]

#endif