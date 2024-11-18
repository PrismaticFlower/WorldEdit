#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char meta_draw_wireframe_GS_fallbackPS_6_1_dxil_bytes[3297];
extern const char meta_draw_wireframe_GS_fallbackPS_6_6_dxil_bytes[3417];

auto meta_draw_wireframe_GS_fallbackPS() noexcept -> shader_def
{
   return {
      .name = "meta_draw_wireframe_GS_fallbackPS",
      .entrypoint = L"main",
      .target_6_1 = L"ps_6_1",
      .target_6_6 = L"ps_6_6",
      .file = L"meta_draw_wireframe_GS_fallbackPS.hlsl",
      .dxil_6_1 = {reinterpret_cast<const std::byte*>(meta_draw_wireframe_GS_fallbackPS_6_1_dxil_bytes),
                      sizeof(meta_draw_wireframe_GS_fallbackPS_6_1_dxil_bytes) - 1},
      .dxil_6_6 = {reinterpret_cast<const std::byte*>(meta_draw_wireframe_GS_fallbackPS_6_6_dxil_bytes),
                      sizeof(meta_draw_wireframe_GS_fallbackPS_6_6_dxil_bytes) - 1},
   };
}

const char meta_draw_wireframe_GS_fallbackPS_6_1_dxil_bytes[3297] = "\x44\x58\x42\x43\xb0\x11\x1\x43\x3\x1d\x30\xa\xe5\x77\xd4\x55\xa0\xb\xb1\xeb\x1\x0\x0\x0\xe0\xc\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x20\x1\x0\x0\x5c\x1\x0\x0\x7c\x2\x0\x0\xc0\x2\x0\x0\xdc\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\xcc\x0\x0\x0\x5\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\xa8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xae\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xba\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\xf\xb\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xba\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\xf\xb\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xba\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x4\x0\x0\x0\xf\xb\x0\x0\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\x18\x1\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x3\x1\x0\x5\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x18\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x3\x3\x4\x0\x0\x7\x0\x0\x0\x1\x0\x0\x0\x3\x2\x44\x0\x3\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x49\x4c\x44\x4e\x3c\x0\x0\x0\x0\x0\x37\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x74\x61\x5f\x64\x72\x61\x77\x5f\x77\x69\x72\x65\x66\x72\x61\x6d\x65\x5f\x47\x53\x5f\x66\x61\x6c\x6c\x62\x61\x63\x6b\x50\x53\x5f\x36\x5f\x31\x2e\x70\x64\x62\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\xa2\xa5\x3b\x16\x52\xe7\x9c\x15\xf7\x4e\xc6\x5f\xc5\x2e\x5e\xbc\x44\x58\x49\x4c\xfc\x9\x0\x0\x61\x0\x0\x0\x7f\x2\x0\x0\x44\x58\x49\x4c\x1\x1\x0\x0\x10\x0\x0\x0\xe4\x9\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x76\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x34\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\x8c\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x80\x20\x88\x62\x20\x64\x8e\x20\x28\x3\x40\x28\xb4\x14\x62\x20\x86\x81\x9a\x32\xc\xc4\x40\xcf\x51\xc3\xe5\x4f\xd8\x43\x48\x3e\xb7\x51\xc5\x4a\x4c\x7e\x71\xdb\x88\x18\x86\x61\xa0\xe2\x9e\xe1\xf2\x27\xec\x21\x24\x3f\x4\x9a\x61\x21\x50\x20\x15\x22\x22\x26\x82\xa8\x62\x4c\x44\x41\x10\xa\x59\x3\x1\xc3\x8\xc4\x90\x4\xd9\x30\xc2\x30\xc\x23\x8\xc3\x9d\xc1\xe1\x48\xd3\x2\x60\xe\x35\xf9\xd2\x14\x51\xc2\xe4\x17\x11\xc0\x10\xbf\xe0\x34\xd2\x4\x34\xd3\x1f\x50\x45\x41\x44\xe8\xd0\xb4\x6d\xe0\xb8\xe1\xa6\xc3\x91\xa6\x5\xc0\x1c\x6a\xf2\x85\xe0\x2f\x22\x80\x21\x50\xd0\x91\x97\x16\xc4\x1c\x1\x28\x4c\x1\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x2c\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x69\x80\x0\x8\x0\x0\x0\x0\x0\x0\x0\x60\xc8\xf3\x0\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x60\x0\x0\x0\x0\x0\x0\x0\x80\x21\x8f\x5\x4\xc0\x0\x0\x0\x0\x0\x0\x0\x0\x59\x20\x0\x0\x0\x12\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x1c\x8a\x61\x4\xa0\x90\x5\xca\xa0\x8\xca\xa2\x3c\xa8\x28\x89\x11\x80\x22\x28\x84\x32\x28\x81\x2\xa1\xb0\x20\x8\x1c\xcb\xa2\x88\x40\x20\x0\x4\x1\x0\x4\x1\x10\x4\x1\x10\x4\x1\x10\xfa\x66\x0\x0\x0\x0\x79\x18\x0\x0\x64\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x8\x64\x83\x30\xc\x13\x4\x22\xd9\x20\xc\x5\x5\xb8\xb9\x9\x2\xa1\x6c\x18\xe\x64\x98\x20\x10\xcb\x4\xe1\xd3\x8\x4c\x10\x8\x66\x3\xa2\x2c\x8c\xa2\xc\xd\xb0\x21\x70\x36\x10\x0\xf0\x0\x13\x4\x30\xc8\x36\x4\xd1\x4\x41\x0\x48\xb4\x85\xa5\xb9\xb1\x18\x7a\x62\x7a\x92\x9a\x20\x14\xd1\x4\xa1\x90\x36\x4\xca\x4\xa1\x98\x26\x8\x5\x35\x41\x20\x9a\xd\xc2\x56\x6c\x58\x94\xca\xba\xb0\x6c\xd0\x94\x8b\xe3\x32\x65\xf5\x5\xf5\x36\x97\x46\x97\xf6\xe6\x36\x41\x28\xaa\xd\xc2\xb6\x6d\x58\x6\xcf\xfa\x30\x6d\xd0\x86\xb\xc\x26\x8\x84\xc3\xc7\x88\x29\x88\xea\xb\xea\x69\x2a\x89\x2a\xe9\xc9\x9\x6a\x6a\xc3\xa0\xc\x62\x30\x41\x28\xac\x9\x2\xf1\x6c\x10\x36\x33\xd8\xb0\x88\xc1\x18\x58\x17\x19\x94\xc1\xa6\x89\xc1\x75\x6\x1b\x86\x2e\xc\xd0\x80\xc9\x94\xd5\x17\x55\x98\xdc\x59\x19\xdd\x4\xa1\xb8\x26\x8\x4\xb4\x41\xd8\xd8\x60\xc3\xa2\xa8\x81\xb5\x6\xd8\x35\x68\xca\xd5\x6\x1b\x2\x37\xd8\x30\xa4\xc1\x1b\x0\x13\x84\x30\xc0\x36\x8\x4a\x1c\x6c\x28\x26\xa\xe\x20\x39\xa8\xc2\xc6\x66\xd7\xe6\x92\x46\x56\xe6\x46\x37\x25\x8\xaa\x90\xe1\xb9\xd8\x95\xc9\xcd\xa5\xbd\xb9\x4d\x9\x86\x26\x64\x78\x2e\x76\x61\x6c\x76\x65\x72\x53\x82\xa2\xe\x19\x9e\xcb\x1c\x5a\x18\x59\x99\x5c\xd3\x1b\x59\x19\xdb\x94\x0\x29\x43\x86\xe7\x22\x57\x36\xf7\x56\x27\x37\x56\x36\x37\x25\x78\xea\x90\xe1\xb9\xd8\xa5\x95\xdd\x25\x91\x4d\xd1\x85\xd1\x95\x4d\x9\xa2\x3a\x64\x78\x2e\x65\x6e\x74\x72\x79\x50\x6f\x69\x6e\x74\x73\x53\x2\x39\x0\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\x1e\x0\x0\x0\x46\x0\xd\x97\xef\x3c\x7e\x80\x34\x40\x84\xf9\xc5\x6d\x9b\xc1\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x1d\x48\xc3\xe5\x3b\x8f\x2f\x44\x4\x30\x11\x21\xd0\xc\xb\x61\x3\xcd\x70\xf9\xce\xe3\xf\x88\x24\x0\xd1\x60\x1\xd3\x70\xf9\xce\xe3\x2f\xe\x30\x88\xcd\x43\x4d\x7e\x71\xdb\x26\x50\xd\x97\xef\x3c\xbe\x34\x39\x11\x81\x52\xd3\x43\x4d\x7e\x71\xdb\x56\xf0\xc\x97\xef\x3c\x3e\xd5\x0\x11\xe6\x17\xb7\x6d\x0\x4\x3\x20\xd\x0\x0\x61\x20\x0\x0\xee\x0\x0\x0\x13\x4\x43\x2c\x10\x0\x0\x0\x10\x0\x0\x0\xa4\xd4\xc0\x8\x0\x11\x45\x1a\x50\x12\x25\x57\x2a\x85\x30\x3\x50\x48\xa5\x50\x76\x5\x53\x14\x65\x44\xc3\x18\x81\xc8\x82\x22\xde\xb\x63\x4\x20\x8\x82\x20\x38\x8c\x11\x80\x20\x8\x82\x60\x30\x2\x30\x46\x0\x82\x20\x88\x7f\x63\x4\x20\x8\x82\xf0\x7\x23\x6\x9\x0\x82\x60\x60\xa1\x1\xc4\x91\x1\x19\x4c\x23\x6\x9\x0\x82\x60\x60\xbc\xc1\x83\x6\x65\xe0\x39\x23\x6\x9\x0\x82\x60\x60\xc0\x1\x94\x6\x66\xb0\x3d\x23\x6\x9\x0\x82\x60\x60\xc4\x41\x74\x6\x67\x0\x6\xd0\x88\x41\x2\x80\x20\x18\x18\x72\x20\xa1\x1\x1a\x74\xd1\x88\x41\x2\x80\x20\x18\x18\x73\x30\xa5\x41\x1a\x84\x81\x34\x62\x90\x0\x20\x8\x6\x6\x1d\x50\x67\xa0\x6\x63\x30\x8d\x18\x24\x0\x8\x82\x81\x51\x7\x15\x1a\xac\x1\x18\x50\x23\x6\x9\x0\x82\x60\x60\xd8\x81\x95\x6\x6c\x20\x6\xd5\x88\x41\x2\x80\x20\x18\x18\x77\x70\xa9\x1\x1c\x98\x81\x35\x62\x90\x0\x20\x8\x6\x6\x1e\x60\x6b\x10\x7\x63\x70\x8d\x18\x24\x0\x8\x82\x81\x91\x7\x19\x1b\xc8\x41\x19\x60\x23\x6\x9\x0\x82\x60\x60\xe8\x81\xd6\x6\x6d\x90\x6\xd9\x88\x41\x2\x80\x20\x18\x18\x7b\xb0\xb9\x81\x1b\x98\x81\x36\x62\x90\x0\x20\x8\x6\x6\x1f\x70\x6f\xf0\x6\x68\xb0\x59\x72\xd0\xc7\x12\x84\x3e\x23\x6\x7\x0\x82\x60\x40\xe9\xc1\x16\x89\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\x83\x1d\x18\x7c\x2c\xc0\xe4\x63\x81\x21\x1f\xb\xa\xf8\x18\xa7\xc4\xc7\x2\x4e\x3e\x16\x24\xf2\xb1\x0\x81\x8f\x55\x13\x7d\xac\xa2\xe8\x63\x82\x18\xc0\xc7\x2\x31\x90\x8f\x5\x90\x7c\x2c\x78\xe0\x63\x66\x50\xc4\xc7\x2\x33\x90\x8f\x5\x93\x7c\x2c\x90\xe0\x63\x9c\x46\x1f\xe3\x36\xfa\x98\xc0\x6\xf0\xb1\x80\xd\xe4\x63\x81\x26\x1f\xb\x32\xf8\x18\x1c\x14\xf1\xb1\x0\xe\xe4\x63\x41\x27\x1f\xb\x38\xf8\x98\x83\xc5\xc7\x2\x3a\x90\x8f\x85\x41\x20\x1f\xb\xe8\x40\x3e\x66\x81\x41\x7c\x2c\xc0\x3\xf9\x98\x19\x4\xf2\xb1\xe0\xe\xe4\x63\x57\x27\x1f\xa3\xc4\x40\x3e\x26\x4\xf1\xb1\x80\x88\x8f\x5\x9\x7c\x46\xc\xc\x0\x4\xc1\xe0\x49\x7\x72\x8\x8c\x61\xe4\x63\x49\x22\x1f\x13\x2\xf8\x8c\x18\x18\x0\x8\x82\xc1\xd3\xe\xac\x10\x58\x11\xd0\xc7\xae\x30\x88\x8f\x5\xa7\x20\x1f\xa3\x83\x40\x3e\x16\x9c\x82\x7c\xec\x4b\x83\xf8\x58\xb0\xa\xf2\xb1\x3c\x8\xe4\x63\x81\x2a\xc8\xc7\xc0\xc0\xc\xe4\x63\xdd\x1a\xc8\xc7\x84\x20\x3e\x16\x10\xf1\xb1\x20\x81\xcf\x88\x81\x1\x80\x20\x18\x3c\xfc\x70\xf\x81\x31\x8c\x7c\x2c\x49\xe4\x63\x42\x0\x9f\x11\x3\x3\x0\x41\x30\x78\x40\xe2\x17\x2\x2b\x2\xfa\x98\x1f\xa8\x41\x7c\x2c\xd0\x5\xf9\xd8\x29\x4\xf2\xb1\x40\x17\xe4\x63\xa6\x20\x7\xf1\xb1\xc0\x17\xe4\x63\xac\x10\xc8\xc7\x82\x5e\x90\x8f\xbd\xc1\x29\xc8\xc7\xe8\x80\x14\xe4\x63\x42\x10\x1f\xb\x88\xf8\x58\x90\xc0\x67\xc4\xc0\x0\x40\x10\xc\x9e\x97\x50\x89\xc0\x18\x46\x3e\x96\x24\xf2\x31\x21\x80\xcf\x88\x81\x1\x80\x20\x18\x3c\x33\x21\xf\x81\x15\x1\x7d\x46\xc\xe\x0\x4\xc1\xa0\xb1\x89\x7b\x38\x3\x6a\xc4\xe0\x0\x40\x10\xc\x9a\x9b\xc0\x87\x40\x18\x31\x38\x0\x10\x4\x3\xca\x26\xee\xa1\x1d\xea\x61\x34\x21\x0\x2c\x80\x7\xf9\x18\x21\xf\xf0\xb1\x40\x88\xcf\x88\xc1\x1\x80\x20\x18\x34\x3d\x91\xf\x41\x3d\x8c\x18\x1c\x0\x8\x82\x41\xe3\x13\xff\x10\xdc\x83\x5\x81\x7c\x2c\xd0\x7\xf9\x8c\x18\x18\x0\x8\x82\xc1\x3\x16\x25\x11\x58\xd0\xf\xf0\x19\x8e\x8\xf6\x21\xf8\x66\x19\x2\x21\x18\x31\x30\x0\x10\x4\x83\xa5\x2c\x56\xa2\x25\x66\x9\x84\x11\x83\x3\x0\x41\x30\xa0\xc4\x62\x24\xf2\x21\x24\x46\x13\x2\xc0\x2\x7e\x90\x8f\x31\x41\x7c\x46\xc\xe\x0\x4\xc1\xa0\x41\xb\x92\x8\x40\x62\xc4\xe0\x0\x40\x10\xc\x9a\xb4\x50\x89\x40\x24\x2c\x8\xe4\x63\x41\x49\xc8\x67\xc4\xc0\x0\x40\x10\xc\x9e\xb5\x80\x89\xc0\x2\x94\x80\xcf\x88\x41\x2\x80\x20\x18\x20\x70\xe1\x12\x67\x71\x16\x60\x1\x12\x23\x6\x9\x0\x82\x60\x80\xc0\x85\x4b\x9c\xc5\x59\xf0\xc4\x3f\x8c\x18\x24\x0\x8\x82\x1\x2\x17\x2e\x71\x16\x67\xf1\x13\xfe\x30\x62\x90\x0\x20\x8\x6\x8\x5c\xb8\xc4\x59\x9c\x45\x4f\x4\x8\x0\x0\x0\x0";

