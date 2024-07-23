#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char terrain_foliage_mapPS_6_1_dxil_bytes[4425];
extern const char terrain_foliage_mapPS_6_6_dxil_bytes[4369];

auto terrain_foliage_mapPS() noexcept -> shader_def
{
   return {
      .name = "terrain_foliage_mapPS",
      .entrypoint = L"main",
      .target_6_1 = L"ps_6_1",
      .target_6_6 = L"ps_6_6",
      .file = L"terrain_foliage_mapPS.hlsl",
      .dxil_6_1 = {reinterpret_cast<const std::byte*>(terrain_foliage_mapPS_6_1_dxil_bytes),
                      sizeof(terrain_foliage_mapPS_6_1_dxil_bytes) - 1},
      .dxil_6_6 = {reinterpret_cast<const std::byte*>(terrain_foliage_mapPS_6_6_dxil_bytes),
                      sizeof(terrain_foliage_mapPS_6_6_dxil_bytes) - 1},
   };
}

const char terrain_foliage_mapPS_6_1_dxil_bytes[4425] = "\x44\x58\x42\x43\xb9\xe6\xf\x5a\xea\x3f\x4f\xd6\xd7\x42\x84\xcc\x8\x22\x82\x1f\x1\x0\x0\x0\x48\x11\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x38\x1\x0\x0\x74\x1\x0\x0\xd8\x2\x0\x0\x10\x3\x0\x0\x2c\x3\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\xe4\x0\x0\x0\x5\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\xa8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x5\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xb3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xc1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd6\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x4\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x57\x53\x0\x54\x45\x52\x52\x41\x49\x4e\x43\x4f\x4f\x52\x44\x53\x0\x41\x43\x54\x49\x56\x45\x54\x45\x58\x54\x55\x52\x45\x53\x0\x4c\x49\x47\x48\x54\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\x5c\x1\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x5\x1\x0\x5\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x4\x0\x0\x0\x4\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x11\x27\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x2\x0\x0\x0\x0\x0\x0\x0\x30\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x57\x53\x0\x54\x45\x52\x52\x41\x49\x4e\x43\x4f\x4f\x52\x44\x53\x0\x41\x43\x54\x49\x56\x45\x54\x45\x58\x54\x55\x52\x45\x53\x0\x4c\x49\x47\x48\x54\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x2\x0\x0\xc\x0\x0\x0\x0\x0\x0\x0\x1\x1\x42\x0\x3\x2\x0\x0\x1a\x0\x0\x0\x0\x0\x0\x0\x1\x2\x41\x0\x1\x1\x0\x0\x29\x0\x0\x0\x0\x0\x0\x0\x1\x3\x43\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x4\x44\x3\x3\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x30\x0\x0\x0\x0\x0\x2b\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x74\x65\x72\x72\x61\x69\x6e\x5f\x66\x6f\x6c\x69\x61\x67\x65\x5f\x6d\x61\x70\x50\x53\x5f\x36\x5f\x31\x2e\x70\x64\x62\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x19\xfc\xf8\x22\xb7\x35\x5c\xc7\xbb\x80\xae\x6\xb6\x61\x6f\x40\x44\x58\x49\x4c\x14\xe\x0\x0\x61\x0\x0\x0\x85\x3\x0\x0\x44\x58\x49\x4c\x1\x1\x0\x0\x10\x0\x0\x0\xfc\xd\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x7c\x3\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x69\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\xb8\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x80\x20\x88\x62\x20\x64\x8e\x20\x28\x3\x40\x28\xb4\xdc\x34\x5c\xfe\x84\x3d\x84\xe4\xaf\x84\xb4\x12\x93\x8f\xdc\x36\x2a\x8\x82\x20\x8\x2a\xee\x19\x2e\x7f\xc2\x1e\x42\xf2\x43\xa0\x19\x16\x2\x5\x4e\x51\x1a\xe2\x21\x8\x82\x20\x8\x82\xa0\x42\xc\xc4\x30\x90\x54\x86\x81\x18\x88\x3a\x6a\xb8\xfc\x9\x7b\x8\xc9\xe7\x36\xaa\x58\x89\xc9\x47\x6e\x1b\x11\x4\x41\x10\x85\xb0\x88\x87\xa0\xeb\xa8\xe1\xf2\x27\xec\x21\x24\x9f\xdb\xa8\x62\x25\x26\xbf\xb8\x6d\x44\xc\xc3\x30\x14\x22\x23\x1e\x82\xb4\x62\x3c\x44\x41\x10\xa\x71\x3\x1\xc3\x8\x4\x31\x13\x1d\x8c\x3\x3b\x84\xc3\x3c\xcc\x83\x1b\xd0\x42\x39\xe0\x3\x3d\xd4\x83\x3c\x94\x83\x1c\x90\x2\x1f\xd8\x43\x39\x8c\x3\x3d\xbc\x83\x3c\xf0\x41\x3d\xb8\xc3\x3c\xa4\xc3\x39\xb8\x43\x39\x90\x3\x18\xa4\x83\x3b\xd0\x3\x1b\x80\x1\x1d\xf8\x1\x18\xf8\x81\x1e\xe8\x41\x3b\xa4\x3\x3c\xcc\xc3\x2f\xd0\x43\x3e\xc0\x43\x39\xa0\x80\x98\x29\xd\xc6\x81\x1d\xc2\x61\x1e\xe6\xc1\xd\x68\xa1\x1c\xf0\x81\x1e\xea\x41\x1e\xca\x41\xe\x48\x81\xf\xec\xa1\x1c\xc6\x81\x1e\xde\x41\x1e\xf8\xa0\x1e\xdc\x61\x1e\xd2\xe1\x1c\xdc\xa1\x1c\xc8\x1\xc\xd2\xc1\x1d\xe8\x81\xd\xc0\x80\xe\xfc\x0\xc\xfc\x0\x9\x1f\x98\x0\x21\x89\xc3\x8\xc2\x30\x8c\x30\xc\x9\x41\x24\x44\x79\x99\x34\x45\x94\x30\xf9\x13\x11\x45\x0\xd2\xfc\x82\xd3\x48\x13\xd0\x4c\xd2\x8f\x2a\x72\x18\x88\x81\x20\x8\x62\x18\x88\xd2\x3c\xcf\x72\x28\xcb\xf2\xa2\x89\x88\x22\x0\x69\x7e\xc1\x69\xa4\x9\x68\x26\x9\x5\x28\xa5\x69\x41\xcc\x11\x80\xc2\x14\x0\x0\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x2c\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x81\x80\x0\x18\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x23\x1\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x21\xcf\x5\x4\xc0\x0\x0\x0\x0\x0\x0\x0\x0\x43\x1e\xd\x8\x80\x1\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x1c\x10\x0\x3\x0\x0\x0\x0\x0\x0\x0\x64\x81\x0\x15\x0\x0\x0\x32\x1e\x98\x18\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x1c\x8a\x61\x4\xa0\x44\x39\x3\x6a\xa0\x8\x4a\xa1\x10\xa\xbf\xa1\xc\xca\x83\x8a\x92\x18\x1\x28\x82\x32\x28\x85\x12\x28\x84\x2\x21\xb6\x20\x48\x9d\x1\xa0\x75\x2c\x8b\x22\x1e\xe0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x68\x9c\x1\x0\x0\x0\x0\x79\x18\x0\x0\x7c\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x8\x65\x83\x30\xc\x13\x4\x62\xd9\x20\xc\x5\x5\xb8\xb9\x9\x2\xc1\x6c\x18\xe\x64\x98\x20\x10\xcd\x4\x61\xc\xc2\x80\xc0\x4\x81\x70\x26\x8\xc4\x33\x41\x20\xa0\x9\x2\x11\x6d\x10\x14\x68\x43\xa2\x2c\x4c\xa3\x38\x8f\x12\x6d\x8\xa4\x9\x82\x1a\x7c\x13\x4\x42\x9a\x20\x10\xd3\x6\x44\xa1\x18\xa5\x1a\x2c\x60\x43\x70\x6d\x20\x26\x0\x3\x26\x8\x6b\x0\x6\x1b\x2\x6d\x82\x20\x0\x24\xda\xc2\xd2\xdc\xa8\x40\x3d\x4d\x25\x51\x25\x3d\x39\x5d\x4d\x4d\x10\xa\x6b\x82\x50\x5c\x1b\x2\x65\x82\x50\x60\x13\x84\x22\x9b\x20\x10\xd4\x6\x81\xc\xa0\xd\x8b\xe2\x7d\x60\x10\x6\x62\x30\x8c\x81\x2\x6\x65\xc0\x86\xaa\x48\x4a\x2a\x28\xc9\x69\xe8\xe9\x49\x8a\x68\x6a\xc3\x32\x9c\xc1\x7\x6\x61\x20\x6\x83\x18\xc\x60\x0\xd0\x9\x1a\xa2\x4a\xb2\x2a\xa2\x2a\xc2\xa2\xaa\x92\x2a\x9a\x9a\x20\x14\xda\x4\xa1\xd8\x36\x2c\x4f\x1a\xa8\x1\x18\x84\xc1\x1a\xc\x6b\xf0\x80\x1\xc0\x82\x29\xe9\x8\x89\x6a\xc3\x42\x6\x6d\xf0\x81\x41\x18\x88\xc1\x30\x6\x64\x0\x6\x0\x97\x29\xab\x2f\xa8\xb7\xb9\x34\xba\xb4\x37\xb7\x9\x42\xc1\x6d\x58\xaa\x37\xf8\xc6\x20\xc\xe0\x60\x80\x83\xa\xc\x80\xd\x85\x19\xa0\x1\x1b\xb8\x41\x1c\x30\x99\xb2\xfa\xa2\xa\x93\x3b\x2b\xa3\x9b\x20\x14\xdd\x4\x81\xa8\x36\x8\x64\x50\x7\x1b\x16\x65\xe\x3e\x3a\x8\x3\x30\x18\xe0\x40\x1\x3\x3b\xd8\x10\xdc\xc1\x86\x41\xe\xf0\x0\x98\x20\xb0\x81\xb7\x41\x50\xf4\x60\x43\xc1\x75\x79\x90\xed\x41\x15\x36\x36\xbb\x36\x97\x34\xb2\x32\x37\xba\x29\x41\x50\x85\xc\xcf\xc5\xae\x4c\x6e\x2e\xed\xcd\x6d\x4a\x30\x34\x21\xc3\x73\xb1\xb\x63\xb3\x2b\x93\x9b\x12\x14\x75\xc8\xf0\x5c\xe6\xd0\xc2\xc8\xca\xe4\x9a\xde\xc8\xca\xd8\xa6\x4\x48\x19\x32\x3c\x17\xb9\xb2\xb9\xb7\x3a\xb9\xb1\xb2\xb9\x29\x1\x56\x87\xc\xcf\xc5\x2e\xad\xec\x2e\x89\x6c\x8a\x2e\x8c\xae\x6c\x4a\xa0\xd5\x21\xc3\x73\x29\x73\xa3\x93\xcb\x83\x7a\x4b\x73\xa3\x9b\x9b\x12\xec\x1\x0\x0\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\x28\x0\x0\x0\x56\x0\xd\x97\xef\x3c\x7e\x80\x34\x40\x84\xf9\xc5\x6d\x1b\xc2\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x1d\x6c\xc3\xe5\x3b\x8f\x2f\x4\x54\x51\x10\x51\xe9\x0\x43\x49\x18\x80\x80\xf9\xc8\x6d\x5b\x82\x34\x5c\xbe\xf3\xf8\x42\x44\x0\x13\x11\x2\xcd\xb0\x10\x36\xd0\xc\x97\xef\x3c\xfe\x80\x48\x2\x10\xd\x16\x30\xd\x97\xef\x3c\xfe\xe2\x0\x83\xd8\x3c\xd4\xe4\x17\xb7\x6d\x2\xd5\x70\xf9\xce\xe3\x4b\x93\x13\x11\x28\x35\x3d\xd4\xe4\x17\xb7\x6d\x4\xd5\x70\xf9\xce\xe3\x4f\xc4\x35\x51\x11\x51\x3a\xc0\xe0\x23\xb7\x6d\x6\xcf\x70\xf9\xce\xe3\x53\xd\x10\x61\x7e\x71\xdb\x6\x40\x30\x0\xd2\x0\x0\x61\x20\x0\x0\x94\x1\x0\x0\x13\x4\x4d\x2c\x10\x0\x0\x0\x1a\x0\x0\x0\xa4\xd4\xc0\x8\x0\x11\xe5\x56\x10\xc5\x56\x6a\x85\x56\x84\x1\x45\x1a\x50\x72\x85\x1a\x50\x34\x65\x54\x76\x33\x0\x65\x1a\x40\xc3\x18\x1\x8\x82\x20\xdc\x8d\x11\x80\x20\x8\xea\xbf\x30\x46\x0\x82\x20\x88\x7f\x63\x4\x20\x8\x82\x24\x18\x8c\x11\x80\x20\x8\xf2\xdf\x18\x1\x8\x82\x20\xfd\x8d\x11\x80\x20\x8\xd2\xbf\x30\x2\x30\x46\x0\x82\x20\x88\xfe\xc2\x18\x1\x8\x82\x20\xfc\x8d\x11\x80\x20\x8\x82\x60\x0\x23\x6\x9\x0\x82\x60\xc0\xd1\x81\x54\x6\x70\xb0\x6\xda\x88\x41\x2\x80\x20\x18\x18\x7d\xc0\x6\x71\x10\x7\x67\xe0\x8c\x18\x24\x0\x8\x82\x81\xe1\x7\x6d\x20\x7\x72\x70\x6\xcf\x88\xc1\x1\x80\x20\x18\x5c\x79\x10\xd\x70\x30\x9a\x10\x4\x15\xd4\x1\x8c\x18\x24\x0\x8\x82\x1\xa7\x7\x18\x1b\xd8\x41\x0\x6\x23\x6\x7\x0\x82\x60\xa0\xf1\x41\x75\xdc\xc1\x68\x42\x0\x8c\x26\x8\x81\x9\x9\x7c\x4c\x48\xe0\x33\x9a\x50\x8\x26\x4\xf0\x31\x42\xa0\x8f\x9\x3\x7d\x4e\x20\xe8\x4\x82\x4a\xf8\x3\x2a\x1\x14\xc8\x2\x51\x0\xc1\x88\x41\x3\x80\x20\x18\x40\xae\x90\x6\x4f\x29\xc\x81\x18\x88\x81\x18\x88\xc1\x68\x42\x0\xc\x37\x4\xa7\x0\x6\xb3\xc\x81\x10\x8c\x18\x18\x0\x8\x82\xc1\x22\xb\x6b\x30\x7\xb3\x4\xc2\x88\xc1\x1\x80\x20\x18\x68\xad\x60\x6\x58\x1b\x8c\x26\x4\xc3\x88\xc1\x1\x80\x20\x18\x68\xaf\x80\x6\xda\x29\x8c\x26\x4\x82\x5\x92\x7c\x4c\x80\xe4\x53\x48\x2c\xe8\x70\x43\xf0\xa\x60\x30\xcb\x40\xc\x81\x11\x65\x0\x1f\x23\xcc\x0\x3e\x26\x90\x81\x7c\x4c\x28\x3\xf9\x8c\x18\x18\x0\x8\x82\x1\xb5\xb\x74\x20\x8c\x18\x18\x0\x8\x82\x1\xc5\xb\x75\x20\x98\x70\x6\xf2\x31\x1\xd\xe4\x63\x88\x10\x1f\x43\x84\xf8\x8c\x18\x18\x0\x8\x82\x1\x15\xe\xbb\x20\x8c\x18\x18\x0\x8\x82\x1\x25\xe\xbc\x20\x98\xf0\x6\xf0\x31\x1\xe\xe0\x33\x62\x70\x0\x20\x8\x6\x92\x39\xf4\x81\x10\x8c\x18\x18\x0\x8\x82\x1\x65\xe\x7b\x10\x8c\x18\x18\x0\x8\x82\x1\x75\xe\xe1\x10\x8c\x18\x18\x0\x8\x82\x1\x85\xe\xa2\x30\x8c\x18\x18\x0\x8\x82\x1\x95\xe\xe3\x10\x58\x30\xc0\xc7\x2\x3d\x90\x8f\x9\x7c\x20\x1f\x13\x10\xf8\x58\x20\xd0\x67\xc4\xc0\x0\x40\x10\xc\xa8\x77\x50\x87\xc0\x2\x3d\x90\x8f\x89\x42\x10\x1f\x1b\x6\xf9\x58\x20\xc8\xc7\x4c\x21\x88\xcf\x68\x82\x19\x0\xa3\x9\x67\x10\x8c\x26\xa0\x81\x60\xc4\x20\x1f\x2b\x6\xf9\x98\x31\xc8\xc7\x8e\x35\x90\xcf\x88\xc1\x1\x80\x20\x18\x48\xfd\x40\xb\xa9\x10\xcc\x12\x10\x3\x15\x83\x32\xd0\x84\x30\x50\x31\x28\x83\x4d\x8\x3\x15\x83\x32\xe0\x84\x30\x50\x31\x20\x83\x4e\x8\x45\x7\xf7\xa0\xc3\xd\xc1\x3e\x80\xc1\x2c\x83\x51\x4\x6\x7\xb1\x0\x1f\x83\x83\x5b\x80\x8f\x9\xb0\x20\x1f\x13\x62\x41\x3e\x23\x6\x6\x0\x82\x60\x40\x9d\x4\x38\x8\x23\x6\x6\x0\x82\x60\x40\xa1\x44\x38\x8\x26\xcc\x82\x7c\x4c\xa0\x5\xf9\x18\x22\xc4\xc7\x10\x21\x3e\x23\x6\x6\x0\x82\x60\x40\xb5\xc4\x49\x8\x23\x6\x6\x0\x82\x60\x40\xb9\x4\x4a\x8\x26\xec\x2\x7c\x4c\xe0\x5\xf8\x8c\x18\x1c\x0\x8\x82\x81\x24\x13\xe9\x20\x4\x23\x6\x6\x0\x82\x60\x40\xc9\xc4\x39\x4\x23\x6\x6\x0\x82\x60\x40\xcd\x44\x4b\x4\x23\x6\x6\x0\x82\x60\x40\xd1\x84\x3b\xc\x23\x6\x6\x0\x82\x60\x40\xd5\xc4\x4b\x4\x16\xc\xf0\xb1\xc0\x1c\xe4\x63\x2\x3a\xc8\xc7\x4\x4\x3e\x16\x8\xf4\x19\x31\x30\x0\x10\x4\x3\x6a\x27\x6c\x22\xb0\xc0\x1c\xe4\x63\xee\x10\xc4\xc7\x86\x41\x3e\x16\x8\xf2\x31\x79\x8\xe2\x33\x62\x70\x0\x20\x8\x6\xda\x4f\xe0\x83\x3a\x80\xc4\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\xd8\x80\x6\xf1\xb1\x1\xd\xe2\x63\x3\x1a\xc4\xc7\x90\x41\x3e\x96\xc\xf2\x31\x65\x90\x8f\xd\x6e\x0\x1f\x1b\xdc\x0\x3e\x36\xb8\x1\x7c\xcc\xf1\x5\xf9\x8c\x18\x1c\x0\x8\x82\x81\x4\x17\x27\xf1\x6\xc1\x2c\x81\x31\x50\x31\x28\x85\x39\x10\x3\x15\x83\x52\x98\x3\x31\x50\x31\x28\x85\x39\x10\x3\x15\x3\x52\x98\x3\x51\xe7\x80\x16\x3a\xdc\x10\xb8\x5\x18\xcc\x32\x20\x47\x60\xe3\x90\x12\xf0\xb1\x71\x28\x9\xf8\x98\x30\x12\xf2\x31\x81\x24\xe4\x33\x62\x60\x0\x20\x8\x6\x94\x5e\xcc\x84\x30\x62\x60\x0\x20\x8\x6\xd4\x5e\xd0\x84\x60\x82\x49\xc8\xc7\x84\x93\x90\x8f\x21\x42\x7c\xc\x11\xe2\x33\x62\x60\x0\x20\x8\x6\x14\x68\xe8\x85\x30\x62\x60\x0\x20\x8\x6\x54\x68\xec\x85\x60\x82\x4b\xc0\xc7\x84\x97\x80\xcf\x88\xc1\x1\x80\x20\x18\x48\xa5\xc1\x13\x42\x30\x62\x60\x0\x20\x8\x6\x54\x69\xe8\x44\x30\x62\x60\x0\x20\x8\x6\x94\x69\x80\x46\x30\x62\x60\x0\x20\x8\x6\xd4\x69\x84\xc5\x30\x62\x60\x0\x20\x8\x6\x14\x6a\x88\x46\x60\xc1\x0\x1f\xb\x72\x42\x3e\x26\xec\x84\x7c\x4c\x40\xe0\x63\x81\x40\x9f\x11\x3\x3\x0\x41\x30\xa0\x5c\x23\x35\x2\xb\x72\x42\x3e\x16\x16\x41\x7c\x6c\x18\xe4\x63\x81\x20\x1f\x2b\x8b\x20\x3e\x23\x6\x7\x0\x82\x60\xa0\xc9\xc6\x5a\xf4\x4\x5d\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\x82\xd\x68\x10\x1f\x1b\xd0\x20\x3e\x36\xa0\x41\x7c\xc\x19\xe4\x63\xc9\x20\x1f\x53\x6\xf9\xd8\xe0\x6\xf0\xb1\xc1\xd\xe0\x63\x83\x1b\xc0\xc7\x9c\x98\x90\xcf\x88\xc1\x1\x80\x20\x18\x48\xe3\xa1\x17\x6f\x10\xcc\x12\x20\x3\x15\x83\x72\x98\x83\x31\x50\x31\x28\x87\x39\x18\x3\x15\x83\x72\x98\x83\x31\x50\x31\x20\x87\x39\x18\xa5\x13\xa8\xa1\xc3\xd\x41\x78\x80\xc1\x2c\x83\x92\x4\x66\x13\x7c\x1\x1f\xb3\x89\xbe\x80\x8f\x9\x76\x21\x1f\x13\xee\x42\x3e\x23\x6\x6\x0\x82\x60\x40\xb5\x87\x69\x8\x23\x6\x6\x0\x82\x60\x40\xb9\xc7\x69\x8\x26\xe4\x85\x7c\x4c\xd0\xb\xf9\x18\x22\xc4\xc7\x10\x21\x3e\x23\x6\x6\x0\x82\x60\x40\xcd\x47\x7b\x8\x23\x6\x6\x0\x82\x60\x40\xd1\x87\x7b\x8\x26\x84\x6\x7c\x4c\x10\xd\xf8\x8c\x18\x1c\x0\x8\x82\x81\x84\x1f\xaf\x21\x4\x23\x6\x6\x0\x82\x60\x40\xe1\x47\x6b\x4\x23\x6\x6\x0\x82\x60\x40\xe5\xc7\x7c\x4\x23\x6\x6\x0\x82\x60\x40\xe9\x7\x6d\xc\x23\x6\x6\x0\x82\x60\x40\xed\x47\x7d\x4\x16\xc\xf0\xb1\x80\x35\xe4\x63\x82\x6b\xc8\xc7\x4\x4\x3e\x16\x8\xf4\x19\x31\x30\x0\x10\x4\x3\x2a\x44\xf8\x23\xb0\x80\x35\xe4\x63\xb4\x11\xc4\xc7\x86\x41\x3e\x16\x8\xf2\x31\xdc\x8\xe2\x33\x62\x70\x0\x20\x8\x6\x5a\x89\xf8\x6\x6c\xa4\xc7\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\xd8\x80\x6\xf1\xb1\x1\xd\xe2\x63\x3\x1a\xc4\xc7\x90\x41\x3e\x96\xc\xf2\x31\x65\x90\x8f\xd\x6e\x0\x1f\x1b\xdc\x0\x3e\x36\xb8\x1\x7c\xcc\x21\xd\xf9\x8c\x18\x1c\x0\x8\x82\x81\x64\x23\xed\xf1\x6\xc1\x2c\x81\x32\x50\x31\x28\x89\x39\x20\x3\x15\x83\x92\x98\x3\x32\x50\x31\x28\x89\x39\x20\x3\x15\x3\x92\x98\x3\x32\x1c\x11\xb8\x87\xf2\xcd\x32\x30\x4b\x30\x62\x60\x0\x20\x8\x6\x8b\x8f\xdc\xc7\x7f\xcc\x12\x30\x23\x6\x9\x0\x82\x60\x80\xfc\x8\x8c\xd0\x8\x8d\xa8\x48\x31\x62\x90\x0\x20\x8\x6\xc8\x8f\xc0\x8\x8d\xd0\x88\x89\x10\x23\x6\x9\x0\x82\x60\x80\xfc\x8\x8c\xd0\x8\x8d\xa4\xc8\x30\x62\x90\x0\x20\x8\x6\xc8\x8f\xc0\x8\x8d\xd0\x8\x8a\x8\x8\x0\x0\x0\x0";

