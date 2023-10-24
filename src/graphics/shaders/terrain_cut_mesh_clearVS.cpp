#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char terrain_cut_mesh_clearVS_dxil_bytes[2641];

auto terrain_cut_mesh_clearVS() noexcept -> shader_def
{
   return {
      .name = "terrain_cut_mesh_clearVS",
      .entrypoint = L"main",
      .target = L"vs_6_6",
      .file = L"terrain_cut_mesh_clearVS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(terrain_cut_mesh_clearVS_dxil_bytes),
               sizeof(terrain_cut_mesh_clearVS_dxil_bytes) - 1},
   };
}

const char terrain_cut_mesh_clearVS_dxil_bytes[2641] = "\x44\x58\x42\x43\x66\x66\x8b\xd5\xeb\x26\xfc\xa\xf6\x5f\x40\xf2\xa\x37\x74\x7f\x1\x0\x0\x0\x50\xa\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x88\x0\x0\x0\xc4\x0\x0\x0\x84\x1\x0\x0\xbc\x1\x0\x0\xd8\x1\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x0\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x50\x53\x56\x30\xb8\x0\x0\x0\x30\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x1\x0\x0\x0\x1\x1\x0\x1\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x2\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\xc\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x3\x3\x4\x0\x0\xf\x0\x0\x0\xf\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x30\x0\x0\x0\x0\x0\x2a\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x74\x65\x72\x72\x61\x69\x6e\x5f\x63\x75\x74\x5f\x6d\x65\x73\x68\x5f\x63\x6c\x65\x61\x72\x56\x53\x2e\x70\x64\x62\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x7\xd5\xa6\xcc\x98\xb5\x3a\x98\x86\x8a\xa7\x44\x18\xa9\xc1\x81\x44\x58\x49\x4c\x70\x8\x0\x0\x66\x0\x1\x0\x1c\x2\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x58\x8\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x13\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x4a\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\x90\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x80\x20\x88\x62\x20\xe4\xa8\xe1\xf2\x27\xec\x21\x24\x9f\xdb\xa8\x62\x25\x26\xbf\xb8\x6d\x44\xc\xc3\x30\x50\x71\xcf\x70\xf9\x13\xf6\x10\x92\x1f\x2\xcd\xb0\x10\x28\x58\xa\xa1\x10\xc\x41\x4d\x29\x6\x62\x18\x6\x7a\x6e\x1b\x2e\x7f\xc2\x1e\x42\xf2\x57\x42\x72\xa8\x48\x20\xd2\xc8\x79\x88\x68\x42\x8\x9\x9\x4\x51\x8\x86\x60\x22\x92\xe\x1a\x2e\x7f\xc2\x1e\x42\xf2\x57\x42\xda\x90\x66\x40\x4\x41\x10\xc5\x1c\x41\x50\xa\x86\xa0\x88\x8a\xac\x81\x80\x61\x4\x62\x48\x82\x6c\x18\x61\x18\x86\x11\x84\xe1\xce\xe0\x70\xa4\x69\x1\x30\x87\x9a\x7c\x69\x8a\x28\x61\xf2\x8b\x8\x60\x88\x5f\x70\x1a\x69\x2\x9a\xe9\xf\xa8\xa2\x20\x22\x64\x68\xdb\xc0\x71\xc3\x4d\x87\x23\x4d\xb\x80\x39\xd4\xe4\xb\xc1\x5f\x44\x0\x43\xa0\xa0\x23\xef\xc4\xe0\x70\xa4\x69\x1\x30\x87\x9a\x7c\x69\x8a\x28\x61\xf2\x9d\x20\x21\x84\xe9\x17\x9c\x46\x9a\x80\x66\x92\x50\xa0\x9d\x77\x38\xd2\xb4\x0\x98\x43\x4d\xbe\x10\xfc\x4e\x90\x10\xc2\xf4\xb\x4e\x23\x4d\x40\x33\x49\x28\x0\x29\xcc\x2\x2\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x34\x40\x0\xc\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x79\x80\x0\x8\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x23\x1\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\xc7\x2\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x2c\x10\xe\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\x90\x3\x8a\xa0\x0\x3\xca\xa0\x3c\xa8\x28\x89\x11\x80\x32\x28\x4\xfa\x66\x0\x48\x9c\x1\xa0\x71\xac\x62\x20\x9e\xe7\x1\x0\x0\x79\x18\x0\x0\x53\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x88\x63\x82\x40\x20\x1b\x84\x81\x98\x20\x10\xc9\x6\x61\x30\x28\xd8\xcd\x6d\x18\x10\x82\x98\x20\x10\xca\x4\xe1\xa3\x8\x4c\x10\x88\x65\x3\xa2\x2c\x8c\xa2\xc\xd\x30\x41\x10\x83\x6a\x82\x40\x30\x13\x4\xa2\xd9\x80\xc\xf\xa3\x40\x43\x4\x6c\x10\x1c\x69\x3\x1\x0\x13\x30\x41\x18\x3\x6b\x43\x50\x4d\x10\x4\x80\x44\x5b\x58\x9a\x1b\x11\xa8\xa7\xa9\x24\xaa\xa4\x27\xa7\x9\x42\x1\x4d\x10\x8a\x68\x43\xa0\x4c\x10\xa\x69\x82\x40\x38\x1b\x4\xcf\xd8\xb0\x28\x99\xb6\x71\xdb\xd0\x29\xdb\xb7\x21\x0\x3\x2e\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x13\x84\x62\x9a\x20\x10\xcf\x6\xc1\x23\x83\xd\x8b\x22\x6\x5a\xc7\x8d\xc1\x30\x6\xca\x56\x6\x1b\x2\x33\xd8\x30\x84\xc1\x19\x0\x1b\x8a\xb\x43\x3\xa\xa8\xc2\xc6\x66\xd7\xe6\x92\x46\x56\xe6\x46\x37\x25\x8\xaa\x90\xe1\xb9\xd8\x95\xc9\xcd\xa5\xbd\xb9\x4d\x9\x88\x26\x64\x78\x2e\x76\x61\x6c\x76\x65\x72\x53\x2\xa3\xe\x19\x9e\xcb\x1c\x5a\x18\x59\x99\x5c\xd3\x1b\x59\x19\xdb\x94\x0\x29\x43\x86\xe7\x22\x57\x36\xf7\x56\x27\x37\x56\x36\x37\x25\x98\xea\x90\xe1\xb9\xd8\xa5\x95\xdd\x25\x91\x4d\xd1\x85\xd1\x95\x4d\x9\xaa\x3a\x64\x78\x2e\x65\x6e\x74\x72\x79\x50\x6f\x69\x6e\x74\x73\x53\x2\x34\x0\x0\x0\x79\x18\x0\x0\x4c\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\x8c\xc8\x21\x7\x7c\x70\x3\x72\x10\x87\x73\x70\x3\x7b\x8\x7\x79\x60\x87\x70\xc8\x87\x77\xa8\x7\x7a\x98\x81\x3c\xe4\x80\xf\x6e\x40\xf\xe5\xd0\xe\xf0\x0\x0\x0\x71\x20\x0\x0\x1e\x0\x0\x0\x56\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\xd8\xc0\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x19\x74\xc3\xe5\x3b\x8f\x2f\x44\x4\x30\x11\x21\xd0\xc\xb\xf1\x45\xe\xb3\x21\xcd\x80\x34\x86\x5\x4c\xc3\xe5\x3b\x8f\xbf\x38\xc0\x20\x36\xf\x35\xf9\xc5\x6d\x9b\x40\x35\x5c\xbe\xf3\xf8\xd2\xe4\x44\x4\x4a\x4d\xf\x35\xf9\xc5\x6d\x1b\x81\x34\x5c\xbe\xf3\xf8\x13\x11\x4d\x8\x10\x61\x7e\x71\xdb\x6\x40\x30\x0\xd2\x0\x61\x20\x0\x0\x93\x0\x0\x0\x13\x4\x41\x2c\x10\x0\x0\x0\xc\x0\x0\x0\x44\x14\x57\xd9\x95\x42\x21\xcc\x0\x94\xec\x40\xc1\xe\x94\x6\x15\x45\x50\x2\x54\x8d\x0\x50\x34\x87\xe0\x2d\x44\xcd\x41\x30\x8c\xf2\xcd\x41\x28\x8a\xf2\x51\x34\x87\xe0\x35\x0\x0\x0\x0\x23\x6\x8\x0\x82\x60\x60\x81\x81\x32\x68\xc5\x88\x1\x2\x80\x20\x18\x58\x61\xb0\xc\x9d\x31\x62\x70\x0\x20\x8\x6\xd2\x18\x2c\xc2\x30\x62\x70\x0\x20\x8\x6\x12\x19\x30\xc2\x31\x62\x90\x0\x20\x8\x6\x46\x1a\x40\x60\x0\x6\xd9\x33\x62\x90\x0\x20\x8\x6\x86\x1a\x44\x61\x10\x6\xb\x34\x62\x90\x0\x20\x8\x6\xc6\x1a\x48\x62\x20\x6\x4d\x34\x62\x70\x0\x20\x8\x6\x8d\x1a\x54\xc5\x18\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x88\xc1\x1\x80\x20\x18\x34\x6e\x90\x25\x6a\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\x23\x6\x7\x0\x82\x60\xd0\xc8\x41\xd7\xa4\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x18\x1c\x0\x8\x82\x41\x63\x7\x61\x10\xad\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\xd8\x33\xc9\x67\xc4\x0\x1\x40\x10\xc\x1e\x3d\x38\x3\x66\xa\x46\xc\x10\x0\x4\xc1\xe0\xd9\x3\x34\x48\xa6\xc0\x2\x3\x3a\x26\x5d\xf2\x19\x31\x40\x0\x10\x4\x83\xc7\xf\xd6\xe0\xb9\x82\x11\x3\x4\x0\x41\x30\x78\xfe\x80\xd\x98\x2b\xb0\x20\x81\x8e\x55\x9b\x7c\x46\xc\x10\x0\x4\xc1\xe0\x11\x85\x37\x90\xb6\x60\xc4\x0\x1\x40\x10\xc\x9e\x51\x80\x83\x67\xb\x2c\x60\xa0\x33\x62\x70\x0\x20\x8\x6\x8d\x29\xc4\x1\x18\xfc\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x30\x8c\x18\x1c\x0\x8\x82\x41\xa3\xa\x75\x40\x6\xa6\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\xc\x23\x6\x7\x0\x82\x60\xd0\xb8\x42\x1e\xa0\x41\x29\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc3\x88\xc1\x1\x80\x20\x18\x34\xb2\xd0\x7\x6c\x70\xa\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\xc3\x60\x4f\x26\x9f\x11\x3\x4\x0\x41\x30\x78\x6c\x61\x14\x18\x2b\x18\x31\x40\x0\x10\x4\x83\xe7\x16\x48\x21\x99\x2\xb\xc\xe8\x98\xd4\xc9\x67\xc4\x0\x1\x40\x10\xc\x1e\x5d\x38\x85\x47\xb\x46\xc\x10\x0\x4\xc1\xe0\xd9\x5\x54\x60\xae\xc0\x82\x4\x3a\x56\x85\x81\x7c\x46\xc\x10\x0\x4\xc1\xe0\xf1\x85\x55\x90\xbc\x60\xc4\x0\x1\x40\x10\xc\x9e\x5f\x60\x85\x67\xb\x2c\x60\xa0\x33\x62\x90\x0\x20\x8\x6\xc8\x38\xb0\xc2\x2e\xec\x2\x2d\x24\x23\x6\x9\x0\x82\x60\x80\x8c\x3\x2b\xec\xc2\x2e\x94\x42\x31\x62\x90\x0\x20\x8\x6\xc8\x38\xb0\xc2\x2e\xec\x82\x29\x4\x23\x6\x9\x0\x82\x60\x80\x8c\x3\x2b\xec\xc2\x2e\xcc\x42\x80\x0\x0\x0\x0\x0\x0";

}