#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char meta_draw_sphereVS_6_1_dxil_bytes[3073];
extern const char meta_draw_sphereVS_6_6_dxil_bytes[3229];

auto meta_draw_sphereVS() noexcept -> shader_def
{
   return {
      .name = "meta_draw_sphereVS",
      .entrypoint = L"main",
      .target_6_1 = L"vs_6_1",
      .target_6_6 = L"vs_6_6",
      .file = L"meta_draw_sphereVS.hlsl",
      .dxil_6_1 = {reinterpret_cast<const std::byte*>(meta_draw_sphereVS_6_1_dxil_bytes),
                      sizeof(meta_draw_sphereVS_6_1_dxil_bytes) - 1},
      .dxil_6_6 = {reinterpret_cast<const std::byte*>(meta_draw_sphereVS_6_6_dxil_bytes),
                      sizeof(meta_draw_sphereVS_6_6_dxil_bytes) - 1},
   };
}

const char meta_draw_sphereVS_6_1_dxil_bytes[3073] = "\x44\x58\x42\x43\x89\x98\x5c\x47\x76\xb9\x1f\x3\x2f\x2c\x51\x73\xd9\xe2\x6a\x14\x1\x0\x0\x0\x0\xc\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xb4\x0\x0\x0\x48\x1\x0\x0\x5c\x2\x0\x0\x94\x2\x0\x0\xb0\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x60\x0\x0\x0\x2\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x48\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x51\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\x1\x1\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x53\x56\x5f\x49\x6e\x73\x74\x61\x6e\x63\x65\x49\x44\x0\x0\x4f\x53\x47\x31\x8c\x0\x0\x0\x3\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x68\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x6e\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x7a\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x50\x53\x56\x30\xc\x1\x0\x0\x30\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x1\x0\x0\x0\x2\x3\x0\x2\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x5\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\xc\x0\x0\x0\x0\x0\x0\x0\x20\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x43\x4f\x4c\x4f\x52\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x41\x2\x1\x0\x0\x0\xa\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x3\x3\x4\x0\x0\x10\x0\x0\x0\x0\x0\x0\x0\x1\x2\x44\x0\x3\x2\x0\x0\xf0\xf\x0\x0\xf0\xf\x0\x0\xf0\xf\x0\x0\x0\x0\x0\x0\xff\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x30\x0\x0\x0\x0\x0\x28\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x74\x61\x5f\x64\x72\x61\x77\x5f\x73\x70\x68\x65\x72\x65\x56\x53\x5f\x36\x5f\x31\x2e\x70\x64\x62\x0\x0\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\xd7\x5b\x8a\xef\x3c\x16\xac\x17\x89\xbd\xc0\x6b\xe\xa1\x1e\x30\x44\x58\x49\x4c\x48\x9\x0\x0\x61\x0\x1\x0\x52\x2\x0\x0\x44\x58\x49\x4c\x1\x1\x0\x0\x10\x0\x0\x0\x30\x9\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x49\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x50\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\x98\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x82\x20\x88\x82\x20\xa4\x18\x0\x41\x10\xc5\x40\xca\x51\xc3\xe5\x4f\xd8\x43\x48\x3e\xb7\x51\xc5\x4a\x4c\x7e\x71\xdb\x88\x18\x86\x61\xa0\xe2\x9e\xe1\xf2\x27\xec\x21\x24\x3f\x4\x9a\x61\x21\x50\xd0\x14\x82\x21\x1c\x82\x9e\x52\xc\xc4\x30\xc\x14\xcd\x11\x4\xc5\x70\x88\x82\x20\x26\xa2\x6e\x1a\x2e\x7f\xc2\x1e\x42\xf2\x57\x42\x5a\x89\xc9\x2f\x6e\x1b\x15\xc3\x30\xc\x44\x29\x2c\xc2\x21\x8\xba\x6\x2\x86\x11\x86\x61\x18\x81\x18\xee\x92\xa6\x88\x12\x26\x9f\x21\x26\xe0\x1f\x22\xc0\xfa\xa5\xe7\x20\x22\x2\xd\xda\xb0\xcd\x84\x6\xe3\xc0\xe\xe1\x30\xf\xf3\xe0\x6\xb3\x40\xf\xf2\x50\xf\xe3\x40\xf\xf5\x20\xf\xe5\x40\xe\xa2\x50\xf\xe6\x60\xe\xe5\x20\xf\x7c\xd0\xe\xe5\x40\xf\xe1\xf0\xb\xe4\x20\xf\xe1\x70\xf\xbf\x30\xf\xf0\x80\xe\xe5\x20\xf\xe5\xe0\x7\x28\xe0\xa8\x4b\x82\x6d\x18\x41\x18\xee\xc\xe\x47\x9a\x16\x0\x73\xa8\xc9\x97\xa6\x88\x12\x26\xbf\x88\x0\x86\xf8\x5\xa7\x91\x26\xa0\x99\xfe\x80\x2a\xa\x22\x42\xc7\xf7\x69\x3\x8\xe\x37\x1d\x8e\x34\x2d\x0\xe6\x50\x93\x2f\x4\x7f\x11\x1\xc\x81\x82\x90\xc4\x3c\x20\xe6\x8\x40\x1\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x2\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x28\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x79\x80\x0\x18\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x13\x1\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x60\x0\x0\x0\x0\x0\x0\x0\x80\x21\xcf\x5\x4\xc0\x0\x0\x0\x0\x0\x0\x0\x0\x59\x20\x0\x0\x0\x12\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x1c\x8a\x61\x4\xa0\x30\xa\xa8\x90\x5\xca\xa0\x3c\x8a\x80\x8a\x92\x18\x1\x28\x83\x52\x28\x82\x12\x28\x4\x2a\xb\x84\xc6\x19\x0\x22\xc7\x3a\xa\x6\xfe\x7\xf8\x1f\xe0\x7f\x0\xfe\x7f\x20\x6f\x6\x0\x0\x0\x0\x79\x18\x0\x0\x70\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x8\x64\x83\x30\xc\x13\x4\x22\xd9\x20\xc\x5\x5\xbb\xb9\x9\x2\xa1\x6c\x18\xe\x64\x98\x20\x10\xcb\x4\xc1\xe3\x8\x4c\x10\x8\x66\x82\x40\x34\x1b\x84\xc1\xd9\x90\x28\xb\xa3\xc\x43\xa3\x3c\x1b\x2\x68\x82\x30\x6\xda\x4\x81\x70\x36\x20\x8a\xc4\x28\xca\x30\x1\x1b\x2\x6a\x3\x11\x1\x15\x30\x41\x20\x83\x6d\x43\x70\x4d\x10\x4\x80\x44\x5b\x58\x9a\x1b\x11\xa8\xa7\xa9\x24\xaa\xa4\x27\xa7\x9\x42\x21\x4d\x10\x8a\x69\x43\xa0\x4c\x10\xa\x6a\x82\x40\x3c\x1b\x4\x30\x28\x36\x2c\xca\xc6\x75\x5e\x37\x7c\x4a\x17\x6\x6c\xa6\xac\xbe\x92\xdc\xe6\xe8\xc2\xdc\xc6\xca\x92\x88\x26\x8\x45\x35\x41\x28\xac\x9\x42\x71\x6d\x10\xc0\x60\xd8\xb0\xc\x63\x40\x6\x65\xe0\x75\x83\x19\xc\xdd\x19\x6c\x10\xc4\x0\xd\x58\xc\x3d\x31\x3d\x49\x4d\x10\xa\x6c\x82\x40\x40\x1b\x4\x30\x60\x83\xd\x8b\xa2\x6\x5c\xe7\x95\xc1\xb0\x6\x4a\xd7\x6\x5c\xa6\xac\xbe\xa0\xde\xe6\xd2\xe8\xd2\xde\xdc\x36\x2c\xc3\x1b\x70\x9f\xb7\x6\xc3\x1a\xc\x5d\x1b\x4c\x10\x88\x88\x8f\x11\x53\x10\xd5\x17\xd4\xd3\x54\x12\x55\xd2\x93\x13\xd4\xd4\x86\x25\xe\xe4\x80\xeb\xbc\x32\x18\xd6\x20\xe\xba\x36\xd8\x30\xb8\x1\x1c\xcc\xc1\x86\x21\xd\xe8\x0\x98\x20\x94\x41\xb6\x41\x50\xec\x60\x43\x91\x69\x75\x60\xdd\x41\x15\x36\x36\xbb\x36\x97\x34\xb2\x32\x37\xba\x29\x41\x50\x85\xc\xcf\xc5\xae\x4c\x6e\x2e\xed\xcd\x6d\x4a\x30\x34\x21\xc3\x73\xb1\xb\x63\xb3\x2b\x93\x9b\x12\x14\x75\xc8\xf0\x5c\xe6\xd0\xc2\xc8\xca\xe4\x9a\xde\xc8\xca\xd8\xa6\x4\x48\x19\x32\x3c\x17\xb9\xb2\xb9\xb7\x3a\xb9\xb1\xb2\xb9\x29\x41\x55\x87\xc\xcf\xc5\x2e\xad\xec\x2e\x89\x6c\x8a\x2e\x8c\xae\x6c\x4a\x70\xd5\x21\xc3\x73\x29\x73\xa3\x93\xcb\x83\x7a\x4b\x73\xa3\x9b\x9b\x12\xdc\x1\x0\x79\x18\x0\x0\x4c\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\x8c\xc8\x21\x7\x7c\x70\x3\x72\x10\x87\x73\x70\x3\x7b\x8\x7\x79\x60\x87\x70\xc8\x87\x77\xa8\x7\x7a\x98\x81\x3c\xe4\x80\xf\x6e\x40\xf\xe5\xd0\xe\xf0\x0\x0\x0\x71\x20\x0\x0\x20\x0\x0\x0\x76\x40\xd\x97\xef\x3c\x7e\x40\x15\x5\x11\x95\xe\x30\xf8\xc5\x6d\x1b\xc1\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x19\x48\xc3\xe5\x3b\x8f\x2f\x44\x4\x30\x11\x21\xd0\xc\xb\x61\x1\xd3\x70\xf9\xce\xe3\x2f\xe\x30\x88\xcd\x43\x4d\x7e\x71\xdb\x26\x30\xd\x97\xef\x3c\xfe\xe2\x0\x83\xd8\x3c\xd4\xe4\x23\xb7\x6d\x3\xd5\x70\xf9\xce\xe3\x4b\x93\x13\x11\x28\x35\x3d\xd4\xe4\x17\xb7\x6d\x5\xd2\x70\xf9\xce\xe3\x4f\x44\x34\x21\x40\x84\xf9\xc5\x6d\x1b\x0\xc1\x0\x48\x3\x61\x20\x0\x0\x9c\x0\x0\x0\x13\x4\x41\x2c\x10\x0\x0\x0\x5\x0\x0\x0\x44\x94\x42\x71\x15\xc2\xc\x40\xd9\x15\x62\x40\xc9\x15\x8\x4d\x23\x0\x0\x0\x23\x6\x9\x0\x82\x60\x40\x81\xc1\x30\x6d\x5e\x30\x62\x90\x0\x20\x8\x6\x54\x18\x10\x11\xc7\x9\x23\x6\x9\x0\x82\x60\x80\x98\x41\x2\x6\x5d\x85\x8c\x18\x24\x0\x8\x82\x81\x81\x6\x8a\xe7\x59\xc9\x88\x41\x2\x80\x20\x18\x18\x69\xb0\x7c\xdf\xa4\x8c\x18\x24\x0\x8\x82\x81\xa1\x6\xc\x18\x80\x41\xb5\x8c\x18\x20\x0\x8\x82\xc1\x55\x6\x8a\x41\x84\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x18\x20\x0\x8\x82\xc1\x95\x6\x8e\x82\x90\xc1\x68\x42\x0\x8c\x18\x20\x0\x8\x82\xc1\xb5\x6\x10\xa3\x38\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x30\x9a\x40\xc\x66\x38\xf2\xb1\xc3\x91\x8f\x21\x8e\x7c\x6c\x68\xe0\x63\x43\x3\x1f\x1b\x1a\xf8\x8c\x18\x1c\x0\x8\x82\xc1\x93\x7\x9c\x25\x7\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x30\x9a\x40\xc\x23\x6\x7\x0\x82\x60\xf0\xf8\x41\x18\x6c\x7a\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xa3\x9\xc4\x30\x62\x70\x0\x20\x8\x6\xcf\x28\x98\x1\x18\xd8\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\xc3\x88\xc1\x1\x80\x20\x18\x3c\xa8\xb0\x6\x65\xd0\x7\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x30\x9a\x40\xc\x36\x5d\xf2\x19\x31\x40\x0\x10\x4\x83\xa8\x15\xe8\xe0\xb9\x82\x11\x3\x4\x0\x41\x30\x88\x5c\xa1\xe\x96\x2b\xb0\xe0\x80\x8e\x59\x9b\x7c\x46\xc\x10\x0\x4\xc1\x20\x8a\x5\x3c\x90\xb6\x60\xc4\x0\x1\x40\x10\xc\x22\x59\xc8\x3\x67\xb\x2c\x50\xa0\x63\xd9\x27\x9f\x11\x3\x4\x0\x41\x30\x88\x6a\x81\xf\xaa\x2f\x18\x31\x40\x0\x10\x4\x83\xc8\x16\xfa\x20\xfa\x2\xb\x1a\xe8\x18\x37\x6\xf2\x19\x31\x40\x0\x10\x4\x83\x28\x17\x40\x1\x1b\x83\x60\xc4\x0\x1\x40\x10\xc\x22\x5d\x8\x5\x6a\xc\x2\xb\x20\xe8\x8c\x18\x24\x0\x8\x82\x81\xe2\xb\xa4\x60\xb\xb6\xe0\xa\x6e\x30\x62\x90\x0\x20\x8\x6\x8a\x2f\x90\x82\x2d\xd8\x82\x2a\xb4\xc1\x88\x41\x2\x80\x20\x18\x28\xbe\x40\xa\xb6\x60\xb\xab\xc0\x6\x23\x6\x9\x0\x82\x60\xa0\xf8\x2\x29\xd8\x82\x2d\xb4\xc2\x1a\x8c\x18\x24\x0\x8\x82\x81\xe2\xb\xa4\x90\xb\xb6\xe0\xa\xcd\x88\x41\x2\x80\x20\x18\x28\xbe\x40\xa\xb9\x60\xb\xaa\x90\x8c\x18\x24\x0\x8\x82\x81\xe2\xb\xa4\x90\xb\xb6\xb0\xa\xc5\x88\x41\x2\x80\x20\x18\x28\xbe\x40\xa\xb9\x60\xb\xad\x10\x8c\x18\x24\x0\x8\x82\x81\xe2\xb\xa4\x0\xb\xb6\xe0\xa\xcd\x88\x41\x2\x80\x20\x18\x28\xbe\x40\xa\xb0\x60\xb\xaa\x90\x8c\x18\x24\x0\x8\x82\x81\xe2\xb\xa4\x0\xb\xb6\xb0\xa\xc5\x88\x41\x2\x80\x20\x18\x28\xbe\x40\xa\xb0\x60\xb\xad\x10\x20\x0\x0\x0\x0\x0\x0\x0\x0";

