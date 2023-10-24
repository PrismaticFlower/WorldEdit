#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char thumbnail_mesh_alpha_cutoutPS_dxil_bytes[4929];

auto thumbnail_mesh_alpha_cutoutPS() noexcept -> shader_def
{
   return {
      .name = "thumbnail_mesh_alpha_cutoutPS",
      .entrypoint = L"main",
      .target = L"ps_6_6",
      .file = L"thumbnail_mesh_alpha_cutoutPS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(thumbnail_mesh_alpha_cutoutPS_dxil_bytes),
               sizeof(thumbnail_mesh_alpha_cutoutPS_dxil_bytes) - 1},
   };
}

const char thumbnail_mesh_alpha_cutoutPS_dxil_bytes[4929] = "\x44\x58\x42\x43\x51\xe0\x8f\xd3\xc6\x7c\xe0\x3f\x9d\x1f\x2a\x17\x80\x54\x55\x8a\x1\x0\x0\x0\x40\x13\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x7c\x1\x0\x0\xb8\x1\x0\x0\x60\x3\x0\x0\x9c\x3\x0\x0\xb8\x3\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x6\x0\x0\x0\x0\x49\x53\x47\x31\x28\x1\x0\x0\x7\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\xe8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xf1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xf8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x4\x0\x0\x0\x3\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x13\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x5\x0\x0\x0\xf\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x19\x1\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x6\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x4e\x4f\x52\x4d\x41\x4c\x0\x54\x41\x4e\x47\x45\x4e\x54\x0\x42\x49\x54\x41\x4e\x47\x45\x4e\x54\x0\x54\x45\x58\x43\x4f\x4f\x52\x44\x0\x43\x4f\x4c\x4f\x52\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x0\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\xa0\x1\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x7\x1\x0\x7\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x34\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x4e\x4f\x52\x4d\x41\x4c\x0\x54\x41\x4e\x47\x45\x4e\x54\x0\x42\x49\x54\x41\x4e\x47\x45\x4e\x54\x0\x54\x45\x58\x43\x4f\x4f\x52\x44\x0\x43\x4f\x4c\x4f\x52\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x2\x0\x0\xa\x0\x0\x0\x0\x0\x0\x0\x1\x1\x43\x0\x3\x2\x0\x0\x11\x0\x0\x0\x0\x0\x0\x0\x1\x2\x43\x0\x3\x2\x0\x0\x19\x0\x0\x0\x0\x0\x0\x0\x1\x3\x43\x0\x3\x2\x0\x0\x23\x0\x0\x0\x0\x0\x0\x0\x1\x4\x42\x0\x3\x2\x0\x0\x2c\x0\x0\x0\x0\x0\x0\x0\x1\x5\x44\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x6\x44\x3\x3\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x4\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x34\x0\x0\x0\x0\x0\x2f\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x74\x68\x75\x6d\x62\x6e\x61\x69\x6c\x5f\x6d\x65\x73\x68\x5f\x61\x6c\x70\x68\x61\x5f\x63\x75\x74\x6f\x75\x74\x50\x53\x2e\x70\x64\x62\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x7e\x8b\x6e\x2\x21\xf5\x51\x5\xd3\x33\xe8\x25\xaf\x1c\x2f\x84\x44\x58\x49\x4c\x80\xf\x0\x0\x66\x0\x0\x0\xe0\x3\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x68\xf\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\xd7\x3\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x54\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\xc8\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x80\x20\x88\x62\x20\x64\x8e\x20\x28\x3\x40\x28\xb4\x50\x71\xcf\x70\xf9\x13\xf6\x10\x92\x1f\x2\xcd\xb0\x10\x28\x68\x4a\xe1\x10\x84\xa2\xd0\x73\xd3\x70\xf9\x13\xf6\x10\x92\xbf\x12\xd2\x4a\x4c\x7e\x71\xdb\xa8\x18\x86\x61\x20\xa\x13\x11\x8e\x33\xc\xc3\x40\x10\xc4\x40\x52\x41\x6\x62\x18\x86\x61\x18\x88\x2a\xc3\x40\xc\x64\x1d\x35\x5c\xfe\x84\x3d\x84\xe4\x73\x1b\x55\xac\xc4\xe4\x23\xb7\x8d\x8\x82\x20\x88\x42\x60\x84\x43\x50\x76\xd4\x70\xf9\x13\xf6\x10\x92\xcf\x6d\x54\xb1\x12\x93\x5f\xdc\x36\x22\x86\x61\x18\xa\xb1\x11\xe\x41\x5c\x29\x6\x62\x18\x6\xf2\x6e\x1b\x2e\x7f\xc2\x1e\x42\xf2\x57\x42\x72\xa8\x48\x20\xd2\xc8\x79\x88\x68\x42\x8\x9\x9\x4\x51\x8\x87\x70\x40\xa\xf\x1a\x2e\x7f\xc2\x1e\x42\xf2\x57\x42\xda\x90\x66\x40\x4\x41\x10\x45\x29\x1c\x62\x24\x14\x22\x7\x2\x86\x11\x86\x61\x18\x41\x18\xae\x92\xa6\x88\x12\x26\x9f\x1\x26\x22\x42\x80\xe5\x47\x9a\x87\x9a\x50\x42\x10\x4\x71\x12\x28\x7a\x1e\xc2\x0\x13\x11\x21\xc0\x82\x82\x94\xd4\xbb\xa4\x29\xa2\x84\xc9\x17\x0\x86\x88\x80\x5f\x70\x1a\x69\x2\x9a\x49\x42\xc1\x79\x89\x10\xfc\x2\xc0\x10\x11\x80\x2\x96\xda\xf4\x20\xe6\x8\x40\x61\xa\x0\x0\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x2c\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x79\x80\x0\x8\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x23\x1\x1\x30\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x21\x8f\x5\x4\x40\x0\x0\x0\x0\x0\x0\x0\x0\x43\x9e\xc\x8\x80\x1\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x1c\x10\x0\x3\x0\x0\x0\x0\x0\x0\x0\xc\x79\x3c\x20\x0\x2\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x84\x1\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\xc8\x0\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\xb2\x40\x16\x0\x0\x0\x32\x1e\x98\x18\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\xc\xa\xaf\x20\xa\xa3\x8\xa\xa1\x14\xca\x83\x8a\x92\x18\x1\x28\x82\x32\x28\x84\x2\x21\xb8\x0\x1\x1\x1\x31\xc8\x9d\x1\xa0\x77\x2c\x8f\x23\x0\x0\x0\x8e\xe3\x0\x8e\xe3\x0\x8e\xe3\x0\x9e\x7\x0\x2\x81\x78\x0\x0\x0\x68\x9d\x1\x0\x0\x0\x0\x79\x18\x0\x0\x7d\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x88\x66\x82\x40\x38\x1b\x84\x81\x98\x20\x10\xcf\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\x10\xd0\x4\x61\xd\xc4\x80\xc0\x4\x81\x88\x26\x8\x84\xb4\x1\x51\x16\x46\x69\x6\x7\x98\x20\xb8\x1\x18\x4c\x10\x88\x69\x82\x40\x50\x1b\x90\x1\x62\x94\x68\x90\x80\xd\xc2\x33\x6d\x20\x0\x80\x2\x26\x8\x6f\x10\x6\x1b\x2\x6b\x82\x20\x0\x24\xda\xc2\xd2\xdc\x88\x40\x3d\x4d\x25\x51\x25\x3d\x39\x4d\x10\x8a\x6c\x82\x50\x68\x1b\x2\x65\x82\x50\x6c\x13\x84\x82\xdb\xb0\x28\xda\xc6\x75\xde\xf0\x29\x1c\x40\xc3\xe9\x49\xaa\x29\x88\x69\x83\xd0\x18\x1b\x96\x21\xc\x36\xae\xf3\x86\x6f\xe0\xc4\x60\x82\x40\x54\x3c\xa8\x82\x9c\x8e\x8a\x9c\xa8\x36\x2c\x64\x50\x6\x1b\xd7\x79\xc3\x47\x6\x9c\x18\x30\x11\x4a\xa2\xa\x72\x3a\x2a\x72\xa2\xda\xb0\x34\x67\xb0\x71\x9d\x37\x7c\xd\x27\x6\x13\x4\xc2\x22\x42\x55\x84\x35\xf4\xf4\x24\x45\xb4\x41\x68\x9a\xd\x4b\x1a\xa8\xc1\xc6\x75\xde\xe0\xa5\x1\xb7\x6\x13\x4\xe2\x62\x31\xf4\xc4\xf4\x24\x35\x41\x28\xba\x9\x2\x81\x6d\x10\x1a\x38\xd8\xb0\xb4\x81\x1b\x6c\x5c\xe7\xd\x6f\xd0\x6\x5c\x1c\x70\x99\xb2\xfa\x82\x7a\x9b\x4b\xa3\x4b\x7b\x73\xdb\xb0\x10\x73\xb0\x7d\xdd\x1b\xc\x6f\x40\x70\xc0\x86\x3\xc\xc6\xc0\xc\xd0\x80\xd\xe4\x80\xe\x98\x4c\x59\x7d\x51\x85\xc9\x9d\x95\xd1\x4d\x10\xa\x6f\xc3\xa2\xd8\xc1\x76\x7\x1d\x37\xbc\x81\xc2\xc5\xc1\x86\x0\xf\x36\xc\x75\x90\x7\xc0\x4\x1\xe\xbe\xd\x82\xb2\x7\x1b\xa\x2c\xd3\x83\x8a\xf\xaa\xb0\xb1\xd9\xb5\xb9\xa4\x91\x95\xb9\xd1\x4d\x9\x82\x2a\x64\x78\x2e\x76\x65\x72\x73\x69\x6f\x6e\x53\x2\xa2\x9\x19\x9e\x8b\x5d\x18\x9b\x5d\x99\xdc\x94\xc0\xa8\x43\x86\xe7\x32\x87\x16\x46\x56\x26\xd7\xf4\x46\x56\xc6\x36\x25\x40\xca\x90\xe1\xb9\xc8\x95\xcd\xbd\xd5\xc9\x8d\x95\xcd\x4d\x9\xa8\x3a\x64\x78\x2e\x76\x69\x65\x77\x49\x64\x53\x74\x61\x74\x65\x53\x2\xab\xe\x19\x9e\x4b\x99\x1b\x9d\x5c\x1e\xd4\x5b\x9a\x1b\xdd\xdc\x94\x80\xf\x0\x0\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\x37\x0\x0\x0\xb6\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x58\xc2\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x21\x6c\xc3\xe5\x3b\x8f\x2f\x4\x54\x51\x10\x51\xe9\x0\x43\x49\x18\x80\x80\xf9\xc8\x6d\x1b\x43\x37\x5c\xbe\xf3\xf8\x42\x44\x0\x13\x11\x2\xcd\xb0\x10\x5f\xe4\x30\x1b\xd2\xc\x48\x63\x18\x81\x36\x5c\xbe\xf3\xf8\x42\x44\x0\x13\x11\x2\xcd\xb0\x10\x5f\xe4\x30\x21\x1\x3c\x36\xd0\xc\x97\xef\x3c\xfe\x80\x48\x2\x10\xd\x66\xe0\xc\x97\xef\x3c\xfe\xe0\x4c\xb7\x5f\xdc\xb6\x5\x4c\xc3\xe5\x3b\x8f\xbf\x38\xc0\x20\x36\xf\x35\xf9\xc5\x6d\x5b\x1\x34\x5c\xbe\xf3\xf8\x12\xc0\x3c\xb\xe1\x17\xb7\x6d\x2\xd5\x70\xf9\xce\xe3\x4b\x93\x13\x11\x28\x35\x3d\xd4\xe4\x17\xb7\x6d\xa\xd2\x70\xf9\xce\xe3\x4f\x44\x34\x21\x40\x84\xf9\xc5\x6d\xdb\xc1\x33\x5c\xbe\xf3\xf8\x54\x3\x44\x98\x5f\xdc\xb6\x1\x10\xc\x80\x34\x0\x0\x0\x0\x61\x20\x0\x0\xe9\x1\x0\x0\x13\x4\x54\x2c\x10\x0\x0\x0\x27\x0\x0\x0\xa4\xd4\xc0\x8\x0\x15\x25\x40\x44\x1\xa\x94\x4a\xb9\x14\x48\x1\x6\x14\x69\x40\x1\x12\x14\xd0\xc\x40\xd9\x15\xec\x40\xb9\x15\x57\xd1\xe\x94\x4c\xc9\xe\x94\x46\x49\x32\x14\x47\x49\x42\xd0\x30\x46\x0\x82\x20\x88\x86\xc1\x8\xc0\x18\x1\x8\x82\x20\x8\xa\x63\x4\xad\x39\xe7\xbc\x37\x46\xa0\xb3\xe6\x1c\x7f\x63\x4\x20\x8\x82\xf8\x2f\x8c\x11\x80\x20\x8\xe2\xdf\x18\x1\x8\x82\x20\xfc\x8d\x11\xcc\x67\xa8\xce\xdf\x18\x1\x8\x82\x20\x8\x6\x33\x0\x4\xce\x21\x54\x79\x30\x87\x60\x7\xd4\x1c\x82\x1d\x48\x73\x8\x78\x0\xcd\x21\x54\x77\x30\x87\x70\xdd\x1\x8d\x73\x10\xd3\x4\x6d\x73\x10\x51\x4\x6d\x0\x0\x0\x23\x6\x8\x0\x82\x60\x40\x6\xa1\x80\x9\x7a\x90\x6\x23\x6\x8\x0\x82\x60\x40\x6\xa2\x90\x9\x7d\xa0\x6\x23\x6\x7\x0\x82\x60\x10\x6\xa4\xf0\x9\xc9\x88\xc1\x1\x80\x20\x18\x84\x41\x29\x80\x81\x90\x8c\x18\x24\x0\x8\x82\x81\x1\xb\x7b\xa0\x7\xa1\x70\x7\x63\x30\x62\x90\x0\x20\x8\x6\x46\x2c\xf0\xc1\x1e\x88\x42\x1b\x90\xc1\x88\x41\x2\x80\x20\x18\x18\xb2\xd0\x7\x7c\x30\xa\x78\x50\x6\x23\x6\x9\x0\x82\x60\x60\xcc\x82\x1f\xf4\x1\x29\xe0\x81\x19\x8c\x18\x24\x0\x8\x82\x81\x41\xb\x7f\xf0\x7\xa5\xb0\x7\x67\x30\x62\x90\x0\x20\x8\x6\x46\x2d\x80\x2\x28\x98\x42\x1c\xa0\xc1\x88\x41\x2\x80\x20\x18\x18\xb6\x10\xa\xa6\x70\xa\x7d\x90\x6\x23\x6\x9\x0\x82\x60\x60\xdc\x82\x28\x9c\x2\x2a\xcc\x81\x1a\x8c\x18\x24\x0\x8\x82\x81\x81\xb\xa3\x80\xa\xa9\xe0\x7\x6b\x30\x62\x90\x0\x20\x8\x6\x46\x2e\x90\x42\x29\xa8\x2\x28\xb0\xc1\x88\x41\x2\x80\x20\x18\x18\xba\x50\xa\xa6\xb0\xa\x76\xd0\x6\x23\x6\x9\x0\x82\x60\x60\xec\x82\x29\x9c\x2\x2b\x84\x82\x1b\x8c\x18\x24\x0\x8\x82\x81\xc1\xb\xa7\x0\xb\xad\x30\xa\x6f\x30\x62\x90\x0\x20\x8\x6\x46\x2f\xa0\x42\x2c\xb8\x42\x1e\xc0\xc1\x88\x41\x2\x80\x20\x18\x18\xbe\x90\xa\xb2\xf0\xa\xa4\x10\x7\x23\x6\x8\x0\x82\x60\xf0\xf0\x42\x1b\xac\x42\x1f\xf0\xc1\x88\xc1\x1\x80\x20\x18\x84\x81\x2d\xc4\x41\x90\x8d\x18\x1c\x0\x8\x82\x41\xa6\xb\x73\x20\xc9\xc2\x68\x42\x10\x8c\x18\x20\x0\x8\x82\xc1\x3\xe\x71\x10\x80\x2\x28\x8c\x18\x1c\x0\x8\x82\x41\x18\xe8\x42\x1d\x4\x60\x30\x9a\x40\x0\x15\xe8\x81\xe\x37\x4\xb8\x0\x6\xb3\xc\x44\x10\x8c\x26\x1c\xc2\x88\x1\x2\x80\x20\x18\x3c\xe6\x70\x7\x81\x29\x98\x42\x15\x7f\xa0\xc3\xd\x1\x2f\x80\xc1\x88\xc1\x1\x80\x20\x18\x84\x81\x38\xf4\xc1\x80\x6\xb3\xc\x83\x20\x8c\x18\x1c\x0\x8\x82\x1\x57\xe\x7f\xe0\xe5\xc2\x68\x42\x20\x8c\x26\x8\x83\x9\x9c\x7c\x4c\xe0\xe4\x33\x62\xf0\x0\x20\x8\x6\x92\x3b\x84\x83\x21\x9\xc1\x1b\xbc\xc1\x38\x8c\x43\x29\xbc\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\xc3\x2c\x1\x31\x62\xf0\x0\x20\x8\x6\xd2\x3c\x98\xc3\x72\x8d\x81\x18\xd0\x1\x1d\xa0\x3\x3a\xa8\x2\x1d\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x68\x2\x31\xcc\x12\x10\x3\x1d\x3\x3e\x0\x92\x80\xc\x3\x1d\x83\x3e\x0\x92\x80\xc\x3\x1d\x83\x3f\x0\x92\x80\xc\x3\x1d\x3\x48\x0\x92\x80\xc\x46\xf8\x81\x7c\x8c\xf8\x3\xf9\x18\x1\xa\xf2\xb1\xa1\x14\xe0\x63\x83\x29\xc0\xc7\x86\x53\x80\x8f\xd\x6d\x20\x9f\x11\x3\x4\x0\x41\x30\xf0\xfc\xc1\x16\x86\x38\x8\x46\xc\x10\x0\x4\xc1\xc0\xfb\x87\x5b\x18\xd8\x20\x30\xe3\xd\xe4\x33\x62\x80\x0\x20\x8\x6\x5e\x48\xe4\x82\x31\x7\xc1\x88\x1\x2\x80\x20\x18\x78\x22\xa1\xb\x86\x1b\x4\x96\xc4\x81\x7c\x46\xc\x10\x0\x4\xc1\xc0\x23\x9\x5e\x48\xea\x20\x18\x31\x40\x0\x10\x4\x3\xaf\x24\x7a\x21\x81\x83\x60\xc4\x60\x1\x40\x10\xc\x28\x95\xf8\x85\x83\x8\xe\x22\x18\x31\x30\x0\x10\x4\x3\x4b\x25\x7a\x21\xb0\x20\x91\x8f\x9\x87\x7c\x6c\x28\xe4\x33\x62\x70\x0\x20\x8\x6\x1c\x4b\x98\x83\x29\x94\xc4\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\xd8\x70\xc4\xc7\x86\x23\x3e\x36\x1c\xf1\x19\x31\x58\x0\x10\x4\x3\xca\x26\xd6\x61\x10\x82\x41\x8\x46\xc\xc\x0\x4\xc1\xc0\xb2\x89\x74\x8\xac\x8\xe4\x63\x85\x20\x1f\x2b\x6\xf9\x8c\x18\x3c\x0\x8\x82\x81\xc4\x13\x2f\xc1\x7\xa0\xc0\xa\xab\xd0\xb\xbd\x10\x13\x31\x31\xf\xbd\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xa3\x9\xc4\x30\x1c\x11\x94\x3\xf1\xcd\x32\x14\x46\x30\x62\x60\x0\x20\x8\x6\xb\x59\xf0\x3\x49\xcc\x12\x18\x23\x6\x7\x0\x82\x60\x90\xfd\x4\x3e\xdc\xc2\x4d\x8c\x26\x4\x40\x5\xff\xa0\xc3\xd\x81\x4e\x80\xc1\x2c\x3\x72\x4\xa3\x9\xc4\x30\x62\x80\x0\x20\x8\x6\xf\x5a\xe4\x43\x80\x12\x28\x31\x62\x70\x0\x20\x8\x6\x61\x20\x16\xfd\x10\x90\xc3\x88\xc1\x1\x80\x20\x18\x70\x65\xf1\xf\xbe\x90\x13\xa3\x9\x81\x30\x9a\x20\xc\x26\xf0\x82\x7c\x4c\xe0\x5\xf9\x8c\x18\x3c\x0\x8\x82\x81\xe4\x16\x61\x61\xc8\x82\x10\xbc\xc3\x3b\x8c\xc5\x58\x94\xc4\x3b\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\x82\x55\xf4\x20\x1f\xb\x8\xf9\x98\x65\xf\xf2\xb1\xa0\x90\x8f\x5d\xf8\x20\x1f\xb\xc\xf9\xcc\x12\x20\x3\x15\x83\x72\xd8\x81\x31\x50\x31\x20\x87\x1d\x18\x3\x15\x83\x71\xd8\x81\x51\xd8\x4d\xe8\x70\x43\x10\x17\x61\x30\xcb\xa0\x24\x81\x15\xf1\x20\x1f\x2b\xe2\x41\x3e\x56\xc4\x83\x7c\x66\x9\x94\x81\x8a\xc1\x48\x20\x64\xa0\x62\x30\x12\x8\x19\xa8\x18\x8c\x4\x42\x6c\xc\xe8\x41\x3e\x15\x6\x7c\xa1\xc3\xd\x81\x5e\x80\xc1\x2c\x3\xb3\x4\x66\xc\xf2\x31\x83\x90\x8f\x19\x85\x7c\x66\x9\x98\x81\x8a\xc1\x58\x24\x65\xa0\x62\x30\x16\x49\x19\xa8\x18\x8c\x45\x52\x2a\xd\xf8\x42\x87\x1b\x2\xd1\x0\x83\x59\x86\x66\xa\x6a\xd\xfa\x42\x87\x1b\x2\xd2\x8\x83\xe9\x86\xa6\x16\x82\x59\x6\xe7\xa9\xc\x29\x9\xf9\x18\x52\x12\xf2\x31\xa4\x24\xe4\x33\x4b\x0\x8d\x18\x1c\x0\x8\x82\x1\x7\x1b\x6a\x91\x12\xac\x31\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\x36\x20\xf2\xb1\x21\x91\x8f\xd\x8a\x7c\x46\xc\x16\x0\x4\xc1\x80\xd2\x8d\xb7\x70\x85\x56\x60\x85\xb2\xf0\x9\x9f\x18\x31\x30\x0\x10\x4\x3\x4b\x37\x64\x23\xb0\x80\x92\x8f\x9\x94\x7c\x6c\xa0\xe4\x63\xa6\x30\x16\xf0\x31\x53\x20\xb\xf8\x8c\x18\x2c\x0\x8\x82\x1\x15\x1e\x76\x91\xa\x42\x90\xa\x42\x30\x62\x60\x0\x20\x8\x6\x56\x78\xd0\x45\x60\xc1\x2a\xc8\xc7\x84\x42\x3e\x36\x14\xf2\x19\x31\x58\x0\x10\x4\x3\xca\x3c\xf6\x42\x17\x72\x1\x17\x6\x21\x18\x31\x30\x0\x10\x4\x3\xcb\x3c\x7c\x23\x18\x31\x30\x0\x10\x4\x3\xeb\x3c\x4c\x23\xb0\xa0\x2e\xe4\x33\x62\x60\x0\x20\x8\x6\x56\x7a\xa4\x46\x60\x54\x20\x1f\xa3\x4\xf9\x18\x35\xc8\xc7\x6\x9\x3e\x36\x48\xf0\xb1\x41\x82\x8f\xd\x79\x21\x1f\x1b\xf4\x42\x3e\x36\xec\x85\x7c\x8c\xd\xfa\x42\x3e\xc6\x6\x7e\x21\x1f\x63\x83\xbf\x90\x8f\x19\x3\x7c\xcc\x18\xe0\x63\xc6\x0\x9f\x59\x2\x68\xa0\x62\xa0\x5\xc7\x78\x6\x2a\x6\x5a\x70\x8c\x67\xa0\x62\xa0\x5\xc7\x78\x6a\x17\x7a\x43\x87\x1b\x2\xfa\x0\x83\x59\x6\x29\xa\x46\xc\xe\x0\x4\xc1\x20\xeb\xf\xdb\xa8\xb\xfc\x18\x4d\x8\x86\x11\x3\x4\x0\x41\x30\x78\x46\x84\x36\x82\xf1\x18\x8f\x11\x83\x3\x0\x41\x30\x8\x83\xfe\xc0\x8d\xc0\x2f\xac\x35\xd6\x21\x3e\xe6\x1a\xeb\x10\x1f\x7b\x8d\x75\x88\xcf\x88\xc1\x2\x80\x20\x18\x50\x26\xb2\x1b\x83\x10\xe8\x43\x3e\xe0\x83\x5\xaa\x21\x1f\xb\xf8\x41\x3e\x26\xf0\x83\x7c\x6c\xe0\x7\xf9\x18\x32\xc4\xc7\x90\x21\x3e\x86\xc\xf1\x19\x31\x58\x0\x10\x4\x3\xca\x45\xc6\x63\x10\x82\x41\x8\x46\xc\xc\x0\x4\xc1\xc0\x72\x91\xf0\x8\xac\x8\xe4\x63\x85\x20\x1f\x2b\x6\xf9\x8c\x18\x3c\x0\x8\x82\x81\x44\x23\x27\x12\xe1\xc5\x20\x4\xb5\xb1\x1e\xeb\xb1\x1e\xb5\x31\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\x23\x6\x7\x0\x82\x60\xc0\xd1\x88\x7b\xb4\x6\x8b\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\x82\x1d\xac\x20\x1f\xb\x8\xf9\x18\xe2\xa\xf2\xb1\xa0\x90\x8f\x25\xb0\x20\x1f\xb\xc\xf9\x58\x71\x6\xf0\x31\xe2\xc\xe0\x63\xc3\x19\xc0\x67\x96\x40\x1a\xa8\x18\x8c\x88\x16\xa0\x81\x8a\xc1\x88\x68\x1\x1a\xa8\x18\x8c\x88\x16\xa0\xe1\x6\x72\xe8\x91\x30\x98\x6e\x30\x7\xf7\x8\x66\x9\xa6\x81\x8a\x41\x91\x44\x83\x19\xa8\x18\x14\x49\x34\x98\x81\x8a\x41\x91\x44\x83\x19\xa8\x18\x10\x89\x36\x98\x11\x83\x4\x0\x41\x30\x40\xe2\x84\x47\xc6\x64\x4c\x72\x84\x18\x31\x48\x0\x10\x4\x3\x24\x4e\x78\x64\x4c\xc6\xc4\x45\x86\x11\x83\x4\x0\x41\x30\x40\xe2\x84\x47\xc6\x64\x4c\x70\x44\x18\x31\x48\x0\x10\x4\x3\x24\x4e\x78\x64\x4c\xc6\xe4\x46\x2\x4\x0\x0\x0\x0\x0";

}