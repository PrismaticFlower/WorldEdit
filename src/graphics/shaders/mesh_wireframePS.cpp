#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char mesh_wireframePS_dxil_bytes[3313];

auto mesh_wireframePS() noexcept -> shader_def
{
   return {
      .name = "mesh_wireframePS",
      .entrypoint = L"main",
      .target = L"ps_6_6",
      .file = L"mesh_wireframePS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(mesh_wireframePS_dxil_bytes),
               sizeof(mesh_wireframePS_dxil_bytes) - 1},
   };
}

const char mesh_wireframePS_dxil_bytes[3313] = "\x44\x58\x42\x43\x92\x58\xf1\x1\x6e\xe2\x24\xa1\x70\x5\x54\x23\x4e\xe7\xfe\xc1\x1\x0\x0\x0\xf0\xc\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xb8\x0\x0\x0\xf4\x0\x0\x0\xdc\x1\x0\x0\xc\x2\x0\x0\x28\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x2\x0\x0\x0\x0\x0\x49\x53\x47\x31\x64\x0\x0\x0\x2\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x48\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x54\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x4f\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x40\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x54\x61\x72\x67\x65\x74\x0\x0\x0\x50\x53\x56\x30\xe0\x0\x0\x0\x30\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x0\x0\x0\x0\x2\x1\x0\x2\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x5\x0\x0\x0\x5\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x14\x0\x0\x0\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x3\x3\x4\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x0\x3\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x10\x3\x0\x0\x0\x8\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x28\x0\x0\x0\x0\x0\x22\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x73\x68\x5f\x77\x69\x72\x65\x66\x72\x61\x6d\x65\x50\x53\x2e\x70\x64\x62\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\x6\x34\x46\x3a\xa0\x40\x89\x7\x37\x69\x94\x57\xcb\x1a\x76\xf5\x44\x58\x49\x4c\xc0\xa\x0\x0\x66\x0\x0\x0\xb0\x2\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\xa8\xa\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\xa7\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x4c\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\xb0\xc1\x8\x40\x9\x0\xa\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x40\x10\x44\x41\x90\x51\xc\x80\x20\x88\x62\x20\x64\x8e\x20\x28\x3\x40\x28\xb4\x14\x63\x20\x8\xa2\x28\xa8\x29\xc4\x40\xc\x3\x3d\x65\x18\x88\x81\xa2\xa3\x86\xcb\x9f\xb0\x87\x90\x7c\x6e\xa3\x8a\x95\x98\xfc\xe2\xb6\x11\x31\xc\xc3\x40\xc5\x3d\xc3\xe5\x4f\xd8\x43\x48\x7e\x8\x34\xc3\x42\xa0\x80\x2a\xc4\x44\x54\x4\x59\xb7\xd\x97\x3f\x61\xf\x21\xf9\x2b\x21\x39\x54\x24\x10\x69\xe4\x3c\x44\x34\x21\x84\x84\x4\x82\x28\x44\x45\x54\x18\x65\x7\xd\x97\x3f\x61\xf\x21\xf9\x2b\x21\x6d\x48\x33\x20\x82\x20\x88\xa2\x14\x15\xb1\x11\xa\x71\x3\x1\xc3\x8\xc4\x90\x4\xdf\x30\xc2\x30\xc\x23\x8\xc3\x9d\xc1\xe1\x48\xd3\x2\x60\xe\x35\xf9\xd2\x14\x51\xc2\xe4\x17\x11\xc0\x10\xbf\xe0\x34\xd2\x4\x34\xd3\x1f\x50\x45\x41\x44\xc8\x0\xc3\x41\x14\x87\x9b\xe\x47\x9a\x16\x0\x73\xa8\xc9\x17\x82\xbf\x88\x0\x86\x40\xc1\x48\xe4\x69\xd2\x14\x51\xc2\xe4\x5b\x48\x44\x14\x11\xc0\x10\xbf\xe0\x34\xd2\x4\x34\x93\x84\x82\xf0\x2c\x21\xf8\x2d\x24\x22\x8a\x8\x60\x88\x5f\x70\x1a\x69\x2\x9a\x49\x42\x81\x49\x67\x2a\x10\x73\x4\xa0\x30\x5\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x2c\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x69\x80\x0\x8\x0\x0\x0\x0\x0\x0\x0\x60\xc8\xf3\x0\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\x27\x2\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x21\x8f\x5\x4\xc0\x0\x0\x0\x0\x0\x0\x0\x0\x43\x9e\xc\x8\x80\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x1c\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\x64\x81\x0\x11\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x4a\xa0\x18\xca\x61\x4\xa0\x90\x3\x4a\xa1\x30\xca\xa0\x3c\xa8\x28\x89\x32\x28\x84\x11\x80\x12\x28\x10\x52\xb\x12\x10\x90\x80\xd0\x19\x0\x4a\xc7\x52\x10\x2\x41\x0\x0\x0\x0\x80\xca\x19\x0\x0\x0\x0\x79\x18\x0\x0\x5f\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x8\x65\x82\x40\x2c\x1b\x84\x81\x98\x20\x10\xcc\x6\x61\x30\x28\xc0\xcd\x6d\x18\x10\x82\x98\x20\x10\xcd\x4\xa1\xc\x38\x2\x13\x4\xc2\xd9\x80\x28\xb\xa3\x28\x43\x3\x4c\x10\xd0\x40\x9b\x20\x10\xcf\x4\x81\x80\x36\x20\xc3\xc3\x28\xd0\x10\x1\x1b\x4\x47\xda\x40\x0\xc0\x4\x4c\x10\xd2\x60\xdb\x10\x54\x13\x4\x1\x20\xd1\x16\x96\xe6\xc6\x65\xca\xea\xb\xea\x6d\x2e\x8d\x2e\xed\xcd\x6d\x82\x50\x4c\x13\x84\x82\xda\x10\x28\x13\x84\xa2\x9a\x20\x14\xd6\x4\x81\x88\x36\x8\xdf\xb7\x61\x51\x32\x6d\xe3\xba\xa1\x53\x3c\x30\xe0\x63\xc4\x14\x44\xf5\x5\xf5\x34\x95\x44\x95\xf4\xe4\x4\x35\x35\x41\x28\xae\xd\xcb\x20\x6\x9a\xc7\x8d\xc1\xd0\xd\x1e\xb0\x41\x8\x3\x32\x60\x32\x65\xf5\x45\x15\x26\x77\x56\x46\x37\x41\x28\xb0\x9\x2\x21\x6d\x10\x3e\x34\xd8\xb0\x28\x66\xa0\x9d\x1\xe7\xd\x9d\xe2\xa5\xc1\x86\x40\xd\x36\xc\x65\xb0\x6\xc0\x4\x41\xd\xb2\xd\x82\xd2\x6\x1b\x8a\xb\x63\x3\xca\xd\xaa\xb0\xb1\xd9\xb5\xb9\xa4\x91\x95\xb9\xd1\x4d\x9\x82\x2a\x64\x78\x2e\x76\x65\x72\x73\x69\x6f\x6e\x53\x2\xa2\x9\x19\x9e\x8b\x5d\x18\x9b\x5d\x99\xdc\x94\xc0\xa8\x43\x86\xe7\x32\x87\x16\x46\x56\x26\xd7\xf4\x46\x56\xc6\x36\x25\x40\xca\x90\xe1\xb9\xc8\x95\xcd\xbd\xd5\xc9\x8d\x95\xcd\x4d\x9\xa6\x3a\x64\x78\x2e\x76\x69\x65\x77\x49\x64\x53\x74\x61\x74\x65\x53\x82\xaa\xe\x19\x9e\x4b\x99\x1b\x9d\x5c\x1e\xd4\x5b\x9a\x1b\xdd\xdc\x94\xc0\xd\x0\x0\x0\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\x2a\x0\x0\x0\x86\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x18\xc1\x36\x5c\xbe\xf3\xf8\xc0\x34\x45\x48\x40\x4d\x84\x36\xbd\x44\x34\x11\x97\x5f\xdc\xb6\x15\x40\xc3\xe5\x3b\x8f\x1f\x20\xd\x10\x61\x7e\x71\xdb\x76\xb0\xd\x97\xef\x3c\xbe\x10\x50\x45\x41\x44\xa5\x3\xc\x25\x61\x0\x2\xe6\x17\xb7\x6d\x9\xdd\x70\xf9\xce\xe3\xb\x11\x1\x4c\x44\x8\x34\xc3\x42\x7c\x91\xc3\x6c\x48\x33\x20\x8d\x61\x3\xcd\x70\xf9\xce\xe3\xf\x88\x24\x0\xd1\x60\x1\xd3\x70\xf9\xce\xe3\x2f\xe\x30\x88\xcd\x43\x4d\x7e\x71\xdb\x26\x50\xd\x97\xef\x3c\xbe\x34\x39\x11\x81\x52\xd3\x43\x4d\x7e\x71\xdb\x66\xf0\xc\x97\xef\x3c\x3e\xd5\x0\x11\xe6\x17\xb7\x6d\x0\x4\x3\x20\xd\x0\x0\x61\x20\x0\x0\xfb\x0\x0\x0\x13\x4\x43\x2c\x10\x0\x0\x0\x17\x0\x0\x0\xa4\xd4\xc0\x8\x0\x15\x45\x40\x44\x91\x6\x94\x4a\x19\x95\xa4\x40\xd9\x15\x52\xc1\x94\xec\x40\xc1\xe\x94\x46\x21\xcc\x0\xd0\x30\x46\x20\xb2\xa0\x88\xf7\xc2\x18\x1\x8\x82\x20\x8\xe\x63\x4\x20\x8\x82\x20\x18\x8c\x0\x8c\x11\x80\x20\x8\xe2\xdf\x18\x1\x8\x82\x20\xfc\x11\x36\x87\x90\x6\xd0\x1c\x42\x1a\x38\xb4\xcd\x41\x3c\x4f\xf3\xcd\x41\x34\x4d\xf3\x1\x0\x23\x6\x8\x0\x82\x60\xc0\xb5\xc1\x23\x9c\x1\x36\x62\x80\x0\x20\x8\x6\x9c\x1b\x40\x82\x1a\x64\x23\x6\x7\x0\x82\x60\x90\xc1\x1\x24\x18\x23\x6\x7\x0\x82\x60\x90\xc5\x41\x24\x18\x23\x6\x9\x0\x82\x60\x60\xe4\x1\xd4\x6\x6d\x40\x6\xcf\x88\x41\x2\x80\x20\x18\x18\x7a\x10\xb9\x81\x1b\x90\x1\x34\x62\x90\x0\x20\x8\x6\xd\x1e\x64\x72\xf0\x6\x66\x60\x6\x23\x6\x9\x0\x82\x60\xd0\xe4\x81\x36\x7\x70\x60\x6\x67\x30\x62\x90\x0\x20\x8\x6\x8d\x1e\x6c\x74\x10\x7\x6a\x80\x6\x23\x6\x9\x0\x82\x60\xd0\xec\x1\x57\x7\x72\x90\x6\x68\x30\x62\x90\x0\x20\x8\x6\xd\x1f\x74\x76\x30\x7\x69\x90\x6\x23\x6\x9\x0\x82\x60\xd0\xf4\x81\x77\x7\x74\xd0\x6\x6a\x30\x62\x90\x0\x20\x8\x6\x8d\x1f\x7c\x78\x50\x7\x6c\x30\x6\x23\x6\x9\x0\x82\x60\xd0\xfc\x1\x18\xe4\x81\x1d\xb0\x1\x19\x8c\x18\x24\x0\x8\x82\x41\x3\xa\x61\xa0\x7\x77\x0\x7\x65\x60\xc9\x41\x1f\x4b\x10\xfa\x8c\x18\x1c\x0\x8\x82\x81\x5\xa\x63\xe0\xe0\xc1\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\x83\x1d\x1c\x7c\x2c\xe0\xe4\x63\x81\x21\x1f\xb\xa\xf8\x18\x18\x28\xf1\xb1\x0\xc\xe4\x63\x41\x22\x1f\xb\x10\xf8\x58\x35\xd1\xc7\x2a\x8a\x3e\x26\x98\x1\x7c\x2c\x30\x3\xf9\x58\x0\xc9\xc7\x82\x7\x3e\xa6\x6\x45\x7c\x2c\x50\x3\xf9\x58\x30\xc9\xc7\x2\x9\x3e\xc6\x69\xf4\x31\x6e\xa3\x8f\x9\x70\x0\x1f\xb\xe0\x40\x3e\x16\x68\xf2\xb1\x20\x83\x8f\xd1\x41\x11\x1f\xb\xe8\x40\x3e\x16\x74\xf2\xb1\x80\x83\x8f\x39\x58\x7c\x2c\xc0\x3\xf9\x58\x18\x4\xf2\xb1\x20\xe\xe4\x63\x16\x18\xc4\xc7\x2\x3e\x90\x8f\x99\x41\x20\x1f\xb\xe8\x40\x3e\x76\x75\xf2\x31\x4a\xc\xe4\x63\x42\x10\x1f\xb\x88\xf8\x58\x90\xc0\x67\xc4\xc0\x0\x40\x10\xc\xa2\x77\x50\x87\xc0\x18\x46\x3e\x96\x24\xf2\x31\x21\x80\xcf\x88\x81\x1\x80\x20\x18\x44\xf3\x30\xb\x81\x15\x1\x7d\xec\xa\x83\xf8\x58\xb0\xa\xf2\x31\x3a\x8\xe4\x63\x1\x29\xc8\xc7\xbe\x34\x88\x8f\x5\xaf\x20\x1f\xcb\x83\x40\x3e\x16\x9c\x82\x7c\xc\xc\xcc\x40\x3e\xd6\xad\x81\x7c\x4c\x8\xe2\x63\x1\x11\x1f\xb\x12\xf8\x8c\x18\x18\x0\x8\x82\x41\x24\x12\xfd\x10\x18\xc3\xc8\xc7\x92\x44\x3e\x26\x4\xf0\x19\x31\x30\x0\x10\x4\x83\xc8\x24\xcc\x21\xb0\x22\xa0\x8f\xf9\x81\x1a\xc4\xc7\x2\x5f\x90\x8f\x9d\x42\x20\x1f\xb\x6e\x41\x3e\x66\xa\x72\x10\x1f\xb\xc4\x41\x3e\xc6\xa\x81\x7c\x2c\xd0\x5\xf9\xd8\x1b\x9c\x82\x7c\x8c\xe\x48\x41\x3e\x26\x4\xf1\xb1\x80\x88\x8f\x5\x9\x7c\x46\xc\xc\x0\x4\xc1\x20\xaa\x9\x98\x8\x8c\x61\xe4\x63\x49\x22\x1f\x13\x2\xf8\x8c\x18\x18\x0\x8\x82\x41\x94\x13\xf9\x10\x58\x11\xd0\x67\xc4\xe0\x0\x40\x10\xc\x1e\x9e\xe0\x87\x33\xa0\x46\xc\xe\x0\x4\xc1\xe0\xe9\x89\x7e\x8\x84\x11\x83\x3\x0\x41\x30\xb0\x78\xe2\x1f\xd4\x1\x27\x46\x13\x2\xc0\x2\x7a\x90\x8f\x11\xf6\x0\x1f\xb\x84\xf8\x8c\x18\x1c\x0\x8\x82\xc1\x33\x16\x26\x11\xe4\xc3\x88\xc1\x1\x80\x20\x18\x3c\x64\x41\x12\xc1\x3e\x58\x10\xc8\xc7\x2\x7f\x90\xcf\x88\x81\x1\x80\x20\x18\x44\x66\xb1\x12\x81\x5\x21\x1\x9f\xe1\x88\xe0\x1f\x82\x6f\x96\x21\x10\x82\x11\x3\x3\x0\x41\x30\x58\xd8\xe2\x25\x64\x62\x96\x40\x18\x31\x38\x0\x10\x4\x3\xb\x2d\x56\xe2\x1e\xc4\x62\x34\x21\x0\x46\x13\x84\x60\x34\x61\x10\x46\xc\xe\x0\x4\xc1\xc0\x62\x8b\x97\xd0\x7\xb4\x18\x4d\x8\x0\xb\x48\x42\x3e\x6\x5\xf1\x19\x31\x38\x0\x10\x4\x83\x47\x2e\x6a\x22\x40\x89\x11\x83\x3\x0\x41\x30\x78\xe6\x62\x26\x2\x95\xb0\x20\x90\x8f\x5\x2d\x21\x9f\x11\x3\x3\x0\x41\x30\x88\xea\x42\x27\x2\xb\x60\x2\x3e\x23\x6\x9\x0\x82\x60\x80\xec\x85\x5b\xc0\x5\x5c\x9c\x45\x33\x62\x90\x0\x20\x8\x6\xc8\x5e\xb8\x5\x5c\xc0\x85\x59\x30\x23\x6\x9\x0\x82\x60\x80\xec\x85\x5b\xc0\x5\x5c\xf8\xc4\x32\x62\x90\x0\x20\x8\x6\xc8\x5e\xb8\x5\x5c\xc0\x45\x5a\x4\x8\x0\x0\x0\x0\x0";

}