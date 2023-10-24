#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char ai_overlay_shapeVS_dxil_bytes[3025];

auto ai_overlay_shapeVS() noexcept -> shader_def
{
   return {
      .name = "ai_overlay_shapeVS",
      .entrypoint = L"main",
      .target = L"vs_6_6",
      .file = L"ai_overlay_shapeVS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(ai_overlay_shapeVS_dxil_bytes),
               sizeof(ai_overlay_shapeVS_dxil_bytes) - 1},
   };
}

const char ai_overlay_shapeVS_dxil_bytes[3025] = "\x44\x58\x42\x43\x6a\x7\x38\x4b\x28\x3b\x62\x3b\xd6\x8b\x7c\xc4\xa\x7\x19\x55\x1\x0\x0\x0\xd0\xb\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xb4\x0\x0\x0\xf0\x0\x0\x0\xd0\x1\x0\x0\x4\x2\x0\x0\x20\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x60\x0\x0\x0\x2\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x48\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x51\x0\x0\x0\x0\x0\x0\x0\x8\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\x1\x1\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x53\x56\x5f\x49\x6e\x73\x74\x61\x6e\x63\x65\x49\x44\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x50\x53\x56\x30\xd8\x0\x0\x0\x30\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x1\x0\x0\x0\x2\x1\x0\x2\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x5\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\xc\x0\x0\x0\x0\x0\x0\x0\xc\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x41\x2\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x3\x3\x4\x0\x0\xf\x0\x0\x0\xf\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x2c\x0\x0\x0\x0\x0\x24\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x61\x69\x5f\x6f\x76\x65\x72\x6c\x61\x79\x5f\x73\x68\x61\x70\x65\x56\x53\x2e\x70\x64\x62\x0\x0\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x83\xf7\xd6\x7f\xd7\x65\x39\x4a\xa4\x67\x28\xb1\x56\xb6\x51\x27\x44\x58\x49\x4c\xa8\x9\x0\x0\x66\x0\x1\x0\x6a\x2\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x90\x9\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x61\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x62\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\xa8\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x82\x20\x88\x82\x20\xa4\x18\x0\x41\x10\xc5\x40\xca\x4d\xc3\xe5\x4f\xd8\x43\x48\xfe\x4a\x48\x2b\x31\xf9\xc5\x6d\xa3\x62\x18\x86\x81\xa0\xe2\x9e\xe1\xf2\x27\xec\x21\x24\x3f\x4\x9a\x61\x21\x50\xd0\x94\x83\x21\x1c\x82\x28\x8\x7a\x8e\x1a\x2e\x7f\xc2\x1e\x42\xf2\xb9\x8d\x2a\x56\x62\xf2\x8b\xdb\x46\xc4\x30\xc\x43\x21\x22\xc2\x21\x48\x2a\xc5\x40\xc\xc3\x40\xd4\x6d\xc3\xe5\x4f\xd8\x43\x48\xfe\x4a\x48\xe\x15\x9\x44\x1a\x39\xf\x11\x4d\x8\x21\x21\x81\x20\xa\xe1\x10\x8e\x45\xd7\x41\xc3\xe5\x4f\xd8\x43\x48\xfe\x4a\x48\x1b\xd2\xc\x88\x20\x8\xa2\x98\x23\x8\x4a\xe1\x10\x19\xa1\xd1\x36\x10\x30\x8c\x40\xc\x49\xe0\x9d\x18\x1c\x8e\x34\x2d\x0\xe6\x50\x93\x2f\x4d\x11\x25\x4c\x3e\x43\x4c\xc0\x3f\x44\x80\xf5\x3b\x41\x42\x8\x13\xa\xbe\x99\xe6\x0\x1d\xde\x61\x1e\xe8\x81\x1d\xc2\x21\x1f\xde\xa1\x1e\xe8\xc1\xd\xc6\x81\x1d\xc2\x61\x1e\xe6\xc1\xd\x66\x81\x1e\xe4\xa1\x1e\xc6\x81\x1e\xea\x41\x1e\xca\x81\x1c\x44\xa1\x1e\xcc\xc1\x1c\xca\x41\x1e\xf8\xa0\x1d\xca\x81\x1e\xc2\xe1\x17\xc8\x41\x1e\xc2\xe1\x1e\x7e\xe1\x1d\xc4\x41\x1d\xca\x61\x1c\xe8\xc1\xf\x50\x0\x52\x38\x8c\x30\xc\xc3\x8\xc2\x70\x67\x70\x38\xd2\xb4\x0\x98\x43\x4d\xbe\x34\x45\x94\x30\xf9\x45\x4\x30\xc4\x2f\x38\x8d\x34\x1\xcd\xf4\x7\x54\x51\x10\x11\x32\xbe\x71\x20\xc9\xe1\xa6\xc3\x91\xa6\x5\xc0\x1c\x6a\xf2\x85\xe0\x2f\x22\x80\x21\x50\x50\x92\x99\x7\xc4\x1c\x1\x28\x0\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x2\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x28\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x79\x80\x0\x18\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x23\x1\x1\x30\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x21\xcf\x5\x4\x40\x0\x0\x0\x0\x0\x0\x0\x0\x43\x9e\xd\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\xb2\x40\x0\x0\x10\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\x30\xa\x30\xa0\x90\x3\xca\xa0\x3c\xa8\x28\x89\x11\x80\x32\x28\x85\x22\x28\x81\x42\xa0\xb4\x40\xe8\x9c\x1\x20\x74\xac\xa3\x20\x9e\xe7\x1\x1e\x12\x67\x0\x0\x0\x79\x18\x0\x0\x62\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x88\x64\x82\x40\x28\x1b\x84\x81\x98\x20\x10\xcb\x6\x61\x30\x28\xd8\xcd\x6d\x18\x10\x82\x98\x20\x10\xcc\x4\x41\xc\x38\x2\x13\x4\xa2\x99\x20\x10\xce\x6\x61\x70\x36\x24\xca\xc2\x28\xc3\xd0\x28\xcf\x86\x0\x9a\x20\x9c\x81\x36\x41\x20\x9e\xd\x88\x22\x31\x8a\x32\x4c\xc0\x86\x80\xda\x40\x44\x40\x5\x4c\x10\xd0\x60\xdb\x10\x5c\x13\x4\x1\x20\xd1\x16\x96\xe6\x46\x4\xea\x69\x2a\x89\x2a\xe9\xc9\x69\x82\x50\x48\x13\x84\x62\xda\x10\x28\x13\x84\x82\x9a\x20\x10\xd0\x6\x1\xc\x8c\xd\x8b\xb2\x71\x9d\xd7\xd\x9f\xd2\x85\x1\x9b\x29\xab\xaf\x24\xb7\x39\xba\x30\xb7\xb1\xb2\x24\xa2\x9\x42\x51\x4d\x10\xa\x6b\x82\x50\x5c\x1b\x4\x30\x18\x36\x2c\xc3\x18\x90\x41\x19\x78\xdd\x60\x6\x43\x77\x6\x1b\x4\x31\x40\x3\x2e\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x13\x84\x2\x9b\x20\x10\xd1\x6\x1\xc\xd8\x60\xc3\xa2\xa8\x1\xf7\x79\x6b\x30\xac\x81\xd2\xb5\xc1\x86\xc0\xd\x36\xc\x69\xf0\x6\xc0\x4\x21\xd\xb2\xd\x82\x12\x7\x1b\x8a\x4c\x83\x3\x4b\xe\xaa\xb0\xb1\xd9\xb5\xb9\xa4\x91\x95\xb9\xd1\x4d\x9\x82\x2a\x64\x78\x2e\x76\x65\x72\x73\x69\x6f\x6e\x53\x2\xa2\x9\x19\x9e\x8b\x5d\x18\x9b\x5d\x99\xdc\x94\xc0\xa8\x43\x86\xe7\x32\x87\x16\x46\x56\x26\xd7\xf4\x46\x56\xc6\x36\x25\x40\xca\x90\xe1\xb9\xc8\x95\xcd\xbd\xd5\xc9\x8d\x95\xcd\x4d\x9\xaa\x3a\x64\x78\x2e\x76\x69\x65\x77\x49\x64\x53\x74\x61\x74\x65\x53\x82\xab\xe\x19\x9e\x4b\x99\x1b\x9d\x5c\x1e\xd4\x5b\x9a\x1b\xdd\xdc\x94\x40\xe\x0\x79\x18\x0\x0\x4c\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x0\x0\x0\x71\x20\x0\x0\x27\x0\x0\x0\x76\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x58\xc1\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x21\x74\xc3\xe5\x3b\x8f\x2f\x44\x4\x30\x11\x21\xd0\xc\xb\xf1\x45\xe\xb3\x21\xcd\x80\x34\x86\x5\x4c\xc3\xe5\x3b\x8f\xbf\x38\xc0\x20\x36\xf\x35\xf9\xc5\x6d\x9b\xc0\x34\x5c\xbe\xf3\xf8\x8b\x3\xc\x62\xf3\x50\x93\x8f\xdc\xb6\x11\x5c\xc3\xe5\x3b\x8f\x1f\x1\xd6\x46\x15\x5\x11\x95\xe\x30\xf8\xc5\x6d\xdb\x40\x35\x5c\xbe\xf3\xf8\xd2\xe4\x44\x4\x4a\x4d\xf\x35\xf9\xc5\x6d\x9b\x81\x34\x5c\xbe\xf3\xf8\x13\x11\x4d\x8\x10\x61\x7e\x71\xdb\x6\x40\x30\x0\xd2\x0\x61\x20\x0\x0\xa8\x0\x0\x0\x13\x4\x41\x2c\x10\x0\x0\x0\xe\x0\x0\x0\x44\x14\x57\x21\xcc\x0\x94\xa5\x40\xd9\x95\x42\xc9\xe\x14\xec\x40\x69\x14\x26\x42\x81\x14\x50\x81\x15\x1\x15\xe5\x41\xda\x8\x0\x59\x73\x8\x66\xe0\x50\x36\x7\x91\x24\xcc\x34\x7\xc1\x30\x8c\x45\xd6\x1c\x42\x19\x3c\x0\x23\x6\x8\x0\x82\x60\xb0\xa5\x81\x33\xa0\x41\x31\x62\x80\x0\x20\x8\x6\x9b\x1a\x3c\x83\x19\x18\x23\x6\x7\x0\x82\x60\x70\xb1\xc1\x13\xc\x23\x6\x9\x0\x82\x60\x80\xc8\x81\xb5\x6\x68\x10\x6\xd5\x88\x41\x2\x80\x20\x18\x18\x74\x70\xa5\x41\x1a\x88\x81\x35\x62\x90\x0\x20\x8\x6\x46\x1d\x60\x6a\xa0\x6\xdf\x35\x62\x90\x0\x20\x8\x6\x86\x1d\x64\x6b\xb0\x6\x61\x80\x8d\x18\x1c\x0\x8\x82\xc1\x15\x7\xd4\xb1\x8c\x18\x28\x0\x8\x82\xc1\x53\x7\x59\x50\xb4\x81\xb3\x8d\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x88\x81\x2\x80\x20\x18\x3c\x79\xd0\x15\x89\x25\x7d\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x30\x62\xa0\x0\x20\x8\x6\x4f\x1f\x84\x41\xd2\x64\xd6\x18\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x88\x81\x2\x80\x20\x18\x3c\xa1\x50\x6\x4d\xc4\x69\x67\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xf6\x50\xf2\x19\x31\x40\x0\x10\x4\x3\x8a\x14\xda\x80\xa1\x82\x11\x3\x4\x0\x41\x30\xa0\x4a\xc1\xd\x12\x2a\xb0\xc0\x80\x8e\x49\x98\x7c\x46\xc\x10\x0\x4\xc1\x80\x42\x85\x38\x78\xb0\x60\xc4\x0\x1\x40\x10\xc\xa8\x54\x90\x3\x6\xb\x2c\x48\xa0\x63\x15\x27\x9f\x11\x3\x4\x0\x41\x30\xa0\x58\xa1\xe\x24\x2e\x18\x31\x40\x0\x10\x4\x3\xaa\x15\xec\xe0\xe1\x2\xb\x18\xe8\x8c\x18\x1c\x0\x8\x82\x81\x4\xb\x74\x20\x6\xa9\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xa3\x9\xc4\x30\x62\x70\x0\x20\x8\x6\x52\x2d\xe4\xc1\x19\xc4\xc2\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\xc3\x88\xc1\x1\x80\x20\x18\x48\xba\xe0\x7\x6c\x50\x7\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x30\x9a\x40\xc\x23\x6\x7\x0\x82\x60\x20\xfd\xc2\x28\xc4\x1\x2d\x8c\x26\x4\xc0\x68\x82\x10\x8c\x26\xc\xc2\x68\x2\x31\xd8\xd4\xc9\x67\xc4\x0\x1\x40\x10\xc\x28\x72\x68\x85\x47\xb\x46\xc\x10\x0\x4\xc1\x80\x2a\x7\x57\x58\xae\xc0\x82\x3\x3a\x66\x85\x81\x7c\x46\xc\x10\x0\x4\xc1\x80\x42\x87\x58\x90\xbc\x60\xc4\x0\x1\x40\x10\xc\xa8\x74\x90\x5\x67\xb\x2c\x50\xa0\x63\x59\x19\xc8\x67\xc4\x0\x1\x40\x10\xc\x28\x76\xa8\x85\x4a\xc\x82\x11\x3\x4\x0\x41\x30\xa0\xda\xc1\x16\xa2\x2f\xb0\xa0\x81\x8e\x71\x69\x20\x9f\x11\x3\x4\x0\x41\x30\xa0\xe0\x21\x17\x30\x33\x8\x46\xc\x10\x0\x4\xc1\x80\x8a\x7\x5d\xa0\xc6\x20\xb0\x0\x82\xce\x88\x41\x2\x80\x20\x18\x28\xf6\x70\xb\xed\xd0\xe\xe6\xd0\x8c\x18\x24\x0\x8\x82\x81\x62\xf\xb7\xd0\xe\xed\x20\xe\xc9\x88\x41\x2\x80\x20\x18\x28\xf6\x70\xb\xed\xd0\xe\xe3\x50\x8c\x18\x24\x0\x8\x82\x81\x62\xf\xb7\xd0\xe\xed\x50\xe\x1\x2\x0\x0\x0\x0\x0";

}