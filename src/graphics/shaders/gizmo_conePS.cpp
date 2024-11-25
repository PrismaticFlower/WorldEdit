#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char gizmo_conePS_6_1_dxil_bytes[3661];
extern const char gizmo_conePS_6_6_dxil_bytes[3785];

auto gizmo_conePS() noexcept -> shader_def
{
   return {
      .name = "gizmo_conePS",
      .entrypoint = L"main",
      .target_6_1 = L"ps_6_1",
      .target_6_6 = L"ps_6_6",
      .file = L"gizmo_conePS.hlsl",
      .dxil_6_1 = {reinterpret_cast<const std::byte*>(gizmo_conePS_6_1_dxil_bytes),
                      sizeof(gizmo_conePS_6_1_dxil_bytes) - 1},
      .dxil_6_6 = {reinterpret_cast<const std::byte*>(gizmo_conePS_6_6_dxil_bytes),
                      sizeof(gizmo_conePS_6_6_dxil_bytes) - 1},
   };
}

const char gizmo_conePS_6_1_dxil_bytes[3661] = "\x44\x58\x42\x43\x2a\xbe\xe1\x6d\xe\xfe\x4b\x58\x1c\xaa\x50\x11\x95\x9b\x4a\xc9\x1\x0\x0\x0\x4c\xe\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x88\x0\x0\x0\xc4\x0\x0\x0\x6c\x1\x0\x0\x9c\x1\x0\x0\xb8\x1\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x52\x41\x59\x44\x49\x52\x56\x53\x0\x0\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\xa0\x0\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x1\x1\x0\x1\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\xa\x0\x0\x0\xa\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\xc\x0\x0\x0\x0\x52\x41\x59\x44\x49\x52\x56\x53\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x28\x0\x0\x0\x0\x0\x22\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x67\x69\x7a\x6d\x6f\x5f\x63\x6f\x6e\x65\x50\x53\x5f\x36\x5f\x31\x2e\x70\x64\x62\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x95\x5f\x1\x89\x16\x67\x2e\xad\xd2\xf9\x73\xd5\xf4\xe4\xb1\x47\x44\x58\x49\x4c\x8c\xc\x0\x0\x61\x0\x0\x0\x23\x3\x0\x0\x44\x58\x49\x4c\x1\x1\x0\x0\x10\x0\x0\x0\x74\xc\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x1a\x3\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x14\x45\x2\x42\x92\xb\x42\xa4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x52\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x91\x22\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x29\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x88\xe0\xff\xff\xff\xff\x7\x40\xda\x60\x8\xff\xff\xff\xff\x3f\x0\x12\x40\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x2b\x0\x0\x0\x32\x22\x48\x9\x20\x64\x85\x4\x93\x22\xa4\x84\x4\x93\x22\xe3\x84\xa1\x90\x14\x12\x4c\x8a\x8c\xb\x84\xa4\x4c\x10\x74\x23\x0\x25\x0\x14\xe6\x8\xc0\x60\x8e\x0\x99\x1\x28\x6\x18\x63\x90\x42\xa6\x20\x65\x94\x52\x4a\x29\x84\xca\x50\x46\x21\x55\x8e\x32\xc6\x20\x63\x10\x3b\x6a\xb8\xfc\x9\x7b\x8\xc9\xe7\x36\xaa\x58\x89\xc9\x2f\x6e\x1b\x11\xa5\x94\x42\xe4\x9e\xe1\xf2\x27\xec\x21\x24\x3f\x4\x9a\x61\x21\x50\xf0\xa\xe1\x6\x1c\x14\xe7\x8\x82\x62\xc0\x41\xc6\x98\x44\x7\x2\x86\x11\x86\xe2\x36\x69\x8a\x28\x61\xf2\xd\x24\x63\x9c\x5f\x70\x1a\xe2\x17\x9c\x46\x9a\x80\x66\x92\x90\x71\xef\x2d\xf7\x5e\x23\x4\xbf\x81\x64\x8c\xf3\xb\x4e\x43\xa0\x0\x53\xce\x82\x31\x5\x0\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x28\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x61\x80\x0\x10\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x13\x1\x1\x30\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x60\x0\x0\x0\x0\x0\x0\x0\x80\x2c\x10\xd\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x1a\x25\x50\xe\xc5\x30\x2\x50\x14\x85\x19\x50\x6\xe5\x41\xa4\x24\x46\x0\x8a\xa0\xc\xa\xa4\x10\x48\xcf\x0\xd0\x1e\xab\x18\x8\x0\x0\x0\x0\x79\x18\x0\x0\x4e\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x86\x63\x83\x30\xc\x13\x84\x1\xd9\x20\xc\x5\x5\xb8\xb9\x9\xc2\x90\x6c\x18\xe\x64\x98\x20\xc\xca\x4\x41\xab\x8\x4c\x10\x86\x65\x82\x30\x30\x1b\x10\x65\x61\x94\x66\x70\x80\xd\xc1\xb3\x81\x0\x0\x8\x98\x20\x6c\xd6\x86\x40\x9a\x20\x8\x0\x89\xb6\xb0\x34\x37\x22\x52\x41\x59\x44\x49\x52\x56\x53\x13\x4\xe2\x99\x20\x10\xd0\x86\x40\x99\x20\x10\xd1\x4\x81\x90\x36\x2c\x8a\x75\x61\x99\x36\x6c\xa\x6\x6c\x8\x38\x26\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x13\x4\x62\x9a\x20\x10\xd4\x4\x61\x68\x26\x8\x83\xb3\x41\x8\x3\x31\xd8\xb0\x28\xde\xf5\x65\xd8\x0\x6\xa\x36\x6\x1b\x2\x32\xd8\x30\x74\x65\x0\x6c\x28\xa8\xca\xc\x22\xa0\xa\x1b\x9b\x5d\x9b\x4b\x1a\x59\x99\x1b\xdd\x94\x20\xa8\x42\x86\xe7\x62\x57\x26\x37\x97\xf6\xe6\x36\x25\x18\x9a\x90\xe1\xb9\xd8\x85\xb1\xd9\x95\xc9\x4d\x9\x8a\x3a\x64\x78\x2e\x73\x68\x61\x64\x65\x72\x4d\x6f\x64\x65\x6c\x53\x2\xa4\xc\x19\x9e\x8b\x5c\xd9\xdc\x5b\x9d\xdc\x58\xd9\xdc\x94\x0\xaa\x43\x86\xe7\x62\x97\x56\x76\x97\x44\x36\x45\x17\x46\x57\x36\x25\x90\xea\x90\xe1\xb9\x94\xb9\xd1\xc9\xe5\x41\xbd\xa5\xb9\xd1\xcd\x4d\x9\xcc\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\x1b\x0\x0\x0\x56\xb0\xd\x97\xef\x3c\xbe\x10\x50\x45\x41\x44\xa5\x3\xc\x25\x61\x0\x2\xe6\x17\xb7\x6d\x6\xd2\x70\xf9\xce\xe3\xb\x11\x1\x4c\x44\x8\x34\xc3\x42\x98\x80\x33\x5c\xbe\xf3\xf8\x83\x33\xdd\x7e\x71\xdb\x46\x50\xd\x97\xef\x3c\x3e\x51\x1\xb\xdb\x0\xcf\x43\xc\x7e\x71\xdb\x16\x50\xd\x97\xef\x3c\xbe\x34\x39\x11\x81\x52\xd3\x43\x4d\x7e\x71\xdb\x36\xf0\xc\x97\xef\x3c\x3e\xd5\x0\x11\xe6\x17\xb7\x6d\x0\x4\x3\x20\xd\x0\x0\x61\x20\x0\x0\xbd\x1\x0\x0\x13\x4\x51\x2c\x10\x0\x0\x0\xd\x0\x0\x0\x34\xca\xad\x5c\x3\x8a\xa0\xa\xaa\xa1\x64\xa\xa6\x14\xca\xae\xe4\xa\x81\x48\x9\xd0\x1c\x1\xa0\x32\x46\x0\x82\x20\x88\x7e\x63\x4\x20\x8\x82\x20\x28\x8c\x11\x80\x20\x8\x82\x60\x30\x2\x0\x0\x0\x0\x23\x6\x9\x0\x82\x60\x40\x89\x1\x72\x79\x5d\x31\x62\x70\x0\x20\x8\x6\x11\x19\x28\x1\x34\x9a\x10\xc\x16\x4\xf2\x19\x31\x38\x0\x10\x4\x83\xe8\xc\x1a\xe2\x1b\x4d\x8\x80\xd1\x4\x21\x18\x4d\x18\x84\xd1\x84\x3\x18\x4d\x40\x82\xd1\x84\x44\x30\x63\x88\x8f\x19\x43\x7c\xcc\x18\xe2\x33\x62\xb0\x0\x20\x8\x6\x8\x1d\x7c\x83\x10\xc\x42\x60\x1\x3\x9f\x11\x3\x5\x0\x41\x30\x60\xe8\x0\xc\xdc\xc0\xd\xd0\xc0\xeb\x46\xc\x14\x0\x4\xc1\x80\xa9\x83\x30\x78\x83\x37\xb8\x3e\x6f\xc4\x40\x1\x40\x10\xc\x18\x3b\x10\x3\x38\x80\x83\x34\x0\x83\x6f\xc4\x60\x1\x40\x10\xc\x90\x3c\x20\x83\x41\x8\x6\x21\x18\x31\x30\x0\x10\x4\x3\x25\xf\xc0\x20\xb0\xa0\x90\x8f\x9\x85\x7c\x6c\x28\xe4\x63\x1b\x14\x1f\xe3\xa0\xf8\x58\x7\xc5\x67\xc4\x60\x1\x40\x10\xc\x90\x50\x60\x83\x41\x8\xa0\xc7\x19\x31\x58\x0\x10\x4\x3\x44\x14\xda\xe0\x30\x8a\x8\x7a\x86\x23\x4\x8f\xf8\x4c\x10\xe4\x33\xcb\x10\x8\x82\xd\x87\x7c\x8c\x38\xe4\x63\xc5\x21\x1f\x3b\x1a\xf9\x18\xd2\xc8\xc7\x92\x46\x3e\x66\xc\xf1\x31\x63\x88\x8f\x19\x43\x7c\x46\xc\x16\x0\x4\xc1\x0\x71\x85\x3c\x18\x84\x60\x10\x2\x5b\xcc\x40\x3e\xc3\x11\x42\x40\x7c\xb3\xc\x84\x10\x8c\x18\x2c\x0\x8\x82\x1\x12\xb\x7c\x60\x55\xd4\x24\x45\x23\x6\xb\x0\x82\x60\x80\xc8\x42\x1f\x50\x93\x44\x4d\x92\x7d\x9f\x7c\xc\xfa\xe4\x63\x42\x10\x1f\x1b\xa\xf9\x98\x25\x6\xf2\xb1\xc0\x92\x8f\xd\x41\x7c\x2c\xe\xcc\x40\x3e\x16\x64\xf2\xb1\x21\x80\x8f\x2d\x69\x20\x1f\xeb\x3a\xf9\x58\xa0\x6\xf2\x31\x3c\xe0\x3\xf9\x58\x0\x6\xf2\x31\x37\xd0\x3\xf9\x98\x10\xc4\xc7\x2\x3e\x90\x8f\x21\x1\x7c\x2c\x90\x3\xf9\x58\x80\xc4\xc7\x18\x46\x3e\x26\x50\xf2\x31\x21\x88\xcf\x70\x44\x60\xa\xc4\x37\xcb\x40\xc\x81\xa5\x2\x14\x9f\x11\x3\x3\x0\x41\x30\x50\xd8\x41\x16\x6\x13\x82\xf8\x58\xa0\xd1\xc7\x2\x37\x90\x8f\x5\x70\x0\x9f\xe1\x88\xa0\x15\x84\x6f\x38\x42\xf8\x3\xe2\x2b\x21\xd0\xe9\x86\x59\x80\x85\x60\x96\x80\x18\xe8\x28\xd0\x22\x10\xb\x41\x18\x46\xc\x14\x0\x4\xc1\x80\xa9\x87\x70\x78\x87\x77\x48\x7\x78\xf8\x85\x11\x3\x5\x0\x41\x30\x60\xec\x41\x1c\xe0\x1\x1e\x70\x21\x1e\xc0\x61\xc4\x40\x1\x40\x10\xc\x98\x7b\x18\x87\x78\x88\x7\x75\x90\x87\x70\x18\x31\x58\x0\x10\x4\x3\x44\x1f\xca\x61\x10\x82\x41\x8\x46\xc\xc\x0\x4\xc1\x40\xd1\x87\x70\x8\x2c\x28\xe4\x63\x42\x21\x1f\x1b\xa\xf9\x8c\x18\x2c\x0\x8\x82\x1\xf2\xf\xea\x30\x8\x81\x2b\xb4\x2\x2b\x58\x10\xc8\x67\x96\xa1\x30\xfc\xc0\x84\x51\x90\x8f\xd\xa3\x20\x1f\x23\x46\x41\x3e\x36\xa\x88\x7c\x8c\x14\x10\xf9\x58\x29\x20\xf2\x31\x63\x88\x8f\x19\x43\x7c\xcc\x18\xe2\x33\x62\xb0\x0\x20\x8\x6\x88\x4a\xd4\xc3\x20\x4\x83\x10\xd8\x22\xe\xf2\x19\x8e\x10\x2\xe2\x9b\x65\x40\x8c\x60\xc4\x60\x1\x40\x10\xc\x90\x96\xc0\x87\x8\x7a\x5e\xc1\x15\x5a\x61\xc4\x60\x1\x40\x10\xc\x10\x97\xc8\x7\x58\x78\x5\x57\x80\x85\x57\x70\x5\xdb\x85\x5d\x90\x8f\x41\xbb\x20\x1f\x13\x82\xf8\xd8\x50\xc8\xc7\x64\xc1\x17\xe4\x63\x41\x25\x1f\x1b\x82\xf8\x58\x3b\x88\x83\x7c\x2c\xc0\xe4\x63\x43\x0\x1f\x5b\xca\x41\x3e\x96\xb\xb9\x20\x1f\xb\xcc\x41\x3e\x46\xf\xf8\x20\x1f\xb\x78\x41\x3e\xa6\xe\xf6\x20\x1f\x13\x82\xf8\x58\x80\xf\xf2\x31\x24\x80\x8f\x5\xee\x20\x1f\xb\x90\xf8\x18\xc3\xc8\xc7\x4\x4a\x3e\x26\x4\xf1\x19\x8e\x8\x44\x82\xf8\x66\x19\x90\x23\xb0\x92\x80\xe2\x33\x62\x60\x0\x20\x8\x6\xa\x5a\xb8\xc4\x60\x42\x10\x1f\xb\x34\xfa\x58\xd0\x6\xf2\xb1\x80\x1d\xe0\x33\x1c\x11\xa4\x84\xf0\xd\x47\x8\xfb\x40\x7c\x25\x4\x3a\xdd\xf0\x12\x2c\x11\xcc\x12\x20\x3\x1d\x5\x98\x14\x3a\x62\x8\x87\x5\x7d\x0\x9f\x11\x3\x5\x0\x41\x30\x60\xe4\xc2\x27\xd8\x82\x2d\xcc\x62\x27\x7a\x62\xc4\x40\x1\x40\x10\xc\x98\xb9\xf8\x89\xb6\x68\x8b\x9a\xe0\x9\x9f\x18\x31\x50\x0\x10\x4\x3\x86\x2e\xc0\xc2\x2d\xdc\xe2\x2c\x7a\xe2\x27\x46\xc\x16\x0\x4\xc1\x0\xb9\xb\xb1\x18\x84\x60\x10\x82\x11\x3\x3\x0\x41\x30\x50\xee\xc2\x27\x2\xb\xa\xf9\x98\x50\xc8\xc7\x86\x42\x3e\x23\x6\xb\x0\x82\x60\x80\xf0\xc5\x59\xc\x42\xb0\x12\x2a\x91\x12\x16\x4\xf2\x99\x65\x48\x94\x7d\x30\x1\x24\xe4\x63\x3\x48\xc8\xc7\x8\x90\x90\x8f\x81\x4\x22\x1f\xb\x9\x44\x3e\x26\x12\x88\x7c\xcc\x18\xe2\x63\xc6\x10\x1f\x33\x86\xf8\x8c\x18\x2c\x0\x8\x82\x1\x72\x1a\x72\x31\x8\xc1\x20\x4\xb6\xfc\x84\x7c\x86\x23\x84\x80\xf8\x66\x19\x18\x25\x18\x31\x58\x0\x10\x4\x3\x44\x35\xea\x22\x82\x1e\x96\x58\x9\x95\x18\x31\x58\x0\x10\x4\x3\x64\x35\xec\xa2\x25\x58\x62\x25\x5a\x82\x25\x56\xc2\x70\x2\x27\xe4\x63\x10\x4e\xc8\xc7\x84\x20\x3e\x36\x14\xf2\xb1\x97\xd8\x9\xf9\x58\x50\xc9\xc7\x86\x20\x3e\xa6\x16\x3f\x21\x1f\xb\x30\xf9\xd8\x10\xc0\xc7\x16\xb1\x90\x8f\xd9\x84\x4d\xc8\xc7\x82\xb1\x90\x8f\xc5\x45\x5d\xc8\xc7\x82\x9c\x90\x8f\x9d\xc5\x5c\xc8\xc7\x84\x20\x3e\x16\xd4\x85\x7c\xc\x9\xe0\x63\xc1\x5a\xc8\xc7\x2\x24\x3e\xc6\x30\xf2\x31\x81\x92\x8f\x9\x41\x7c\x86\x23\x82\xbf\x20\xbe\x59\x6\x66\x9\x4c\x34\xa0\xf8\x8c\x18\x18\x0\x8\x82\x81\x52\x1e\xab\x31\x98\x10\xc4\xc7\x2\x8d\x3e\x16\xb4\x81\x7c\x2c\x48\xb\xf8\xc\x47\x4\xa6\x21\x7c\xc3\x11\x2\x5e\x10\x5f\x9\x81\x4e\x37\xb0\x46\x6a\x4\xb3\x4\xcc\x40\x47\xa1\x33\x9\xcd\x28\xc2\x62\x7d\x10\xc0\x67\xc4\x40\x1\x40\x10\xc\x98\xf7\xd8\x8d\xf4\x48\x8f\xf1\xd0\xd\xf5\x18\x31\x50\x0\x10\x4\x3\x6\x3e\x78\x43\x3d\xd4\x43\x36\x76\x63\x3d\x46\xc\x14\x0\x4\xc1\x80\x89\x8f\xde\x58\x8f\xf5\x20\xf\xde\x60\x8f\x11\x83\x5\x0\x41\x30\x40\xe8\xe3\x37\x6\x21\x18\x84\x60\xc4\xc0\x0\x40\x10\xc\x14\xfa\xd8\x8d\xc0\x82\x42\x3e\x26\x14\xf2\xb1\xa1\x90\xcf\x88\xc1\x2\x80\x20\x18\x20\xf9\x41\x1e\x83\x10\xa0\xc6\x69\x98\x86\x5\x81\x7c\x66\x19\x1a\x7\x2f\x4c\xe8\xb\xf9\xd8\xd0\x17\xf2\x31\xa2\x2f\xe4\x63\x7d\x81\xc8\xc7\xfc\x2\x91\x8f\xfd\x5\x22\x1f\x33\x86\xf8\x98\x31\xc4\xc7\x8c\x21\x3e\x23\x6\xb\x0\x82\x60\x80\x90\xc8\x7b\xc\x42\x30\x8\x81\x2d\xbc\x21\x9f\xe1\x8\x21\x20\xbe\x59\x6\xc8\x9\x46\xc\x16\x0\x4\xc1\x0\x39\x11\xf9\x88\xa0\x27\x35\x50\xe3\x34\x46\xc\x16\x0\x4\xc1\x0\x41\x91\xf9\x50\x8d\xd4\x40\xd\xd5\x48\xd\xd4\xb0\xda\xa8\xd\xf9\x18\x54\x1b\xf2\x31\x21\x88\x8f\xd\x85\x7c\x8c\x35\x70\x43\x3e\x16\x54\xf2\xb1\x21\x88\x8f\x9d\x7\x6f\xc8\xc7\x2\x4c\x3e\x36\x4\xf0\xb1\xe5\x37\xe4\x63\xb3\x31\x1b\xf2\xb1\x0\x3c\xe4\x63\xee\x21\x1f\xf2\xb1\xc0\x36\xe4\x63\xe4\x1\x1f\xf2\x31\x21\x88\x8f\x5\xf2\x21\x1f\x43\x2\xf8\x58\x80\x1e\xf2\xb1\x0\x89\x8f\x31\x8c\x7c\x4c\xa0\xe4\x63\x42\x10\x9f\xe1\x88\x80\x3f\x88\x6f\x96\x1\x7a\x2\xfb\xf\x28\x3e\x23\x6\x6\x0\x82\x60\xa0\x88\x9\x8a\xc\x26\x4\xf1\xb1\x40\xa3\x8f\x5\x6d\x20\x1f\xb\xcc\x3\x3e\xc3\x11\xc1\x88\x8\xdf\x70\x84\x50\x1f\xc4\x57\x42\xa0\xd3\xd\x29\x62\x22\xc1\x2c\x1\x34\xd0\x51\xd0\x50\xe3\x42\x8e\xf0\x58\x1f\x4\xf0\x19\x31\x38\x0\x10\x4\x83\x68\x4d\x62\x4\x45\x5e\x64\x34\x21\x0\x46\x13\x84\x60\x34\x61\x10\x46\xc\x12\x0\x4\xc1\xc0\x98\x13\x1b\x51\x13\x35\x21\x93\x61\xc4\x20\x1\x40\x10\xc\x8c\x39\xb1\x11\x35\x51\x13\x19\x11\x46\xc\x12\x0\x4\xc1\xc0\x98\x13\x1b\x51\x13\x35\x19\x93\x60\xc4\x20\x1\x40\x10\xc\x8c\x39\xb1\x11\x35\x51\x13\x31\x29\x10\x0\x0\x0\x0\x0\x0";

