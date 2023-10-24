#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char mesh_basic_lightingPS_dxil_bytes[6737];

auto mesh_basic_lightingPS() noexcept -> shader_def
{
   return {
      .name = "mesh_basic_lightingPS",
      .entrypoint = L"main",
      .target = L"ps_6_6",
      .file = L"mesh_basic_lightingPS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(mesh_basic_lightingPS_dxil_bytes),
               sizeof(mesh_basic_lightingPS_dxil_bytes) - 1},
   };
}

const char mesh_basic_lightingPS_dxil_bytes[6737] = "\x44\x58\x42\x43\x49\x49\x4a\x19\xd5\xa4\x80\x80\xd3\xb7\x7c\xc2\xf8\xa8\xef\xf0\x1\x0\x0\x0\x50\x1a\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x7c\x1\x0\x0\xb8\x1\x0\x0\x48\x3\x0\x0\x7c\x3\x0\x0\x98\x3\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x40\x0\x6\x0\x0\x0\x0\x49\x53\x47\x31\x28\x1\x0\x0\x7\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\xe8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xf3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xfa\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xc\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x4\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x15\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x5\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1b\x1\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x6\x0\x0\x0\xf\x3\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x57\x53\x0\x4e\x4f\x52\x4d\x41\x4c\x0\x54\x41\x4e\x47\x45\x4e\x54\x0\x42\x49\x54\x41\x4e\x47\x45\x4e\x54\x0\x54\x45\x58\x43\x4f\x4f\x52\x44\x0\x43\x4f\x4c\x4f\x52\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\x88\x1\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x7\x1\x0\x7\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x34\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x57\x53\x0\x4e\x4f\x52\x4d\x41\x4c\x0\x54\x41\x4e\x47\x45\x4e\x54\x0\x42\x49\x54\x41\x4e\x47\x45\x4e\x54\x0\x54\x45\x58\x43\x4f\x4f\x52\x44\x0\x43\x4f\x4c\x4f\x52\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x2\x0\x0\xc\x0\x0\x0\x0\x0\x0\x0\x1\x1\x43\x0\x3\x2\x0\x0\x13\x0\x0\x0\x0\x0\x0\x0\x1\x2\x43\x0\x3\x2\x0\x0\x1b\x0\x0\x0\x0\x0\x0\x0\x1\x3\x43\x0\x3\x2\x0\x0\x25\x0\x0\x0\x0\x0\x0\x0\x1\x4\x42\x0\x3\x2\x0\x0\x2e\x0\x0\x0\x0\x0\x0\x0\x1\x5\x44\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x6\x44\x3\x3\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x2c\x0\x0\x0\x0\x0\x27\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x73\x68\x5f\x62\x61\x73\x69\x63\x5f\x6c\x69\x67\x68\x74\x69\x6e\x67\x50\x53\x2e\x70\x64\x62\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x27\xfa\x7\x21\x82\x76\xcb\x85\x7e\x5d\xef\xa1\xdc\x57\xef\xf2\x44\x58\x49\x4c\xb0\x16\x0\x0\x66\x0\x0\x0\xac\x5\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x98\x16\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\xa3\x5\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x69\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\xf4\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x80\x20\x88\x62\x20\x84\x8a\x7b\x86\xcb\x9f\xb0\x87\x90\xfc\x10\x68\x86\x85\x40\x81\x32\x47\x10\x94\x62\x21\x8\x86\xa1\xa6\xc\x3\x31\xd0\x53\x90\x81\x18\x86\x61\x18\x6\x8a\xa\x41\x10\x44\x41\xd3\x4d\xc3\xe5\x4f\xd8\x43\x48\xfe\x4a\x48\x2b\x31\xf9\xc8\x6d\xa3\x82\x20\x8\x82\x28\x47\x45\x2c\x4\x51\x10\x64\x95\x81\x20\x8\xc2\x6e\x1a\x2e\x7f\xc2\x1e\x42\xf2\x57\x42\x5a\x89\xc9\x2f\x6e\x1b\x15\xc3\x30\xc\x44\x39\x34\x62\x21\x88\x82\xa0\xad\x10\x3\x31\xc\xd4\x15\x46\x23\x96\x65\x18\x86\x81\x20\x88\x81\xbe\xa3\x86\xcb\x9f\xb0\x87\x90\x7c\x6e\xa3\x8a\x95\x98\xfc\xe2\xb6\x11\x31\xc\xc3\x50\x88\x90\x58\x8\x12\x8f\x1a\x2e\x7f\xc2\x1e\x42\xf2\xb9\x8d\x2a\x56\x62\xf2\x91\xdb\x46\x4\x41\x10\x44\x21\x48\x62\x21\xa8\x2c\xc5\x40\xc\xc3\x40\xe7\x6d\xc3\xe5\x4f\xd8\x43\x48\xfe\x4a\x48\xe\x15\x9\x44\x1a\x39\xf\x11\x4d\x8\x21\x21\x81\x20\xa\xb1\x10\x4b\x4a\xea\x41\xc3\xe5\x4f\xd8\x43\x48\xfe\x4a\x48\x1b\xd2\xc\x88\x20\x8\xa2\x28\xc5\x42\xb0\x4\x43\xed\x40\xc0\x30\xc2\x30\xc\x23\x10\x43\x12\xc4\x49\x20\xf\x23\x8\xc3\x61\xd2\x14\x51\xc2\xe4\x2f\x88\x71\x4c\xff\x40\x48\x42\x84\x3c\x13\xe2\x34\x48\x81\x9\x78\x80\x87\x81\x20\x88\x4\x14\xa8\xef\xc\xe\x47\x9a\x16\x0\x73\xa8\xc9\x97\xa6\x88\x12\x26\x7f\x41\x8c\x63\xfa\x5\xa7\x91\x26\xa0\x99\xfe\x80\x2a\xa\x22\x42\xc\x41\x10\x4\x4c\xc0\xc4\x4c\xd3\xf6\x69\x87\x23\x4d\xb\x80\x39\xd4\xe4\x2f\x88\x71\x4c\xbf\xe0\x34\xd2\x4\x34\x93\x84\x82\x9b\xf0\xf4\x20\xe6\x8\x40\x61\xa\x0\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x34\x40\x0\x4\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x79\x80\x0\x8\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x13\x1\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x67\x2\x2\x40\x0\x0\x0\x0\x0\x0\x0\x80\x21\x8f\x5\x4\xc0\x0\x0\x0\x0\x0\x0\x0\x0\x43\x1e\xc\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x1b\x10\x0\x3\x0\x0\x0\x0\x0\x0\x0\xc\x79\x3a\x20\x0\x2\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x7c\x40\x0\xc\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x11\x3\x20\x0\x6\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x94\x1\x10\x0\x3\x0\x0\x0\x0\x0\x0\x0\xc\x79\xce\x0\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6a\x0\x4\x40\x0\x0\x0\x0\x0\x0\x0\x0\x43\x9e\x36\x0\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x2c\x10\x0\x15\x0\x0\x0\x32\x1e\x98\x18\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\x0\xb\x5\xca\xa0\x8\xa\xa1\x14\xca\x83\x8a\x92\x18\x1\x28\x82\x32\x28\x84\x2\xa1\xbd\x0\x1\x81\x1\x31\x48\x1f\xcb\xe3\x88\xe3\x38\x80\xe3\x38\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x80\xe3\x0\x0\xca\x67\x0\x0\x0\x0\x0\x79\x18\x0\x0\x77\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x88\x68\x82\x40\x48\x1b\x84\x81\x98\x20\x10\xd3\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\x10\xd4\x4\x21\xf\xc6\x80\xc0\x4\x81\xa8\x36\x20\xca\xc2\x28\xc3\xd0\x0\x1b\x2\x67\x3\x1\x0\xf\x30\x41\xd0\x3\x31\xd8\x10\x44\x13\x4\x1\x20\xd1\x16\x96\xe6\x46\x5\xea\x69\x2a\x89\x2a\xe9\xc9\xe9\x6a\x6a\x82\x50\x6c\x13\x84\x82\xdb\x10\x28\x13\x84\xa2\x9b\x20\x14\xde\x4\x81\xb0\x36\x8\x9b\xb1\x61\x51\x2a\xeb\xc2\xb2\x41\x53\x2e\x8e\x86\xd3\x93\x54\x53\x10\xd3\x86\x65\xf0\xac\xb\xcb\x6\x6d\xb8\xb8\x9\x2\x71\xf1\xa0\xa\x72\x3a\x2a\x72\xa2\xda\xb0\x80\x41\x18\x58\x17\x96\xd\x1a\x18\x5c\x0\x13\xa1\x24\xaa\x20\xa7\xa3\x22\x27\xaa\xd\xcb\x36\x6\xd6\x85\x65\x83\xb6\x5d\xc0\x4\x81\xc0\x88\x50\x15\x61\xd\x3d\x3d\x49\x11\x6d\x58\xca\xc0\xc\xac\xb\xcb\x86\xac\xc\x2e\x60\x82\x40\x64\x2c\x86\x9e\x98\x9e\xa4\x26\x8\xc5\xb7\x61\x41\x83\x34\xb0\x2e\x2c\x1b\xd4\x0\xd\x2e\x80\xcb\x94\xd5\x17\xd4\xdb\x5c\x1a\x5d\xda\x9b\xdb\x6\x61\xdb\x36\x2c\x4\x1b\x58\x1a\xa6\x6\x83\x1a\x10\x57\x1b\x6c\x38\xba\x4f\xc\xc8\xe0\xc\xd6\xc0\xd\x98\x4c\x59\x7d\x51\x85\xc9\x9d\x95\xd1\x4d\x10\xa\x30\x98\x20\x10\xda\x6\x61\x93\x83\xd\x8b\x2\x7\x56\x1c\x60\xd7\xa0\x6\xca\x35\x7\x1b\x2\x3a\xd8\x30\xbc\x41\x1d\x0\x13\x84\x3d\x8\x83\xd\x82\x72\x7\x1b\x8a\x89\xb2\x3\x8\xf\xaa\xb0\xb1\xd9\xb5\xb9\xa4\x91\x95\xb9\xd1\x4d\x9\x82\x2a\x64\x78\x2e\x76\x65\x72\x73\x69\x6f\x6e\x53\x2\xa2\x9\x19\x9e\x8b\x5d\x18\x9b\x5d\x99\xdc\x94\xc0\xa8\x43\x86\xe7\x32\x87\x16\x46\x56\x26\xd7\xf4\x46\x56\xc6\x36\x25\x40\xca\x90\xe1\xb9\xc8\x95\xcd\xbd\xd5\xc9\x8d\x95\xcd\x4d\x9\x9e\x3a\x64\x78\x2e\x76\x69\x65\x77\x49\x64\x53\x74\x61\x74\x65\x53\x82\xa8\xe\x19\x9e\x4b\x99\x1b\x9d\x5c\x1e\xd4\x5b\x9a\x1b\xdd\xdc\x94\x0\xf\x0\x79\x18\x0\x0\x55\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xca\xc3\xe\xec\x60\xf\xed\xe0\x6\xec\xf0\xe\xef\x0\xf\x33\x22\x88\x1c\xf0\xc1\xd\xc8\x41\x1c\xce\xc1\xd\xec\x21\x1c\xe4\x81\x1d\xc2\x21\x1f\xde\xa1\x1e\xe8\x61\x6\x13\x91\x3\x3e\xb8\x81\x38\xc8\x43\x39\x84\xc3\x3a\xb8\x81\x38\xc8\x3\x0\x0\x0\x71\x20\x0\x0\x4c\x0\x0\x0\xf6\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x98\x2\x34\x5c\xbe\xf3\xf8\x1\xd2\x0\x11\xe6\x17\xb7\x6d\xc\xdb\x70\xf9\xce\xe3\xb\x1\x55\x14\x44\x54\x3a\xc0\x50\x12\x6\x20\x60\x7e\x71\xdb\xd6\xb0\xd\x97\xef\x3c\xbe\x10\x50\x45\x41\x44\xa5\x3\xc\x25\x61\x0\x2\xe6\x23\xb7\x6d\x10\xdd\x70\xf9\xce\xe3\xb\x11\x1\x4c\x44\x8\x34\xc3\x42\x7c\x91\xc3\x6c\x48\x33\x20\x8d\x61\x3\xda\x70\xf9\xce\xe3\xb\x11\x1\x4c\x44\x8\x34\xc3\x42\x7c\x91\xc3\x84\x4\xf0\x58\x81\x33\x5c\xbe\xf3\xf8\x83\x33\xdd\x7e\x71\xdb\x16\x30\xd\x97\xef\x3c\xfe\xe2\x0\x83\xd8\x3c\xd4\xe4\x17\xb7\x6d\x9\xd7\x70\xf9\xce\xe3\x47\x80\xb5\x51\x45\x41\x44\xa5\x3\xc\x7e\x71\xdb\x76\x70\xd\x97\xef\x3c\x7e\x4\x58\x1b\x55\x14\x44\x54\x3a\xc0\xe0\x23\xb7\x6d\xb\xdc\x70\xf9\xce\xe3\x4b\x0\xf3\x2c\x4\xc7\x3c\x25\x51\x11\xcb\x4c\x44\x8e\x5f\xdc\xb6\x9\x54\xc3\xe5\x3b\x8f\x2f\x4d\x4e\x44\xa0\xd4\xf4\x50\x93\x5f\xdc\xb6\x39\x48\xc3\xe5\x3b\x8f\x3f\x11\xd1\x84\x0\x11\xe6\x17\xb7\x6d\x4\xcf\x70\xf9\xce\xe3\x53\xd\x10\x61\x7e\x71\xdb\x86\x30\xd\x97\xef\x3c\x3e\xd5\x0\x11\xb6\x21\x93\xe4\x23\xb7\x6d\x6\xd7\x70\xf9\xce\xe3\x5b\x40\x45\x68\xc2\x84\x54\xc4\x86\x4c\x3e\x72\xdb\x6\x40\x30\x0\xd2\x0\x0\x61\x20\x0\x0\x80\x3\x0\x0\x13\x4\x52\x2c\x10\x0\x0\x0\x31\x0\x0\x0\xc4\xd4\xc0\x8\x0\x15\xe5\x50\x1e\x25\x40\x44\x81\x6\x14\x48\x49\x12\x94\x49\x51\x94\x44\x71\x94\x45\xa1\x94\x5d\x71\xcd\x0\x94\x51\x9\x6\x94\x5b\xc1\xe\x94\xa5\x40\xd1\xe\x14\x4c\x1\x95\x4c\x69\x14\x46\xb9\x14\x44\xa9\x14\x4b\xc9\xe\x14\x27\x20\x41\x61\x22\x14\x70\x40\xc1\x7\x94\x4f\xd\xd0\x30\x46\xd0\x9a\x73\xce\x7b\x63\x4\x20\x8\x82\x20\x18\x8c\x11\x80\x20\x8\xea\xdf\x18\x1\x8\x82\xa0\xff\x8d\x11\x80\x20\x8\xa2\xdf\x18\xc1\x3b\x93\x26\xda\xb\x63\x4\x20\x8\x82\x20\x28\x8c\x11\x80\x20\x8\xc2\xdf\x18\x1\x8\x82\x30\x1b\x6\x63\x4\x20\x8\x82\xf4\x37\x46\x0\x82\x20\x88\x7f\x33\x0\x23\x0\x63\x4\x20\x8\x82\xf0\x2f\x50\x3a\x87\xe0\x7\xd5\x1c\x42\x29\x50\x73\x8\x7f\xc0\x7\x73\x8\xa6\x70\xa\x73\x8\xd3\x1a\x10\x3b\x7\x11\x45\x54\x7\x0\x0\x0\x23\x6\x8\x0\x82\x60\xd0\x6\xaf\xb0\x5\xae\xa0\x7\x23\x6\x7\x0\x82\x60\xa0\x6\xb1\x80\x6\xc1\x31\x62\x90\x0\x20\x8\x6\x6\x38\xa4\xc2\x2b\xb4\x42\x29\xb4\xc1\x88\x41\x2\x80\x20\x18\x18\xe1\xa0\xa\xb0\xe0\xa\x7a\xe0\x6\x23\x6\x9\x0\x82\x60\x60\x88\xc3\x2a\xc8\xc2\x2b\x9c\xc2\x1b\x8c\x18\x24\x0\x8\x82\x81\x31\xe\xac\x30\xb\xb0\xc0\x7\x70\x30\x62\x90\x0\x20\x8\x6\x6\x39\xb4\x2\x2d\xc4\x2\x2a\xc4\xc1\x88\x41\x2\x80\x20\x18\x18\xe5\xe0\xa\xb2\x20\xb\xaa\x20\x7\x23\x6\x9\x0\x82\x60\x60\x98\xc3\x2b\xcc\xc2\x2c\xfc\xc1\x1c\x8c\x18\x24\x0\x8\x82\x81\x71\xe\xb0\x40\xb\xb4\xb0\xa\x74\x30\x62\x80\x0\x20\x8\x6\x8d\x39\xbc\xc1\x2d\x94\x2\x29\x8c\x18\x1c\x0\x8\x82\x81\x1a\xec\x82\x1c\x4\xd0\x88\xc1\x1\x80\x20\x18\x94\x81\x2f\xe4\xc1\x72\xb\xa3\x9\x41\x30\x62\x80\x0\x20\x8\x6\x8d\x3a\xcc\x41\x80\xa\xa8\x30\x62\x70\x0\x20\x8\x6\x6a\xf0\xb\x76\x10\x4c\xa3\x9\x84\x30\x62\x80\x0\x20\x8\x6\x4d\x3b\xd8\x41\xb0\xa\xab\x30\x62\x70\x0\x20\x8\x6\x6a\x20\xe\x79\x10\x54\x23\x6\xb\x0\x82\x60\x10\xb5\xc3\x1e\x3c\x4e\xf3\x38\xcd\x88\x81\x1\x80\x20\x18\x3c\xef\x60\x7\x81\x5\x91\x7c\x4c\x88\xe4\x63\x43\x24\x9f\xb3\x88\x39\x8b\x18\x23\xce\x40\x3e\x86\x6\x41\x7c\x46\xc\xe\x0\x4\xc1\x40\xc\xde\x21\x15\x36\x75\x18\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x4\x73\x83\x22\x3e\x46\x4\xf2\x31\x42\x90\x8f\x11\x83\x7c\x6c\x38\xe0\x63\xc3\x1\x1f\x1b\xe\xf8\x8c\x18\x1c\x0\x8\x82\x81\x18\xe8\x3\x2d\x98\xc1\x3b\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\x82\xd\x90\x7c\x6c\x88\xe4\x63\x83\x24\x1f\x53\x6\xf8\x98\x32\xc0\xc7\x94\x1\x3e\x36\xf0\x81\x7c\x6c\xe8\x3\xf9\xd8\xe0\x7\xf2\x29\xae\x1f\xb0\x38\x7f\xc0\xd1\x4\x35\x0\x2a\x10\xa4\x2\x2\x66\x9\x82\x81\x8a\x1\x2\xd6\xc\x1a\xa8\x18\x20\x60\xcd\xa0\x81\x8a\x1\x2\xd6\xc\x1a\xa8\x20\x5c\x4\x58\x33\xa8\x82\x92\xb8\x11\x3\x5\x0\x41\x30\xb0\x6c\x82\x1c\xde\xc0\x8\xea\xa1\x24\x46\x13\x2\x60\xc4\xe0\x0\x40\x10\xc\xa6\x9c\xb8\x85\xe0\x1e\x86\x1b\x2\x96\x0\x83\x59\x6\x48\x8\x66\x9\x86\x81\x8a\x21\xc6\x1c\x49\x18\xa8\x18\x62\xcc\x91\x84\x81\x8a\x21\xc6\x1c\x49\x18\xa8\x20\x16\x47\x11\x46\xc\xc\x0\x4\xc1\x0\xeb\x89\x74\x8\x2a\xe8\x5\x2d\x9b\x8\xae\x2\x5f\xe0\x2a\x2\xad\x27\x26\xae\x8c\x0\x2a\xa8\x89\xab\x80\x1f\x60\xc4\xe0\x0\x40\x10\xc\xca\x20\x2c\xf8\xc1\x15\x82\xd1\x84\x60\x18\x26\x21\x2\x87\x22\x22\xe6\x6a\x8a\x80\x7\x18\x31\x38\x0\x10\x4\x83\x32\x20\x8b\x7f\x88\x85\x60\x34\x21\x8\x86\x1b\x82\x9f\x0\x83\x59\x86\xe2\x8\x46\x13\x86\xa1\x2\xb2\xd0\xe1\x86\x40\x2c\xc0\x60\x96\x61\x31\x82\x11\x83\x3\x0\x41\x30\x10\x83\xb5\x28\x89\x5b\x38\x89\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\x18\x31\x38\x0\x10\x4\x3\x31\x78\x8b\x94\xd8\x5\x7e\x18\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x84\x11\x83\x3\x0\x41\x30\x10\x83\xb9\x68\x89\x5f\x8\x89\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\x18\x31\x38\x0\x10\x4\x3\x31\xb8\x8b\x98\x18\x7\x9a\x18\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x4\x7b\xc2\x41\x3e\x23\x6\x8\x0\x82\x60\x70\x6\x7a\x51\x13\x4c\x38\x4\x23\x6\x8\x0\x82\x60\x70\x6\x7b\x61\x13\x49\x38\x4\x16\x91\x83\x7c\x46\xc\x10\x0\x4\xc1\xe0\xc\xfa\x2\x27\x1c\x72\x8\x46\xc\x10\x0\x4\xc1\xe0\xc\xfc\x22\x27\x16\x72\x8\x6c\x3a\x7\xf9\x8c\x18\x20\x0\x8\x82\xc1\x19\x80\xc6\x4e\x40\xe7\x10\x8c\x18\x20\x0\x8\x82\xc1\x19\x84\x6\x4f\x34\xe7\x10\x18\x83\xf\xf0\xb1\x0\x81\x8f\x35\xfa\x0\x1f\xb\xe\xf8\x98\xc3\xf\xf0\xb1\xc0\x80\xcf\x88\x81\x1\x80\x20\x18\x3c\xb2\x41\x1a\xc5\x88\x81\x1\x80\x20\x18\x3c\xb3\x51\x1a\xc4\x88\x81\x1\x80\x20\x18\x3c\xb4\x61\x1a\x83\xd\x21\x1\x1f\x1b\x44\x2\x3e\x36\x8c\x4\x7c\x46\xc\xe\x0\x4\xc1\xa0\x93\x8d\xb3\x18\x84\x11\x83\x3\x0\x41\x30\xe8\x66\x3\x2d\x2\x61\x38\x22\x38\x89\xe2\x9b\x6e\x40\x8d\xd4\x8\x46\xc\xe\x0\x4\xc1\x40\xc\x68\xc3\x2d\x40\xa2\x35\x46\x13\x2\x60\x34\x41\x8\x46\x13\x6\x61\xc4\xe0\x0\x40\x10\xc\xc4\x0\x37\xe4\x82\x24\xc6\x62\x34\x21\x0\x46\x13\x84\x60\x34\x61\x10\x46\xc\xe\x0\x4\xc1\x40\xc\x78\xc3\x2e\x50\x42\x2f\x46\x13\x2\x60\x34\x41\x8\x46\x13\x6\x61\xc4\xe0\x0\x40\x10\xc\xc4\x0\x3c\xf4\x82\x25\xfe\x62\x34\x21\x0\x46\x13\x84\x60\x34\x61\x10\xec\x51\x9\xf9\x8c\x18\x20\x0\x8\x82\xc1\x19\x8c\x87\x5f\x30\x2a\x11\x8c\x18\x20\x0\x8\x82\xc1\x19\x90\xc7\x5f\x24\x2a\x11\x58\xd4\x12\xf2\x19\x31\x40\x0\x10\x4\x83\x33\x30\x8f\xd0\x70\x5a\x22\x18\x31\x40\x0\x10\x4\x83\x33\x38\xf\xd1\x58\x5a\x22\xb0\x9\x26\xe4\x33\x62\x80\x0\x20\x8\x6\x67\x90\x1e\xa4\x1\xc1\x44\x30\x62\x80\x0\x20\x8\x6\x67\xa0\x1e\xa5\xd1\xc0\x44\x60\x4c\x58\xc0\xc7\x2\x4\x3e\xd6\x8c\x5\x7c\x2c\x38\xe0\x63\x4e\x59\xc0\xc7\x2\x3\x3e\x23\x6\x6\x0\x82\x60\xf0\xec\x47\x7b\x14\x23\x6\x6\x0\x82\x60\xf0\xf0\x87\x7b\x10\x23\x6\x6\x0\x82\x60\xf0\xf4\xc7\x7b\xc\x36\xa8\x5\x7c\x6c\x58\xb\xf8\xd8\xc0\x16\xf0\x19\x31\x38\x0\x10\x4\x83\x6e\x3f\x60\x63\x10\x46\xc\xe\x0\x4\xc1\xa0\xe3\x8f\xd8\x8\x84\xe1\x88\x0\x2e\x8a\x6f\xba\xe1\x3e\xd2\x20\x18\x31\x38\x0\x10\x4\x3\x31\xe8\x8f\xdb\x48\x8b\xf9\x18\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x84\x11\x83\x3\x0\x41\x30\x10\x83\x10\xd9\x8d\xb6\xa8\x8f\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\x18\x31\x38\x0\x10\x4\x3\x31\x28\x91\xdf\x88\xb\xfc\x18\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x84\x11\x83\x3\x0\x41\x30\x10\x83\x14\x19\x8f\xba\x18\x91\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\xb0\x67\x2e\xe4\x33\x62\x80\x0\x20\x8\x6\x67\xc0\x22\xe7\xc1\xcc\x45\x30\x62\x80\x0\x20\x8\x6\x67\xd0\x22\xe8\x91\xcc\x45\x60\x91\x5d\xc8\x67\xc4\x0\x1\x40\x10\xc\xce\xe0\x45\xd4\xc3\xb1\x8b\x60\xc4\x0\x1\x40\x10\xc\xce\x0\x46\xd6\x63\xb1\x8b\xc0\xa6\xbc\x90\xcf\x88\x1\x2\x80\x20\x18\x9c\x81\x8c\xb4\x7\x94\x17\xc1\x88\x1\x2\x80\x20\x18\x9c\xc1\x8c\xb8\x47\x93\x17\x81\x31\xaa\x1\x1f\xb\x10\xf8\x58\xc3\x1a\xf0\xb1\xe0\x80\x8f\x39\xae\x1\x1f\xb\xc\xf8\x8c\x18\x18\x0\x8\x82\xc1\x43\x26\x36\x52\x8c\x18\x18\x0\x8\x82\xc1\x53\x26\x37\x42\x8c\x18\x18\x0\x8\x82\xc1\x63\x26\x38\x32\xd8\x30\x1b\xf0\xb1\x81\x36\xe0\x63\x43\x6d\xc0\x67\xc4\xe0\x0\x40\x10\xc\x3a\x32\xc9\x8f\x41\x18\x31\x38\x0\x10\x4\x83\xae\x4c\xf4\x23\x10\x86\x23\x82\xdc\x28\xbe\xe9\x86\x1e\x49\x83\xc0\x82\x1d\xb9\x41\x5\x3d\xb2\x23\x6\x7\x0\x82\x60\x20\x6\x68\x22\x22\xb4\x11\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\x82\x15\x67\x2\x83\x11\x83\x3\x0\x41\x30\x10\x83\x36\x39\x91\xdc\x8\x46\x13\x2\x60\x34\x41\x8\x46\x13\x6\xc1\x14\x33\x81\xc1\x88\xc1\x1\x80\x20\x18\x88\x81\x9c\xb0\x88\x6f\x4\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x60\xf\x9b\xc0\x60\xc4\xe0\x0\x40\x10\xc\xc4\xe0\x4e\x62\x64\x3c\x82\xd1\x84\x0\x18\x4d\x10\x82\xd1\x84\x41\x30\x29\x3c\xe4\x33\x62\x80\x0\x20\x8\x6\x67\xa0\x27\x35\xe2\x84\x47\x30\x62\x80\x0\x20\x8\x6\x67\xb0\x27\x36\xa2\x84\x47\x60\x81\x1\x1d\xab\xca\x43\x3e\x23\x6\x8\x0\x82\x60\x70\x6\x7e\x92\x23\x51\x79\x4\x23\x6\x8\x0\x82\x60\x70\x6\x7f\xa2\x23\x4d\x79\x4\x16\x24\xd0\x31\x2c\x3d\xe4\x33\x62\x80\x0\x20\x8\x6\x67\x20\x2a\x3d\x42\xa5\x47\x30\x62\x80\x0\x20\x8\x6\x67\x30\x2a\x3e\x2\xa5\x47\x30\x62\x70\x0\x20\x8\x6\x62\x60\x2a\x60\x22\x1f\x66\x32\x9a\x10\x8\x23\x6\x7\x0\x82\x60\x50\x6\xa7\x22\x26\xf4\x1\x2a\xa3\x9\xc1\x30\x62\x80\x0\x20\x8\x6\xcd\xac\xf0\x48\x10\x27\x71\x32\x62\x70\x0\x20\x8\x6\x6a\x80\x2a\x3f\x12\xe8\x87\x49\x28\x2\x1f\xb\x10\xf8\xd8\xb1\x22\xf2\x31\xa4\x45\xe4\x63\xc9\x8b\xc8\xc7\x94\x14\x91\xcf\xb5\xc1\x50\x23\x6\xf\x0\x82\x60\xf0\xd1\x4a\x9a\x20\xf5\x51\x45\x41\x8a\xac\xca\xaa\xac\x89\x31\x9a\x10\x0\x16\xd4\x88\x7c\xc\x3b\xe2\x63\x49\x5\x9f\x11\x83\x7\x0\x41\x30\xf8\x72\xc5\x4d\x1a\xfd\x10\x2\xc3\x45\x60\x5\x56\xe0\x64\x19\x4d\x8\x0\x2b\x2\xf8\x58\xd7\xc4\xc7\xb4\x26\x3e\x23\x6\xf\x0\x82\x60\xf0\xf9\xca\x9c\x48\xff\x21\x4\xcb\x8c\xd4\x4a\xad\xd4\x9\x34\x9a\x10\x0\x56\x4\xf0\xb1\x48\xc\xe0\x63\xdf\x14\x9f\x11\x83\x7\x0\x41\x30\xf8\xc6\x5\x4f\x2e\x12\x11\x2\x8\x47\x74\x45\x57\xf4\xa4\x1a\x4d\x8\x0\x2b\x2\xf8\xd8\x75\x6\xf0\xb1\x8b\xc\xe0\x33\x62\xf0\x0\x20\x8\x6\x1f\xba\xf4\x9\x97\x22\x42\x50\xf5\xc8\xaf\xfc\xca\x9f\x68\xa3\x9\x1\x60\x45\x0\x1f\x63\x83\x2c\x3e\xb6\xa5\x1\x7c\x46\xc\x1e\x0\x4\xc1\xe0\x6b\x17\x51\x9\x3\x17\x11\x2\x4d\x4c\xc8\x85\x5c\x48\xe5\x1b\x4d\x8\x0\x2b\x2\xf8\x58\x1c\x7c\xf1\x31\x37\xf8\xe2\x33\x62\xf0\x0\x20\x8\x6\x9f\xbc\x9c\x8a\x19\xcc\x88\x10\x7c\x67\x92\x2e\xe9\x92\x2a\x64\x30\x9a\x10\x0\x56\x4\xf0\xb1\x31\xb0\x3\xf8\xd8\x1c\x94\x41\x7c\x46\xc\x1e\x0\x4\xc1\xe0\xbb\x17\x56\x59\x3\x1c\x11\x2\x32\x60\x13\x77\x71\x17\x57\x49\x83\xd1\x84\x0\xb0\x22\x80\x8f\xa5\xc1\x1e\xc0\xc7\xd2\x0\xf\xe0\x33\x62\xf0\x0\x20\x8\x6\x1f\xbf\xc4\xa\x1c\xf4\x88\x10\xa4\x41\x9c\xcc\xcb\xbc\xcc\x8a\x1b\x8c\x26\x4\x80\x15\x1\x7c\x2c\xf8\x13\xf9\xcc\x12\x2c\xa3\x9\xba\x21\x8c\x18\x28\x0\x8\x82\xc1\x36\x32\xb3\xb2\x23\x1\xbe\x94\xb\xbd\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x68\x2\x31\x8c\x18\x28\x0\x8\x82\xc1\x86\x32\xb8\x2\x26\xc6\xb9\xa8\x4b\xbe\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x68\x2\x31\x8c\x18\x28\x0\x8\x82\xc1\xd6\x32\xbd\x52\x26\x8b\xae\xbc\x8b\xbf\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x68\x2\x31\x8c\x18\x28\x0\x8\x82\xc1\x26\x33\xe2\xa2\x26\x90\xbc\xd4\xcb\xc8\x8c\x26\x4\xc0\x68\x82\x10\x58\x84\x27\xf2\x19\x31\x40\x0\x10\x4\x83\x33\x88\x19\x76\xc1\x93\x28\x18\x31\x40\x0\x10\x4\x83\x33\x90\x99\x76\xc1\x93\x28\xb0\x20\x82\x8e\x41\x7c\x22\x9f\x11\x3\x4\x0\x41\x30\x38\x83\x9a\x81\x17\x3e\x81\x82\x11\x3\x4\x0\x41\x30\x38\x3\x9b\x89\x17\x3e\x81\x2\xb\x20\xe8\xd8\x3\x2a\xf2\x19\x31\x40\x0\x10\x4\x83\x33\xc8\x19\x7a\x1\x95\x27\x18\x31\x40\x0\x10\x4\x83\x33\xd0\x99\x7a\x1\x95\x27\xb0\xe0\x81\xce\x30\x9\x81\x1f\x8b\xa5\x44\xc8\x95\x8c\x26\x3c\xc2\x88\x81\x1\x80\x20\x18\x3c\x67\x93\x33\xca\x88\x81\x1\x80\x20\x18\x3c\x68\xa3\x33\xc7\x88\x81\x1\x80\x20\x18\x3c\x69\xb3\x33\x84\xd\x52\x7c\x6c\x90\xe2\x63\x83\x11\x9f\x11\x83\x3\x0\x41\x30\xe8\xce\x86\x5f\x6\x5d\x19\x31\x38\x0\x10\x4\x83\xe\x6d\xfa\x65\xd8\x95\x11\x83\x3\x0\x41\x30\xe8\xd2\xc6\x5f\x6\x5e\x19\x31\x58\x0\x10\x4\x83\xe8\x6d\xfa\x65\x10\x82\x41\x8\x2c\x10\x17\xf9\x8c\x18\x18\x0\x8\x82\xc1\x23\x37\x63\x13\x98\xb8\x4\xf1\x99\x25\x58\xec\xba\xe4\x63\x14\x25\x1f\xb\x4\xf8\x98\x24\xc9\xc7\x84\x0\x3e\x23\x6\x6\x0\x82\x60\xf0\xe4\x8d\xc8\x4\x16\x88\x41\x7c\x46\xc\xe\x0\x4\xc1\xa0\xab\x1b\x95\x9\xd0\xc5\x82\x40\x3e\x16\xb8\x8b\x7c\x46\xc\xc\x0\x4\xc1\xe0\xf1\x9b\xb7\x9\xcc\x5d\x82\xf8\xcc\x12\x2c\x23\x6\x6\x0\x82\x60\xf0\x80\x8e\xdc\x7c\x16\xa0\x41\x7c\x46\xc\xe\x0\x4\xc1\xa0\xe3\x9b\x98\x9\xde\xc5\xcc\xc0\xc\xe4\x63\xdf\x27\x1f\xb\x4\xf8\x8c\x18\x18\x0\x8\x82\xc1\x63\x3a\x2f\x13\x58\xf0\x6\xf1\x19\x31\x38\x0\x10\x4\x83\x4e\x74\x6e\x26\xa8\x97\x11\x83\x3\x0\x41\x30\xe8\x46\x7\x67\x8e\xc0\x82\x40\x3e\x16\xf0\x8b\x7c\x46\xc\xc\x0\x4\xc1\xe0\x61\x9d\xbe\x9\x8c\x5f\x82\xf8\xcc\x12\x2c\x3\x35\x3\x4c\x18\xfa\x55\x8\x8a\x97\xd8\x1\xa2\x5f\xc7\x88\xc1\x1\x80\x20\x18\x88\xc1\xe9\x84\xcd\xbc\x98\xc9\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\xcc\x12\x38\xb5\x26\x35\x3\x23\x6\x7\x0\x82\x60\x20\x6\xac\x63\x36\xf8\x12\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\x82\xd\xf6\x12\x1f\x1b\xec\x25\x3e\x36\xd8\x4b\x7c\x46\xc\x16\x0\x4\xc1\x20\xd2\x1d\xb4\x19\x84\x60\x10\x82\x11\x3\x3\x0\x41\x30\x78\x78\x67\x6c\x2\x2b\x2\xf9\x58\x21\xc8\xc7\x8a\x41\x3e\x86\x20\xf2\x31\x4\x91\x8f\x9\x1\x7c\x2c\x49\xe4\x63\x42\x0\x9f\x11\x3\x3\x0\x41\x30\x78\xca\xc7\x6d\x2\xb\x2\xf9\x8c\x26\x4c\x83\x5\x81\x7c\x6c\x8\xe8\x63\x39\x13\xc4\x67\xc4\xc0\x0\x40\x10\xc\x9e\xf5\xe1\x9d\x60\x96\xc0\x29\x52\x71\x1b\x18\x31\x38\x0\x10\x4\x3\x31\x28\x9f\xbf\x89\x99\x60\x34\x21\x0\x46\x13\x84\x60\x34\x61\x10\x6c\x78\x99\xf8\xd8\xf0\x32\xf1\xb1\xe1\x65\xe2\x33\x62\xb0\x0\x20\x8\x6\xd1\xfc\x84\xce\x20\x4\x83\x10\x8c\x18\x18\x0\x8\x82\xc1\x53\x3f\x7c\x13\x58\x11\xc8\xc7\xa\x41\x3e\x56\xc\xf2\x31\x4\x91\x8f\x21\x88\x7c\x4c\x8\xe0\x63\x49\x22\x1f\x13\x2\xf8\x8c\x18\x18\x0\x8\x82\xc1\xe3\x3f\xa7\x13\x58\x10\xc8\x67\x34\x61\x1a\x2c\x8\xe4\x63\x43\x40\x1f\x93\x9b\x20\x3e\x23\x6\x6\x0\x82\x60\xf0\x90\x50\xfd\x4\x23\x6\x7\x0\x82\x60\x20\x6\xfd\x73\x3b\x69\xc3\x2b\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x30\x62\xb0\x0\x20\x8\x6\x11\xa\xd9\xce\x24\x45\x83\x10\x8c\x18\x18\x0\x8\x82\xc1\xa3\x42\xfb\x13\xd4\xb8\xb0\xe\x8c\x18\x1c\x0\x8\x82\x81\x18\x90\x90\xef\xc0\x4d\x30\x9a\x10\xc\x46\x4\xf1\xa9\x73\x99\x1d\x18\x31\x38\x0\x10\x4\x3\x31\x40\x21\xf1\xa1\x9b\x60\x34\x21\x0\x8c\x8\xe4\x33\x62\x60\x0\x20\x8\x6\xcf\xc\x91\x50\x60\x1\x24\x9f\x59\x2\x67\xa0\x64\x20\xb9\x41\x68\x68\x81\x71\x89\x65\xa0\x64\x30\xb9\x1\x14\x1a\x79\x60\x58\x62\x19\x28\x19\x50\x6e\x0\x85\x46\x1e\x18\x96\x58\x6\x4a\x6\x95\x1b\x40\xa1\x91\x7\x86\x25\x96\x11\x83\x5\x0\x41\x30\x88\x70\xc8\x7c\xd2\x6\x6d\xce\x66\x10\x82\x11\x3\x3\x0\x41\x30\x78\x74\x68\x85\x82\x9a\x17\xde\x81\x11\x83\x3\x0\x41\x30\x10\x3\x1a\x72\x1f\xd0\x9\x46\x13\x2\x60\x34\x41\x8\x46\x13\x6\xc1\x16\xd8\x91\x8f\x5\x87\x7c\x2c\x28\xe4\x63\x42\x21\x1f\x1b\xa\xf9\xd8\x80\x32\xf0\xb1\x1\x65\xe0\x63\x3\xca\xc0\x67\xb8\x61\x64\x70\x8\xc\x66\x19\x9e\x21\x98\x25\x80\x6\x2a\x6\x35\xb\x90\x67\xa0\x62\x50\xb3\x0\x79\x6\x2a\x6\x35\xb\x90\xc7\x6a\xe6\x87\x60\x30\xdc\x10\xc0\xf\x18\xcc\x32\x44\x41\x30\x62\x90\x0\x20\x8\x6\x8\x1c\xe5\x90\xf\xf9\x90\xd\x15\x23\x6\x9\x0\x82\x60\x80\xc0\x51\xe\xf9\x90\xf\xa9\x10\x31\x62\x90\x0\x20\x8\x6\x8\x1c\xe5\x90\xf\xf9\x50\xd\xd\x23\x6\x9\x0\x82\x60\x80\xc0\x51\xe\xf9\x90\xf\xd1\x50\xef\x20\x0\x0\x0\x0\x0\x0\x0";

}