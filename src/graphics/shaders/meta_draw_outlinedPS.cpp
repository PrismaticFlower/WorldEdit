#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char meta_draw_outlinedPS_dxil_bytes[3377];

auto meta_draw_outlinedPS() noexcept -> shader_def
{
   return {
      .name = "meta_draw_outlinedPS",
      .entrypoint = L"main",
      .target = L"ps_6_6",
      .file = L"meta_draw_outlinedPS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(meta_draw_outlinedPS_dxil_bytes),
               sizeof(meta_draw_outlinedPS_dxil_bytes) - 1},
   };
}

const char meta_draw_outlinedPS_dxil_bytes[3377] = "\x44\x58\x42\x43\x5c\x81\xd2\xfe\xa6\x84\xbf\xf9\x63\x35\x43\x23\x34\x31\xbf\xf0\x1\x0\x0\x0\x30\xd\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xc\x1\x0\x0\x48\x1\x0\x0\x6c\x2\x0\x0\xa0\x2\x0\x0\xbc\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x2\x0\x0\x0\x0\x0\x49\x53\x47\x31\xb8\x0\x0\x0\x4\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x88\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x94\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x9a\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\xf\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x43\x4f\x4c\x4f\x52\x0\x4f\x55\x54\x4c\x49\x4e\x45\x5f\x43\x4f\x4c\x4f\x52\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\x1c\x1\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x4\x1\x0\x4\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x4f\x55\x54\x4c\x49\x4e\x45\x5f\x43\x4f\x4c\x4f\x52\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x3\x3\x4\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x0\x3\x2\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x1\x2\x44\x0\x3\x2\x0\x0\x15\x0\x0\x0\x0\x0\x0\x0\x1\x3\x44\x0\x3\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x2c\x0\x0\x0\x0\x0\x26\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x74\x61\x5f\x64\x72\x61\x77\x5f\x6f\x75\x74\x6c\x69\x6e\x65\x64\x50\x53\x2e\x70\x64\x62\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x3c\xe3\xf5\x61\x97\xfd\x6e\x95\x12\x10\x94\xdb\x84\xa0\x9c\xce\x44\x58\x49\x4c\x6c\xa\x0\x0\x66\x0\x0\x0\x9b\x2\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x54\xa\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x92\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x40\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\x98\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x80\x20\x88\x62\x20\xa4\x18\x3\x41\x10\x45\x41\x4a\x21\x6\x62\x18\x88\x29\xc3\x40\xc\xe4\x1c\x35\x5c\xfe\x84\x3d\x84\xe4\x73\x1b\x55\xac\xc4\xe4\x17\xb7\x8d\x88\x61\x18\x6\x2a\xee\x19\x2e\x7f\xc2\x1e\x42\xf2\x43\xa0\x19\x16\x2\x5\x51\x21\x20\x42\x22\x68\xba\x6d\xb8\xfc\x9\x7b\x8\xc9\x5f\x9\xc9\xa1\x22\x81\x48\x23\xe7\x21\xa2\x9\x21\x24\x24\x10\x44\x21\x24\x42\xaa\xc8\x3a\x68\xb8\xfc\x9\x7b\x8\xc9\x5f\x9\x69\x43\x9a\x1\x11\x4\x41\x14\x73\x4\x41\x29\x24\x2\x23\x32\xd2\x6\x2\x86\x11\x88\x21\x9\xba\x61\x84\x61\x18\x46\x10\x86\x3b\x83\xc3\x91\xa6\x5\xc0\x1c\x6a\xf2\xa5\x29\xa2\x84\xc9\x2f\x22\x80\x21\x7e\xc1\x69\xa4\x9\x68\xa6\x3f\xa0\x8a\x82\x88\x90\xe1\x7d\x3\x8\xe\x37\x1d\x8e\x34\x2d\x0\xe6\x50\x93\x2f\x4\x7f\x11\x1\xc\x81\x82\x90\xc4\x94\x20\xe6\x8\x40\x1\x0\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x28\x40\x0\x4\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x61\x80\x0\x8\x0\x0\x0\x0\x0\x0\x0\x60\xc8\xe3\x0\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x67\x2\x2\x60\x0\x0\x0\x0\x0\x0\x0\x80\x21\x8f\x5\x4\x40\x0\x0\x0\x0\x0\x0\x0\x0\x43\x1e\xd\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\xb2\x40\x0\x0\x12\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\x90\x3\xca\xa0\x8\xca\x83\x8a\x92\x28\x83\x42\x18\x1\x28\x82\x12\x28\x10\x2a\xb\x10\x10\x90\x80\xc8\xb1\x24\x88\x38\xe\x0\x8\x4\x2\x8\x4\x2\x0\x0\x0\xa0\x71\x6\x0\x0\x0\x0\x0\x79\x18\x0\x0\x68\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x88\x64\x82\x40\x28\x1b\x84\x81\x98\x20\x10\xcb\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\x10\xcc\x4\x61\xc\x34\x2\x13\x4\xa2\xd9\x80\x28\xb\xa3\x28\x43\x3\x6c\x8\x9c\xd\x4\x0\x3c\xc0\x4\x81\xc\xb2\xd\x41\x34\x41\x10\x0\x12\x6d\x61\x69\x6e\x5c\xa6\xac\xbe\xa0\xde\xe6\xd2\xe8\xd2\xde\xdc\x26\x8\x45\x34\x41\x28\xa4\xd\x81\x32\x41\x28\xa6\x9\x42\x41\x4d\x10\x8\x67\x83\xb0\x6d\x1b\x16\xa5\xb2\x2e\x2c\x1b\x32\x45\xe3\x58\xc\x3d\x31\x3d\x49\x4d\x10\x8a\x6a\x83\xb0\x19\x1b\x96\xc1\xb3\x34\xec\x1b\xb2\x41\x3\x83\x9\x2\xf1\xb0\x79\xaa\xa2\x62\x4a\x72\x2a\xfa\x1a\x7a\x62\x7a\x92\xda\xb0\x88\xc1\x18\x58\x1a\xf6\xd\x99\x18\x68\x60\xc0\xc7\x88\x29\x88\xea\xb\xea\x69\x2a\x89\x2a\xe9\xc9\x9\x6a\x6a\x82\x50\x58\x1b\x96\xad\xc\x2c\xd\x33\x83\x21\xdb\x34\x60\x3\xd1\x85\x1\x19\x9c\x1\x93\x29\xab\x2f\xaa\x30\xb9\xb3\x32\xba\x9\x42\x71\x4d\x10\x8\x68\x83\xb0\xad\xc1\x86\x45\x49\x3\x4b\xd\x30\x6d\xc8\x14\x8d\xd\x36\x4\x6d\xb0\x61\x40\x3\x37\x0\x26\x8\x65\x80\x6d\x10\x14\x38\xd8\x50\x4c\xd4\x1b\x40\x71\x50\x85\x8d\xcd\xae\xcd\x25\x8d\xac\xcc\x8d\x6e\x4a\x10\x54\x21\xc3\x73\xb1\x2b\x93\x9b\x4b\x7b\x73\x9b\x12\x10\x4d\xc8\xf0\x5c\xec\xc2\xd8\xec\xca\xe4\xa6\x4\x46\x1d\x32\x3c\x97\x39\xb4\x30\xb2\x32\xb9\xa6\x37\xb2\x32\xb6\x29\x1\x52\x86\xc\xcf\x45\xae\x6c\xee\xad\x4e\x6e\xac\x6c\x6e\x4a\xf0\xd4\x21\xc3\x73\xb1\x4b\x2b\xbb\x4b\x22\x9b\xa2\xb\xa3\x2b\x9b\x12\x44\x75\xc8\xf0\x5c\xca\xdc\xe8\xe4\xf2\xa0\xde\xd2\xdc\xe8\xe6\xa6\x4\x71\x0\x0\x0\x0\x79\x18\x0\x0\x4c\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x0\x0\x0\x71\x20\x0\x0\x27\x0\x0\x0\x76\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\xd8\xc0\x36\x5c\xbe\xf3\xf8\xc0\x34\x45\x48\x40\x4d\x84\x36\xbd\x44\x34\x11\x97\x5f\xdc\xb6\x11\x40\xc3\xe5\x3b\x8f\x1f\x20\xd\x10\x61\x7e\x71\xdb\x66\xb0\xd\x97\xef\x3c\xbe\x10\x50\x45\x41\x44\xa5\x3\xc\x25\x61\x0\x2\xe6\x17\xb7\x6d\x8\xdd\x70\xf9\xce\xe3\xb\x11\x1\x4c\x44\x8\x34\xc3\x42\x7c\x91\xc3\x6c\x48\x33\x20\x8d\x61\x1\xd3\x70\xf9\xce\xe3\x2f\xe\x30\x88\xcd\x43\x4d\x7e\x71\xdb\x26\x50\xd\x97\xef\x3c\xbe\x34\x39\x11\x81\x52\xd3\x43\x4d\x7e\x71\xdb\x56\xf0\xc\x97\xef\x3c\x3e\xd5\x0\x11\xe6\x17\xb7\x6d\x0\x4\x3\x20\xd\x0\x0\x61\x20\x0\x0\xf3\x0\x0\x0\x13\x4\x41\x2c\x10\x0\x0\x0\x14\x0\x0\x0\x94\x8d\x0\x10\x51\x2a\x65\x54\x1a\x5\x3b\x50\xb2\x3\x25\x29\x50\x8\x33\x0\xa5\x50\x30\x85\x54\x76\x34\x8c\x11\x88\x2c\x28\xe2\xbd\x30\x46\x0\x82\x20\x8\x82\xc3\x18\x1\x8\x82\x20\x8\x6\x23\x0\x63\x4\x20\x8\x82\xf8\x47\xd5\x1c\x82\xd7\x10\x36\x7\xc1\x30\x4c\x45\xc3\x18\x1\x8\x82\x20\xfc\x1\x0\x0\x0\x23\x6\x8\x0\x82\x60\xa0\xa1\x1\x24\x90\x41\x35\x62\x70\x0\x20\x8\x6\x96\x1a\x48\x1\x31\x62\x90\x0\x20\x8\x6\x46\x1c\x40\x63\x60\x6\xde\x33\x62\x90\x0\x20\x8\x6\x86\x1c\x44\x64\x70\x6\x1d\x34\x62\x90\x0\x20\x8\x6\xc6\x1c\x48\x65\x80\x6\x5f\x34\x62\x90\x0\x20\x8\x6\x6\x1d\x4c\x6c\x90\x6\x61\x20\x8d\x18\x24\x0\x8\x82\x81\x51\x7\x54\x1b\xa8\x1\x18\x4c\x23\x6\x9\x0\x82\x60\x60\xd8\x41\xe5\x6\x6b\x20\x6\xd4\x88\x41\x2\x80\x20\x18\x18\x77\x60\xb1\x1\x1b\x90\x41\x35\x62\x90\x0\x20\x8\x6\x6\x1e\x5c\x6d\xd0\x6\x63\x60\x8d\x18\x24\x0\x8\x82\x81\x72\x7\x19\x1b\xb8\x81\x19\x98\xc1\x88\x41\x2\x80\x20\x18\x28\x78\xa0\xb5\xc1\x1b\x94\xc1\x19\x8c\x18\x24\x0\x8\x82\x81\x92\x7\x9b\x1b\xc0\x81\x1a\xa0\xc1\x88\x41\x2\x80\x20\x18\x28\x7a\xc0\xbd\x41\x1c\xa4\xc1\x19\x8c\x18\x24\x0\x8\x82\x81\xb2\x7\x1d\x1c\xc8\x1\x1a\xa0\xc1\x88\x41\x2\x80\x20\x18\x28\x7c\xe0\xc5\xc1\x1c\xb4\x41\x1a\x8c\x18\x24\x0\x8\x82\x81\xd2\x7\x9f\x1c\xd0\x1\x1b\xac\xc1\x88\x41\x2\x80\x20\x18\x28\x7e\x0\x6\x73\x50\x7\x6b\xc0\x6\x23\x6\x9\x0\x82\x60\xa0\xfc\x41\x18\xd0\x81\x1d\xc0\x41\x1b\x58\x72\xd0\xc7\x12\x84\x3e\x23\x6\x7\x0\x82\x60\x30\xfd\x81\x47\x85\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\x83\x1d\x1e\x7c\x2c\xe0\xe4\x63\x81\x21\x1f\xb\xa\xf8\x98\x18\x28\xf1\xb1\x0\xc\xe4\x63\x41\x22\x1f\xb\x10\xf8\x58\x35\xd1\xc7\x2a\x8a\x3e\x26\xa0\x1\x7c\x2c\x30\x3\xf9\x58\x0\xc9\xc7\x82\x7\x3e\xc6\x6\x45\x7c\x2c\x50\x3\xf9\x58\x30\xc9\xc7\x2\x9\x3e\xc6\x69\xf4\x31\x6e\xa3\x8f\x9\x72\x0\x1f\xb\xe0\x40\x3e\x16\x68\xf2\xb1\x20\x83\x8f\xd9\x41\x11\x1f\xb\xe8\x40\x3e\x16\x74\xf2\xb1\x80\x83\x8f\x39\x58\x7c\x2c\xc0\x3\xf9\x58\x18\x4\xf2\xb1\x20\xe\xe4\x63\x16\x18\xc4\xc7\x2\x3e\x90\x8f\x99\x41\x20\x1f\xb\xe8\x40\x3e\x76\x75\xf2\x31\x4a\xc\xe4\x63\x42\x10\x1f\xb\x88\xf8\x58\x90\xc0\x67\xc4\xc0\x0\x40\x10\xc\x1c\x77\x48\x87\xc0\x18\x46\x3e\x96\x24\xf2\x31\x21\x80\xcf\x88\x81\x1\x80\x20\x18\x38\xf2\x20\xb\x81\x15\x1\x7d\xec\xa\x83\xf8\x58\xb0\xa\xf2\x31\x3a\x8\xe4\x63\x1\x29\xc8\xc7\xbe\x34\x88\x8f\x5\xaf\x20\x1f\xcb\x83\x40\x3e\x16\x9c\x82\x7c\xc\xc\xcc\x40\x3e\xd6\xad\x81\x7c\x4c\x8\xe2\x63\x1\x11\x1f\xb\x12\xf8\x8c\x18\x18\x0\x8\x82\x81\x13\x12\xfc\x10\x18\xc3\xc8\xc7\x92\x44\x3e\x26\x4\xf0\x19\x31\x30\x0\x10\x4\x3\xa7\x24\xca\x21\xb0\x22\xa0\x8f\xf9\x81\x1a\xc4\xc7\x2\x5f\x90\x8f\x9d\x42\x20\x1f\xb\x6e\x41\x3e\x66\xa\x72\x10\x1f\xb\xc4\x41\x3e\xc6\xa\x81\x7c\x2c\xd0\x5\xf9\xd8\x1b\x9c\x82\x7c\x8c\xe\x48\x41\x3e\x26\x4\xf1\xb1\x80\x88\x8f\x5\x9\x7c\x46\xc\xc\x0\x4\xc1\xc0\xa1\x89\x97\x8\x8c\x61\xe4\x63\x49\x22\x1f\x13\x2\xf8\x8c\x18\x18\x0\x8\x82\x81\x83\x13\xf8\x10\x58\x11\xd0\x67\xc4\xe0\x0\x40\x10\xc\x98\x9d\xc8\x87\x33\xa0\x46\xc\xe\x0\x4\xc1\x80\xe1\x9\x7d\x8\x84\x11\x83\x3\x0\x41\x30\x98\x76\x42\x1f\xe0\xe1\x26\x46\x13\x2\xc0\x2\x7a\x90\x8f\x11\xf8\x0\x1f\xb\x84\xf8\x8c\x18\x1c\x0\x8\x82\x1\x23\x16\x29\x11\xec\xc3\x88\xc1\x1\x80\x20\x18\x30\x63\x11\x12\x41\x3f\x58\x10\xc8\xc7\x2\x90\x90\xcf\x88\x81\x1\x80\x20\x18\x38\x65\xe1\x12\x81\x5\x23\x1\x1f\xd3\x87\x7b\x88\x8f\xe9\xc3\x3d\xc4\xc7\xf4\xe1\x1e\xe2\x63\xc4\x20\x1f\x2b\x6\xf9\x98\x31\xc8\xc7\x86\x7e\x80\x8f\xd\xfd\x0\x1f\x1b\xfa\x1\x3e\x23\x6\x9\x0\x82\x60\x80\xcc\x45\x4c\xa4\x45\x5a\x84\xc5\x30\x62\x90\x0\x20\x8\x6\xc8\x5c\xc4\x44\x5a\xa4\xc5\x4f\x8\x23\x6\x9\x0\x82\x60\x80\xcc\x45\x4c\xa4\x45\x5a\x80\x45\x30\x62\x90\x0\x20\x8\x6\xc8\x5c\xc4\x44\x5a\xa4\xc5\x58\xa4\x4\x2\x0\x0\x0\x0\x0";

}