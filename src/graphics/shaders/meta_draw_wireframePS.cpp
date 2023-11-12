#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char meta_draw_wireframePS_dxil_bytes[3317];

auto meta_draw_wireframePS() noexcept -> shader_def
{
   return {
      .name = "meta_draw_wireframePS",
      .entrypoint = L"main",
      .target = L"ps_6_6",
      .file = L"meta_draw_wireframePS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(meta_draw_wireframePS_dxil_bytes),
               sizeof(meta_draw_wireframePS_dxil_bytes) - 1},
   };
}

const char meta_draw_wireframePS_dxil_bytes[3317] = "\x44\x58\x42\x43\x24\xb1\x2e\xb2\xe5\xde\x2d\xc5\xd2\x89\x1b\x3a\xb8\x42\x3d\x3c\x1\x0\x0\x0\xf4\xc\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xe0\x0\x0\x0\x1c\x1\x0\x0\x10\x2\x0\x0\x44\x2\x0\x0\x60\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x2\x0\x0\x0\x0\x0\x49\x53\x47\x31\x8c\x0\x0\x0\x3\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x68\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x6e\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x7a\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\xec\x0\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x3\x1\x0\x3\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x18\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x3\x3\x4\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x1\x2\x44\x0\x3\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x2c\x0\x0\x0\x0\x0\x27\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x74\x61\x5f\x64\x72\x61\x77\x5f\x77\x69\x72\x65\x66\x72\x61\x6d\x65\x50\x53\x2e\x70\x64\x62\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\xc5\x31\x9c\xa4\x3c\x39\x3f\x3b\x1b\x56\x70\x2a\x95\xcf\x77\xc4\x44\x58\x49\x4c\x8c\xa\x0\x0\x66\x0\x0\x0\xa3\x2\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x74\xa\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x9a\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x41\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\xa4\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x80\x20\x88\x62\x20\x64\x8e\x20\x28\x3\x40\x28\xb4\x14\x63\x20\x8\xa2\x28\xa8\x29\xc4\x40\xc\x3\x3d\x65\x18\x88\x81\xa2\xa3\x86\xcb\x9f\xb0\x87\x90\x7c\x6e\xa3\x8a\x95\x98\xfc\xe2\xb6\x11\x31\xc\xc3\x40\xc5\x3d\xc3\xe5\x4f\xd8\x43\x48\x7e\x8\x34\xc3\x42\xa0\x80\x2a\xc4\x44\x54\x4\x59\xb7\xd\x97\x3f\x61\xf\x21\xf9\x2b\x21\x39\x54\x24\x10\x69\xe4\x3c\x44\x34\x21\x84\x84\x4\x82\x28\x44\x45\x54\x18\x65\x7\xd\x97\x3f\x61\xf\x21\xf9\x2b\x21\x6d\x48\x33\x20\x82\x20\x88\xa2\x14\x15\xb1\x11\xa\x71\x3\x1\xc3\x8\xc4\x90\x4\xdf\x30\xc2\x30\xc\x23\x8\xc3\x9d\xc1\xe1\x48\xd3\x2\x60\xe\x35\xf9\xd2\x14\x51\xc2\xe4\x17\x11\xc0\x10\xbf\xe0\x34\xd2\x4\x34\xd3\x1f\x50\x45\x41\x44\xc8\x0\xc3\x41\x14\x87\x9b\xe\x47\x9a\x16\x0\x73\xa8\xc9\x17\x82\xbf\x88\x0\x86\x40\xc1\x48\x64\x3a\x10\x73\x4\xa0\x30\x5\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x2c\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x69\x80\x0\x8\x0\x0\x0\x0\x0\x0\x0\x60\xc8\xf3\x0\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x27\x2\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x21\x8f\x5\x4\xc0\x0\x0\x0\x0\x0\x0\x0\x0\x43\x9e\xc\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x1c\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\x64\x81\x0\x11\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\x90\x3\xca\xa0\x8\xca\x83\x8a\x92\x18\x1\x28\x82\x42\x28\x83\x12\x28\x10\x3a\xb\x12\x10\x90\x80\xcc\xb1\x1c\x86\x8\x4\x2\x40\x10\x0\x0\x0\x0\xa0\x72\x6\x0\x0\x0\x0\x79\x18\x0\x0\x61\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x8\x65\x82\x40\x2c\x1b\x84\x81\x98\x20\x10\xcc\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\x10\xcd\x4\xa1\xc\x36\x2\x13\x4\xc2\xd9\x80\x28\xb\xa3\x28\x43\x3\x6c\x8\x9c\xd\x4\x0\x3c\xc0\x4\xc1\xc\xb4\xd\x41\x34\x41\x10\x0\x12\x6d\x61\x69\x6e\x2c\x86\x9e\x98\x9e\xa4\x26\x8\x85\x34\x41\x28\xa6\xd\x81\x32\x41\x28\xa8\x9\x42\x51\x4d\x10\x88\x67\x83\xb0\x19\x1b\x16\xa5\xb2\x2e\x2c\x1b\x34\xe5\xe2\xb8\x4c\x59\x7d\x41\xbd\xcd\xa5\xd1\xa5\xbd\xb9\x4d\x10\xa\x6b\x83\xb0\x6d\x1b\x96\xc1\xb3\x3e\x4c\x1b\xb4\xe1\x2\x83\x9\x2\x1\xf1\x31\x62\xa\xa2\xfa\x82\x7a\x9a\x4a\xa2\x4a\x7a\x72\x82\x9a\x9a\x20\x14\xd7\x86\x45\xc\xc6\xc0\xba\x30\x32\x18\x34\x31\xb8\x80\xd\x43\x17\x6\x65\xc0\x64\xca\xea\x8b\x2a\x4c\xee\xac\x8c\x6e\x82\x50\x60\x13\x4\x22\xda\x20\x6c\x69\xb0\x61\x51\xce\xc0\x42\x3\xec\x1a\x34\xe5\x52\x83\xd\xc1\x1a\x6c\x18\xcc\x80\xd\x80\x9\xc2\x19\x64\x1b\x4\xc5\xd\x36\x14\x13\xd5\x6\xd0\x1b\x54\x61\x63\xb3\x6b\x73\x49\x23\x2b\x73\xa3\x9b\x12\x4\x55\xc8\xf0\x5c\xec\xca\xe4\xe6\xd2\xde\xdc\xa6\x4\x44\x13\x32\x3c\x17\xbb\x30\x36\xbb\x32\xb9\x29\x81\x51\x87\xc\xcf\x65\xe\x2d\x8c\xac\x4c\xae\xe9\x8d\xac\x8c\x6d\x4a\x80\x94\x21\xc3\x73\x91\x2b\x9b\x7b\xab\x93\x1b\x2b\x9b\x9b\x12\x3c\x75\xc8\xf0\x5c\xec\xd2\xca\xee\x92\xc8\xa6\xe8\xc2\xe8\xca\xa6\x4\x51\x1d\x32\x3c\x97\x32\x37\x3a\xb9\x3c\xa8\xb7\x34\x37\xba\xb9\x29\xc1\x1b\x0\x0\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\x2a\x0\x0\x0\x86\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x18\xc1\x36\x5c\xbe\xf3\xf8\xc0\x34\x45\x48\x40\x4d\x84\x36\xbd\x44\x34\x11\x97\x5f\xdc\xb6\x15\x40\xc3\xe5\x3b\x8f\x1f\x20\xd\x10\x61\x7e\x71\xdb\x76\xb0\xd\x97\xef\x3c\xbe\x10\x50\x45\x41\x44\xa5\x3\xc\x25\x61\x0\x2\xe6\x17\xb7\x6d\x9\xdd\x70\xf9\xce\xe3\xb\x11\x1\x4c\x44\x8\x34\xc3\x42\x7c\x91\xc3\x6c\x48\x33\x20\x8d\x61\x3\xcd\x70\xf9\xce\xe3\xf\x88\x24\x0\xd1\x60\x1\xd3\x70\xf9\xce\xe3\x2f\xe\x30\x88\xcd\x43\x4d\x7e\x71\xdb\x26\x50\xd\x97\xef\x3c\xbe\x34\x39\x11\x81\x52\xd3\x43\x4d\x7e\x71\xdb\x66\xf0\xc\x97\xef\x3c\x3e\xd5\x0\x11\xe6\x17\xb7\x6d\x0\x4\x3\x20\xd\x0\x0\x61\x20\x0\x0\xf7\x0\x0\x0\x13\x4\x43\x2c\x10\x0\x0\x0\x14\x0\x0\x0\xa4\xd4\xc0\x8\x0\x11\x45\x1a\x50\x1a\x5\x3b\x50\xb2\x3\x25\x29\x50\x8\x33\x0\xa5\x50\x48\x65\x57\x30\x65\x54\x2a\x34\x8c\x11\x88\x2c\x28\xe2\xbd\x30\x46\x0\x82\x20\x8\x82\xc3\x18\x1\x8\x82\x20\x8\x6\x23\x0\x63\x4\x20\x8\x82\xf8\x37\x46\x0\x82\x20\x8\x7f\x84\xcd\x21\x7c\xe\x6d\x73\x10\x4d\xd3\x50\x0\x23\x6\x8\x0\x82\x60\xc0\xa9\x81\x14\x98\x81\x35\x62\x70\x0\x20\x8\x6\x19\x1b\x50\xc1\x30\x62\x90\x0\x20\x8\x6\x6\x1d\x48\x6b\x80\x6\x62\x10\x8d\x18\x24\x0\x8\x82\x81\x51\x7\x13\x1b\xa4\xc1\x27\x8d\x18\x24\x0\x8\x82\x81\x61\x7\x94\x1a\xa8\x1\x19\x4c\x23\x6\x9\x0\x82\x60\x60\xdc\x41\xb5\x6\x6b\x10\x6\xd4\x88\x41\x2\x80\x20\x18\x18\x78\x60\xb1\x1\x1b\x94\x41\x35\x62\x90\x0\x20\x8\x6\x8d\x1d\x60\x6a\xd0\x6\x67\x70\x6\x23\x6\x9\x0\x82\x60\xd0\xdc\x41\xb6\x6\x6e\x40\x6\x68\x30\x62\x90\x0\x20\x8\x6\xd\x1e\x68\x6c\xf0\x6\x66\x90\x6\x23\x6\x9\x0\x82\x60\xd0\xe4\xc1\xd6\x6\x70\xa0\x6\x66\x30\x62\x90\x0\x20\x8\x6\x8d\x1e\x70\x6e\x10\x7\x67\x70\x6\x23\x6\x9\x0\x82\x60\xd0\xec\x41\xf7\x6\x72\x90\x6\x68\x30\x62\x90\x0\x20\x8\x6\xd\x1f\x78\x70\x30\x7\x6d\xc0\x6\x23\x6\x9\x0\x82\x60\xd0\xf4\xc1\x17\x7\x74\xa0\x6\x6d\x30\x62\x90\x0\x20\x8\x6\x8d\x1f\x80\x81\x1c\xd4\x1\x1b\xb8\x81\x25\x7\x7d\x2c\x41\xe8\x33\x62\x70\x0\x20\x8\x6\x96\x1f\x78\x11\x18\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x68\x2\x31\xd8\xb1\xc1\xc7\x82\x4d\x3e\x16\x18\xf2\xb1\xa0\x80\x8f\x7d\x4a\x7c\x2c\xf8\xe4\x63\x41\x22\x1f\xb\x10\xf8\x58\x35\xd1\xc7\x2a\x8a\x3e\x26\x94\x1\x7c\x2c\x28\x3\xf9\x58\x0\xc9\xc7\x82\x7\x3e\x96\x6\x45\x7c\x2c\x48\x3\xf9\x58\x30\xc9\xc7\x2\x9\x3e\xc6\x69\xf4\x31\x6e\xa3\x8f\x9\x6f\x0\x1f\xb\xde\x40\x3e\x16\x68\xf2\xb1\x20\x83\x8f\xcd\x41\x11\x1f\xb\xe6\x40\x3e\x16\x74\xf2\xb1\x80\x83\x8f\x39\x58\x7c\x2c\xb8\x3\xf9\x58\x18\x4\xf2\xb1\x80\xe\xe4\x63\x16\x18\xc4\xc7\x82\x3d\x90\x8f\x99\x41\x20\x1f\xb\xee\x40\x3e\x76\x75\xf2\x31\x4a\xc\xe4\x63\x42\x10\x1f\xb\x88\xf8\x58\x90\xc0\x67\xc4\xc0\x0\x40\x10\xc\xa2\x76\x40\x87\xc0\x18\x46\x3e\x96\x24\xf2\x31\x21\x80\xcf\x88\x81\x1\x80\x20\x18\x44\xf1\xf0\xa\x81\x15\x1\x7d\xec\xa\x83\xf8\x58\xa0\xa\xf2\x31\x3a\x8\xe4\x63\xc1\x29\xc8\xc7\xbe\x34\x88\x8f\x5\xae\x20\x1f\xcb\x83\x40\x3e\x16\xa8\x82\x7c\xc\xc\xcc\x40\x3e\xd6\xad\x81\x7c\x4c\x8\xe2\x63\x1\x11\x1f\xb\x12\xf8\x8c\x18\x18\x0\x8\x82\x41\x4\x12\xfb\x10\x18\xc3\xc8\xc7\x92\x44\x3e\x26\x4\xf0\x19\x31\x30\x0\x10\x4\x83\x88\x24\xc4\x21\xb0\x22\xa0\x8f\xf9\x81\x1a\xc4\xc7\x82\x5e\x90\x8f\x9d\x42\x20\x1f\xb\x74\x41\x3e\x66\xa\x72\x10\x1f\xb\xc2\x41\x3e\xc6\xa\x81\x7c\x2c\xe8\x5\xf9\xd8\x1b\x9c\x82\x7c\x8c\xe\x48\x41\x3e\x26\x4\xf1\xb1\x80\x88\x8f\x5\x9\x7c\x46\xc\xc\x0\x4\xc1\x20\x9a\x9\x97\x8\x8c\x61\xe4\x63\x49\x22\x1f\x13\x2\xf8\x8c\x18\x18\x0\x8\x82\x41\x74\x13\xf5\x10\x58\x11\xd0\x67\xc4\xe0\x0\x40\x10\xc\x1e\x9d\xc8\x87\x33\xa0\x46\xc\xe\x0\x4\xc1\xe0\xd9\x9\x7d\x8\x84\x11\x83\x3\x0\x41\x30\xb0\x74\x42\x1f\xda\xc1\x26\x46\x13\x2\xc0\x82\x79\x90\x8f\x11\xf5\x0\x1f\xb\x84\xf8\x8c\x18\x1c\x0\x8\x82\xc1\x13\x16\xfd\x10\xe0\xc3\x88\xc1\x1\x80\x20\x18\x3c\x62\x11\x12\x81\x3e\x58\x10\xc8\xc7\x82\x7e\x90\xcf\x88\x81\x1\x80\x20\x18\x44\x64\x1\x12\x81\x5\x20\x1\x9f\xe1\x88\xc0\x1f\x82\x6f\x96\x21\x10\x82\x11\x3\x3\x0\x41\x30\x58\xd4\xe2\x25\x62\x62\x96\x40\x18\x31\x38\x0\x10\x4\x3\xcb\x2c\x4c\x22\x1f\xc4\x62\x34\x21\x0\x2c\xf8\x7\xf9\x18\x13\xc4\x67\xc4\xe0\x0\x40\x10\xc\x1e\xb6\x40\x89\x60\x24\x46\xc\xe\x0\x4\xc1\xe0\x69\xb\x96\x8\x4a\xc2\x82\x40\x3e\x16\xa0\x84\x7c\x46\xc\xc\x0\x4\xc1\x20\x7a\x8b\x95\x8\x2c\x58\x9\xf8\x8c\x18\x24\x0\x8\x82\x1\x52\x17\x32\xa1\x16\x6a\x41\x16\x20\x31\x62\x90\x0\x20\x8\x6\x48\x5d\xc8\x84\x5a\xa8\x5\x58\xfc\xc3\x88\x41\x2\x80\x20\x18\x20\x75\x21\x13\x6a\xa1\x16\x63\xe1\xf\x23\x6\x9\x0\x82\x60\x80\xd4\x85\x4c\xa8\x85\x5a\x84\x45\x80\x0\x0\x0\x0\x0\x0";

}