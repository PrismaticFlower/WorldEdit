#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char tile_lightsVS_dxil_bytes[3357];

auto tile_lightsVS() noexcept -> shader_def
{
   return {
      .name = "tile_lightsVS",
      .entrypoint = L"mainVS",
      .target = L"vs_6_6",
      .file = L"tile_lights.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(tile_lightsVS_dxil_bytes),
               sizeof(tile_lightsVS_dxil_bytes) - 1},
   };
}

const char tile_lightsVS_dxil_bytes[3357] = "\x44\x58\x42\x43\x6\xef\xb6\x5d\xce\xd6\xd5\xc\x1f\x41\x1d\x78\xc2\x50\x42\xf8\x1\x0\x0\x0\x1c\xd\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xb4\x0\x0\x0\x44\x1\x0\x0\x58\x2\x0\x0\x84\x2\x0\x0\xa0\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x60\x0\x0\x0\x2\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x48\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x51\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\x1\x1\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x53\x56\x5f\x49\x6e\x73\x74\x61\x6e\x63\x65\x49\x44\x0\x0\x4f\x53\x47\x31\x88\x0\x0\x0\x3\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x68\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\xe\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x72\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x2\xd\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x7b\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x4c\x49\x47\x48\x54\x57\x4f\x52\x44\x0\x4c\x49\x47\x48\x54\x42\x49\x54\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x0\x50\x53\x56\x30\xc\x1\x0\x0\x30\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x1\x0\x0\x0\x2\x3\x0\x2\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x5\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xc\x0\x0\x0\x0\x0\x0\x0\x20\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x4c\x49\x47\x48\x54\x57\x4f\x52\x44\x0\x4c\x49\x47\x48\x54\x42\x49\x54\x0\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x41\x2\x1\x0\x0\x0\xa\x0\x0\x0\x0\x0\x0\x0\x1\x0\x41\x0\x1\x1\x0\x0\x14\x0\x0\x0\x0\x0\x0\x0\x1\x0\x51\x0\x1\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x3\x3\x4\x0\x0\xf0\x0\x0\x0\xf0\x0\x0\x0\xf0\x0\x0\x0\x0\x0\x0\x0\xf3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x24\x0\x0\x0\x0\x0\x1f\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x74\x69\x6c\x65\x5f\x6c\x69\x67\x68\x74\x73\x56\x53\x2e\x70\x64\x62\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\xbe\x58\x90\x27\x1a\x95\xe6\x93\x2\x49\x24\xd3\xfa\x9e\xb\xa2\x44\x58\x49\x4c\x74\xa\x0\x0\x66\x0\x1\x0\x9d\x2\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x5c\xa\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x94\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x68\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\xc0\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x82\x20\x88\x82\x20\xa4\x18\x0\x41\x10\x5\x41\x4a\x31\x0\x82\x20\x8a\x81\x98\x9b\x86\xcb\x9f\xb0\x87\x90\xfc\x95\x90\x56\x62\xf2\x91\xdb\x46\x5\x41\x10\x4\x41\xc5\x3d\xc3\xe5\x4f\xd8\x43\x48\x7e\x8\x34\xc3\x42\xa0\xe0\x29\x87\x43\x40\x4\x51\x10\x14\xdd\x34\x5c\xfe\x84\x3d\x84\xe4\xaf\x84\xb4\x12\x93\x5f\xdc\x36\x2a\x86\x61\x18\x88\x72\x4c\x4\x44\x10\x5\x41\xd4\x51\xc3\xe5\x4f\xd8\x43\x48\x3e\xb7\x51\xc5\x4a\x4c\x7e\x71\xdb\x88\x18\x86\x61\x28\x84\x45\x40\x4\x5d\xa5\x18\x88\x61\x18\x28\xbb\x6d\xb8\xfc\x9\x7b\x8\xc9\x5f\x9\xc9\xa1\x22\x81\x48\x23\xe7\x21\xa2\x9\x21\x24\x24\x10\x44\x21\x20\x2\xda\x88\x3b\x68\xb8\xfc\x9\x7b\x8\xc9\x5f\x9\x69\x43\x9a\x1\x11\x4\x41\x14\x73\x4\x41\x29\x20\xc2\x23\x3e\x2\x7\x2\x86\x11\x88\x21\x7\xc6\x61\x84\x81\xb8\xef\x70\xa4\x69\x1\x30\x87\x9a\x7c\x69\x8a\x28\x61\xf2\x91\x46\x9a\x80\x46\x20\xfe\x1\x98\x0\x34\x90\x44\x39\x13\x1c\xa0\xc3\x3b\xcc\x3\x3d\xb0\x43\x38\xe4\xc3\x3b\xd4\x3\x3d\xb8\xc1\x38\xb0\x43\x38\xcc\xc3\x3c\xb8\xc1\x2c\xd0\x83\x3c\xd4\xc3\x38\xd0\x43\x3d\xc8\x43\x39\x90\x83\x28\xd4\x83\x39\x98\x43\x39\xc8\x3\x1f\xa4\x83\x3b\xcc\x3\x3d\x84\x83\x3b\x8c\x43\x39\xfc\x2\x39\x84\x3\x3d\x84\x83\x1f\xa0\xc0\xa4\x73\x18\x41\x20\x92\x60\xbc\xef\x70\xa4\x69\x1\x30\x87\x9a\x7c\x69\x8a\x28\x61\xf2\x27\x64\x41\x1a\xe3\x47\x9a\x87\x9a\x24\x24\xa4\xea\x59\x87\x23\x4d\xb\x80\x39\xd4\xe4\x4f\xc8\x42\xfc\x48\xf3\x50\x93\x84\x82\x95\xd8\x3c\x20\xe6\x8\x40\x1\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x2\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x28\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x61\x80\x0\x10\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x13\x1\x1\x30\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x60\x0\x0\x0\x0\x0\x0\x0\x80\x21\xcf\x5\x4\xc0\x0\x0\x0\x0\x0\x0\x0\x0\x43\x9e\xc\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x1c\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\xc0\x0\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\xb2\x40\x0\x0\x0\x11\x0\x0\x0\x32\x1e\x98\x18\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\x30\xa\x30\xa0\x40\x3\xca\xa0\x8\xca\x83\x8a\x92\x18\x1\x28\x83\x52\x28\x82\x12\x28\x4\x7a\xb\x84\xda\x19\x0\x72\xc7\x3a\xa\x4\x3e\xe0\x3\x3e\x80\xf9\x20\x74\x6\x0\x0\x79\x18\x0\x0\x71\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x88\x65\x82\x40\x30\x1b\x84\x81\x98\x20\x10\xcd\x6\x61\x30\x28\xd8\xcd\x6d\x18\x10\x82\x98\x20\x10\xce\x4\x1\xd\x3e\x2\x13\x4\xe2\x99\x20\x10\xd0\x6\x61\x70\x36\x24\xca\xc2\x28\xca\xd0\x28\xcf\x86\x0\x9a\x20\xb4\x41\x37\x41\x20\xa2\xd\x88\x22\x31\x8a\x32\x4c\xc0\x86\x80\xda\x40\x44\x40\x5\x4c\x10\xdc\xc0\xdb\x10\x5c\x13\x4\x1\xa0\xd1\x16\x96\xe6\x66\x35\x45\x4\xea\x69\x2a\x89\x2a\xe9\xc9\x69\x82\x50\x54\x13\x84\xc2\xda\x10\x28\x13\x84\xe2\x9a\x20\x10\xd2\x6\x1\xc\x8c\xd\x8b\xb2\x71\x9d\xd7\xd\x9f\xd2\x85\x1\x9b\x29\xab\xaf\x24\xb7\x39\xba\x30\xb7\xb1\xb2\x24\xa2\x9\x42\x81\x4d\x10\x8a\x6c\x82\x50\x68\x1b\x4\x30\x18\x36\x2c\xc3\x18\x90\x41\x19\x78\xdd\x60\x6\x43\x77\x6\x1b\x4\x31\x40\x3\x26\x4c\x49\x47\x48\x54\x57\x4f\x52\x44\x1b\x16\x45\xd\xc8\xa0\xf3\xcc\x60\x30\x3\xa5\x3b\x3\x22\x4c\x49\x47\x48\x54\x42\x49\x54\x1b\x96\x81\xd\xc8\xa0\xf3\xcc\x60\x30\x3\xc5\xc\xce\x60\x82\x40\x4c\x5c\xa6\xac\xbe\xa0\xde\xe6\xd2\xe8\xd2\xde\xdc\x26\x8\xc5\x36\x41\x20\xa8\xd\x2\x18\xc4\xc1\x86\xc5\xd\xde\x80\xfb\x3c\x38\x18\xe0\x60\xe8\xe4\x60\xc3\xb0\x6\x6d\x30\x7\x1b\x86\x34\xa0\x3\x60\x82\xf0\x6\xdc\x6\x41\xb1\x83\xd\x45\xa6\xd5\x81\x75\x7\x55\xd8\xd8\xec\xda\x5c\xd2\xc8\xca\xdc\xe8\xa6\x4\x41\x15\x32\x3c\x17\xbb\x32\xb9\xb9\xb4\x37\xb7\x29\x1\xd1\x84\xc\xcf\xc5\x2e\x8c\xcd\xae\x4c\x6e\x4a\x60\xd4\x21\xc3\x73\x99\x43\xb\x23\x2b\x93\x6b\x7a\x23\x2b\x63\x9b\x12\x20\x65\xc8\xf0\x5c\xe4\xca\xe6\xde\xea\xe4\xc6\xca\xe6\xa6\x4\x55\x1d\x32\x3c\x17\xbb\xb4\xb2\xbb\x24\xb2\x29\xba\x30\xba\xb2\x29\xc1\x55\x87\xc\xcf\xa5\xcc\x8d\x4e\x2e\xf\xea\x2d\xcd\x8d\x6e\x6e\x4a\x70\x7\x0\x0\x0\x0\x79\x18\x0\x0\x4c\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\x8c\xc8\x21\x7\x7c\x70\x3\x72\x10\x87\x73\x70\x3\x7b\x8\x7\x79\x60\x87\x70\xc8\x87\x77\xa8\x7\x7a\x98\x81\x3c\xe4\x80\xf\x6e\x40\xf\xe5\xd0\xe\xf0\x0\x0\x0\x71\x20\x0\x0\x31\x0\x0\x0\x96\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\xd8\xc1\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x29\x74\xc3\xe5\x3b\x8f\x2f\x44\x4\x30\x11\x21\xd0\xc\xb\xf1\x45\xe\xb3\x21\xcd\x80\x34\x86\x5\x4c\xc3\xe5\x3b\x8f\xbf\x38\xc0\x20\x36\xf\x35\xf9\xc5\x6d\x9b\xc0\x34\x5c\xbe\xf3\xf8\x8b\x3\xc\x62\xf3\x50\x93\x8f\xdc\xb6\x19\x5c\xc3\xe5\x3b\x8f\x1f\x1\xd6\x46\x15\x5\x11\x95\xe\x30\xf8\xc5\x6d\x5b\xc1\x35\x5c\xbe\xf3\xf8\x11\x60\x6d\x54\x51\x10\x51\xe9\x0\x83\x8f\xdc\xb6\x11\x54\xc3\xe5\x3b\x8f\x2f\x4d\x4e\x44\xa0\xd4\xf4\x50\x93\x5f\xdc\xb6\xd\x54\xc3\xe5\x3b\x8f\x2f\x4d\x4e\x44\xa0\xd4\xf4\x50\x93\x8f\xdc\xb6\x21\x48\xc3\xe5\x3b\x8f\x3f\x11\xd1\x84\x0\x11\xe6\x17\xb7\x6d\x0\x6\x3\x20\xcd\xcb\x2\x0\x61\x20\x0\x0\xb4\x0\x0\x0\x13\x4\x41\x2c\x10\x0\x0\x0\xe\x0\x0\x0\x44\x14\x57\x21\x94\xc2\xc\x40\x59\xa\x94\x5d\xc9\xe\x14\xec\x40\x69\x14\x26\x42\x81\x14\x50\x81\x95\xf\x15\xe5\x41\xdf\x8\x0\x6d\x73\x8\x69\x0\x91\x37\x2\x30\x7\xe1\x38\x4e\x46\xdb\x1c\x2\x1a\x44\x0\x0\x0\x0\x23\x6\x8\x0\x82\x60\x0\x6\x6a\xe0\xc\x66\x50\x8c\x18\x20\x0\x8\x82\x1\x18\xac\xc1\x33\x9c\x81\x31\x62\x70\x0\x20\x8\x6\x5c\x1b\x3c\xc1\x30\x62\x90\x0\x20\x8\x6\x48\x1d\x58\x6c\x90\x6\x61\x40\x8d\x18\x24\x0\x8\x82\x81\x71\x7\x97\x1a\xa8\x81\x18\x54\x23\x6\x9\x0\x82\x60\x60\xe0\x1\xb6\x6\x6b\xf0\x59\x23\x6\x9\x0\x82\x60\x60\xe4\x41\xc6\x6\x6c\x10\x6\xd7\x88\xc1\x1\x80\x20\x18\x70\x72\x40\x1d\xcb\x88\x81\x2\x80\x20\x18\x50\x76\x80\x5\x85\x1b\x38\xdb\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\xc3\x88\x81\x2\x80\x20\x18\x50\x7b\xd0\x19\xca\x35\x81\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\xc3\x88\x81\x2\x80\x20\x18\x50\xa0\x20\x6\xcb\xb3\x61\x65\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xa3\x9\xc4\x30\x62\xa0\x0\x20\x8\x6\x91\x29\x9c\x1\x44\x7d\x71\xa0\x6\xa3\x9\x1\x60\x50\x25\x9f\x11\x3\x4\x0\x41\x30\xc8\x4c\xc1\xd\xa0\x2a\x18\x31\x40\x0\x10\x4\x83\xec\x14\xde\x0\xaa\x2\xb\x20\xe8\xd8\x93\xc9\x67\xc4\x0\x1\x40\x10\xc\x32\x55\x90\x83\x27\xb\x46\xc\x10\x0\x4\xc1\x20\x5b\x85\x39\x78\xb2\xc0\x82\x7\x3a\xe6\x74\xf2\x19\x31\x40\x0\x10\x4\x83\xcc\x15\xec\xc0\xe9\x82\x11\x3\x4\x0\x41\x30\xc8\x5e\xe1\xe\x9c\x2e\xb0\xc0\x81\xce\x88\xc1\x1\x80\x20\x18\x5c\xb2\x40\x7\x63\xe0\xa\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x30\x9a\x40\xc\x23\x6\x7\x0\x82\x60\x70\xdd\x42\x1e\xa0\xc1\x2a\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x68\x2\x31\x8c\x18\x1c\x0\x8\x82\xc1\xc5\xb\x7e\xd0\x6\xb1\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xa3\x9\xc4\x30\x62\x70\x0\x20\x8\x6\x57\x38\x8c\x82\x1c\x9c\xc2\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\x83\x4d\x9d\x7c\x46\xc\x10\x0\x4\xc1\x20\x33\x7\x57\x78\xb4\x60\xc4\x0\x1\x40\x10\xc\xb2\x73\x78\x85\xe5\xa\x2c\x38\xa0\x63\x56\x18\xc8\x67\xc4\x0\x1\x40\x10\xc\x32\x75\x90\x5\xc9\xb\x46\xc\x10\x0\x4\xc1\x20\x5b\x87\x59\x70\xb6\xc0\x2\x5\x3a\x96\x95\x81\x7c\x46\xc\x10\x0\x4\xc1\x20\x73\x7\x5b\xa8\xc4\x20\x18\x31\x40\x0\x10\x4\x83\xec\x1d\x6e\x21\xfa\x2\xb\x1a\xe8\x18\x97\x6\xf2\x19\x31\x40\x0\x10\x4\x83\x4c\x1e\x74\x1\x33\x83\x60\xc4\x0\x1\x40\x10\xc\xb2\x79\xd8\x5\x6a\xc\x2\xb\x20\xe8\x54\x1c\xec\x2\x96\x1c\xc4\x82\x16\x3d\x4\x37\x62\x90\x0\x20\x8\x6\x4a\x3f\xf8\x82\x3c\xc8\x83\x3a\xc\x23\x6\x9\x0\x82\x60\xa0\xf4\x83\x2f\xd4\x83\x3c\xa8\x43\x30\x62\x90\x0\x20\x8\x6\xc\x3f\xf8\x42\x3b\xc8\x83\x3a\x40\x23\x6\x9\x0\x82\x60\xc0\xf0\x83\x2f\xb4\x83\x3c\x98\x3\x33\x62\x90\x0\x20\x8\x6\xc\x3f\xf8\x42\x3b\xc8\xc3\x39\x20\x23\x6\x9\x0\x82\x60\xc0\xf0\x83\x2f\xb4\x83\x3c\xa4\x3\x81\x0\x0\x0\x0\x0";

}