const char meta_draw_wireframe_GS_fallbackPS_6_6_dxil_bytes[3417] = "\x44\x58\x42\x43\xdb\x6b\xa8\xa8\xc1\x64\xc1\x98\x5e\xf3\x24\xe4\x2b\xb6\x6\xbd\x1\x0\x0\x0\x58\xd\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x20\x1\x0\x0\x5c\x1\x0\x0\x7c\x2\x0\x0\xc0\x2\x0\x0\xdc\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\xcc\x0\x0\x0\x5\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\xa8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xae\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xba\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\xf\xb\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xba\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\xf\xb\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xba\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x4\x0\x0\x0\xf\xb\x0\x0\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\x18\x1\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x3\x1\x0\x5\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x18\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x3\x3\x4\x0\x0\x7\x0\x0\x0\x1\x0\x0\x0\x3\x2\x44\x0\x3\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x49\x4c\x44\x4e\x3c\x0\x0\x0\x0\x0\x37\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x74\x61\x5f\x64\x72\x61\x77\x5f\x77\x69\x72\x65\x66\x72\x61\x6d\x65\x5f\x47\x53\x5f\x66\x61\x6c\x6c\x62\x61\x63\x6b\x50\x53\x5f\x36\x5f\x36\x2e\x70\x64\x62\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x75\x54\x8a\x22\xc6\x6c\x4b\x35\x38\x39\xc\xa8\x8d\x63\xfd\xac\x44\x58\x49\x4c\x74\xa\x0\x0\x66\x0\x0\x0\x9d\x2\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x5c\xa\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x94\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x40\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\x9c\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x80\x20\x88\x62\x20\x64\x8e\x20\x28\x3\x40\x28\xb4\x14\x62\x20\x86\x81\x9a\x32\xc\xc4\x40\xcf\x51\xc3\xe5\x4f\xd8\x43\x48\x3e\xb7\x51\xc5\x4a\x4c\x7e\x71\xdb\x88\x18\x86\x61\xa0\xe2\x9e\xe1\xf2\x27\xec\x21\x24\x3f\x4\x9a\x61\x21\x50\x20\x15\x22\x22\x26\x82\xa8\xdb\x86\xcb\x9f\xb0\x87\x90\xfc\x95\x90\x1c\x2a\x12\x88\x34\x72\x1e\x22\x9a\x10\x42\x42\x2\x41\x14\x62\x22\x26\x8b\xae\x83\x86\xcb\x9f\xb0\x87\x90\xfc\x95\x90\x36\xa4\x19\x10\x41\x10\x44\x51\x8a\x89\xc8\x8\x85\xb4\x81\x80\x61\x4\x62\x48\x82\x6e\x18\x61\x18\x86\x11\x84\xe1\xce\xe0\x70\xa4\x69\x1\x30\x87\x9a\x7c\x69\x8a\x28\x61\xf2\x8b\x8\x60\x88\x5f\x70\x1a\x69\x2\x9a\xe9\xf\xa8\xa2\x20\x22\x74\x78\xde\x37\x80\xe0\x70\xd3\xe1\x48\xd3\x2\x60\xe\x35\xf9\x42\xf0\x17\x11\xc0\x10\x28\x8\x49\x4c\xb\x62\x8e\x0\x14\xa6\x0\x0\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x2c\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x69\x80\x0\x8\x0\x0\x0\x0\x0\x0\x0\x60\xc8\xf3\x0\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x60\x0\x0\x0\x0\x0\x0\x0\x80\x21\xcf\x5\x4\x40\x0\x0\x0\x0\x0\x0\x0\x0\x43\x1e\xd\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\xb2\x40\x0\x0\x12\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\x90\x5\xca\xa0\x8\xca\xa2\x3c\xa8\x28\x89\x11\x80\x22\x28\x84\x32\x28\x81\x2\xa1\xb2\x20\x88\x1c\xcb\xa2\x88\x40\x20\x0\x4\x1\x0\x4\x1\x10\x4\x1\x10\x4\x1\x10\x1a\x67\x0\x0\x0\x0\x79\x18\x0\x0\x64\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x88\x64\x82\x40\x28\x1b\x84\x81\x98\x20\x10\xcb\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\x10\xcc\x4\x61\xc\x36\x2\x13\x4\xa2\xd9\x80\x28\xb\xa3\x28\x43\x3\x6c\x8\x9c\xd\x4\x0\x3c\xc0\x4\x81\xc\xb4\xd\x41\x34\x41\x10\x0\x12\x6d\x61\x69\x6e\x2c\x86\x9e\x98\x9e\xa4\x26\x8\x85\x34\x41\x28\xa6\xd\x81\x32\x41\x28\xa8\x9\x42\x51\x4d\x10\x8\x67\x83\xb0\x19\x1b\x16\xa5\xb2\x2e\x2c\x1b\x34\xe5\xe2\xb8\x4c\x59\x7d\x41\xbd\xcd\xa5\xd1\xa5\xbd\xb9\x4d\x10\xa\x6b\x83\xb0\x6d\x1b\x96\xc1\xb3\x3e\x4c\x1b\xb4\xe1\x2\x83\x9\x2\xf1\xf0\x31\x62\xa\xa2\xfa\x82\x7a\x9a\x4a\xa2\x4a\x7a\x72\x82\x9a\xda\x30\x28\x83\x18\x4c\x10\x8a\x6b\x82\x40\x40\x1b\x84\xcd\xc\x36\x2c\x62\x30\x6\xd6\x45\x6\x65\xb0\x69\x62\x70\x9d\xc1\x86\xa1\xb\x3\x34\x60\x32\x65\xf5\x45\x15\x26\x77\x56\x46\x37\x41\x28\xb0\x9\x2\x11\x6d\x10\x36\x36\xd8\xb0\x28\x6a\x60\xad\x1\x76\xd\x9a\x72\xb5\xc1\x86\xc0\xd\x36\xc\x69\xf0\x6\xc0\x4\xa1\xc\xb2\xd\x82\x12\x7\x1b\x8a\x89\x82\x3\x48\xe\xaa\xb0\xb1\xd9\xb5\xb9\xa4\x91\x95\xb9\xd1\x4d\x9\x82\x2a\x64\x78\x2e\x76\x65\x72\x73\x69\x6f\x6e\x53\x2\xa2\x9\x19\x9e\x8b\x5d\x18\x9b\x5d\x99\xdc\x94\xc0\xa8\x43\x86\xe7\x32\x87\x16\x46\x56\x26\xd7\xf4\x46\x56\xc6\x36\x25\x40\xca\x90\xe1\xb9\xc8\x95\xcd\xbd\xd5\xc9\x8d\x95\xcd\x4d\x9\x9e\x3a\x64\x78\x2e\x76\x69\x65\x77\x49\x64\x53\x74\x61\x74\x65\x53\x82\xa8\xe\x19\x9e\x4b\x99\x1b\x9d\x5c\x1e\xd4\x5b\x9a\x1b\xdd\xdc\x94\x40\xe\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\x24\x0\x0\x0\x76\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x18\x1\x34\x5c\xbe\xf3\xf8\x1\xd2\x0\x11\xe6\x17\xb7\x6d\x6\xdb\x70\xf9\xce\xe3\xb\x1\x55\x14\x44\x54\x3a\xc0\x50\x12\x6\x20\x60\x7e\x71\xdb\x86\xd0\xd\x97\xef\x3c\xbe\x10\x11\xc0\x44\x84\x40\x33\x2c\xc4\x17\x39\xcc\x86\x34\x3\xd2\x18\x36\xd0\xc\x97\xef\x3c\xfe\x80\x48\x2\x10\xd\x16\x30\xd\x97\xef\x3c\xfe\xe2\x0\x83\xd8\x3c\xd4\xe4\x17\xb7\x6d\x2\xd5\x70\xf9\xce\xe3\x4b\x93\x13\x11\x28\x35\x3d\xd4\xe4\x17\xb7\x6d\x5\xcf\x70\xf9\xce\xe3\x53\xd\x10\x61\x7e\x71\xdb\x6\x40\x30\x0\xd2\x0\x61\x20\x0\x0\xf7\x0\x0\x0\x13\x4\x43\x2c\x10\x0\x0\x0\x15\x0\x0\x0\xa4\xd4\xc0\x8\x0\x11\x45\x1a\x50\x12\xa5\x51\xb0\x3\x85\x30\x3\x50\x48\xa5\x50\x76\x5\x53\x14\x65\x54\x2a\x25\x3b\x40\xc3\x18\x81\xc8\x82\x22\xde\xb\x63\x4\x20\x8\x82\x20\x38\x8c\x11\x80\x20\x8\x82\x60\x30\x2\x30\x46\x0\x82\x20\x88\x7f\x63\x4\x20\x8\x82\xf0\x47\xd6\x1c\x2\x18\x34\x94\xcd\x41\x30\xc\x43\x1\x0\x0\x0\x23\x6\x8\x0\x82\x60\xa0\xb1\x41\x12\xa0\xc1\x35\x62\x70\x0\x20\x8\x6\x97\x1b\x50\xc1\x30\x62\x90\x0\x20\x8\x6\x46\x1d\x50\x6d\xa0\x6\x63\x30\x8d\x18\x24\x0\x8\x82\x81\x61\x7\x95\x1b\xac\x1\x18\x50\x23\x6\x9\x0\x82\x60\x60\xdc\x81\xc5\x6\x6c\x50\x6\xd5\x88\x41\x2\x80\x20\x18\x18\x78\x70\xb5\x41\x1b\x88\x81\x35\x62\x90\x0\x20\x8\x6\x46\x1e\x60\x6e\xe0\x6\x66\x70\x8d\x18\x24\x0\x8\x82\x81\xa1\x7\x19\x1b\xbc\x1\x1a\x60\x23\x6\x9\x0\x82\x60\x60\xec\x81\xd6\x6\x70\x50\x6\xd9\x88\x41\x2\x80\x20\x18\x18\x7c\xb0\xb9\x41\x1c\x9c\x81\x36\x62\x90\x0\x20\x8\x6\x46\x1f\x70\x6f\x50\x7\x6b\xb0\x8d\x18\x24\x0\x8\x82\x81\xe1\x7\x1d\x1c\xd8\x1\x1a\x70\x23\x6\x9\x0\x82\x60\x60\xfc\x81\x17\x7\x77\xa0\x6\xdd\x88\x41\x2\x80\x20\x18\x18\xa0\xf0\xc9\x81\x1c\xb8\x81\x37\x62\x90\x0\x20\x8\x6\x46\x28\x80\xc1\x1c\xcc\xc1\x1a\x7c\x23\x6\x9\x0\x82\x60\x60\x88\x42\x18\xd0\x1\x1d\xb4\x1\x18\x58\x72\xd0\xc7\x12\x84\x3e\x23\x6\x7\x0\x82\x60\x40\x81\x2\x18\x44\x67\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xa3\x9\xc4\x60\xc7\x6\x1f\xb\x36\xf9\x58\x60\xc8\xc7\x82\x2\x3e\xf6\x29\xf1\xb1\xe0\x93\x8f\x5\x89\x7c\x2c\x40\xe0\x63\xd5\x44\x1f\xab\x28\xfa\x98\x50\x6\xf0\xb1\xa0\xc\xe4\x63\x1\x24\x1f\xb\x1e\xf8\x58\x1a\x14\xf1\xb1\x20\xd\xe4\x63\xc1\x24\x1f\xb\x24\xf8\x18\xa7\xd1\xc7\xb8\x8d\x3e\x26\xbc\x1\x7c\x2c\x78\x3\xf9\x58\xa0\xc9\xc7\x82\xc\x3e\x36\x7\x45\x7c\x2c\x98\x3\xf9\x58\xd0\xc9\xc7\x2\xe\x3e\xe6\x60\xf1\xb1\xe0\xe\xe4\x63\x61\x10\xc8\xc7\x2\x3a\x90\x8f\x59\x60\x10\x1f\xb\xf6\x40\x3e\x66\x6\x81\x7c\x2c\xb8\x3\xf9\xd8\xd5\xc9\xc7\x28\x31\x90\x8f\x9\x41\x7c\x2c\x20\xe2\x63\x41\x2\x9f\x11\x3\x3\x0\x41\x30\x78\xde\x41\x1d\x2\x63\x18\xf9\x58\x92\xc8\xc7\x84\x0\x3e\x23\x6\x6\x0\x82\x60\xf0\xcc\x43\x2c\x4\x56\x4\xf4\xb1\x2b\xc\xe2\x63\x81\x2a\xc8\xc7\xe8\x20\x90\x8f\x5\xa7\x20\x1f\xfb\xd2\x20\x3e\x16\xb8\x82\x7c\x2c\xf\x2\xf9\x58\xa0\xa\xf2\x31\x30\x30\x3\xf9\x58\xb7\x6\xf2\x31\x21\x88\x8f\x5\x44\x7c\x2c\x48\xe0\x33\x62\x60\x0\x20\x8\x6\x8f\x48\xf4\x43\x60\xc\x23\x1f\x4b\x12\xf9\x98\x10\xc0\x67\xc4\xc0\x0\x40\x10\xc\x1e\x93\x20\x87\xc0\x8a\x80\x3e\xe6\x7\x6a\x10\x1f\xb\x7a\x41\x3e\x76\xa\x81\x7c\x2c\xd0\x5\xf9\x98\x29\xc8\x41\x7c\x2c\x8\x7\xf9\x18\x2b\x4\xf2\xb1\xa0\x17\xe4\x63\x6f\x70\xa\xf2\x31\x3a\x20\x5\xf9\x98\x10\xc4\xc7\x2\x22\x3e\x16\x24\xf0\x19\x31\x30\x0\x10\x4\x83\xa7\x26\x60\x22\x30\x86\x91\x8f\x25\x89\x7c\x4c\x8\xe0\x33\x62\x60\x0\x20\x8\x6\x4f\x4e\xdc\x43\x60\x45\x40\x9f\x11\x83\x3\x0\x41\x30\x68\x78\x82\x1f\xce\x80\x1a\x31\x38\x0\x10\x4\x83\xa6\x27\xfa\x21\x10\x46\xc\xe\x0\x4\xc1\x80\xe2\x9\x7e\x68\x7\x7d\x18\x4d\x8\x0\xb\xe6\x41\x3e\x46\xd4\x3\x7c\x2c\x10\xe2\x33\x62\x70\x0\x20\x8\x6\xcd\x58\xf8\x43\x80\xf\x23\x6\x7\x0\x82\x60\xd0\x90\x5\x49\x4\xfa\x60\x41\x20\x1f\xb\xfa\x41\x3e\x23\x6\x6\x0\x82\x60\xf0\x98\x45\x48\x4\x16\x80\x4\x7c\x86\x23\x2\x7f\x8\xbe\x59\x86\x40\x8\x46\xc\xc\x0\x4\xc1\x60\x59\xb\x98\x90\x89\x59\x2\x61\xc4\xe0\x0\x40\x10\xc\x28\xb4\x40\x89\x7c\x30\x89\xd1\x84\x0\xb0\xe0\x1f\xe4\x63\x4c\x10\x9f\x11\x83\x3\x0\x41\x30\x68\xdc\x22\x25\x82\x91\x18\x31\x38\x0\x10\x4\x83\xe6\x2d\x5e\x22\x28\x9\xb\x2\xf9\x58\x80\x12\xf2\x19\x31\x30\x0\x10\x4\x83\x27\x2e\x58\x22\xb0\x60\x25\xe0\x33\x62\x90\x0\x20\x8\x6\x88\x5d\xcc\x4\x5b\xb0\x45\x59\x80\xc4\x88\x41\x2\x80\x20\x18\x20\x76\x31\x13\x6c\xc1\x16\x61\xf1\xf\x23\x6\x9\x0\x82\x60\x80\xd8\xc5\x4c\xb0\x5\x5b\x90\x85\x3f\x8c\x18\x24\x0\x8\x82\x1\x62\x17\x33\xc1\x16\x6c\x21\x16\x1\x2\x0\x0\x0\x0\x0";

}