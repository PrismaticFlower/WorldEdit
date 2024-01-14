#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char mesh_wireframe_GS_fallbackGS_6_1_dxil_bytes[2381];
extern const char mesh_wireframe_GS_fallbackGS_6_6_dxil_bytes[2381];

auto mesh_wireframe_GS_fallbackGS() noexcept -> shader_def
{
   return {
      .name = "mesh_wireframe_GS_fallbackGS",
      .entrypoint = L"main",
      .target_6_1 = L"gs_6_1",
      .target_6_6 = L"gs_6_6",
      .file = L"mesh_wireframe_GS_fallbackGS.hlsl",
      .dxil_6_1 = {reinterpret_cast<const std::byte*>(mesh_wireframe_GS_fallbackGS_6_1_dxil_bytes),
                      sizeof(mesh_wireframe_GS_fallbackGS_6_1_dxil_bytes) - 1},
      .dxil_6_6 = {reinterpret_cast<const std::byte*>(mesh_wireframe_GS_fallbackGS_6_6_dxil_bytes),
                      sizeof(mesh_wireframe_GS_fallbackGS_6_6_dxil_bytes) - 1},
   };
}

const char mesh_wireframe_GS_fallbackGS_6_1_dxil_bytes[2381] = "\x44\x58\x42\x43\x84\x76\x9f\xb6\xd\xc4\xec\x95\x9c\x2\x74\x29\x2b\xba\xe6\xb1\x1\x0\x0\x0\x4c\x9\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xb8\x0\x0\x0\x64\x1\x0\x0\x44\x2\x0\x0\x84\x2\x0\x0\xa0\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x64\x0\x0\x0\x2\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x48\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x54\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\xf\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x4f\x53\x47\x31\xa4\x0\x0\x0\x4\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x88\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x94\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x94\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x94\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x50\x53\x56\x30\xd8\x0\x0\x0\x30\x0\x0\x0\x3\x0\x0\x0\x5\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x2\x0\x3\x0\x2\x2\x0\x2\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x24\x0\x0\x0\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x10\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x3\x3\x4\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x0\x3\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x3\x3\x4\x0\x0\x11\x0\x0\x0\x1\x0\x0\x0\x3\x1\x44\x0\x3\x1\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x4\x0\x0\x0\x8\x0\x0\x0\x10\x11\x0\x0\x20\x22\x0\x0\x40\x44\x0\x0\x80\x88\x0\x0\x49\x4c\x44\x4e\x38\x0\x0\x0\x0\x0\x32\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x73\x68\x5f\x77\x69\x72\x65\x66\x72\x61\x6d\x65\x5f\x47\x53\x5f\x66\x61\x6c\x6c\x62\x61\x63\x6b\x47\x53\x5f\x36\x5f\x31\x2e\x70\x64\x62\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x1e\x19\x36\xc0\xff\x13\x25\x19\xe5\x22\x96\xff\xd9\xdd\x4e\xb5\x44\x58\x49\x4c\xa4\x6\x0\x0\x61\x0\x2\x0\xa9\x1\x0\x0\x44\x58\x49\x4c\x1\x1\x0\x0\x10\x0\x0\x0\x8c\x6\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\xa0\x1\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x14\x45\x2\x42\x92\xb\x42\xa4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x52\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x91\x22\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x29\x46\x6\x51\x18\x0\x0\x6\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x1\x0\x0\x0\x49\x18\x0\x0\x2\x0\x0\x0\x13\x82\x60\x42\x20\x0\x0\x0\x89\x20\x0\x0\x12\x0\x0\x0\x32\x22\x48\x9\x20\x64\x85\x4\x93\x22\xa4\x84\x4\x93\x22\xe3\x84\xa1\x90\x14\x12\x4c\x8a\x8c\xb\x84\xa4\x4c\x10\x40\x23\x0\x25\x0\x14\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x20\x84\x14\x42\xa6\x18\x80\x10\x52\x6\xa1\x32\x0\x52\x48\xd\x4\x64\x3\x99\x2\x98\x23\x8\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x28\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x90\x5\x2\x0\x0\x0\xf\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x25\x50\xe\xc5\x30\x2\x50\x6\xe5\x51\x4\xa5\x40\xa5\x24\xca\xa0\x10\x46\x0\x4a\x80\xda\x58\xd\x2\x5\x2\x81\xc0\x28\x1\xc6\x8\x28\x46\x20\x29\x6\x0\x0\x0\x0\x0\x79\x18\x0\x0\x4d\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x8\x62\x83\x30\xc\x13\x4\xa2\xd8\x20\xc\x5\x85\xb3\xb9\x9\x2\x61\x6c\x18\xe\x64\x98\x20\x34\xd1\x86\x40\x99\x20\x8\x0\x89\xb6\xb0\x34\xb7\x9\x2\x71\x70\x99\xb2\xfa\x82\x7a\x9b\x4b\xa3\x4b\x7b\x73\x9b\x20\x14\xcc\x4\xa1\x68\x36\x4\xce\x4\xa1\x70\x26\x8\xc5\x33\x41\x20\x90\x9\x2\x91\x6c\x10\x2a\x6b\xc3\xe2\x3c\x50\x24\x4d\xc3\xe4\x50\x17\x1f\x23\xa6\x20\xaa\x2f\xa8\xa7\xa9\x24\xaa\xa4\x27\x27\xa8\xa9\x9\x42\x1\x6d\x58\x86\xc\xa2\x24\x6d\x98\x6\xea\xda\x20\x60\xdb\x4\x81\x50\x36\xc\xce\xd0\x6d\x58\x86\xc\xa2\x3c\xad\x9a\x6\xea\xda\x20\x60\xdf\x86\x81\x3\x3\x60\x82\x40\x2c\x1b\x8a\xaa\x1a\xc4\x60\xd8\x20\xc\x63\xb0\xa1\x60\x9a\x30\x0\xc8\xa0\xa\x1b\x9b\x5d\x9b\x4b\x1a\x59\x99\x1b\xdd\x94\x20\xa8\x42\x86\xe7\x62\x57\x26\x37\x97\xf6\xe6\x36\x25\x18\x9a\x90\xe1\xb9\xd8\x85\xb1\xd9\x95\xc9\x4d\x9\x8a\x3a\x64\x78\x2e\x73\x68\x61\x64\x65\x72\x4d\x6f\x64\x65\x6c\x53\x2\xa4\xe\x19\x9e\x8b\x5d\x5a\xd9\x5d\x12\xd9\x14\x5d\x18\x5d\xd9\x94\x40\xa9\x43\x86\xe7\x52\xe6\x46\x27\x97\x7\xf5\x96\xe6\x46\x37\x37\x25\x20\x3\x0\x0\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\x8c\xc8\x21\x7\x7c\x70\x3\x72\x10\x87\x73\x70\x3\x7b\x8\x7\x79\x60\x87\x70\xc8\x87\x77\xa8\x7\x7a\x98\x81\x3c\xe4\x80\xf\x6e\x40\xf\xe5\xd0\xe\xf0\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\xe\x0\x0\x0\x36\x0\xd\x97\xef\x3c\x3e\xc1\x20\x13\x3b\x45\x4\xc0\x58\xc0\x34\x5c\xbe\xf3\xf8\x8b\x3\xc\x62\xf3\x50\x93\x5f\xdc\xb6\x9\x54\xc3\xe5\x3b\x8f\x2f\x4d\x4e\x44\xa0\xd4\xf4\x50\x93\x5f\xdc\xb6\x1\x10\xc\x80\x34\x0\x0\x61\x20\x0\x0\x74\x0\x0\x0\x13\x4\x43\x2c\x10\x0\x0\x0\x3\x0\x0\x0\x44\xa\xa1\x84\x3\xa8\x14\x1\x0\x0\x0\x0\x23\x6\x9\x0\x82\x60\x60\x50\x43\xe4\x18\xce\x88\x41\x2\x80\x20\x18\x18\x15\x21\x3d\xc6\x33\x62\x90\x0\x20\x8\x6\x86\x55\x4c\xd0\x0\x8d\x18\x24\x0\x8\x82\x81\x71\x19\x54\xb4\x44\x23\x6\x9\x0\x82\x60\x60\x60\x47\x25\x29\xd5\x88\x41\x2\x80\x20\x18\x18\x19\x62\x4d\x8a\x35\x62\x90\x0\x20\x8\x6\x86\x96\x5c\xd4\x71\x8d\x18\x24\x0\x8\x82\x81\xb1\x29\x58\xf5\x60\x23\x6\x9\x0\x82\x60\x60\x70\x4b\x66\x39\xd3\x88\x41\x2\x80\x20\x18\x18\x1d\xa3\x5d\xe\x35\x62\x90\x0\x20\x8\x6\x86\xd7\x6c\xd8\x52\x8d\x18\x24\x0\x8\x82\x81\xf1\x39\x5c\x36\x59\xb3\x4\xc1\x40\x5\x41\x7\xc0\x12\x8c\x18\x24\x0\x8\x82\x81\x11\x6\xd0\xb6\x4d\xc1\x88\x41\x2\x80\x20\x18\x18\x62\x10\x71\xdc\x24\x8c\x18\x24\x0\x8\x82\x81\x31\x6\x52\xd7\x41\xc3\x88\x41\x2\x80\x20\x18\x18\x64\x30\x79\x1e\x46\x8c\x18\x24\x0\x8\x82\x1\x42\x6\xdb\xf7\x5d\xc4\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x7d\x9f\x35\x8c\x18\x24\x0\x8\x82\x1\x42\x6\xdb\xf7\x49\xc2\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x7d\x5f\x16\x8c\x18\x24\x0\x8\x82\x1\x42\x6\x9b\x18\x7c\x57\x34\x62\x90\x0\x20\x8\x6\x8\x19\x6c\x62\xf0\x59\xd0\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x89\xc1\x27\x3d\x23\x6\x9\x0\x82\x60\x80\x90\xc1\x26\x6\x5f\xe6\x8c\x18\x24\x0\x8\x82\x1\x42\x6\x9b\x18\x88\xc1\xd5\x8c\x18\x24\x0\x8\x82\x1\x42\x6\x9b\x18\x88\x81\xc5\x8c\x18\x24\x0\x8\x82\x1\x42\x6\x9b\x18\x88\x81\xb4\x8c\x18\x24\x0\x8\x82\x1\x42\x6\x9b\x18\x88\x41\xa6\x8c\x18\x24\x0\x8\x82\x1\x42\x6\x9b\x18\x70\x57\x32\x62\x90\x0\x20\x8\x6\x8\x19\x6c\x62\xc0\x59\xc8\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x89\x1\x27\x1d\x23\x6\x9\x0\x82\x60\x80\x90\xc1\x26\x6\x5c\x66\x8c\x18\x18\x0\x8\x82\x81\x32\x6\xd3\x65\x85\x18\xc0\x60\xb8\x21\xf8\xc0\x60\x96\x41\x8\x2\x4\x0\x0\x0\x0\x0\x0\x0";

