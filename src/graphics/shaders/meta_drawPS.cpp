#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char meta_drawPS_dxil_bytes[1653];

auto meta_drawPS() noexcept -> shader_def
{
   return {
      .name = "meta_drawPS",
      .entrypoint = L"main",
      .target = L"ps_6_6",
      .file = L"meta_drawPS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(meta_drawPS_dxil_bytes),
               sizeof(meta_drawPS_dxil_bytes) - 1},
   };
}

const char meta_drawPS_dxil_bytes[1653] = "\x44\x58\x42\x43\x2a\x5\x92\x4e\x4\x77\xe0\x65\x50\xd1\xf5\x58\x86\x4\x99\x6e\x1\x0\x0\x0\x74\x6\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x84\x0\x0\x0\xc0\x0\x0\x0\x48\x1\x0\x0\x74\x1\x0\x0\x90\x1\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x30\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\xf\x0\x0\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\x80\x0\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x1\x1\x0\x1\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x4\x0\x0\x0\xf\x0\x0\x0\x49\x4c\x44\x4e\x24\x0\x0\x0\x0\x0\x1d\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x74\x61\x5f\x64\x72\x61\x77\x50\x53\x2e\x70\x64\x62\x0\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\xf9\x66\x8a\x1b\x6f\x74\x91\x4b\x72\xce\x9e\x91\x5e\xbe\x55\xb\x44\x58\x49\x4c\xdc\x4\x0\x0\x66\x0\x0\x0\x37\x1\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\xc4\x4\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x2e\x1\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x10\x45\x2\x42\x92\xb\x42\x84\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x42\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x22\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x21\x46\x6\x51\x18\x0\x0\x6\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x1\x0\x0\x0\x49\x18\x0\x0\x2\x0\x0\x0\x13\x82\x60\x42\x20\x0\x0\x0\x89\x20\x0\x0\xf\x0\x0\x0\x32\x22\x8\x9\x20\x64\x85\x4\x13\x22\xa4\x84\x4\x13\x22\xe3\x84\xa1\x90\x14\x12\x4c\x88\x8c\xb\x84\x84\x4c\x10\x30\x23\x0\x25\x0\x8a\x19\x80\x39\x2\x30\x98\x23\x40\x8a\x31\x44\x54\x44\x56\xc\x20\xa2\x1a\xc2\x81\x80\x34\x20\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\xc8\x2\x1\xb\x0\x0\x0\x32\x1e\x98\x10\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\xa2\x12\x28\x86\x72\x18\x1\x28\x83\xf2\xa0\x2a\x89\x11\x80\x22\x28\x84\x2\xa1\x1d\xcb\x20\x88\x40\x20\x1e\x0\x0\x79\x18\x0\x0\x41\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x88\x61\x82\x40\x10\x1b\x84\x81\x98\x20\x10\xc5\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\x2c\xce\x86\x40\x99\x20\x8\x0\x89\xb6\xb0\x34\xb7\x9\x2\x61\xb0\x18\x7a\x62\x7a\x92\x9a\x20\x14\xc9\x4\xa1\x50\x36\x4\xce\x4\xa1\x58\x26\x8\x5\x33\x41\x20\x8e\x9\x2\x81\x6c\x10\x2a\x6b\xc3\xe2\x3c\x50\x24\x4d\x3\xe5\x44\xd7\x86\x0\x63\x32\x65\xf5\x45\x15\x26\x77\x56\x46\x37\x41\x28\x9a\xd\x8b\xa3\x41\x9b\x14\xd\x94\x13\x5d\x1b\x2\x6e\xc3\x90\x75\xc0\x86\x82\x69\x3c\x0\xa8\xc2\xc6\x66\xd7\xe6\x92\x46\x56\xe6\x46\x37\x25\x8\xaa\x90\xe1\xb9\xd8\x95\xc9\xcd\xa5\xbd\xb9\x4d\x9\x88\x26\x64\x78\x2e\x76\x61\x6c\x76\x65\x72\x53\x2\xa3\xe\x19\x9e\xcb\x1c\x5a\x18\x59\x99\x5c\xd3\x1b\x59\x19\xdb\x94\x0\xa9\x43\x86\xe7\x62\x97\x56\x76\x97\x44\x36\x45\x17\x46\x57\x36\x25\x50\xea\x90\xe1\xb9\x94\xb9\xd1\xc9\xe5\x41\xbd\xa5\xb9\xd1\xcd\x4d\x9\x3c\x0\x0\x0\x79\x18\x0\x0\x4c\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x0\x0\x0\x71\x20\x0\x0\xb\x0\x0\x0\x16\x30\xd\x97\xef\x3c\xfe\xe2\x0\x83\xd8\x3c\xd4\xe4\x17\xb7\x6d\x2\xd5\x70\xf9\xce\xe3\x4b\x93\x13\x11\x28\x35\x3d\xd4\xe4\x17\xb7\x6d\x0\x4\x3\x20\xd\x0\x0\x0\x0\x61\x20\x0\x0\x21\x0\x0\x0\x13\x4\x41\x2c\x10\x0\x0\x0\x3\x0\x0\x0\x44\x85\x30\x3\x50\xa\x54\x25\x50\x6\x0\x0\x23\x6\x9\x0\x82\x60\x60\x4c\x85\xe3\x28\xc4\x88\x41\x2\x80\x20\x18\x18\x94\xf1\x3c\x43\x31\x62\x90\x0\x20\x8\x6\x46\x75\x40\xd0\x62\x8c\x18\x24\x0\x8\x82\x81\x61\x21\x51\x44\x1c\x16\x10\xf2\x31\x81\x90\x8f\xd\x84\x7c\x46\xc\x12\x0\x4\xc1\x0\xc9\x94\xaa\x8a\x86\x11\x83\x4\x0\x41\x30\x40\x32\xa5\xaa\x12\x61\xc4\x20\x1\x40\x10\xc\x90\x4c\xa9\x2a\x28\x18\x31\x48\x0\x10\x4\x3\x24\x53\xaa\xa\x21\x10\x0\x0\x0\x0\x0\x0\x0";

}