#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char ai_overlay_applyPS_dxil_bytes[1957];

auto ai_overlay_applyPS() noexcept -> shader_def
{
   return {
      .name = "ai_overlay_applyPS",
      .entrypoint = L"main",
      .target = L"ps_6_6",
      .file = L"ai_overlay_applyPS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(ai_overlay_applyPS_dxil_bytes),
               sizeof(ai_overlay_applyPS_dxil_bytes) - 1},
   };
}

const char ai_overlay_applyPS_dxil_bytes[1957] = "\x44\x58\x42\x43\xd0\xcc\x3c\xb6\x6e\x39\x52\xf4\x3f\x82\xef\x4d\x86\xa2\xd1\x9\x1\x0\x0\x0\xa4\x7\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x88\x0\x0\x0\xc4\x0\x0\x0\x64\x1\x0\x0\x98\x1\x0\x0\xb4\x1\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\x98\x0\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x1\x1\x0\x1\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x3\x3\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x2c\x0\x0\x0\x0\x0\x24\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x61\x69\x5f\x6f\x76\x65\x72\x6c\x61\x79\x5f\x61\x70\x70\x6c\x79\x50\x53\x2e\x70\x64\x62\x0\x0\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x4a\xb0\x35\x19\x6a\xad\xe1\x7\x6c\x37\xa7\x14\x43\xf9\x7\xb7\x44\x58\x49\x4c\xe8\x5\x0\x0\x66\x0\x0\x0\x7a\x1\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\xd0\x5\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x71\x1\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x14\x45\x2\x42\x92\xb\x42\xa4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x52\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x91\x22\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x29\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x88\xe0\xff\xff\xff\xff\x7\x40\xda\x60\x8\xff\xff\xff\xff\x3f\x0\x12\x50\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa0\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x2d\x0\x0\x0\x32\x22\x48\x9\x20\x64\x85\x4\x93\x22\xa4\x84\x4\x93\x22\xe3\x84\xa1\x90\x14\x12\x4c\x8a\x8c\xb\x84\xa4\x4c\x10\x68\x23\x0\x25\x0\x14\xe6\x8\xc0\x60\x8e\x0\x99\x1\x28\x6\x18\x63\x90\x42\xe6\xa8\xe1\xf2\x27\xec\x21\x24\x9f\xdb\xa8\x62\x25\x26\xbf\xb8\x6d\x44\x94\x52\xa\x91\x7b\x86\xcb\x9f\xb0\x87\x90\xfc\x10\x68\x86\x85\x40\x41\x2a\x4\x1a\x6a\xd0\xba\x6d\xb8\xfc\x9\x7b\x8\xc9\x5f\x9\xc9\xa1\x22\x81\x48\x23\xe7\x21\xa2\x9\x21\x24\x24\x8c\x51\x8\x35\x54\x23\x77\xd0\x70\xf9\x13\xf6\x10\x92\xbf\x12\xd2\x86\x34\x3\x22\xc6\x18\x64\x8e\x20\x28\x85\x1a\x70\x44\x92\x3\x1\xc3\x8\x44\x71\x93\x34\x45\x94\x30\xf9\xc0\xf3\x2c\xd8\x8f\x34\xf\x35\x49\x28\xa8\x57\x20\xcd\x43\x4d\x28\xb0\x74\xd3\x60\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x16\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x38\x40\x0\xc\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x91\x80\x0\x18\x0\x0\x0\x0\x0\x0\x0\x20\xb\x4\x0\x0\xc\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x1a\x25\x50\xc\xe5\x30\x2\x50\x20\x65\x50\x1e\x44\x4a\xa2\xc\xa\x61\x4\xa0\x40\x8\xcf\x0\x50\x1e\xcb\x20\x8\x0\x0\x0\x0\x79\x18\x0\x0\x4d\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x86\x62\x82\x30\x18\x1b\x84\x81\x98\x20\xc\xc7\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\xc\xc8\x4\x1\x8b\x8\x4c\x10\x86\x64\x3\xa2\x2c\x8c\xa2\xc\xd\xb0\x21\x70\x36\x10\x0\xf0\x0\x13\x84\x4c\xda\x10\x44\x13\x4\x1\x20\xd1\x16\x96\xe6\xc6\x65\xca\xea\xb\xea\x6d\x2e\x8d\x2e\xed\xcd\x6d\x82\x40\x30\x13\x4\xa2\xd9\x10\x28\x13\x4\xc2\x99\x20\x10\xcf\x86\x45\xa9\xac\xb\xcb\x86\x4c\xd1\x80\xd\xc1\xc6\x64\xca\xea\x8b\x2a\x4c\xee\xac\x8c\x6e\x82\x40\x40\x13\x84\x41\x99\x20\xc\xcb\x6\xe1\x3\x83\xd\x8b\xd2\x59\x1e\xa6\xd\x99\xa2\x85\xc1\x86\x40\xc\x36\xc\xdc\x18\x0\x1b\x8a\x89\x22\x3\x8\xa8\xc2\xc6\x66\xd7\xe6\x92\x46\x56\xe6\x46\x37\x25\x8\xaa\x90\xe1\xb9\xd8\x95\xc9\xcd\xa5\xbd\xb9\x4d\x9\x88\x26\x64\x78\x2e\x76\x61\x6c\x76\x65\x72\x53\x2\xa3\xe\x19\x9e\xcb\x1c\x5a\x18\x59\x99\x5c\xd3\x1b\x59\x19\xdb\x94\x0\x29\x43\x86\xe7\x22\x57\x36\xf7\x56\x27\x37\x56\x36\x37\x25\x78\xea\x90\xe1\xb9\xd8\xa5\x95\xdd\x25\x91\x4d\xd1\x85\xd1\x95\x4d\x9\xa2\x3a\x64\x78\x2e\x65\x6e\x74\x72\x79\x50\x6f\x69\x6e\x74\x73\x53\x2\x32\x0\x0\x79\x18\x0\x0\x4c\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x0\x0\x0\x71\x20\x0\x0\x16\x0\x0\x0\x36\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x98\xc0\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x11\x74\xc3\xe5\x3b\x8f\x2f\x44\x4\x30\x11\x21\xd0\xc\xb\xf1\x45\xe\xb3\x21\xcd\x80\x34\x86\x5\x54\xc3\xe5\x3b\x8f\x2f\x4d\x4e\x44\xa0\xd4\xf4\x50\x93\x5f\xdc\xb6\x1\x10\xc\x80\x34\x0\x61\x20\x0\x0\x24\x0\x0\x0\x13\x4\x41\x2c\x10\x0\x0\x0\x8\x0\x0\x0\x34\x4a\xa1\x64\x7\xa\x76\xa0\x34\xca\x8e\x48\x11\x94\x0\xc5\x11\x0\x82\x73\x10\x8\x82\x60\xd4\xe6\x10\xac\x4\x0\x0\x0\x23\x6\x8\x0\x82\x60\x20\x65\x89\x50\xd\x23\x6\x7\x0\x82\x60\xe0\x6c\x49\x20\x8c\x18\x1c\x0\x8\x82\xc1\xd2\x21\xc1\x35\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xa3\x9\xc4\x30\x62\x90\x0\x20\x8\x6\xc6\x18\x44\x1c\x57\x11\x23\x6\x9\x0\x82\x60\x60\x8c\x41\xc4\x71\xcb\x30\x62\x90\x0\x20\x8\x6\xc6\x18\x44\x1c\xc7\x8\x23\x6\x9\x0\x82\x60\x60\x8c\x41\xc4\x71\x57\x80\x0\x0\x0\x0\x0\x0\x0\x0";

}