const char mesh_wireframe_GS_fallbackGS_6_6_dxil_bytes[2381] = "\x44\x58\x42\x43\x20\xf5\xea\xb7\x30\xe0\xe8\x10\xb0\xc9\x78\x4a\x93\x85\xe6\xc1\x1\x0\x0\x0\x4c\x9\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xb8\x0\x0\x0\x64\x1\x0\x0\x44\x2\x0\x0\x84\x2\x0\x0\xa0\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x64\x0\x0\x0\x2\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x48\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x54\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\xf\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x4f\x53\x47\x31\xa4\x0\x0\x0\x4\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x88\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x94\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x94\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x94\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x50\x53\x56\x30\xd8\x0\x0\x0\x30\x0\x0\x0\x3\x0\x0\x0\x5\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x2\x0\x3\x0\x2\x2\x0\x2\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x24\x0\x0\x0\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x10\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x3\x3\x4\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x0\x3\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x3\x3\x4\x0\x0\x11\x0\x0\x0\x1\x0\x0\x0\x3\x1\x44\x0\x3\x1\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x4\x0\x0\x0\x8\x0\x0\x0\x10\x11\x0\x0\x20\x22\x0\x0\x40\x44\x0\x0\x80\x88\x0\x0\x49\x4c\x44\x4e\x38\x0\x0\x0\x0\x0\x32\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x73\x68\x5f\x77\x69\x72\x65\x66\x72\x61\x6d\x65\x5f\x47\x53\x5f\x66\x61\x6c\x6c\x62\x61\x63\x6b\x47\x53\x5f\x36\x5f\x36\x2e\x70\x64\x62\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x75\x3b\x64\x13\x12\x93\xcc\xc9\x7d\xf2\x5a\x61\x6\xf8\x1b\x7\x44\x58\x49\x4c\xa4\x6\x0\x0\x66\x0\x2\x0\xa9\x1\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x8c\x6\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\xa0\x1\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x14\x45\x2\x42\x92\xb\x42\xa4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x52\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x91\x22\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x29\x46\x6\x51\x18\x0\x0\x6\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x1\x0\x0\x0\x49\x18\x0\x0\x2\x0\x0\x0\x13\x82\x60\x42\x20\x0\x0\x0\x89\x20\x0\x0\x12\x0\x0\x0\x32\x22\x48\x9\x20\x64\x85\x4\x93\x22\xa4\x84\x4\x93\x22\xe3\x84\xa1\x90\x14\x12\x4c\x8a\x8c\xb\x84\xa4\x4c\x10\x40\x23\x0\x25\x0\x14\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x20\x84\x14\x42\xa6\x18\x80\x10\x52\x6\xa1\x32\x0\x52\x48\xd\x4\x64\x3\x99\x2\x98\x23\x8\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x28\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x90\x5\x2\x0\x0\x0\xf\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x25\x50\xc\xe5\x30\x2\x50\x6\xe5\x51\x4\xa5\x40\xa5\x24\xca\xa0\x10\x46\x0\x4a\x80\xda\x58\xd\x2\x5\x2\x81\xc0\x28\x1\xc6\x8\x28\x46\x20\x29\x6\x0\x0\x0\x0\x0\x79\x18\x0\x0\x4d\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x8\x62\x82\x40\x14\x1b\x84\x81\x98\x20\x10\xc6\x6\x61\x30\x28\x9c\xcd\x6d\x18\x10\x82\x98\x20\x34\xd1\x86\x40\x99\x20\x8\x0\x89\xb6\xb0\x34\xb7\x9\x2\x71\x70\x99\xb2\xfa\x82\x7a\x9b\x4b\xa3\x4b\x7b\x73\x9b\x20\x14\xcc\x4\xa1\x68\x36\x4\xce\x4\xa1\x70\x26\x8\xc5\x33\x41\x20\x90\x9\x2\x91\x6c\x10\x2a\x6b\xc3\xe2\x3c\x50\x24\x4d\xc3\xe4\x50\x17\x1f\x23\xa6\x20\xaa\x2f\xa8\xa7\xa9\x24\xaa\xa4\x27\x27\xa8\xa9\x9\x42\x1\x6d\x58\x86\xc\xa2\x24\x6d\x98\x6\xea\xda\x20\x60\xdb\x4\x81\x50\x36\xc\xce\xd0\x6d\x58\x86\xc\xa2\x3c\xad\x9a\x6\xea\xda\x20\x60\xdf\x86\x81\x3\x3\x60\x82\x40\x2c\x1b\x8a\xaa\x1a\xc4\x60\xd8\x20\xc\x63\xb0\xa1\x60\x9a\x30\x0\xc8\xa0\xa\x1b\x9b\x5d\x9b\x4b\x1a\x59\x99\x1b\xdd\x94\x20\xa8\x42\x86\xe7\x62\x57\x26\x37\x97\xf6\xe6\x36\x25\x20\x9a\x90\xe1\xb9\xd8\x85\xb1\xd9\x95\xc9\x4d\x9\x8c\x3a\x64\x78\x2e\x73\x68\x61\x64\x65\x72\x4d\x6f\x64\x65\x6c\x53\x2\xa4\xe\x19\x9e\x8b\x5d\x5a\xd9\x5d\x12\xd9\x14\x5d\x18\x5d\xd9\x94\x40\xa9\x43\x86\xe7\x52\xe6\x46\x27\x97\x7\xf5\x96\xe6\x46\x37\x37\x25\x20\x3\x0\x0\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\x8c\xc8\x21\x7\x7c\x70\x3\x72\x10\x87\x73\x70\x3\x7b\x8\x7\x79\x60\x87\x70\xc8\x87\x77\xa8\x7\x7a\x98\x81\x3c\xe4\x80\xf\x6e\x40\xf\xe5\xd0\xe\xf0\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\xe\x0\x0\x0\x36\x0\xd\x97\xef\x3c\x3e\xc1\x20\x13\x3b\x45\x4\xc0\x58\xc0\x34\x5c\xbe\xf3\xf8\x8b\x3\xc\x62\xf3\x50\x93\x5f\xdc\xb6\x9\x54\xc3\xe5\x3b\x8f\x2f\x4d\x4e\x44\xa0\xd4\xf4\x50\x93\x5f\xdc\xb6\x1\x10\xc\x80\x34\x0\x0\x61\x20\x0\x0\x74\x0\x0\x0\x13\x4\x43\x2c\x10\x0\x0\x0\x3\x0\x0\x0\x44\xa\xa1\x84\x3\xa8\x14\x1\x0\x0\x0\x0\x23\x6\x9\x0\x82\x60\x60\x50\x43\xe4\x18\xce\x88\x41\x2\x80\x20\x18\x18\x15\x21\x3d\xc6\x33\x62\x90\x0\x20\x8\x6\x86\x55\x4c\xd0\x0\x8d\x18\x24\x0\x8\x82\x81\x71\x19\x54\xb4\x44\x23\x6\x9\x0\x82\x60\x60\x60\x47\x25\x29\xd5\x88\x41\x2\x80\x20\x18\x18\x19\x62\x4d\x8a\x35\x62\x90\x0\x20\x8\x6\x86\x96\x5c\xd4\x71\x8d\x18\x24\x0\x8\x82\x81\xb1\x29\x58\xf5\x60\x23\x6\x9\x0\x82\x60\x60\x70\x4b\x66\x39\xd3\x88\x41\x2\x80\x20\x18\x18\x1d\xa3\x5d\xe\x35\x62\x90\x0\x20\x8\x6\x86\xd7\x6c\xd8\x52\x8d\x18\x24\x0\x8\x82\x81\xf1\x39\x5c\x36\x59\xb3\x4\x2\xe\x54\x10\x74\x0\x2c\xc2\x88\x41\x2\x80\x20\x18\x18\x61\x0\x6d\xdb\x14\x8c\x18\x24\x0\x8\x82\x81\x21\x6\x11\xc7\x4d\xc2\x88\x41\x2\x80\x20\x18\x18\x63\x20\x75\x1d\x34\x8c\x18\x24\x0\x8\x82\x81\x41\x6\x93\xe7\x61\xc4\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x7d\xdf\x45\x8c\x18\x24\x0\x8\x82\x1\x42\x6\xdb\xf7\x59\xc3\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x7d\x9f\x24\x8c\x18\x24\x0\x8\x82\x1\x42\x6\xdb\xf7\x65\xc1\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x89\xc1\x77\x45\x23\x6\x9\x0\x82\x60\x80\x90\xc1\x26\x6\x9f\x5\x8d\x18\x24\x0\x8\x82\x1\x42\x6\x9b\x18\x7c\xd2\x33\x62\x90\x0\x20\x8\x6\x8\x19\x6c\x62\xf0\x65\xce\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x89\x81\x18\x5c\xcd\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x89\x81\x18\x58\xcc\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x89\x81\x18\x48\xcb\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x89\x81\x18\x64\xca\x88\x41\x2\x80\x20\x18\x20\x64\xb0\x89\x1\x77\x25\x23\x6\x9\x0\x82\x60\x80\x90\xc1\x26\x6\x9c\x85\x8c\x18\x24\x0\x8\x82\x1\x42\x6\x9b\x18\x70\xd2\x31\x62\x90\x0\x20\x8\x6\x8\x19\x6c\x62\xc0\x65\xc6\x88\x81\x1\x80\x20\x18\x28\x63\x30\x5d\x56\x88\x1\xc\x86\x1b\x82\xf\xc\x66\x19\x2\x21\x0\x0\x0\x0\x0\x0\x0\x0";

}