const char meta_draw_sphereVS_6_6_dxil_bytes[3229] = "\x44\x58\x42\x43\x1f\xae\xec\x74\x7c\xa9\x67\x9\x32\x20\x67\x72\x28\xa8\xf9\x8a\x1\x0\x0\x0\x9c\xc\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xb4\x0\x0\x0\x48\x1\x0\x0\x5c\x2\x0\x0\x94\x2\x0\x0\xb0\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x60\x0\x0\x0\x2\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x48\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x51\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\x1\x1\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x53\x56\x5f\x49\x6e\x73\x74\x61\x6e\x63\x65\x49\x44\x0\x0\x4f\x53\x47\x31\x8c\x0\x0\x0\x3\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x68\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x6e\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x7a\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x50\x53\x56\x30\xc\x1\x0\x0\x30\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x1\x0\x0\x0\x2\x3\x0\x2\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x5\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\xc\x0\x0\x0\x0\x0\x0\x0\x20\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x43\x4f\x4c\x4f\x52\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x41\x2\x1\x0\x0\x0\xa\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x3\x3\x4\x0\x0\x10\x0\x0\x0\x0\x0\x0\x0\x1\x2\x44\x0\x3\x2\x0\x0\xf0\xf\x0\x0\xf0\xf\x0\x0\xf0\xf\x0\x0\x0\x0\x0\x0\xff\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x30\x0\x0\x0\x0\x0\x28\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x74\x61\x5f\x64\x72\x61\x77\x5f\x73\x70\x68\x65\x72\x65\x56\x53\x5f\x36\x5f\x36\x2e\x70\x64\x62\x0\x0\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x5b\x9e\x5\x9a\x5f\x82\x47\x67\xd1\x5e\xc\x13\xf8\xbe\x37\x76\x44\x58\x49\x4c\xe4\x9\x0\x0\x66\x0\x1\x0\x79\x2\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\xcc\x9\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x70\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x5c\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\xa8\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x82\x20\x88\x82\x20\xa4\x18\x0\x41\x10\xc5\x40\xca\x4d\xc3\xe5\x4f\xd8\x43\x48\xfe\x4a\x48\x2b\x31\xf9\xc5\x6d\xa3\x62\x18\x86\x81\xa0\xe2\x9e\xe1\xf2\x27\xec\x21\x24\x3f\x4\x9a\x61\x21\x50\xd0\x94\x83\x21\x1c\x82\x28\x8\x7a\x8e\x1a\x2e\x7f\xc2\x1e\x42\xf2\xb9\x8d\x2a\x56\x62\xf2\x8b\xdb\x46\xc4\x30\xc\x43\x21\x22\xc2\x21\x48\x2a\xc5\x40\xc\xc3\x40\xd4\x6d\xc3\xe5\x4f\xd8\x43\x48\xfe\x4a\x48\xe\x15\x9\x44\x1a\x39\xf\x11\x4d\x8\x21\x21\x81\x20\xa\xe1\x10\x8e\x45\xd7\x41\xc3\xe5\x4f\xd8\x43\x48\xfe\x4a\x48\x1b\xd2\xc\x88\x20\x8\xa2\x98\x23\x8\x4a\xe1\x10\x19\xa1\xd1\x36\x10\x30\x8c\x30\xc\xc3\x8\xc4\x70\x97\x34\x45\x94\x30\xf9\xc\x31\x1\xff\x10\x1\xd6\x2f\x3d\x7\x11\x11\x68\xf0\x86\x6f\x26\x34\x18\x7\x76\x8\x87\x79\x98\x7\x37\x98\x5\x7a\x90\x87\x7a\x18\x7\x7a\xa8\x7\x79\x28\x7\x72\x10\x85\x7a\x30\x7\x73\x28\x7\x79\xe0\x83\x76\x28\x7\x7a\x8\x87\x5f\x20\x7\x79\x8\x87\x7b\xf8\x85\x79\x80\x7\x74\x28\x7\x79\x28\x7\x3f\x40\x1\x48\x61\x12\x7c\xc3\x8\xc2\x70\x67\x70\x38\xd2\xb4\x0\x98\x43\x4d\xbe\x34\x45\x94\x30\xf9\x45\x4\x30\xc4\x2f\x38\x8d\x34\x1\xcd\xf4\x7\x54\x51\x10\x11\x3a\xc6\xd1\x1b\x48\x72\xb8\xe9\x70\xa4\x69\x1\x30\x87\x9a\x7c\x21\xf8\x8b\x8\x60\x8\x14\x94\x64\xe6\x1\x31\x47\x0\xa\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x2\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x28\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x79\x80\x0\x18\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x23\x1\x1\x30\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x21\xcf\x5\x4\x40\x0\x0\x0\x0\x0\x0\x0\x0\x43\x9e\xd\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\xb2\x40\x0\x0\x12\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\x30\xa\xa8\x90\x5\xca\xa0\x3c\x8a\x80\x8a\x92\x18\x1\x28\x83\x52\x28\x82\x12\x28\x4\x4a\xb\x84\xce\x19\x0\x42\xc7\x3a\xa\x6\xfe\x7\xf8\x1f\xe0\x7f\x0\xfe\x7f\x20\x71\x6\x0\x0\x0\x0\x79\x18\x0\x0\x70\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x88\x64\x82\x40\x28\x1b\x84\x81\x98\x20\x10\xcb\x6\x61\x30\x28\xd8\xcd\x6d\x18\x10\x82\x98\x20\x10\xcc\x4\x41\xc\x3a\x2\x13\x4\xa2\x99\x20\x10\xce\x6\x61\x70\x36\x24\xca\xc2\x28\xc3\xd0\x28\xcf\x86\x0\x9a\x20\x9c\xc1\x36\x41\x20\x9e\xd\x88\x22\x31\x8a\x32\x4c\xc0\x86\x80\xda\x40\x44\x40\x5\x4c\x10\xd0\x80\xdb\x10\x5c\x13\x4\x1\x20\xd1\x16\x96\xe6\x46\x4\xea\x69\x2a\x89\x2a\xe9\xc9\x69\x82\x50\x4c\x13\x84\x82\xda\x10\x28\x13\x84\xa2\x9a\x20\x10\xd0\x6\x1\xc\x8c\xd\x8b\xb2\x71\x9d\xd7\xd\x9f\xd2\x85\x1\x9b\x29\xab\xaf\x24\xb7\x39\xba\x30\xb7\xb1\xb2\x24\xa2\x9\x42\x61\x4d\x10\x8a\x6b\x82\x50\x60\x1b\x4\x30\x18\x36\x2c\xc3\x18\x90\x41\x19\x78\xdd\x60\x6\x43\x77\x6\x1b\x4\x31\x40\x3\x16\x43\x4f\x4c\x4f\x52\x13\x84\x22\x9b\x20\x10\xd1\x6\x1\xc\xd8\x60\xc3\xa2\xa8\x1\xd7\x79\x65\x30\xac\x81\xd2\xb5\x1\x97\x29\xab\x2f\xa8\xb7\xb9\x34\xba\xb4\x37\xb7\xd\xcb\xf0\x6\xdc\xe7\xad\xc1\xb0\x6\x43\xd7\x6\x13\x4\x42\xe2\x63\xc4\x14\x44\xf5\x5\xf5\x34\x95\x44\x95\xf4\xe4\x4\x35\xb5\x61\x89\x3\x39\xe0\x3a\xaf\xc\x86\x35\x88\x83\xae\xd\x36\xc\x6e\x0\x7\x73\xb0\x61\x48\x3\x3a\x0\x26\x8\x69\xa0\x6d\x10\x14\x3b\xd8\x50\x64\x5a\x1d\x58\x77\x50\x85\x8d\xcd\xae\xcd\x25\x8d\xac\xcc\x8d\x6e\x4a\x10\x54\x21\xc3\x73\xb1\x2b\x93\x9b\x4b\x7b\x73\x9b\x12\x10\x4d\xc8\xf0\x5c\xec\xc2\xd8\xec\xca\xe4\xa6\x4\x46\x1d\x32\x3c\x97\x39\xb4\x30\xb2\x32\xb9\xa6\x37\xb2\x32\xb6\x29\x1\x52\x86\xc\xcf\x45\xae\x6c\xee\xad\x4e\x6e\xac\x6c\x6e\x4a\x50\xd5\x21\xc3\x73\xb1\x4b\x2b\xbb\x4b\x22\x9b\xa2\xb\xa3\x2b\x9b\x12\x5c\x75\xc8\xf0\x5c\xca\xdc\xe8\xe4\xf2\xa0\xde\xd2\xdc\xe8\xe6\xa6\x4\x77\x0\x79\x18\x0\x0\x4c\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\x8c\xc8\x21\x7\x7c\x70\x3\x72\x10\x87\x73\x70\x3\x7b\x8\x7\x79\x60\x87\x70\xc8\x87\x77\xa8\x7\x7a\x98\x81\x3c\xe4\x80\xf\x6e\x40\xf\xe5\xd0\xe\xf0\x0\x0\x0\x71\x20\x0\x0\x27\x0\x0\x0\x76\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x58\xc1\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x21\x74\xc3\xe5\x3b\x8f\x2f\x44\x4\x30\x11\x21\xd0\xc\xb\xf1\x45\xe\xb3\x21\xcd\x80\x34\x86\x5\x4c\xc3\xe5\x3b\x8f\xbf\x38\xc0\x20\x36\xf\x35\xf9\xc5\x6d\x9b\xc0\x34\x5c\xbe\xf3\xf8\x8b\x3\xc\x62\xf3\x50\x93\x8f\xdc\xb6\x11\x5c\xc3\xe5\x3b\x8f\x1f\x1\xd6\x46\x15\x5\x11\x95\xe\x30\xf8\xc5\x6d\xdb\x40\x35\x5c\xbe\xf3\xf8\xd2\xe4\x44\x4\x4a\x4d\xf\x35\xf9\xc5\x6d\x9b\x81\x34\x5c\xbe\xf3\xf8\x13\x11\x4d\x8\x10\x61\x7e\x71\xdb\x6\x40\x30\x0\xd2\x0\x61\x20\x0\x0\xad\x0\x0\x0\x13\x4\x41\x2c\x10\x0\x0\x0\xe\x0\x0\x0\x44\x94\x42\x71\x15\xc2\xc\x40\xd9\x95\xa5\x40\xc9\xe\x14\xec\x40\x69\x14\x26\x42\x81\x50\x51\xe\xe5\x41\xda\x8\x0\x59\x73\x8\x67\xe0\x50\x36\x7\x91\x24\xc\x35\x7\xc1\x30\xcc\x45\xd6\x1c\x82\x19\x3c\x0\x0\x0\x0\x23\x6\x8\x0\x82\x60\xb0\xa1\x1\x33\x9c\x41\x31\x62\x80\x0\x20\x8\x6\x5b\x1a\x34\x43\x19\x18\x23\x6\x7\x0\x82\x60\x70\xad\x41\x13\xc\x23\x6\x9\x0\x82\x60\x80\xc4\xc1\xa4\x6\x67\xf0\x49\x23\x6\x9\x0\x82\x60\x60\xcc\x1\x85\x6\x68\x0\x6\xd3\x88\x41\x2\x80\x20\x18\x18\x74\x50\xa5\x41\x1a\x74\xd4\x88\x41\x2\x80\x20\x18\x18\x75\x60\xa9\x81\x1a\x7c\xd5\x88\xc1\x1\x80\x20\x18\x5c\x70\x20\x1d\xcb\x88\x81\x2\x80\x20\x18\x3c\x74\x50\x5\x5\x1b\x3c\xd8\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x18\x28\x0\x8\x82\xc1\x83\x7\x59\x91\xbc\x1\x19\x70\xa3\x9\x1\x30\x62\xa0\x0\x20\x8\x6\x8f\x1e\x6c\xc7\x62\x51\xde\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\x83\x19\x8f\x7c\xec\x78\xe4\x63\xc8\x23\x1f\x1b\x1a\xf8\xd8\xd0\xc0\xc7\x86\x6\x3e\x23\x6\x7\x0\x82\x60\x20\x91\xc2\x19\x5c\x7d\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xa3\x9\xc4\x30\x62\x70\x0\x20\x8\x6\x52\x2a\xb0\x1\x57\xa\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x30\x9a\x40\xc\x23\x6\x7\x0\x82\x60\x20\xb9\x42\x1c\x84\x41\x28\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x68\x2\x31\x8c\x18\x1c\x0\x8\x82\x81\x34\xb\x76\x60\x6\xa8\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xa3\x9\xc4\x60\xd3\x25\x9f\x11\x3\x4\x0\x41\x30\xa0\x70\xe1\xf\x9e\x2b\x18\x31\x40\x0\x10\x4\x3\x2a\x17\x40\x61\xb9\x2\xb\xe\xe8\x98\xb5\xc9\x67\xc4\x0\x1\x40\x10\xc\x28\x5e\x18\x5\x69\xb\x46\xc\x10\x0\x4\xc1\x80\xea\x5\x52\x70\xb6\xc0\x2\x5\x3a\x96\x7d\xf2\x19\x31\x40\x0\x10\x4\x3\xa\x1c\x4e\xa1\xfa\x82\x11\x3\x4\x0\x41\x30\xa0\xc2\x1\x15\xa2\x2f\xb0\xa0\x81\x8e\x71\x63\x20\x9f\x11\x3\x4\x0\x41\x30\xa0\xc8\x61\x15\xb0\x31\x8\x46\xc\x10\x0\x4\xc1\x80\x2a\x7\x56\xa0\xc6\x20\xb0\x0\x82\xce\x88\x41\x2\x80\x20\x18\x28\xea\xf0\xa\xe1\x10\xe\xb9\xe0\x6\x23\x6\x9\x0\x82\x60\xa0\xa8\xc3\x2b\x84\x43\x38\xd4\x42\x1b\x8c\x18\x24\x0\x8\x82\x81\xa2\xe\xaf\x10\xe\xe1\x60\xb\x6c\x30\x62\x90\x0\x20\x8\x6\x8a\x3a\xbc\x42\x38\x84\x3\x2e\xac\xc1\x88\x41\x2\x80\x20\x18\x28\xea\xf0\xa\xe4\x10\xe\xb9\xd0\x8c\x18\x24\x0\x8\x82\x81\xa2\xe\xaf\x40\xe\xe1\x50\xb\xc9\x88\x41\x2\x80\x20\x18\x28\xea\xf0\xa\xe4\x10\xe\xb6\x50\x8c\x18\x24\x0\x8\x82\x81\xa2\xe\xaf\x40\xe\xe1\x80\xb\xc1\x88\x41\x2\x80\x20\x18\x28\xea\xf0\xa\xbb\x10\xe\xb9\xd0\x8c\x18\x24\x0\x8\x82\x81\xa2\xe\xaf\xb0\xb\xe1\x50\xb\xc9\x88\x41\x2\x80\x20\x18\x28\xea\xf0\xa\xbb\x10\xe\xb6\x50\x8c\x18\x24\x0\x8\x82\x81\xa2\xe\xaf\xb0\xb\xe1\x80\xb\x1\x2\x0\x0\x0\x0\x0\x0\x0";

}