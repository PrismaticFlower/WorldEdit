#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char tile_lightsPS_dxil_bytes[2365];

auto tile_lightsPS() noexcept -> shader_def
{
   return {
      .name = "tile_lightsPS",
      .entrypoint = L"mainPS",
      .target = L"ps_6_6",
      .file = L"tile_lights.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(tile_lightsPS_dxil_bytes),
               sizeof(tile_lightsPS_dxil_bytes) - 1},
   };
}

const char tile_lightsPS_dxil_bytes[2365] = "\x44\x58\x42\x43\x2c\x8f\x7d\x84\xc2\xb\x96\x72\xa\xfa\x80\x3b\x6c\x41\xb2\xee\x1\x0\x0\x0\x3c\x9\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xdc\x0\x0\x0\xec\x0\x0\x0\xb4\x1\x0\x0\xe0\x1\x0\x0\xfc\x1\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x88\x0\x0\x0\x3\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x68\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x72\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x2\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x7b\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x3\x0\x0\x0\x0\x0\x0\x4c\x49\x47\x48\x54\x57\x4f\x52\x44\x0\x4c\x49\x47\x48\x54\x42\x49\x54\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x0\x4f\x53\x47\x31\x8\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x50\x53\x56\x30\xc0\x0\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x3\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xc\x0\x0\x0\x0\x0\x0\x0\x14\x0\x0\x0\x0\x4c\x49\x47\x48\x54\x57\x4f\x52\x44\x0\x4c\x49\x47\x48\x54\x42\x49\x54\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x41\x0\x1\x1\x0\x0\xb\x0\x0\x0\x0\x0\x0\x0\x1\x0\x51\x0\x1\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x3\x3\x4\x0\x0\x49\x4c\x44\x4e\x24\x0\x0\x0\x0\x0\x1f\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x74\x69\x6c\x65\x5f\x6c\x69\x67\x68\x74\x73\x50\x53\x2e\x70\x64\x62\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x54\x29\xf2\xb0\x50\x85\x3c\xb1\x10\x4\xa0\xa5\x54\xe2\x50\xfc\x44\x58\x49\x4c\x38\x7\x0\x0\x66\x0\x0\x0\xce\x1\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x20\x7\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\xc5\x1\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x4c\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\x90\xc1\x8\x40\x9\x0\xa\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x30\xc\x3\x31\x50\x31\x3\x50\x8c\x63\x18\x6\x62\x20\x84\x88\x7b\x86\xcb\x9f\xb0\x87\x90\xfc\x10\x68\x86\x85\x40\x81\x52\x90\x61\x58\x86\x61\x18\x6\x62\x8e\x1a\x2e\x7f\xc2\x1e\x42\xf2\xb9\x8d\x2a\x56\x62\xf2\x91\xdb\x46\xc4\x30\xc\x43\x21\x9c\x61\x19\xe8\xb9\x6d\xb8\xfc\x9\x7b\x8\xc9\x5f\x9\xc9\xa1\x22\x81\x48\x23\xe7\x21\xa2\x9\x21\x24\x24\xc\x43\x21\x96\x61\x89\x48\x3a\x68\xb8\xfc\x9\x7b\x8\xc9\x5f\x9\x69\x43\x9a\x1\x11\xc3\x30\x10\x73\x4\x41\x29\x96\x81\x1a\x2a\xb2\x6\x2\x12\x61\x98\x49\xd\xc6\x81\x1d\xc2\x61\x1e\xe6\xc1\xd\x64\xe1\x16\x66\x81\x1e\xe4\xa1\x1e\xc6\x81\x1e\xea\x41\x1e\xca\x81\x1c\x44\xa1\x1e\xcc\xc1\x1c\xca\x41\x1e\xf8\xa0\x1e\xdc\x61\x1e\xd2\xe1\x1c\xdc\xa1\x1c\xc8\x1\xc\xd2\xc1\x1d\xe8\x1\xc\x76\x1\xf\x7a\xc1\xf\x50\x90\x91\x36\x8c\x20\xc\xc3\x8\xc4\x91\x4\xdd\x7d\x87\x23\x4d\xb\x80\x39\xd4\xe4\x4b\x53\x44\x9\x93\x3f\x21\xb\xd2\x18\x3f\xd2\x3c\xd4\x24\x21\x81\xf3\xce\x3a\x1c\x69\x5a\x0\xcc\xa1\x26\x7f\x42\x16\xe2\x47\x9a\x87\x9a\x24\x14\x7c\x4\xa6\xc0\x30\x47\x0\xa\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x5\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x2\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x30\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x79\x80\x0\x18\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x23\x1\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\xc7\x2\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x2c\x10\xf\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x1a\x4a\xa0\x18\xca\x61\x4\xa0\x30\xa\xa8\x40\x3\xca\xa0\x8\x88\x28\x85\x11\x80\x12\x28\x89\x32\x28\x4\xaa\x46\x0\x68\x2c\x18\xa\x67\x0\x48\x1c\x4b\x40\x0\xda\x66\x0\x0\x79\x18\x0\x0\x61\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x86\x63\x82\x30\x20\x1b\x84\x81\x98\x20\xc\xc9\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\xc\xca\x4\x61\xd3\x8\x4c\x10\x86\x65\x82\x50\x59\x13\x84\x81\xd9\x20\xc\xcf\x86\x45\x59\x18\x45\x19\x1a\xc7\x71\xa0\xd\x41\x34\x41\x8\x3\x6c\x82\x30\x34\x1b\x10\x65\x62\x14\x65\xa0\x80\xd\x41\xb5\x81\x0\x24\xb\x98\x20\x88\x41\xb6\x21\xc0\x26\x8\x2\x40\xa3\x2d\x2c\xcd\xd\x6a\x8a\x9\x53\xd2\x11\x12\xd5\xd5\x93\x14\xd1\x4\x81\x80\x26\x8\x44\xb4\x21\x50\x26\x8\x84\x34\x41\x18\x9c\xd\x42\x18\xc\x1b\x16\x85\xeb\xbc\xf\xc\x6\x30\x50\x3c\x31\x20\xc2\x94\x74\x84\x44\x25\x94\x44\xb5\x61\x19\xc8\xa0\xf3\x3e\x30\x18\xc0\x40\x1\x3\x31\x98\x20\xc\xf\x97\x29\xab\x2f\xa8\xb7\xb9\x34\xba\xb4\x37\xb7\x9\x2\x31\x4d\x10\x8\x6a\x82\x40\x54\x1b\x84\x30\x8\x83\xd\x8b\x19\x9c\x1\x1a\xa4\xc1\xa7\x6\x83\x1a\xc\xde\x1a\x6c\x18\xc6\xa0\xc\xd8\x60\xc3\xd0\x6\x0\x30\x41\x18\x83\x6b\x83\xa0\xbc\xc1\x86\x42\xdb\xdc\xe0\x82\x83\x2a\x6c\x6c\x76\x6d\x2e\x69\x64\x65\x6e\x74\x53\x82\xa0\xa\x19\x9e\x8b\x5d\x99\xdc\x5c\xda\x9b\xdb\x94\x80\x68\x42\x86\xe7\x62\x17\xc6\x66\x57\x26\x37\x25\x30\xea\x90\xe1\xb9\xcc\xa1\x85\x91\x95\xc9\x35\xbd\x91\x95\xb1\x4d\x9\x90\x32\x64\x78\x2e\x72\x65\x73\x6f\x75\x72\x63\x65\x73\x53\x2\xab\xe\x19\x9e\x8b\x5d\x5a\xd9\x5d\x12\xd9\x14\x5d\x18\x5d\xd9\x94\x0\xab\x43\x86\xe7\x52\xe6\x46\x27\x97\x7\xf5\x96\xe6\x46\x37\x37\x25\x80\x3\x0\x79\x18\x0\x0\x4c\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\x8c\xc8\x21\x7\x7c\x70\x3\x72\x10\x87\x73\x70\x3\x7b\x8\x7\x79\x60\x87\x70\xc8\x87\x77\xa8\x7\x7a\x98\x81\x3c\xe4\x80\xf\x6e\x40\xf\xe5\xd0\xe\xf0\x0\x0\x0\x71\x20\x0\x0\x1f\x0\x0\x0\x56\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\xd8\x40\x35\x5c\xbe\xf3\xf8\xc0\xe4\x30\x88\xb0\x21\xd\xfa\xf8\xc8\x6d\x1b\xc1\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x8f\xdc\xb6\x19\x74\xc3\xe5\x3b\x8f\x2f\x44\x4\x30\x11\x21\xd0\xc\xb\xf1\x45\xe\xb3\x21\xcd\x80\x34\x86\x9\x4c\xc3\xe5\x3b\x8f\xbf\x38\xc0\x20\x36\xf\x35\xf9\xc5\x6d\x5b\xc0\x34\x5c\xbe\xf3\xf8\x8b\x3\xc\x62\xf3\x50\x93\x8f\xdc\xb6\x1\x18\xc\x80\x34\x29\xb\x0\x0\x61\x20\x0\x0\x33\x0\x0\x0\x13\x4\x41\x2c\x10\x0\x0\x0\xb\x0\x0\x0\x34\xcc\x0\x14\x42\xc9\xe\x14\xec\x40\x69\x94\x5d\x61\x2\x15\x67\x0\x11\x45\x40\xd1\x1c\xc2\xd7\xcc\x21\x84\x1\x43\xd4\x1c\x84\xa2\x28\xd2\x1c\x84\xa2\x28\x63\x0\x0\x0\x23\x6\x8\x0\x82\x60\x60\x89\xc1\x22\x78\xd2\x88\x1\x2\x80\x20\x18\x58\x63\xc0\x8\xdf\x34\x62\x70\x0\x20\x8\x6\x52\x19\x30\x81\x31\x62\x90\x0\x20\x8\x6\x48\x1a\x3c\x5c\x18\x68\xd0\x88\x41\x2\x80\x20\x18\x20\x6a\x0\x75\x62\xa0\x45\x23\x6\x9\x0\x82\x60\x50\xb0\x41\x64\x6\x63\xc0\x49\x23\x6\x9\x0\x82\x60\x50\xb4\x81\x44\x6\x64\xd0\x4d\x47\xc\x73\xc4\x30\x23\x6\x7\x0\x82\x60\xf0\xb4\x41\x74\x9c\xc1\x68\x42\x0\x54\x30\x48\x5\x5\x8c\x18\x1c\x0\x8\x82\x81\x4\x7\x57\x3\x15\x72\x6\x37\x62\xb0\x0\x20\x8\x6\xc\x1d\x54\x2\x1a\xc\x1\xa7\x20\x0\x0\x0\x0\x0\x0\x0\x0";

}