const char gizmo_conePS_6_6_dxil_bytes[3785] = "\x44\x58\x42\x43\xe4\x93\x13\x10\x4\x69\x37\x9d\xfd\x7a\x34\xfc\x72\x54\xea\x3f\x1\x0\x0\x0\xc8\xe\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x88\x0\x0\x0\xc4\x0\x0\x0\x6c\x1\x0\x0\x9c\x1\x0\x0\xb8\x1\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x52\x41\x59\x44\x49\x52\x56\x53\x0\x0\x0\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\xa0\x0\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x1\x1\x0\x1\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\xa\x0\x0\x0\xa\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\xc\x0\x0\x0\x0\x52\x41\x59\x44\x49\x52\x56\x53\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x28\x0\x0\x0\x0\x0\x22\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x67\x69\x7a\x6d\x6f\x5f\x63\x6f\x6e\x65\x50\x53\x5f\x36\x5f\x36\x2e\x70\x64\x62\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x90\xcd\xe1\xdc\xba\xd4\x61\x72\x85\xc1\x7e\xcd\x2c\x5f\x68\x74\x44\x58\x49\x4c\x8\xd\x0\x0\x66\x0\x0\x0\x42\x3\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\xf0\xc\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x39\x3\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x88\xe0\xff\xff\xff\xff\x7\x40\xda\x60\x8\xff\xff\xff\xff\x3f\x0\x12\x40\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x38\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\x84\xc1\x8\x40\x9\x0\xa\xe6\x8\xc0\x60\x8e\x0\x99\x1\x28\x6\x30\xc\x3\x51\x90\x51\x90\x62\x28\x8a\xa2\x28\xa\x42\xca\x50\xc\x5\x29\xe5\x28\x86\x61\x20\x86\x81\x98\xa3\x86\xcb\x9f\xb0\x87\x90\x7c\x6e\xa3\x8a\x95\x98\xfc\xe2\xb6\x11\x51\x14\x45\x41\xc4\x3d\xc3\xe5\x4f\xd8\x43\x48\x7e\x8\x34\xc3\x42\xa0\xe0\x29\x84\x33\x40\x3\x45\xb7\xd\x97\x3f\x61\xf\x21\xf9\x2b\x21\x39\x54\x24\x10\x69\xe4\x3c\x44\x34\x21\x84\x84\x84\x61\x28\x4\x34\x40\x13\x51\x7\xd\x97\x3f\x61\xf\x21\xf9\x2b\x21\x6d\x48\x33\x20\x62\x18\x6\x62\x8e\x20\x28\x5\x34\x58\xc3\x45\xd8\x40\xc0\x30\xc2\x50\xdc\x26\x4d\x11\x25\x4c\xbe\x81\x64\x8c\xf3\xb\x4e\x43\xfc\x82\xd3\x48\x13\xd0\x4c\x12\x32\xb6\x6d\x2b\xb6\xed\x1a\x21\xf8\xd\x24\x63\x9c\x5f\x70\x1a\x2\x5\x1c\x75\x59\x30\x4c\x1\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x28\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x61\x80\x0\x10\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x13\x1\x1\x30\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x87\x2\x2\x40\x0\x0\x0\x0\x0\x0\x0\x80\x21\xf\x6\x4\x80\x0\x0\x0\x0\x0\x0\x0\x0\x59\x20\x0\x0\x0\xd\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x1a\x4a\xa0\x18\xca\x61\x4\xa0\x28\xa\x33\xa0\xc\xca\x83\x88\x92\x18\x1\x28\x82\x32\x28\x90\x42\x20\x6f\x6\x80\xbe\xb1\x8a\x81\x0\x0\x0\x0\x79\x18\x0\x0\x4e\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x6\x64\x82\x30\x24\x1b\x84\x81\x98\x20\xc\xca\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\xc\xcb\x4\xc1\xb3\x8\x4c\x10\x6\x66\x82\x30\x34\x1b\x10\x65\x61\x94\x66\x70\x80\xd\xc1\xb3\x81\x0\x0\x8\x98\x20\x7c\xd7\x86\x40\x9a\x20\x8\x0\x89\xb6\xb0\x34\x37\x22\x52\x41\x59\x44\x49\x52\x56\x53\x13\x4\x2\x9a\x20\x10\xd1\x86\x40\x99\x20\x10\xd2\x4\x81\x98\x36\x2c\x8a\x75\x61\x99\x36\x6c\xa\x6\x6c\x8\x38\x26\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x13\x4\x82\x9a\x20\x10\xd5\x4\x61\x70\x26\x8\xc3\xb3\x41\x8\x3\x31\xd8\xb0\x28\xde\xf5\x65\xd8\x0\x6\xa\x36\x6\x1b\x2\x32\xd8\x30\x74\x65\x0\x6c\x28\xa8\xca\xc\x22\xa0\xa\x1b\x9b\x5d\x9b\x4b\x1a\x59\x99\x1b\xdd\x94\x20\xa8\x42\x86\xe7\x62\x57\x26\x37\x97\xf6\xe6\x36\x25\x20\x9a\x90\xe1\xb9\xd8\x85\xb1\xd9\x95\xc9\x4d\x9\x8c\x3a\x64\x78\x2e\x73\x68\x61\x64\x65\x72\x4d\x6f\x64\x65\x6c\x53\x2\xa4\xc\x19\x9e\x8b\x5c\xd9\xdc\x5b\x9d\xdc\x58\xd9\xdc\x94\x0\xaa\x43\x86\xe7\x62\x97\x56\x76\x97\x44\x36\x45\x17\x46\x57\x36\x25\x90\xea\x90\xe1\xb9\x94\xb9\xd1\xc9\xe5\x41\xbd\xa5\xb9\xd1\xcd\x4d\x9\xcc\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\x21\x0\x0\x0\x66\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x58\xc1\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x1d\x74\xc3\xe5\x3b\x8f\x2f\x44\x4\x30\x11\x21\xd0\xc\xb\xf1\x45\xe\xb3\x21\xcd\x80\x34\x86\x9\x38\xc3\xe5\x3b\x8f\x3f\x38\xd3\xed\x17\xb7\x6d\x4\xd5\x70\xf9\xce\xe3\x13\x15\xb0\xb0\xd\xf0\x3c\xc4\xe0\x17\xb7\x6d\x1\xd5\x70\xf9\xce\xe3\x4b\x93\x13\x11\x28\x35\x3d\xd4\xe4\x17\xb7\x6d\x3\xcf\x70\xf9\xce\xe3\x53\xd\x10\x61\x7e\x71\xdb\x6\x40\x30\x0\xd2\x0\x61\x20\x0\x0\xc5\x1\x0\x0\x13\x4\x51\x2c\x10\x0\x0\x0\x11\x0\x0\x0\x34\x94\x5b\xb9\x6\x14\x41\x15\x54\x43\xc9\x14\x4c\x29\x94\x5d\xc9\xe\x14\xec\x40\x69\x14\x2\x11\x25\x40\xd7\x8\x0\x15\x63\x4\x20\x8\x82\xe8\x37\x46\x0\x82\x20\x8\x82\xc2\x18\x1\x8\x82\x20\x8\x6\x34\xcd\x21\x8c\x41\x43\xd6\x1c\x4\xc3\x2c\x12\x15\x23\x0\x0\x23\x6\x8\x0\x82\x60\x80\x99\x1\x23\x84\xc1\x31\x62\x70\x0\x20\x8\x6\x14\x1a\x30\x1\x31\x62\x70\x0\x20\x8\x6\x91\x1a\x3c\x41\x35\x9a\x10\xc\x16\x4\xf2\x19\x31\x38\x0\x10\x4\x83\xa8\xd\x24\x82\xc\x46\x13\x2\x60\x34\x41\x8\x46\x13\x6\x61\x34\xe1\x0\x46\x13\x90\x60\x34\x21\x11\xcc\x18\xe2\x63\xc6\x10\x1f\x33\x86\xf8\x8c\x18\x2c\x0\x8\x82\x1\xa2\x7\x64\x30\x8\xc1\x20\x4\x16\x30\xf0\x19\x31\x50\x0\x10\x4\x3\x46\xf\xca\x60\xe\xe6\xa0\xd\xc6\x40\xc\x46\xc\x14\x0\x4\xc1\x80\xd9\x3\x33\xa0\x3\x3a\xd0\xc8\x60\xc\x46\xc\x14\x0\x4\xc1\x80\xe1\x83\x33\xa8\x83\x3a\x70\x83\x32\x20\x83\x11\x83\x5\x0\x41\x30\x40\xfe\x20\xd\x6\x21\x18\x84\x60\xc4\xc0\x0\x40\x10\xc\x94\x3f\x28\x83\xc0\x82\x42\x3e\x26\x14\xf2\xb1\xa1\x90\x8f\x79\x50\x7c\xec\x83\xe2\x63\x60\x0\xc5\x67\xc4\x60\x1\x40\x10\xc\x90\x53\x88\x83\x41\x8\xa0\xc7\x19\x31\x58\x0\x10\x4\x3\x4\x15\xe4\xe0\x30\x8a\x8\x7a\x86\x23\x84\x8f\xf8\x4c\x10\xe4\x33\xcb\x10\x8\x82\xd\x87\x7c\x8c\x38\xe4\x63\xc5\x21\x1f\x3b\x1a\xf9\x18\xd2\xc8\xc7\x92\x46\x3e\x66\xc\xf1\x31\x63\x88\x8f\x19\x43\x7c\x46\xc\x16\x0\x4\xc1\x0\xa1\x5\x3f\x18\x84\x60\x10\x2\x5b\xcc\x40\x3e\xc3\x11\x42\x40\x7c\xb3\xc\x84\x10\x8c\x18\x2c\x0\x8\x82\x1\x72\xb\xa1\x60\x55\xd4\x24\x45\x23\x6\xb\x0\x82\x60\x80\xe0\x82\x28\x50\x93\x44\x4d\x92\x7d\x9f\x7c\xc\xfa\xe4\x63\x42\x10\x1f\x1b\xa\xf9\x98\x25\x6\xf2\xb1\xc0\x92\x8f\xd\x41\x7c\x2c\xe\xcc\x40\x3e\x16\x64\xf2\xb1\x21\x80\x8f\x2d\x69\x20\x1f\xeb\x3a\xf9\x58\xa0\x6\xf2\x31\x3c\xf8\x3\xf9\x58\x0\x6\xf2\x31\x37\xd0\x3\xf9\x98\x10\xc4\xc7\x2\x3e\x90\x8f\x21\x1\x7c\x2c\x90\x3\xf9\x58\x80\xc4\xc7\x18\x46\x3e\x26\x50\xf2\x31\x21\x88\xcf\x70\x44\x70\xa\xc4\x37\xcb\x40\xc\x81\xb1\x2\x14\x9f\x11\x3\x3\x0\x41\x30\x50\xe4\xe1\x16\x6\x13\x82\xf8\x58\xa0\xd1\xc7\x2\x37\x90\x8f\x5\x70\x0\x9f\xe1\x88\xc0\x15\x84\x6f\x38\x42\xf8\x3\xe2\x2b\x21\xd0\xe9\x6\x5b\x88\x85\x60\x96\x80\x18\xe8\x28\xdc\x22\x20\xb\x41\x18\x46\xc\x14\x0\x4\xc1\x80\xd9\x7\x73\xa0\x7\x7a\x70\x7\x7b\x20\x87\x11\x3\x5\x0\x41\x30\x60\xf8\xe1\x1c\xea\xa1\x1e\x76\xe1\x1e\xca\x61\xc4\x40\x1\x40\x10\xc\x98\x7e\x40\x7\x7b\xb0\x87\x77\xc0\x7\x73\x18\x31\x58\x0\x10\x4\x3\x4\x24\xd4\x61\x10\x82\x41\x8\x46\xc\xc\x0\x4\xc1\x40\x1\x9\x73\x8\x2c\x28\xe4\x63\x42\x21\x1f\x1b\xa\xf9\x8c\x18\x2c\x0\x8\x82\x1\x52\x12\xef\x30\x8\x81\x2b\xb4\x2\x2b\x58\x10\xc8\x67\x96\xa1\x30\xfc\xc0\x84\x51\x90\x8f\xd\xa3\x20\x1f\x23\x46\x41\x3e\x36\xa\x88\x7c\x8c\x14\x10\xf9\x58\x29\x20\xf2\x31\x63\x88\x8f\x19\x43\x7c\xcc\x18\xe2\x33\x62\xb0\x0\x20\x8\x6\x8\x4c\xe8\xc3\x20\x4\x83\x10\xd8\x22\xe\xf2\x19\x8e\x10\x2\xe2\x9b\x65\x40\x8c\x60\xc4\x60\x1\x40\x10\xc\x90\x99\xe8\x87\x8\x7a\x5e\xc1\x15\x5a\x61\xc4\x60\x1\x40\x10\xc\x10\x9a\xf0\x7\x58\x78\x5\x57\x80\x85\x57\x70\x5\xdb\x85\x5d\x90\x8f\x41\xbb\x20\x1f\x13\x82\xf8\xd8\x50\xc8\xc7\x64\xc1\x17\xe4\x63\x41\x25\x1f\x1b\x82\xf8\x58\x3b\x88\x83\x7c\x2c\xc0\xe4\x63\x43\x0\x1f\x5b\xca\x41\x3e\x96\xb\xb9\x20\x1f\xb\xcc\x41\x3e\x46\xf\xfb\x20\x1f\xb\x78\x41\x3e\xa6\xe\xf6\x20\x1f\x13\x82\xf8\x58\x80\xf\xf2\x31\x24\x80\x8f\x5\xee\x20\x1f\xb\x90\xf8\x18\xc3\xc8\xc7\x4\x4a\x3e\x26\x4\xf1\x19\x8e\x8\x46\x82\xf8\x66\x19\x90\x23\x30\x94\x80\xe2\x33\x62\x60\x0\x20\x8\x6\x8a\x5b\xcc\xc4\x60\x42\x10\x1f\xb\x34\xfa\x58\xd0\x6\xf2\xb1\x80\x1d\xe0\x33\x1c\x11\xa8\x84\xf0\xd\x47\x8\xfb\x40\x7c\x25\x4\x3a\xdd\x20\x13\x2d\x11\xcc\x12\x20\x3\x1d\x85\x99\x14\x3c\x62\x8\x87\x5\x7d\x0\x9f\x11\x3\x5\x0\x41\x30\x60\xf0\x62\x2c\xe2\x22\x2e\xd6\x2\x2c\xc4\x62\xc4\x40\x1\x40\x10\xc\x98\xbc\x20\xb\xb9\x90\xb\x9c\x8\x8b\xb1\x18\x31\x50\x0\x10\x4\x3\x46\x2f\xca\x62\x2e\xe6\x82\x2d\xc4\x82\x2c\x46\xc\x16\x0\x4\xc1\x0\xe9\x8b\xb3\x18\x84\x60\x10\x82\x11\x3\x3\x0\x41\x30\x50\xfa\x62\x2c\x2\xb\xa\xf9\x98\x50\xc8\xc7\x86\x42\x3e\x23\x6\xb\x0\x82\x60\x80\x88\x6\x5b\xc\x42\xb0\x12\x2a\x91\x12\x16\x4\xf2\x99\x65\x48\x94\x7d\x30\x1\x24\xe4\x63\x3\x48\xc8\xc7\x8\x90\x90\x8f\x81\x4\x22\x1f\xb\x9\x44\x3e\x26\x12\x88\x7c\xcc\x18\xe2\x63\xc6\x10\x1f\x33\x86\xf8\x8c\x18\x2c\x0\x8\x82\x1\xd2\x1a\x77\x31\x8\xc1\x20\x4\xb6\xfc\x84\x7c\x86\x23\x84\x80\xf8\x66\x19\x18\x25\x18\x31\x58\x0\x10\x4\x3\x4\x36\xf4\x22\x82\x1e\x96\x58\x9\x95\x18\x31\x58\x0\x10\x4\x3\x24\x36\xf6\xa2\x25\x58\x62\x25\x5a\x82\x25\x56\xc2\x70\x2\x27\xe4\x63\x10\x4e\xc8\xc7\x84\x20\x3e\x36\x14\xf2\xb1\x97\xd8\x9\xf9\x58\x50\xc9\xc7\x86\x20\x3e\xa6\x16\x3f\x21\x1f\xb\x30\xf9\xd8\x10\xc0\xc7\x16\xb1\x90\x8f\xd9\x84\x4d\xc8\xc7\x82\xb1\x90\x8f\xc5\x5\x5e\xc8\xc7\x82\x9c\x90\x8f\x9d\xc5\x5c\xc8\xc7\x84\x20\x3e\x16\xd4\x85\x7c\xc\x9\xe0\x63\xc1\x5a\xc8\xc7\x2\x24\x3e\xc6\x30\xf2\x31\x81\x92\x8f\x9\x41\x7c\x86\x23\x2\xd0\x20\xbe\x59\x6\x66\x9\xac\x34\xa0\xf8\x8c\x18\x18\x0\x8\x82\x81\xb2\x1e\xb0\x31\x98\x10\xc4\xc7\x2\x8d\x3e\x16\xb4\x81\x7c\x2c\x48\xb\xf8\xc\x47\x4\xa7\x21\x7c\xc3\x11\x2\x5e\x10\x5f\x9\x81\x4e\x37\xbc\x86\x6a\x4\xb3\x4\xcc\x40\x47\x1\x36\x89\xcd\x28\xc2\x62\x7d\x10\xc0\x67\xc4\x40\x1\x40\x10\xc\x98\xfa\x0\xf\xf7\x70\xf\xf4\xf8\xd\xf8\x18\x31\x50\x0\x10\x4\x3\xc6\x3e\xc2\xe3\x3d\xde\xa3\x36\xc0\x23\x3e\x46\xc\x14\x0\x4\xc1\x80\xb9\xf\xf1\x80\xf\xf8\x48\x8f\xf0\x90\x8f\x11\x83\x5\x0\x41\x30\x40\xf4\x83\x3c\x6\x21\x18\x84\x60\xc4\xc0\x0\x40\x10\xc\x14\xfd\x0\x8f\xc0\x82\x42\x3e\x26\x14\xf2\xb1\xa1\x90\xcf\x88\xc1\x2\x80\x20\x18\x20\xff\x91\x1e\x83\x10\xa0\xc6\x69\x98\x86\x5\x81\x7c\x66\x19\x1a\x7\x2f\x4c\xe8\xb\xf9\xd8\xd0\x17\xf2\x31\xa2\x2f\xe4\x63\x7d\x81\xc8\xc7\xfc\x2\x91\x8f\xfd\x5\x22\x1f\x33\x86\xf8\x98\x31\xc4\xc7\x8c\x21\x3e\x23\x6\xb\x0\x82\x60\x80\xa8\x8\x7d\xc\x42\x30\x8\x81\x2d\xbc\x21\x9f\xe1\x8\x21\x20\xbe\x59\x6\xc8\x9\x46\xc\x16\x0\x4\xc1\x0\x69\x91\xfb\x88\xa0\x27\x35\x50\xe3\x34\x46\xc\x16\x0\x4\xc1\x0\x71\x11\xfc\x50\x8d\xd4\x40\xd\xd5\x48\xd\xd4\xb0\xda\xa8\xd\xf9\x18\x54\x1b\xf2\x31\x21\x88\x8f\xd\x85\x7c\x8c\x35\x70\x43\x3e\x16\x54\xf2\xb1\x21\x88\x8f\x9d\x7\x6f\xc8\xc7\x2\x4c\x3e\x36\x4\xf0\xb1\xe5\x37\xe4\x63\xb3\x31\x1b\xf2\xb1\x0\x3c\xe4\x63\xee\x51\x1f\xf2\xb1\xc0\x36\xe4\x63\xe4\x1\x1f\xf2\x31\x21\x88\x8f\x5\xf2\x21\x1f\x43\x2\xf8\x58\x80\x1e\xf2\xb1\x0\x89\x8f\x31\x8c\x7c\x4c\xa0\xe4\x63\x42\x10\x9f\xe1\x88\xa0\x3f\x88\x6f\x96\x1\x7a\x2\x13\x11\x28\x3e\x23\x6\x6\x0\x82\x60\xa0\xa0\x49\x8b\xc\x26\x4\xf1\xb1\x40\xa3\x8f\x5\x6d\x20\x1f\xb\xcc\x3\x3e\xc3\x11\x1\x89\x8\xdf\x70\x84\x50\x1f\xc4\x57\x42\xa0\xd3\xd\x2c\x72\x22\xc1\x2c\x1\x34\xd0\x51\xe8\x50\x3\x43\x8e\xf0\x58\x1f\x4\xf0\x19\x31\x38\x0\x10\x4\x83\x28\x4e\x6c\x4\x45\x64\x64\x34\x21\x0\x46\x13\x84\x60\x34\x61\x10\x46\xc\x12\x0\x4\xc1\xc0\xc8\x93\x1d\x79\x93\x37\x49\x93\x61\xc4\x20\x1\x40\x10\xc\x8c\x3c\xd9\x91\x37\x79\x93\x1a\x11\x46\xc\x12\x0\x4\xc1\xc0\xc8\x93\x1d\x79\x93\x37\x41\x93\x60\xc4\x20\x1\x40\x10\xc\x8c\x3c\xd9\x91\x37\x79\x93\x33\x29\x10\x0\x0\x0\x0\x0";

}