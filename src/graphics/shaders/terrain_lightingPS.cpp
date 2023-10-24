#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char terrain_lightingPS_dxil_bytes[6865];

auto terrain_lightingPS() noexcept -> shader_def
{
   return {
      .name = "terrain_lightingPS",
      .entrypoint = L"main",
      .target = L"ps_6_6",
      .file = L"terrain_lightingPS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(terrain_lightingPS_dxil_bytes),
               sizeof(terrain_lightingPS_dxil_bytes) - 1},
   };
}

const char terrain_lightingPS_dxil_bytes[6865] = "\x44\x58\x42\x43\x19\x99\xb2\xa8\x1c\xd0\xff\xbb\x24\x23\xdf\x6f\x48\x47\xe0\x3a\x1\x0\x0\x0\xd0\x1a\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x10\x1\x0\x0\x4c\x1\x0\x0\x8c\x2\x0\x0\xc0\x2\x0\x0\xdc\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x40\x0\x6\x0\x0\x0\x0\x49\x53\x47\x31\xbc\x0\x0\x0\x4\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x88\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x93\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\x3\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xb0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\xf\x3\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x57\x53\x0\x54\x45\x52\x52\x41\x49\x4e\x43\x4f\x4f\x52\x44\x53\x0\x41\x43\x54\x49\x56\x45\x54\x45\x58\x54\x55\x52\x45\x53\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\x38\x1\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x4\x1\x0\x4\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x4\x0\x0\x0\x4\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x2c\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x57\x53\x0\x54\x45\x52\x52\x41\x49\x4e\x43\x4f\x4f\x52\x44\x53\x0\x41\x43\x54\x49\x56\x45\x54\x45\x58\x54\x55\x52\x45\x53\x0\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x2\x0\x0\xc\x0\x0\x0\x0\x0\x0\x0\x1\x1\x42\x0\x3\x2\x0\x0\x1a\x0\x0\x0\x0\x0\x0\x0\x1\x2\x41\x0\x1\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x3\x44\x3\x3\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x2c\x0\x0\x0\x0\x0\x24\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x74\x65\x72\x72\x61\x69\x6e\x5f\x6c\x69\x67\x68\x74\x69\x6e\x67\x50\x53\x2e\x70\x64\x62\x0\x0\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x9f\xb3\x63\x68\x51\x5a\xb6\x41\x48\x8e\xa9\x7a\x4c\xb7\x30\x22\x44\x58\x49\x4c\xec\x17\x0\x0\x66\x0\x0\x0\xfb\x5\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\xd4\x17\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\xf2\x5\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x1c\x45\x2\x42\x92\xb\x42\xe4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x72\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x91\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x39\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x7a\x0\x0\x0\x32\x22\xc8\x9\x20\x64\x85\x4\x93\x23\xa4\x84\x4\x93\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8e\x8c\xb\x84\xe4\x4c\x10\x88\xc2\x8\x40\x9\x0\x14\x98\x1\x98\x23\x0\x83\x39\x2\xa4\x18\x3\x2\x81\x50\x20\x90\xa1\x18\x0\x2\x81\x50\x18\x10\x82\xa\xf7\xc\x97\x3f\x61\xf\x21\xf9\x21\xd0\xc\xb\x81\x2\xc5\x1c\x41\x50\x8a\x5\x81\xc0\x60\x50\xa3\xc\x3\xc2\x80\x1e\x5\x19\x10\x6\x83\xc1\x60\x30\xa0\xc8\x4d\xc3\xe5\x4f\xd8\x43\x48\xfe\x4a\x48\x2b\x31\xf9\xc5\x6d\xa3\x62\x30\x18\xc\x8\x85\x99\x10\x16\x8b\xc1\x60\x30\x20\x10\x8\x3\xa2\x14\x82\x40\x20\x14\xc8\x72\xd3\x70\xf9\x13\xf6\x10\x92\xbf\x12\xd2\x4a\x4c\x3e\x72\xdb\xa8\x20\x10\x8\x4\x42\x39\x30\x84\x5\x81\x50\x20\x50\xa6\xc\x4\x2\x81\x36\xe5\x98\x10\x16\x4\x42\x81\x40\x9d\x42\xc\x8\x83\x1\x7d\x8e\x1a\x2e\x7f\xc2\x1e\x42\xf2\xb9\x8d\x2a\x56\x62\xf2\x8b\xdb\x46\xc4\x60\x30\x18\x14\x22\x44\x58\x10\x48\x74\xd4\x70\xf9\x13\xf6\x10\x92\xcf\x6d\x54\xb1\x12\x93\x8f\xdc\x36\x22\x8\x4\x2\xa1\x10\x24\xc2\x82\x40\xa5\x52\xc\x8\x83\xc1\x80\x4e\xb7\xd\x97\x3f\x61\xf\x21\xf9\x2b\x21\x39\x54\x24\x10\x69\xe4\x3c\x44\x34\x21\x84\x84\x4\x2\xa1\x10\xb\xc2\x22\x45\xaa\x83\x86\xcb\x9f\xb0\x87\x90\xfc\x95\x90\x36\xa4\x19\x10\x41\x20\x10\xa\xa5\x58\x10\x58\x4\x6\xb5\x6\x2\x86\x11\x86\x61\x18\x81\x18\x92\x20\x96\x4\xb2\x61\x4\x61\x38\x4c\x9a\x22\x4a\x98\xfc\x5\x31\x8e\xe9\x1f\x8\x49\x88\x90\x67\x42\x9c\x6\x29\x30\x2\x6c\x0\x1b\xc\x8\x4\x42\x2\xa\xd4\xee\xc\xe\x47\x9a\x16\x0\x73\xa8\xc9\x97\xa6\x88\x12\x26\x7f\x41\x8c\x63\xfa\x5\xa7\x91\x26\xa0\x99\xfe\x80\x2a\xa\x22\x42\xc\x81\x40\x20\xc0\x8\x30\xc2\x8c\x46\xb3\x9d\x76\x38\xd2\xb4\x0\x98\x43\x4d\xfe\x82\x18\xc7\xf4\xb\x4e\x23\x4d\x40\x33\x49\x28\xb8\x11\x2e\x21\x8\x9\x1\xbb\x4c\x9a\x22\x4a\x98\xfc\x89\x88\x22\x0\x69\x7e\xc1\x69\xa4\x9\x68\x26\xe9\x47\x8\x6d\x30\x20\x10\x74\xbb\xdd\x45\x13\x11\x45\x0\xd2\xfc\x82\xd3\x48\x13\xd0\x4c\x12\xa\x78\xd4\x4b\x9\xc2\x1c\x1\x28\x4c\x1\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x34\x40\x0\x4\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x79\x80\x0\x8\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x13\x1\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x60\x0\x0\x0\x0\x0\x0\x0\x80\x21\x8f\x5\x4\x80\x0\x0\x0\x0\x0\x0\x0\x0\x43\x9e\xc\x8\x80\x1\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x1b\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x3a\x20\x0\x6\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x7c\x40\x0\x4\x0\x0\x0\x0\x0\x0\x0\x30\xe4\xa1\x80\x0\x18\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x23\x6\x40\x0\xc\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x29\x3\x20\x0\x6\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x9c\x1\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\xd4\x0\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6d\x0\x4\x40\x0\x0\x0\x0\x0\x0\x0\x0\x59\x20\x14\x0\x0\x0\x32\x1e\x98\x18\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x94\x40\x31\x94\xc3\x8\x40\x1\x16\xa\x14\x42\xe1\x32\x94\x41\x11\x94\x7\x15\x4a\x62\x4\xa0\x8\xca\xa0\x14\x4a\xa0\x10\xa\x84\x80\x5\x8\x8\xc\x88\x41\xbe\x19\x0\xfa\x8d\x25\x41\xc4\x71\x1c\xc0\x71\x0\x0\x0\x0\xc0\x71\x0\x0\xe5\x66\x0\x0\x79\x18\x0\x0\x71\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x8\x69\x82\x40\x4c\x1b\x84\x81\x98\x20\x10\xd4\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\x10\xd5\x4\x21\xf\xce\x80\xc0\x4\x81\xb0\x36\x20\xca\xc2\x28\xc3\xd0\x0\x13\x4\x3f\x28\x83\x9\x2\x71\x4d\x10\x8\x6c\x3\x32\x3c\x8c\x2\xd\x11\xb0\x41\x70\xa4\xd\x4\x0\x4c\xc0\x4\xe1\xf\xcc\x60\x43\x50\x4d\x10\x4\x80\x44\x5b\x58\x9a\x1b\x15\xa8\xa7\xa9\x24\xaa\xa4\x27\xa7\xab\xa9\x9\x42\xc1\x4d\x10\x8a\x6e\x43\xa0\x4c\x10\xa\x6f\x82\x50\x7c\x13\x4\x22\xdb\x20\x7c\xc6\x86\x45\xc9\xb4\x8d\xeb\x6\x4f\xd9\xc0\x80\xd\x55\x91\x94\x54\x50\x92\xd3\xd0\xd3\x93\x14\xd1\xd4\x6\xe1\xfb\x36\x2c\x83\x18\x68\x1b\xd7\xd\xdd\xb0\x8d\xc1\x4\x81\xd0\xe8\x4\xd\x51\x25\x59\x15\x51\x15\x61\x51\x55\x49\x15\x4d\x4d\x10\xa\x30\x98\x20\x14\x61\xb0\x61\x29\x3\x33\x38\x83\x8d\x43\x83\x1\xd\xca\x60\x3\xb8\x4c\x59\x7d\x41\xbd\xcd\xa5\xd1\xa5\xbd\xb9\x4d\x10\xa\x31\xd8\xb0\x7c\x6a\xa0\x79\xdc\x1a\xc\x6b\xf0\x6d\x63\xb0\x81\x8\x3\x32\x48\x3\x36\x60\x32\x65\xf5\x45\x15\x26\x77\x56\x46\x37\x41\x28\xc6\x60\x82\x40\x6c\x1b\x84\xf\xe\x36\x2c\x8a\x1b\x68\x6f\xc0\x6d\xc3\x1a\x28\x5b\x1c\x6c\x8\xe4\x60\xc3\xd0\x6\x73\x0\x4c\x10\x40\x81\xc\x36\x8\x4a\x1d\x6c\x28\x2e\x8c\xe\x28\x3b\xa8\xc2\xc6\x66\xd7\xe6\x92\x46\x56\xe6\x46\x37\x25\x8\xaa\x90\xe1\xb9\xd8\x95\xc9\xcd\xa5\xbd\xb9\x4d\x9\x88\x26\x64\x78\x2e\x76\x61\x6c\x76\x65\x72\x53\x2\xa3\xe\x19\x9e\xcb\x1c\x5a\x18\x59\x99\x5c\xd3\x1b\x59\x19\xdb\x94\x0\x29\x43\x86\xe7\x22\x57\x36\xf7\x56\x27\x37\x56\x36\x37\x25\x98\xea\x90\xe1\xb9\xd8\xa5\x95\xdd\x25\x91\x4d\xd1\x85\xd1\x95\x4d\x9\xaa\x3a\x64\x78\x2e\x65\x6e\x74\x72\x79\x50\x6f\x69\x6e\x74\x73\x53\x2\x3b\x0\x79\x18\x0\x0\x55\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xca\xc3\xe\xec\x60\xf\xed\xe0\x6\xec\xf0\xe\xef\x0\xf\x33\x22\x88\x1c\xf0\xc1\xd\xc8\x41\x1c\xce\xc1\xd\xec\x21\x1c\xe4\x81\x1d\xc2\x21\x1f\xde\xa1\x1e\xe8\x61\x6\x13\x91\x3\x3e\xb8\x81\x38\xc8\x43\x39\x84\xc3\x3a\xb8\x81\x38\xc8\x3\x0\x0\x0\x71\x20\x0\x0\x50\x0\x0\x0\x6\x41\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\xd8\x2\x34\x5c\xbe\xf3\xf8\x1\xd2\x0\x11\xe6\x17\xb7\x6d\xd\xdb\x70\xf9\xce\xe3\xb\x1\x55\x14\x44\x54\x3a\xc0\x50\x12\x6\x20\x60\x7e\x71\xdb\xe6\xb0\xd\x97\xef\x3c\xbe\x10\x50\x45\x41\x44\xa5\x3\xc\x25\x61\x0\x2\xe6\x23\xb7\x6d\x11\xdd\x70\xf9\xce\xe3\xb\x11\x1\x4c\x44\x8\x34\xc3\x42\x7c\x91\xc3\x6c\x48\x33\x20\x8d\x61\x3\xda\x70\xf9\xce\xe3\xb\x11\x1\x4c\x44\x8\x34\xc3\x42\x7c\x91\xc3\x84\x4\xf0\x58\x81\x33\x5c\xbe\xf3\xf8\x83\x33\xdd\x7e\x71\xdb\x16\x30\xd\x97\xef\x3c\xfe\xe2\x0\x83\xd8\x3c\xd4\xe4\x17\xb7\x6d\xa\xd7\x70\xf9\xce\xe3\x47\x80\xb5\x51\x45\x41\x44\xa5\x3\xc\x7e\x71\xdb\x86\x70\xd\x97\xef\x3c\x7e\x4\x58\x1b\x55\x14\x44\x54\x3a\xc0\xe0\x23\xb7\x6d\x6\xd0\x70\xf9\xce\xe3\x4b\x0\xf3\x2c\x84\x5f\xdc\xb6\x31\x70\xc3\xe5\x3b\x8f\x2f\x1\xcc\xb3\x10\x1c\xf3\x94\x44\x45\x2c\x33\x11\x39\x7e\x71\xdb\x26\x50\xd\x97\xef\x3c\xbe\x34\x39\x11\x81\x52\xd3\x43\x4d\x7e\x71\xdb\xf6\x20\xd\x97\xef\x3c\xfe\x44\x44\x13\x2\x44\x98\x5f\xdc\xb6\x11\x3c\xc3\xe5\x3b\x8f\x4f\x35\x40\x84\xf9\xc5\x6d\x5b\xc2\x34\x5c\xbe\xf3\xf8\x54\x3\x44\xd8\x86\x4c\x92\x8f\xdc\xb6\x1d\x5c\xc3\xe5\x3b\x8f\x6f\x1\x15\xa1\x9\x13\x52\x11\x1b\x32\xf9\xc8\x6d\x1b\x0\xc1\x0\x48\x3\x0\x0\x0\x61\x20\x0\x0\xbe\x3\x0\x0\x13\x4\x52\x2c\x10\x0\x0\x0\x36\x0\x0\x0\xc4\xa8\x81\x11\x0\x2a\x94\x43\x79\x10\xa1\x40\x3\xa\xa4\x4c\x8a\xa2\x24\xca\xa2\x50\xca\xa7\xe0\x3\xa\x38\xa0\xec\x8a\x6b\x6\xa0\x8c\x4a\x30\xa0\x60\x7\x8a\x76\xa0\xdc\x4a\xa1\x2c\x5\xa\xaf\x60\x4a\xa3\x80\x6a\xa0\x64\x4a\x76\xa0\x38\xa\xa3\x24\x9\xca\xa5\x20\x4a\xa5\x58\x8a\x13\x90\xa0\x30\x11\x68\x30\x46\xd0\x9a\x73\xce\x7b\x63\x4\x20\x8\x82\xfa\x37\x46\x0\x82\x20\xe8\x7f\x63\x4\x20\x8\x82\xe8\x37\x46\xf0\xce\xa4\x89\xf6\xc2\x18\x1\x8\x82\x20\x8\xa\x63\x4\x20\x8\x82\xf0\x37\x46\x0\x82\x20\x8\x82\xc1\x18\x1\x8\x82\x30\x1b\x6\x63\x4\x20\x8\x82\xf4\x37\x46\x0\x82\x20\x88\x7f\x23\x0\x63\x4\x20\x8\x82\xf0\x2f\xcc\x0\x50\x6a\xe\x41\x14\xb0\x39\x4\x51\xb0\xe6\x10\x4e\xa1\x9a\x43\x70\x85\x6a\xe\x1\x15\x46\x61\xe\xe1\x15\xea\x60\xe\x41\x4b\x85\x39\x4\x2a\x15\x88\x35\x7\x71\x5d\x95\x37\x7\x21\x49\x95\x7\x0\x0\x0\x23\x6\x8\x0\x82\x60\xd0\x6\xb7\x20\x6\x42\x2c\xfc\xc1\x88\x1\x2\x80\x20\x18\xb4\x1\x2e\x8c\x81\x70\xb\xa0\x30\x62\x70\x0\x20\x8\x6\x6a\xa0\xb\x6f\x20\x30\x23\x6\x7\x0\x82\x60\xa0\x6\xbb\x0\x7\x2\x33\x62\x90\x0\x20\x8\x6\xc6\x3a\xd4\xc2\x2c\xdc\xc2\x2b\xd0\xc1\x88\x41\x2\x80\x20\x18\x18\xec\x60\xb\xb4\x80\xb\xac\x50\x7\x23\x6\x9\x0\x82\x60\x60\xb4\xc3\x2d\xf0\x42\x2e\xc4\x82\x1d\x8c\x18\x24\x0\x8\x82\x81\xe1\xe\xb8\xd0\xb\xba\xe0\xa\x77\x30\x62\x90\x0\x20\x8\x6\xc6\x3b\xe4\xc2\x2e\xec\xc2\x2c\xe0\xc1\x88\x41\x2\x80\x20\x18\x18\xf0\xa0\xb\xbc\xc0\xb\xb0\x90\x7\x23\x6\x9\x0\x82\x60\x60\xc4\xc3\x2e\xf4\x42\x2f\xd0\x82\x1e\x8c\x18\x20\x0\x8\x82\x41\x3\xf\x77\xa0\xb\xab\xa0\xa\x23\x6\x7\x0\x82\x60\xa0\x6\xe4\x90\x7\x1\x35\x62\x80\x0\x20\x8\x6\x8d\x3c\xe4\x81\x38\xb4\x2\x2b\x8c\x18\x1c\x0\x8\x82\x81\x1a\x98\xc3\x1e\x4\xd5\x88\xc1\x1\x80\x20\x18\x94\x41\x3a\x84\x2\x23\xe\xa3\x9\x41\x30\x62\x80\x0\x20\x8\x6\x8d\x3d\xf4\x41\x0\xb\xb0\x30\x62\x70\x0\x20\x8\x6\x6a\xa0\xe\x7f\x10\x60\xa3\x9\x84\x30\x62\x80\x0\x20\x8\x6\x4d\x3e\x80\x42\x30\xb\xb3\x30\x62\x70\x0\x20\x8\x6\x6a\xd0\xe\xa2\x10\x68\x23\x6\x7\x0\x82\x60\x50\x6\xf0\x80\xa\x14\x3b\x8c\x26\x4\xc0\x88\x1\x2\x80\x20\x18\x34\xfd\x40\xa\xc1\x2d\xdc\xc2\x88\xc1\x1\x80\x20\x18\xa8\x41\x3c\x98\x42\xd0\x8d\x18\x1c\x0\x8\x82\x81\x18\xd4\x3\x2b\x60\xed\x30\x9a\x10\xc\xa3\x9\x82\x30\x62\xf0\x0\x20\x8\x6\xd4\x3f\x94\x2\x11\x5d\x16\x1a\xa0\x41\x28\xc0\x43\x2b\xa0\xc1\x68\x42\x0\x8c\x18\x3c\x0\x8\x82\x1\x15\x12\xa7\x60\x4c\x19\xa6\x6\x6a\x50\xf\xf2\xf0\xa\x6a\x30\x9a\x10\x0\x23\x6\xf\x0\x82\x60\x40\x8d\x44\x2a\x20\xd5\xa6\xb1\x1\x1b\xd0\x43\x29\xc4\x2\x1b\x8c\x26\x4\xc0\x88\xc1\x3\x80\x20\x18\x50\x25\xb1\xa\xca\xd5\x71\x6e\xe0\x6\xf6\x90\xf\xb3\xe0\x6\xa3\x9\x1\x60\x47\x11\x1f\xb\x16\xf9\xd8\x82\x7\xf2\x31\x21\xa0\x8f\x1d\x45\x7c\x2c\x78\xe4\x63\x1\x41\x9f\x11\x83\x5\x0\x41\x30\x88\x5e\xe2\x16\x8\x3d\x8\x8\x3d\x8\x46\xc\xc\x0\x4\xc1\xe0\x89\x9\x58\x8\x2c\x30\xe4\x63\x2\x21\x9f\x63\x3\xc2\x1c\x1b\x10\xc6\xa\x52\x90\x8f\x95\x42\x10\x9f\x11\x83\x3\x0\x41\x30\x10\x83\x97\x30\x87\x38\x50\x89\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\xb0\x55\x28\xe2\x63\x44\x20\x1f\x23\x4\xf9\x18\x31\xc8\xc7\x86\x3\x3e\x36\x1c\xf0\xb1\xe1\x80\xcf\x88\xc1\x1\x80\x20\x18\x88\x81\x4e\xc4\x3\x1f\xb4\xc4\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\xd8\x0\xc9\xc7\x86\x48\x3e\x36\x48\xf2\x31\x65\x80\x8f\x29\x3\x7c\x4c\x19\xe0\x63\x3\x2e\xc8\xc7\x86\x5c\x90\x8f\xd\xba\x20\x9f\xe2\x76\x2\x8b\xe3\x9\x1c\x4d\xf8\x3\xa0\x2\x41\x2a\x20\x60\x96\x20\x18\xa8\x18\x20\x60\xcd\xa0\x81\x8a\x1\x2\xd6\xc\x1a\xa8\x18\x20\x60\xcd\xa0\x81\xa\xc2\x55\x80\x35\x83\x2a\x18\x8b\x1b\x31\x50\x0\x10\x4\x83\xcc\x2e\xfe\x81\x14\x8c\xa0\x27\xce\x62\x34\x21\x0\x46\xc\xe\x0\x4\xc1\xc0\xca\xb\x96\x8\x7e\x62\xb8\x21\x60\xb\x30\x98\x65\x80\x84\x60\x96\x60\x18\xa8\x18\x62\xcc\x91\x84\x81\x8a\x21\xc6\x1c\x49\x18\xa8\x18\x62\xcc\x91\x84\x81\xa\x62\x71\x14\x61\xc4\xc0\x0\x40\x10\xc\xb6\xbe\x18\x89\xa0\x2\x9a\xd0\xb2\x8b\xe0\x2a\x28\x9\xae\x22\xd0\x7a\x5a\xe2\xca\x8\xa0\x82\xb9\xb8\xa\x78\x2\x46\xc\xe\x0\x4\xc1\xa0\xc\x42\x23\x27\xc8\x21\x18\x4d\x8\x86\x61\x12\x22\x70\x2a\x42\x62\xb4\xa6\x88\x93\x80\x11\x83\x3\x0\x41\x30\x28\x3\xd2\xe0\x89\x73\x8\x46\x13\x82\x60\xb8\x21\xf8\xb\x30\x98\x65\x28\x8e\x60\x34\x61\x18\x2a\x20\xd\x1d\x6e\x8\x44\x3\xc\x66\x19\x16\x23\x18\x31\x38\x0\x10\x4\x3\x31\x58\xd\xb1\x68\x87\xb3\x18\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x84\x11\x83\x3\x0\x41\x30\x10\x83\xd7\x30\x8b\x78\xa0\x89\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\x18\x31\x38\x0\x10\x4\x3\x31\x98\xd\xb5\xa8\x7\x9f\x18\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x84\x11\x83\x3\x0\x41\x30\x10\x83\xdb\x70\x8b\x7c\xe8\x89\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\xb0\x7\x1f\xe4\x33\x62\x80\x0\x20\x8\x6\x67\xa0\x1b\x72\xc1\xe0\x43\x30\x62\x80\x0\x20\x8\x6\x67\xb0\x1b\x73\x91\xe0\x43\x60\xd1\x3e\xc8\x67\xc4\x0\x1\x40\x10\xc\xce\xa0\x37\xea\xc2\xd9\x87\x60\xc4\x0\x1\x40\x10\xc\xce\xc0\x37\xec\x62\xd9\x87\xc0\x26\x7f\x90\xcf\x88\x1\x2\x80\x20\x18\x9c\x1\x78\xe0\x5\xe4\xf\xc1\x88\x1\x2\x80\x20\x18\x9c\x41\x78\xe4\x45\xe3\xf\x81\x31\x35\x1\x1f\xb\x10\xf8\x58\x73\x13\xf0\xb1\xe0\x80\x8f\x39\x39\x1\x1f\xb\xc\xf8\x8c\x18\x18\x0\x8\x82\xc1\x33\x1f\xe4\x51\x8c\x18\x18\x0\x8\x82\xc1\x43\x1f\xe5\x41\x8c\x18\x18\x0\x8\x82\xc1\x53\x1f\xe6\x31\xd8\xe0\x13\xf0\xb1\xe1\x27\xe0\x63\x3\x58\xc0\x67\xc4\xe0\x0\x40\x10\xc\x3e\xf9\x20\x8d\x41\x18\x31\x38\x0\x10\x4\x83\x6f\x3e\x4a\x23\x10\x86\x23\x2\xb2\x28\xbe\xe9\x6\xf3\x38\x8f\x60\xc4\xe0\x0\x40\x10\xc\xc4\x80\x3e\x56\xc3\x26\xda\x63\x34\x21\x0\x46\x13\x84\x60\x34\x61\x10\x46\xc\xe\x0\x4\xc1\x40\xc\xf0\xe3\x35\x74\x42\x2f\x46\x13\x2\x60\x34\x41\x8\x46\x13\x6\x61\xc4\xe0\x0\x40\x10\xc\xc4\x80\x3f\x66\xc3\x27\x72\x63\x34\x21\x0\x46\x13\x84\x60\x34\x61\x10\x46\xc\xe\x0\x4\xc1\x40\xc\x40\xe4\x36\xc4\xc2\x37\x46\x13\x2\x60\x34\x41\x8\x46\x13\x6\xc1\x9e\xb0\x90\xcf\x88\x1\x2\x80\x20\x18\x9c\xc1\x88\xec\x6\x13\x16\xc1\x88\x1\x2\x80\x20\x18\x9c\x1\x89\xf0\x46\x12\x16\x81\x45\x64\x21\x9f\x11\x3\x4\x0\x41\x30\x38\x3\x13\xf1\xd\x87\x2c\x82\x11\x3\x4\x0\x41\x30\x38\x83\x13\xf9\x8d\x85\x2c\x2\x9b\xce\x42\x3e\x23\x6\x8\x0\x82\x60\x70\x6\x29\x12\x1e\xd0\x59\x4\x23\x6\x8\x0\x82\x60\x70\x6\x2a\x22\x1e\xcd\x59\x4\xc6\xf8\x5\x7c\x2c\x40\xe0\x63\xd\x68\xc0\xc7\x82\x3\x3e\xe6\x88\x6\x7c\x2c\x30\xe0\x33\x62\x60\x0\x20\x8\x6\xf\x8f\xb4\x48\x31\x62\x60\x0\x20\x8\x6\x4f\x8f\xb8\x8\x31\x62\x60\x0\x20\x8\x6\x8f\x8f\xbc\xc8\x60\xc3\x69\xc0\xc7\x6\xd4\x80\x8f\xd\xa9\x1\x9f\x11\x83\x3\x0\x41\x30\xf8\x76\xa4\x3d\x6\x61\xc4\xe0\x0\x40\x10\xc\x3e\x1e\x71\x8f\x40\x18\x8e\x8\x5a\xa3\xf8\xa6\x1b\x6e\x24\xd\x82\x11\x83\x3\x0\x41\x30\x10\x83\x1e\xa1\x8f\xbf\x88\x91\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\x18\x31\x38\x0\x10\x4\x3\x31\x8\x13\xfc\x18\x8d\x1b\x19\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x84\x11\x83\x3\x0\x41\x30\x10\x83\x32\xe1\x8f\xd3\xa0\x8f\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\x18\x31\x38\x0\x10\x4\x3\x31\x48\x13\x10\x59\x8d\x31\x19\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x4\x7b\x54\x43\x3e\x23\x6\x8\x0\x82\x60\x70\x6\x6c\x42\x22\x8c\x6a\x4\x23\x6\x8\x0\x82\x60\x70\x6\x6d\x52\x22\x89\x6a\x4\x16\xb5\x86\x7c\x46\xc\x10\x0\x4\xc1\xe0\xc\xde\xe4\x44\x9c\xd6\x8\x46\xc\x10\x0\x4\xc1\xe0\xc\xe0\x4\x45\x96\xd6\x8\x6c\x82\xd\xf9\x8c\x18\x20\x0\x8\x82\xc1\x19\xc8\x89\x8a\x40\xb0\x11\x8c\x18\x20\x0\x8\x82\xc1\x19\xcc\xc9\x8a\x34\xb0\x11\x18\x73\x1e\xf0\xb1\x0\x81\x8f\x35\xe9\x1\x1f\xb\xe\xf8\x98\xb3\x1e\xf0\xb1\xc0\x80\xcf\x88\x81\x1\x80\x20\x18\x3c\xa5\x62\x27\xc5\x88\x81\x1\x80\x20\x18\x3c\xa6\x72\x27\xc4\x88\x81\x1\x80\x20\x18\x3c\xa7\x82\x27\x83\xd\xf0\x1\x1f\x1b\xe2\x3\x3e\x36\xc8\x7\x7c\x46\xc\xe\x0\x4\xc1\xe0\x23\x15\x1b\x19\x84\x11\x83\x3\x0\x41\x30\xf8\x4a\xe5\x46\x2\x61\x38\x22\xb0\x8f\xe2\x9b\x6e\xe8\x93\x34\x8\x2c\xc8\x93\x1b\x54\xb0\x27\x3b\x62\x70\x0\x20\x8\x6\x62\x80\x2a\x3f\xa2\x1e\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x58\x71\x2a\x30\x18\x31\x38\x0\x10\x4\x3\x31\x68\x15\x32\x79\x8f\x60\x34\x21\x0\x46\x13\x84\x60\x34\x61\x10\x4c\x21\x15\x18\x8c\x18\x1c\x0\x8\x82\x81\x18\xc8\x4a\x9a\xd0\x47\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xf6\xa8\xa\xc\x46\xc\xe\x0\x4\xc1\x40\xc\x6e\xc5\x4d\xf2\x23\x18\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x4\x93\xf0\x43\x3e\x23\x6\x8\x0\x82\x60\x70\x6\xba\x22\x27\xe\x7e\x4\x23\x6\x8\x0\x82\x60\x70\x6\xbb\x32\x27\xa\x7e\x4\x16\x18\xd0\xb1\x8a\x3f\xe4\x33\x62\x80\x0\x20\x8\x6\x67\xe0\x2b\x76\x12\xf1\x47\x30\x62\x80\x0\x20\x8\x6\x67\xf0\x2b\x77\xd2\xf0\x47\x60\x41\x2\x1d\xc3\x40\x44\x3e\x23\x6\x8\x0\x82\x60\x70\x6\xe2\xa2\x27\x14\x88\x4\x23\x6\x8\x0\x82\x60\x70\x6\xe3\xb2\x27\x10\x88\x4\x23\x6\x7\x0\x82\x60\x20\x6\xe6\xd2\x27\x28\x52\x2a\xa3\x9\x81\x30\x62\x70\x0\x20\x8\x6\x65\x70\x2e\x7f\xa2\x22\xe0\x32\x9a\x10\xc\x23\x6\x8\x0\x82\x60\xd0\xd0\xcb\x9e\x4\xae\xe2\x2a\x23\x6\x7\x0\x82\x60\xa0\x6\xe8\xd2\x27\xc1\x8c\x98\x54\x26\xf0\xb1\x0\x81\x8f\x1d\x68\x22\x1f\x43\xd4\x44\x3e\x96\xb0\x89\x7c\x4c\x29\x13\xf9\x5c\x1b\xc\x6a\xc4\xe0\x1\x40\x10\xc\x28\x7a\x31\x15\x44\x45\xaa\x28\x18\x93\x75\x59\x17\x54\x31\x46\x13\x2\xc0\x82\x35\x91\x8f\x61\x47\x7c\x2c\xa9\xe0\x33\x62\xf0\x0\x20\x8\x6\x54\xbe\xac\x4a\xf3\x22\x42\x60\xa0\x9\xbc\xc0\x4b\xab\x2c\xa3\x9\x1\x60\x45\x0\x1f\xeb\x9a\xf8\x98\xd6\xc4\x67\xc4\xe0\x1\x40\x10\xc\x28\x7f\x81\x15\x89\x46\x84\x60\x69\x93\x7a\xa9\x17\x59\x81\x46\x13\x2\xc0\x8a\x0\x3e\x16\x89\x1\x7c\xec\x9b\xe2\x33\x62\xf0\x0\x20\x8\x6\xd4\xc8\xd4\xca\x95\x23\x42\x0\xc9\x89\xbe\xe8\xcb\xad\x54\xa3\x9\x1\x60\x45\x0\x1f\xbb\xce\x0\x3e\x76\x91\x1\x7c\x46\xc\x1e\x0\x4\xc1\x80\x42\x19\x5d\xe1\x7c\x44\x8\xaa\x3b\xf9\x97\x7f\xe1\x15\x6d\x34\x21\x0\xac\x8\xe0\x63\x6c\x90\xc5\xc7\xb6\x34\x80\xcf\x88\xc1\x3\x80\x20\x18\x50\x2d\xf3\x2b\x61\x30\x26\x42\xa0\xf1\x9\xc9\x90\x4c\xb8\x7c\xa3\x9\x1\x60\x45\x0\x1f\x8b\x83\x2f\x3e\xe6\x6\x5f\x7c\x46\xc\x1e\x0\x4\xc1\x80\x92\x19\x72\x31\x3\x34\x11\x82\x2f\x54\x52\x26\x65\xcc\x85\xc\x46\x13\x2\xc0\x8a\x0\x3e\x36\x6\x76\x0\x1f\x9b\x83\x32\x88\xcf\x88\xc1\x3\x80\x20\x18\x50\x37\x93\x2e\x6b\xd0\x26\x42\x40\x6\xa6\xe2\x32\x2e\xb3\x2e\x69\x30\x9a\x10\x0\x56\x4\xf0\xb1\x34\xd8\x3\xf8\x58\x1a\xe0\x1\x7c\x46\xc\x1e\x0\x4\xc1\x80\xe2\x19\x77\x81\x3\x39\x11\x82\x34\x58\x95\x99\x99\x19\x78\x71\x83\xd1\x84\x0\xb0\x22\x80\x8f\x5\xbb\x22\x9f\x59\x82\x65\x34\x41\x37\x84\x11\x3\x5\x0\x41\x30\xe8\xc6\xc6\x5d\xe0\x24\xc0\x19\x91\xb1\x99\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\x18\x4d\x20\x86\x11\x3\x5\x0\x41\x30\xe8\xd0\x66\x5e\xea\xc4\x28\x99\x93\xd9\x99\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\x18\x4d\x20\x86\x11\x3\x5\x0\x41\x30\xe8\xda\x6\x5f\xf4\x64\xa1\x17\x96\x1\x9b\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\x18\x4d\x20\x86\x11\x3\x5\x0\x41\x30\xe8\xe4\xa6\x5f\xfe\x4\x82\x19\x99\x29\x9b\xd1\x84\x0\x18\x4d\x10\x2\x8b\x5e\x45\x3e\x23\x6\x8\x0\x82\x60\x70\x6\x71\x93\x32\xaf\x12\x5\x23\x6\x8\x0\x82\x60\x70\x6\x72\xa3\x32\xaf\x12\x5\x16\x44\xd0\x31\x68\x56\xe4\x33\x62\x80\x0\x20\x8\x6\x67\x50\x37\x2d\x33\x2b\x50\x30\x62\x80\x0\x20\x8\x6\x67\x60\x37\x2e\x33\x2b\x50\x60\x1\x4\x1d\x7b\x6e\x45\x3e\x23\x6\x8\x0\x82\x60\x70\x6\x79\x13\x33\xb7\xf2\x4\x23\x6\x8\x0\x82\x60\x70\x6\x7a\x23\x33\xb7\xf2\x4\x16\x3c\xd0\x19\x26\x21\xf0\x63\xc9\x14\x9\xd1\x92\xd1\x84\x47\x18\x31\x30\x0\x10\x4\x83\x7\x75\xf2\x46\x19\x31\x30\x0\x10\x4\x83\x27\x75\xf4\xe6\x18\x31\x30\x0\x10\x4\x83\x47\x75\xf6\x86\xb0\x41\x8a\x8f\xd\x52\x7c\x6c\x30\xe2\x33\x62\x70\x0\x20\x8\x6\xdf\xe9\xe4\xcc\x70\x2f\x23\x6\x7\x0\x82\x60\xf0\xa1\x8e\xce\xc\xf8\x32\x62\x70\x0\x20\x8\x6\x5f\xea\xec\xcc\x90\x2f\x23\x6\xb\x0\x82\x60\x10\xc1\xe\xce\xc\x42\x30\x8\x81\x5\xfe\x22\x9f\x11\x3\x3\x0\x41\x30\x78\x66\x67\x74\x2\xf3\x97\x20\x3e\xb3\x4\x8b\x5d\x97\x7c\x8c\xa2\xe4\x63\x81\x0\x1f\x93\x24\xf9\x98\x10\xc0\x67\xc4\xc0\x0\x40\x10\xc\x1e\xdd\xe9\x99\xc0\x2\x31\x88\xcf\x88\xc1\x1\x80\x20\x18\x7c\xb5\x73\x36\x41\xc9\x58\x10\xc8\xc7\x2\x95\x91\xcf\x88\x81\x1\x80\x20\x18\x3c\xbf\xf3\x3a\x81\xa9\x4c\x10\x9f\x59\x82\x65\xc4\xc0\x0\x40\x10\xc\x9e\xf0\x91\x9d\xcf\x2\x34\x88\xcf\x88\xc1\x1\x80\x20\x18\x7c\xbc\xe3\x36\x1\xcb\x98\x19\x98\x81\x7c\xec\xfb\xe4\x63\x81\x0\x9f\x11\x3\x3\x0\x41\x30\x78\xce\x47\x6d\x2\xb\xde\x20\x3e\x23\x6\x7\x0\x82\x60\xf0\x89\xf\xdd\x4\x32\x33\x62\x70\x0\x20\x8\x6\xdf\xf8\xd4\xcd\x11\x58\x10\xc8\xc7\x2\x9c\x91\xcf\x88\x81\x1\x80\x20\x18\x3c\xed\xd3\x3b\x81\xe1\x4c\x10\x9f\x59\x82\x65\xa0\x66\x80\x9\x43\xce\xa\x41\xf1\x12\x3b\x40\xe4\xec\x18\x31\x38\x0\x10\x4\x3\x31\x38\x1f\xbf\x49\x19\x33\x19\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x84\x59\x2\xa7\xd6\x84\x6d\x60\xc4\xe0\x0\x40\x10\xc\xc4\x80\x7d\x46\xc7\x65\x82\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\xb0\xa1\x65\xe2\x63\x43\xcb\xc4\xc7\x86\x96\x89\xcf\x88\xc1\x2\x80\x20\x18\x44\xfb\x33\x3a\x83\x10\xc\x42\x30\x62\x60\x0\x20\x8\x6\x4f\xff\xf0\x4d\x60\x45\x20\x1f\x2b\x4\xf9\x58\x31\xc8\xc7\x10\x44\x3e\x86\x20\xf2\x31\x21\x80\x8f\x25\x89\x7c\x4c\x8\xe0\x33\x62\x60\x0\x20\x8\x6\x8f\x9\xa5\x4e\x60\x41\x20\x9f\xd1\x84\x69\xb0\x20\x90\x8f\xd\x1\x7d\xac\x6e\x82\xf8\x8c\x18\x18\x0\x8\x82\xc1\xc3\x42\xfc\x13\xcc\x12\x38\x45\x2a\xa5\x3\x23\x6\x7\x0\x82\x60\x20\x6\x25\xc4\x3b\x67\x13\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\x82\xd\x66\x13\x1f\x1b\xcc\x26\x3e\x36\x98\x4d\x7c\x46\xc\x16\x0\x4\xc1\x20\xa2\x21\xde\x19\x84\x60\x10\x82\x11\x3\x3\x0\x41\x30\x78\x6c\xa8\x76\x2\x2b\x2\xf9\x58\x21\xc8\xc7\x8a\x41\x3e\x86\x20\xf2\x31\x4\x91\x8f\x9\x1\x7c\x2c\x49\xe4\x63\x42\x0\x9f\x11\x3\x3\x0\x41\x30\x78\x7e\x48\x7c\x2\xb\x2\xf9\x8c\x26\x4c\x83\x5\x81\x7c\x6c\x8\xe8\x63\xae\x13\xc4\x67\xc4\xc0\x0\x40\x10\xc\x9e\x32\xaa\xa1\x60\xc4\xe0\x0\x40\x10\xc\xc4\xa0\x87\xe8\xe7\x6f\x78\x65\x34\x21\x0\x46\x13\x84\x60\x34\x61\x10\x46\xc\x16\x0\x4\xc1\x20\x4a\xa3\xf8\x99\xa4\x68\x10\x82\x11\x3\x3\x0\x41\x30\x78\xd6\x68\x87\x82\x1a\x97\xf1\x81\x11\x83\x3\x0\x41\x30\x10\x3\x32\xda\x1f\xd3\x9\x46\x13\x82\xc1\x88\x20\x3e\x75\x2e\xea\x3\x23\x6\x7\x0\x82\x60\x20\x6\x68\xf4\x3f\xaa\x13\x8c\x26\x4\x80\x11\x81\x7c\x46\xc\xc\x0\x4\xc1\xe0\xa1\x23\x32\xa\x2c\x80\xe4\x33\x4b\xe0\xc\x94\xc\x7e\x37\x8\xd\x2d\x30\x2e\xb1\xc\x94\xc\xa0\x37\x80\x42\x23\xf\xc\x4b\x2c\x3\x25\x83\xe8\xd\xa0\xd0\xc8\x3\xc3\x12\xcb\x40\xc9\x40\x7a\x3\x28\x34\xf2\xc0\xb0\xc4\x32\x62\xb0\x0\x20\x8\x6\x51\x1e\x85\x10\xda\xa4\xcd\xd9\xc\x42\x30\x62\x60\x0\x20\x8\x6\xcf\x1e\xad\x51\x50\xf3\x32\x3f\x30\x62\x70\x0\x20\x8\x6\x62\x40\x47\x2b\x64\x3b\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\xd8\xc2\x3e\xf2\xb1\xe0\x90\x8f\x5\x85\x7c\x4c\x28\xe4\x63\x43\x21\x1f\x1b\x50\x6\x3e\x36\xa0\xc\x7c\x6c\x40\x19\xf8\xc\x37\x8c\xc\x1e\x81\xc1\x2c\xc3\x33\x4\xb3\x4\xd0\x40\xc5\xa0\x66\x1\xf2\xc\x54\xc\x6a\x16\x20\xcf\x40\xc5\xa0\x66\x1\xf2\x58\xcd\xfc\x11\xc\x86\x1b\x82\x13\x2\x83\x59\x86\x28\x8\x46\xc\x12\x0\x4\xc1\x0\x89\xa5\x1a\xf2\x23\x3f\xb2\xa3\x62\xc4\x20\x1\x40\x10\xc\x90\x58\xaa\x21\x3f\xf2\x23\x39\x22\x46\xc\x12\x0\x4\xc1\x0\x89\xa5\x1a\xf2\x23\x3f\xaa\xa3\x61\xc4\x20\x1\x40\x10\xc\x90\x58\xaa\x21\x3f\xf2\x23\x3a\xca\x1f\x4\x0\x0\x0\x0";

}