const char terrain_foliage_mapPS_6_6_dxil_bytes[4369] = "\x44\x58\x42\x43\x86\x26\xae\xe9\xa0\x98\xf4\x76\xae\x64\xba\x9e\x46\x12\x9f\xd1\x1\x0\x0\x0\x10\x11\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x38\x1\x0\x0\x74\x1\x0\x0\xc0\x2\x0\x0\xf8\x2\x0\x0\x14\x3\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x0\x49\x53\x47\x31\xe4\x0\x0\x0\x5\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\xa8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x5\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xb3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xc1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd6\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x4\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x57\x53\x0\x54\x45\x52\x52\x41\x49\x4e\x43\x4f\x4f\x52\x44\x53\x0\x41\x43\x54\x49\x56\x45\x54\x45\x58\x54\x55\x52\x45\x53\x0\x4c\x49\x47\x48\x54\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\x44\x1\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x5\x1\x0\x5\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x4\x0\x0\x0\x4\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x30\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x57\x53\x0\x54\x45\x52\x52\x41\x49\x4e\x43\x4f\x4f\x52\x44\x53\x0\x41\x43\x54\x49\x56\x45\x54\x45\x58\x54\x55\x52\x45\x53\x0\x4c\x49\x47\x48\x54\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x2\x0\x0\xc\x0\x0\x0\x0\x0\x0\x0\x1\x1\x42\x0\x3\x2\x0\x0\x1a\x0\x0\x0\x0\x0\x0\x0\x1\x2\x41\x0\x1\x1\x0\x0\x29\x0\x0\x0\x0\x0\x0\x0\x1\x3\x43\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x4\x44\x3\x3\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x30\x0\x0\x0\x0\x0\x2b\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x74\x65\x72\x72\x61\x69\x6e\x5f\x66\x6f\x6c\x69\x61\x67\x65\x5f\x6d\x61\x70\x50\x53\x5f\x36\x5f\x36\x2e\x70\x64\x62\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x75\xfe\xf2\x8f\xfd\x5a\x39\x64\x87\x92\x8c\xd3\x10\x4b\x7d\x5a\x44\x58\x49\x4c\xf4\xd\x0\x0\x66\x0\x0\x0\x7d\x3\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\xdc\xd\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x74\x3\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x4f\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\xbc\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x80\x20\x88\x62\x20\x64\x8e\x20\x28\x3\x40\x28\xb4\x50\x71\xcf\x70\xf9\x13\xf6\x10\x92\x1f\x2\xcd\xb0\x10\x28\x68\x4a\xe1\x10\x84\xa2\xd0\x73\xd3\x70\xf9\x13\xf6\x10\x92\xbf\x12\xd2\x4a\x4c\x3e\x72\xdb\xa8\x20\x8\x82\x20\x8a\x12\x11\xe\x41\x10\x4\x41\x10\x24\x15\x62\x20\x86\x81\xa8\x32\xc\xc4\x40\xd6\x51\xc3\xe5\x4f\xd8\x43\x48\x3e\xb7\x51\xc5\x4a\x4c\x3e\x72\xdb\x88\x20\x8\x82\x28\x4\x46\x38\x4\x65\x47\xd\x97\x3f\x61\xf\x21\xf9\xdc\x46\x15\x2b\x31\xf9\xc5\x6d\x23\x62\x18\x86\xa1\x10\x1b\xe1\x10\xc4\xdd\x36\x5c\xfe\x84\x3d\x84\xe4\xaf\x84\xe4\x50\x91\x40\xa4\x91\xf3\x10\xd1\x84\x10\x12\x12\x8\xa2\x10\xe\xe1\x78\xf4\x1d\x34\x5c\xfe\x84\x3d\x84\xe4\xaf\x84\xb4\x21\xcd\x80\x8\x82\x20\x8a\x52\x38\x44\x48\x28\x24\xe\x4\xc\x23\x8\xc3\x30\xc2\x30\x24\x4\x91\x10\xe6\x65\xd2\x14\x51\xc2\xe4\x4f\x44\x14\x1\x48\xf3\xb\x4e\x23\x4d\x40\x33\x49\x3f\xaa\xca\x61\x20\x6\x82\x20\x88\x61\x20\xcc\x13\x45\xcd\xc1\x34\xcd\x8b\x26\x22\x8a\x0\xa4\xf9\x5\xa7\x91\x26\xa0\x99\x24\x14\xa4\xa4\xa6\x5\x31\x47\x0\xa\x53\x0\x0\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x2c\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x79\x80\x0\x8\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x23\x1\x1\x30\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x21\x8f\x5\x4\x40\x0\x0\x0\x0\x0\x0\x0\x0\x43\x9e\xc\x8\x80\x1\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x1c\x10\x0\x3\x0\x0\x0\x0\x0\x0\x0\xc\x79\x3e\x20\x0\x2\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x88\x1\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\x64\x81\x0\x0\x14\x0\x0\x0\x32\x1e\x98\x18\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\x10\xa\xbf\xa1\xc\x4a\xa1\x8\xca\x83\x8a\x92\x18\x1\x28\x82\x32\x28\x85\x12\x28\x84\x2\xa1\xb6\x20\x1\x1\x1\x11\x88\x1d\xcb\xa2\x88\x7\x78\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x5a\x67\x0\x0\x0\x0\x0\x79\x18\x0\x0\x70\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x8\x66\x82\x40\x34\x1b\x84\x81\x98\x20\x10\xce\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\x10\xcf\x4\x61\xd\xc0\x80\xc0\x4\x81\x80\x26\x8\x44\xb4\x1\x51\x16\x46\x69\x6\x7\xd8\x10\x3c\x1b\x8\x0\x80\x80\x9\x2\x1b\x7c\x1b\x2\x69\x82\x20\x0\x24\xda\xc2\xd2\xdc\xa8\x40\x3d\x4d\x25\x51\x25\x3d\x39\x5d\x4d\x4d\x10\xa\x6b\x82\x50\x5c\x1b\x2\x65\x82\x50\x60\x13\x84\x22\x9b\x20\x10\xd2\x4\x81\x98\x36\x8\x5c\xb7\x61\x51\xac\xb\xcb\xb4\x61\x53\x30\x8f\xd\x55\x91\x94\x54\x50\x92\xd3\xd0\xd3\x93\x14\xd1\xd4\x86\x65\x0\x83\xb\xcb\xb4\x41\x1b\x30\x60\x82\x40\x50\x74\x82\x86\xa8\x92\xac\x8a\xa8\x8a\xb0\xa8\xaa\xa4\x8a\xa6\x26\x8\x85\x36\x41\x28\xb6\xd\x8b\x18\x8c\x1\x19\x60\x59\x19\xc\x65\x20\x6\x18\xc0\x82\x29\xe9\x8\x89\x6a\xc3\xc2\x9d\xc1\x85\x65\xda\xb0\x71\x18\xc0\x65\xca\xea\xb\xea\x6d\x2e\x8d\x2e\xed\xcd\x6d\x82\x50\x70\x1b\x96\x26\xd\xae\x2d\x53\x83\x41\xd\x1a\xc\xd8\x50\x7c\x61\x60\x6\x68\xb0\x6\x4c\xa6\xac\xbe\xa8\xc2\xe4\xce\xca\xe8\x26\x8\x45\x37\x41\x20\xaa\xd\x2\xf7\x6\x1b\x16\xa5\xd\x2e\x37\xc8\xb0\x41\xd\x14\xc\xe\x36\x4\x71\xb0\x61\x60\x3\x39\x0\x26\x8\x6d\xe0\x6d\x10\x14\x3a\xd8\x50\x50\xd5\x1c\x44\x75\x50\x85\x8d\xcd\xae\xcd\x25\x8d\xac\xcc\x8d\x6e\x4a\x10\x54\x21\xc3\x73\xb1\x2b\x93\x9b\x4b\x7b\x73\x9b\x12\x10\x4d\xc8\xf0\x5c\xec\xc2\xd8\xec\xca\xe4\xa6\x4\x46\x1d\x32\x3c\x97\x39\xb4\x30\xb2\x32\xb9\xa6\x37\xb2\x32\xb6\x29\x1\x52\x86\xc\xcf\x45\xae\x6c\xee\xad\x4e\x6e\xac\x6c\x6e\x4a\x0\xd5\x21\xc3\x73\xb1\x4b\x2b\xbb\x4b\x22\x9b\xa2\xb\xa3\x2b\x9b\x12\x48\x75\xc8\xf0\x5c\xca\xdc\xe8\xe4\xf2\xa0\xde\xd2\xdc\xe8\xe6\xa6\x4\x75\x0\x0\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\x34\x0\x0\x0\xa6\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x98\x1\x34\x5c\xbe\xf3\xf8\x1\xd2\x0\x11\xe6\x17\xb7\x6d\x9\xdb\x70\xf9\xce\xe3\xb\x1\x55\x14\x44\x54\x3a\xc0\x50\x12\x6\x20\x60\x7e\x71\xdb\x86\xb0\xd\x97\xef\x3c\xbe\x10\x50\x45\x41\x44\xa5\x3\xc\x25\x61\x0\x2\xe6\x23\xb7\x6d\xb\xdd\x70\xf9\xce\xe3\xb\x11\x1\x4c\x44\x8\x34\xc3\x42\x7c\x91\xc3\x6c\x48\x33\x20\x8d\x61\x4\xda\x70\xf9\xce\xe3\xb\x11\x1\x4c\x44\x8\x34\xc3\x42\x7c\x91\xc3\x84\x4\xf0\xd8\x40\x33\x5c\xbe\xf3\xf8\x3\x22\x9\x40\x34\x58\xc0\x34\x5c\xbe\xf3\xf8\x8b\x3\xc\x62\xf3\x50\x93\x5f\xdc\xb6\x9\x54\xc3\xe5\x3b\x8f\x2f\x4d\x4e\x44\xa0\xd4\xf4\x50\x93\x5f\xdc\xb6\x15\x54\xc3\xe5\x3b\x8f\x3f\x11\xd7\x44\x45\x44\xe9\x0\x83\x8f\xdc\xb6\x1d\x3c\xc3\xe5\x3b\x8f\x4f\x35\x40\x84\xf9\xc5\x6d\x1b\x0\xc1\x0\x48\x3\x0\x0\x61\x20\x0\x0\xa0\x1\x0\x0\x13\x4\x4d\x2c\x10\x0\x0\x0\x21\x0\x0\x0\xa4\xd4\xc0\x8\x0\x11\xe5\x56\x10\xc5\x56\x6a\x85\x56\x84\x1\x35\x50\x8a\x10\x45\x3b\x50\x34\x65\x54\x76\x33\x0\x65\x1a\x50\xa8\x1\x5\x3b\x50\xa4\x1\x25\x3b\x50\x1a\x34\x8c\x11\x80\x20\x8\xc2\xdd\x18\x1\x8\x82\xa0\xfe\xb\x63\x4\x20\x8\x82\xf8\x37\x46\x0\x82\x20\x48\x82\xc1\x18\x1\x8\x82\x20\xff\x8d\x11\x80\x20\x8\xd2\xdf\x18\x1\x8\x82\x20\xfd\xb\x23\x0\x63\x4\x20\x8\x82\xe8\x2f\x8c\x11\x80\x20\x8\xc2\xdf\x18\x1\x8\x82\x20\x8\x6\xe4\xcd\x21\xd4\x41\x34\x87\x40\xa9\x1\x85\x73\x10\x10\xf4\x60\x0\x23\x6\x8\x0\x82\x60\x20\x6\x79\x0\x5\x74\x20\x6\x23\x6\x7\x0\x82\x60\xf0\xed\xc1\x14\x10\x23\x6\x9\x0\x82\x60\x60\x94\x82\x1d\xdc\xc1\x1d\xbc\xc1\x35\x62\x90\x0\x20\x8\x6\x86\x29\xdc\x1\x1e\xe0\xc1\x1b\x60\x23\x6\x7\x0\x82\x60\x90\x81\x82\x36\xd0\xc1\x68\x42\x10\x8c\x18\x20\x0\x8\x82\xc1\x63\xa\x5f\x80\x6\x68\x30\x62\x70\x0\x20\x8\x6\x5f\x28\x64\x41\x32\x62\x70\x0\x20\x8\x6\xdc\x28\x78\x47\x1f\x8c\x26\x4\xc0\x68\x82\x10\x98\x90\xc0\xc7\x84\x4\x3e\xa3\x9\x85\x60\x42\x0\x1f\x23\x4\xfa\x98\x30\xd0\xe7\x4\x82\x4e\x20\xa8\x84\x51\xa0\x12\x48\x81\x2c\x80\x3\x10\x8c\x18\x34\x0\x8\x82\x81\x54\xb\x72\xf0\xac\xc2\x10\xac\xc1\x1a\xac\xc1\x1a\x8c\x26\x4\xc0\x70\x43\xd0\xa\x60\x30\xcb\x10\x8\xc1\x88\x81\x1\x80\x20\x18\x2c\xba\xa0\x6\x7c\x30\x4b\x20\x8c\x18\x1c\x0\x8\x82\x1\x47\xb\x6f\x80\xd9\xc1\x68\x42\x30\x8c\x18\x1c\x0\x8\x82\x1\x67\xb\x71\xa0\xad\xc2\x68\x42\x20\x58\x20\xc9\xc7\x4\x48\x3e\x85\xdc\x82\xe\x37\x4\xb5\x0\x6\xb3\xc\xc4\x10\x18\x91\x6\xf0\x31\x42\xd\xe0\x63\x2\x1a\xc8\xc7\x84\x34\x90\xcf\x88\x81\x1\x80\x20\x18\x58\xe2\xd0\x7\xc2\x88\x81\x1\x80\x20\x18\x58\xe3\xe0\x7\x82\x9\x6b\x20\x1f\x13\xd8\x40\x3e\x86\x8\xf1\x31\x44\x88\xcf\x88\x81\x1\x80\x20\x18\x58\xe8\x20\xe\xc2\x88\x81\x1\x80\x20\x18\x58\xe9\x30\xe\x82\x9\x73\x0\x1f\x13\xe8\x0\x3e\x23\x6\x7\x0\x82\x60\x40\xb5\x83\x29\x8\xc1\x88\x81\x1\x80\x20\x18\x58\xed\x40\xa\xc1\x88\x81\x1\x80\x20\x18\x58\xee\x80\xe\xc1\x88\x81\x1\x80\x20\x18\x58\xef\x50\xa\xc3\x88\x81\x1\x80\x20\x18\x58\xf0\xa0\xe\x81\x5\x3\x7c\x2c\xf0\x3\xf9\x98\x0\xa\xf2\x31\x1\x81\x8f\x5\x2\x7d\x46\xc\xc\x0\x4\xc1\xc0\xb2\x87\x77\x8\x2c\xf0\x3\xf9\x98\x29\x4\xf1\xb1\x61\x90\x8f\x5\x82\x7c\x4c\x15\x82\xf8\x8c\x26\x98\x1\x30\x9a\x70\x6\xc1\x68\x2\x1a\x8\x46\xc\xf2\xb1\x62\x90\x8f\x19\x83\x7c\xec\x58\x3\xf9\x8c\x18\x1c\x0\x8\x82\x1\x45\x12\xbd\xd0\xa\xc1\x2c\x1\x31\x50\x31\x28\x3\x4f\x8\x3\x15\x83\x32\xf8\x84\x30\x50\x31\x28\x3\x58\x8\x3\x15\x3\x32\x88\x85\x50\x74\xb0\xf\x3a\xdc\x10\x84\x4\x18\xcc\x32\x18\x45\x60\x70\x50\xb\xf0\x31\x38\xd8\x5\xf8\x98\x40\xb\xf2\x31\xa1\x16\xe4\x33\x62\x60\x0\x20\x8\x6\x96\x4b\xa4\x83\x30\x62\x60\x0\x20\x8\x6\xd6\x4b\xa8\x83\x60\xc2\x2d\xc8\xc7\x4\x5c\x90\x8f\x21\x42\x7c\xc\x11\xe2\x33\x62\x60\x0\x20\x8\x6\x16\x4d\xb8\x84\x30\x62\x60\x0\x20\x8\x6\x56\x4d\xbc\x84\x60\xc2\x2f\xc0\xc7\x4\x70\x80\xcf\x88\xc1\x1\x80\x20\x18\x50\x39\x21\xf\x42\x30\x62\x60\x0\x20\x8\x6\x56\x4e\xc0\x43\x30\x62\x60\x0\x20\x8\x6\x96\x4e\xd0\x44\x30\x62\x60\x0\x20\x8\x6\xd6\x4e\xc4\xc3\x30\x62\x60\x0\x20\x8\x6\x16\x4f\xd8\x44\x60\xc1\x0\x1f\xb\xd4\x41\x3e\x26\xb0\x83\x7c\x4c\x40\xe0\x63\x81\x40\x9f\x11\x3\x3\x0\x41\x30\xb0\xc4\x62\x27\x2\xb\xd4\x41\x3e\x26\xf\x41\x7c\x6c\x18\xe4\x63\x81\x20\x1f\xb3\x87\x20\x3e\x23\x6\x7\x0\x82\x60\xc0\x99\x45\x48\xa8\x43\x4a\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\x82\xd\x68\x10\x1f\x1b\xd0\x20\x3e\x36\xa0\x41\x7c\xc\x19\xe4\x63\xc9\x20\x1f\x53\x6\xf9\xd8\xe0\x6\xf0\xb1\xc1\xd\xe0\x63\x83\x1b\xc0\xc7\x1c\x5f\x90\xcf\x88\xc1\x1\x80\x20\x18\x50\x77\x1\x13\x6f\x10\xcc\x12\x18\x3\x15\x83\x52\x98\x3\x31\x50\x31\x28\x85\x39\x10\x3\x15\x83\x52\x98\x3\x31\x50\x31\x20\x85\x39\x10\x75\xe\x72\xa1\xc3\xd\x1\x5d\x80\xc1\x2c\x3\x72\x4\x36\xe\x2d\x1\x1f\x1b\x87\x94\x80\x8f\x9\x27\x21\x1f\x13\x50\x42\x3e\x23\x6\x6\x0\x82\x60\x60\x85\x6\x4f\x8\x23\x6\x6\x0\x82\x60\x60\x89\x46\x4f\x8\x26\xa8\x84\x7c\x4c\x58\x9\xf9\x18\x22\xc4\xc7\x10\x21\x3e\x23\x6\x6\x0\x82\x60\x60\x9d\x46\x68\x8\x23\x6\x6\x0\x82\x60\x60\xa1\x86\x68\x8\x26\xc8\x4\x7c\x4c\x98\x9\xf8\x8c\x18\x1c\x0\x8\x82\x1\xc5\x1a\x65\x21\x4\x23\x6\x6\x0\x82\x60\x60\xb1\xc6\x58\x4\x23\x6\x6\x0\x82\x60\x60\xb5\xc6\x69\x4\x23\x6\x6\x0\x82\x60\x60\xb9\x6\x59\xc\x23\x6\x6\x0\x82\x60\x60\xbd\x46\x6a\x4\x16\xc\xf0\xb1\xa0\x27\xe4\x63\xc2\x4f\xc8\xc7\x4\x4\x3e\x16\x8\xf4\x19\x31\x30\x0\x10\x4\x3\xab\x36\x5c\x23\xb0\xa0\x27\xe4\x63\x65\x11\xc4\xc7\x86\x41\x3e\x16\x8\xf2\xb1\xb4\x8\xe2\x33\x62\x70\x0\x20\x8\x6\x5c\x6e\xd0\x45\x4f\xf4\xc5\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\xd8\x80\x6\xf1\xb1\x1\xd\xe2\x63\x3\x1a\xc4\xc7\x90\x41\x3e\x96\xc\xf2\x31\x65\x90\x8f\xd\x6e\x0\x1f\x1b\xdc\x0\x3e\x36\xb8\x1\x7c\xcc\x89\x9\xf9\x8c\x18\x1c\x0\x8\x82\x1\xa5\x1e\xa3\xf1\x6\xc1\x2c\x1\x32\x50\x31\x28\x87\x39\x18\x3\x15\x83\x72\x98\x83\x31\x50\x31\x28\x87\x39\x18\x3\x15\x3\x72\x98\x83\x51\x3a\x11\x1b\x3a\xdc\x10\x9c\x7\x18\xcc\x32\x28\x49\x60\x36\x1\x1a\xf0\x31\x9b\x8\xd\xf8\x98\xa0\x17\xf2\x31\x61\x2f\xe4\x33\x62\x60\x0\x20\x8\x6\x16\x7d\xbc\x86\x30\x62\x60\x0\x20\x8\x6\x56\x7d\xc0\x86\x60\x42\x5f\xc8\xc7\x4\xbf\x90\x8f\x21\x42\x7c\xc\x11\xe2\x33\x62\x60\x0\x20\x8\x6\x96\x7e\xd0\x87\x30\x62\x60\x0\x20\x8\x6\xd6\x7e\xd4\x87\x60\x42\x69\xc0\xc7\x4\xd3\x80\xcf\x88\xc1\x1\x80\x20\x18\x50\xff\x81\x1b\x42\x30\x62\x60\x0\x20\x8\x6\xd6\x7f\xd8\x46\x30\x62\x60\x0\x20\x8\x6\x16\x88\xe8\x47\x30\x62\x60\x0\x20\x8\x6\x56\x88\xdc\xc6\x30\x62\x60\x0\x20\x8\x6\x96\x88\xf0\x47\x60\xc1\x0\x1f\xb\x60\x43\x3e\x26\xc8\x86\x7c\x4c\x40\xe0\x63\x81\x40\x9f\x11\x3\x3\x0\x41\x30\xb0\x50\x24\x44\x2\xb\x60\x43\x3e\x86\x1b\x41\x7c\x6c\x18\xe4\x63\x81\x20\x1f\xe3\x8d\x20\x3e\x23\x6\x7\x0\x82\x60\xc0\xb1\xc8\x79\xc0\x86\x7c\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\x82\xd\x68\x10\x1f\x1b\xd0\x20\x3e\x36\xa0\x41\x7c\xc\x19\xe4\x63\xc9\x20\x1f\x53\x6\xf9\xd8\xe0\x6\xf0\xb1\xc1\xd\xe0\x63\x83\x1b\xc0\xc7\x1c\xd2\x90\xcf\x88\xc1\x1\x80\x20\x18\x50\x3d\x62\x1f\x6f\x10\xcc\x12\x28\x3\x15\x83\x92\x98\x3\x32\x50\x31\x28\x89\x39\x20\x3\x15\x83\x92\x98\x3\x32\x50\x31\x20\x89\x39\x20\xc3\x11\x81\x7c\x28\xdf\x2c\x3\xb3\x4\x23\x6\x6\x0\x82\x60\xb0\x98\x89\x7d\xa0\xc8\x2c\x1\x33\x62\x90\x0\x20\x8\x6\xc8\x99\xd8\x88\x8e\xe8\x88\x8c\x14\x23\x6\x9\x0\x82\x60\x80\x9c\x89\x8d\xe8\x88\x8e\xb8\x8\x31\x62\x90\x0\x20\x8\x6\xc8\x99\xd8\x88\x8e\xe8\x48\x8c\xc\x23\x6\x9\x0\x82\x60\x80\x9c\x89\x8d\xe8\x88\x8e\xc0\x88\x80\x0\x0\x0\x0\x0\x0\x0\x0";

}