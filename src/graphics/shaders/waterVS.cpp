#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char waterVS_dxil_bytes[3097];

auto waterVS() noexcept -> shader_def
{
   return {
      .name = "waterVS",
      .entrypoint = L"main",
      .target = L"vs_6_6",
      .file = L"waterVS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(waterVS_dxil_bytes),
               sizeof(waterVS_dxil_bytes) - 1},
   };
}

const char waterVS_dxil_bytes[3097] = "\x44\x58\x42\x43\xe1\x88\x65\x5a\xf1\xa3\x8b\xe4\x28\x55\x1c\xce\xc1\x73\xa4\xd7\x1\x0\x0\x0\x18\xc\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\x88\x0\x0\x0\xf0\x0\x0\x0\xd8\x1\x0\x0\x0\x2\x0\x0\x1c\x2\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x34\x0\x0\x0\x1\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x28\x0\x0\x0\x0\x0\x0\x0\x6\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x1\x0\x0\x0\x0\x0\x0\x53\x56\x5f\x56\x65\x72\x74\x65\x78\x49\x44\x0\x4f\x53\x47\x31\x60\x0\x0\x0\x2\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x48\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\x7\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x53\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x57\x53\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x0\x50\x53\x56\x30\xe0\x0\x0\x0\x30\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x1\x0\x0\x0\x1\x2\x0\x1\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x18\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x7\x0\x0\x0\x7\x0\x0\x0\xd\x0\x0\x0\x0\x0\x0\x0\x5\x0\x0\x0\x0\x0\x0\x0\x2\x0\x0\x0\x2\x0\x0\x0\xc\x0\x0\x0\x0\x0\x0\x0\xc\x0\x0\x0\x0\x50\x4f\x53\x49\x54\x49\x4f\x4e\x57\x53\x0\x1\x0\x0\x0\x0\x0\x0\x0\x10\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x41\x1\x1\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x43\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x3\x3\x4\x0\x0\xf7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x4c\x44\x4e\x20\x0\x0\x0\x0\x0\x19\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x77\x61\x74\x65\x72\x56\x53\x2e\x70\x64\x62\x0\x0\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\xe6\x47\xb1\xa1\x9\x99\xe7\xa3\x5d\xee\x53\x8a\x93\x8b\xe\xe6\x44\x58\x49\x4c\xf4\x9\x0\x0\x66\x0\x1\x0\x7d\x2\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\xdc\x9\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\x74\x2\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x18\x45\x2\x42\x92\xb\x42\xc4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x62\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x11\x23\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x31\x46\x6\x51\x18\x0\x0\x8\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x6d\x30\x86\xff\xff\xff\xff\x1f\x0\x9\xa8\x0\x49\x18\x0\x0\x3\x0\x0\x0\x13\x82\x60\x42\x20\x4c\x8\x6\x0\x0\x0\x0\x89\x20\x0\x0\x60\x0\x0\x0\x32\x22\x88\x9\x20\x64\x85\x4\x13\x23\xa4\x84\x4\x13\x23\xe3\x84\xa1\x90\x14\x12\x4c\x8c\x8c\xb\x84\xc4\x4c\x10\xb0\xc1\x8\x40\x9\x0\xa\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x30\xc\x3\x31\x50\x31\x3\x50\xc\x60\x18\x6\xe2\x20\xe4\xa6\xe1\xf2\x27\xec\x21\x24\x7f\x25\xa4\x95\x98\xfc\xe2\xb6\x51\x71\x1c\xc7\x31\x10\x71\xcf\x70\xf9\x13\xf6\x10\x92\x1f\x2\xcd\xb0\x10\x28\x58\xca\xa1\xc\xcc\x30\x10\x3\x35\x47\xd\x97\x3f\x61\xf\x21\xf9\xdc\x46\x15\x2b\x31\xf9\xc5\x6d\x23\xe2\x38\x8e\xa3\x10\xcf\xc0\xc\x4\x95\xe2\x18\x8e\xe3\x20\xe9\xb6\xe1\xf2\x27\xec\x21\x24\x7f\x25\x24\x87\x8a\x4\x22\x8d\x9c\x87\x88\x26\x84\x90\x90\x30\xc\x85\x60\x6\x86\xa2\xea\xa0\xe1\xf2\x27\xec\x21\x24\x7f\x25\xa4\xd\x69\x6\x44\xc\xc3\x40\xcc\x11\x4\xa5\x60\x86\x6b\xc0\x28\x1b\x8\x18\x46\x10\x8e\x8b\xa4\x29\xa2\x84\xc9\x7f\x80\x49\x38\xfe\x20\x70\x2e\x24\x70\xdc\x4c\x62\x30\xe\xec\x10\xe\xf3\x30\xf\x6e\x30\xb\xf4\x20\xf\xf5\x30\xe\xf4\x50\xf\xf2\x50\xe\xe4\x20\xa\xf5\x60\xe\xe6\x50\xe\xf2\xc0\x7\xf0\x10\xe\xf4\x30\xe\xe8\xf0\xb\xe2\x20\xe\xef\x80\xf\x7e\x80\x82\x8e\xbc\x61\x4\xe2\x48\x2\x70\x18\x61\x38\xee\xc\xe\x47\x9a\x16\x0\x73\xa8\xc9\x97\xa6\x88\x12\x26\xbf\x88\x0\x86\xf8\x5\xa7\x91\x26\xa0\x99\xfe\x80\x2a\xa\x22\x42\x46\x28\x1e\x1c\x77\xdc\x74\x38\xd2\xb4\x0\x98\x43\x4d\xbe\x10\xfc\x45\x4\x30\x4\xa\x46\x22\x8f\x93\xa6\x88\x12\x26\xdf\x2\x26\x22\xfa\x5\xa7\x91\x26\xa0\x99\xfe\x80\x2a\xa\x22\x42\x3\x78\xc\x87\x8\xc1\x6f\x1\x13\x11\xa1\xc0\xa4\x33\x7\x86\x39\x2\x50\x98\x2\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x5\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x34\x40\x0\xc\x0\x0\x0\x0\x0\x0\x0\x30\xe4\x81\x80\x0\x18\x0\x0\x0\x0\x0\x0\x0\x60\xc8\x23\x1\x1\x10\x0\x0\x0\x0\x0\x0\x0\xc0\x90\xa7\x2\x2\x20\x0\x0\x0\x0\x0\x0\x0\x80\x21\x4f\x6\x4\x40\x0\x0\x0\x0\x0\x0\x0\x0\x59\x20\x0\x0\x0\x11\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x1a\x4a\xa0\x18\xca\x61\x4\xa0\x8\xa\xa3\x40\xa\x39\xa0\x60\xca\xa0\x3c\x88\x28\x85\x12\x18\x1\x28\x89\x22\x28\x83\x42\x20\xb5\x40\xa8\x9c\x1\x20\x74\x6\x80\xd2\xb1\x86\x0\xb9\xf\xfa\x66\x0\x0\x0\x0\x79\x18\x0\x0\x69\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x6\x64\x82\x30\x24\x1b\x84\x81\x98\x20\xc\xca\x6\x61\x30\x28\xd8\xcd\x6d\x18\x10\x82\x98\x20\xc\xcb\x4\xe1\xf3\x8\x4c\x10\x6\x66\x82\x30\x34\x13\x84\xc1\xd9\x20\xc\xcf\x86\x44\x59\x18\xa5\x19\x1c\x5\xda\x10\x44\x13\x84\x32\xd8\x26\x8\xc3\xb3\x1\x51\x26\x46\x51\x6\xa\x98\x20\xa0\x1\x37\x41\x18\xa0\xd\xc8\x60\x31\x8a\x31\x5c\xc0\x6\xa1\xc2\x36\x10\x12\x90\x1\x13\x84\x34\xe8\x36\x4\xdb\x4\x41\x0\x48\xb4\x85\xa5\xb9\x71\x99\xb2\xfa\xb2\x2a\x93\xa3\x2b\xc3\x4b\x22\x9a\x20\x10\xd3\x4\x81\xa0\x36\x4\xca\x4\x81\xa8\x26\x8\x43\xb4\x41\x20\x83\x61\xc3\xa2\x7c\x60\x10\x6\x62\x30\x6\x43\x18\x28\x63\x50\x6\x1b\x2\x33\xa0\x2\xf5\x34\x95\x44\x95\xf4\xe4\x74\x35\x35\x41\x20\xac\x9\x2\x71\x4d\x10\x8\x6c\x83\x40\x6\xc6\x86\x45\x41\x83\x34\x18\x3\x31\x50\x83\x61\xd\x94\x31\x60\x3\x2e\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x13\x4\x22\x9b\x20\xc\xd2\x6\x81\xc\xe0\x60\xc3\x32\xb8\x41\x1a\xac\x81\x18\xbc\xc1\xf0\x6\xc3\x18\xc4\xc1\x6\xa1\xd\xe4\x60\xc3\x70\x6\x73\x0\x4c\x10\xd4\x40\xdb\x20\x28\x75\xb0\xa1\xe8\x3c\x3a\xd0\xec\xa0\xa\x1b\x9b\x5d\x9b\x4b\x1a\x59\x99\x1b\xdd\x94\x20\xa8\x42\x86\xe7\x62\x57\x26\x37\x97\xf6\xe6\x36\x25\x20\x9a\x90\xe1\xb9\xd8\x85\xb1\xd9\x95\xc9\x4d\x9\x8c\x3a\x64\x78\x2e\x73\x68\x61\x64\x65\x72\x4d\x6f\x64\x65\x6c\x53\x2\xa4\xc\x19\x9e\x8b\x5c\xd9\xdc\x5b\x9d\xdc\x58\xd9\xdc\x94\x20\xab\x43\x86\xe7\x62\x97\x56\x76\x97\x44\x36\x45\x17\x46\x57\x36\x25\xd8\xea\x90\xe1\xb9\x94\xb9\xd1\xc9\xe5\x41\xbd\xa5\xb9\xd1\xcd\x4d\x9\xec\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\xc\xc4\x21\x7\x7c\x70\x3\x7a\x28\x87\x76\x80\x87\x19\xd1\x43\xe\xf8\xe0\x6\xe4\x20\xe\xe7\xe0\x6\xf6\x10\xe\xf2\xc0\xe\xe1\x90\xf\xef\x50\xf\xf4\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\x23\x0\x0\x0\x66\x40\xd\x97\xef\x3c\x3e\xd0\x34\xce\x4\x4c\x44\x8\x34\xc3\x42\x18\xc1\x36\x5c\xbe\xf3\xf8\x42\x40\x15\x5\x11\x95\xe\x30\x94\x84\x1\x8\x98\x5f\xdc\xb6\x1d\x74\xc3\xe5\x3b\x8f\x2f\x44\x4\x30\x11\x21\xd0\xc\xb\xf1\x45\xe\xb3\x21\xcd\x80\x34\x86\x5\x4c\xc3\xe5\x3b\x8f\xbf\x38\xc0\x20\x36\xf\x35\xf9\xc8\x6d\xdb\xc0\x35\x5c\xbe\xf3\xf8\x11\x60\x6d\x54\x51\x10\x51\xe9\x0\x83\x5f\xdc\xb6\x9\x54\xc3\xe5\x3b\x8f\x2f\x4d\x4e\x44\xa0\xd4\xf4\x50\x93\x5f\xdc\xb6\x15\x48\xc3\xe5\x3b\x8f\x3f\x11\xd1\x84\x0\x11\xe6\x17\xb7\x6d\x0\x4\x3\x20\xd\x0\x61\x20\x0\x0\xb7\x0\x0\x0\x13\x4\x48\x2c\x10\x0\x0\x0\xf\x0\x0\x0\x34\x94\x5d\x29\x14\x57\x21\x94\xec\x40\xc1\xe\x94\x46\x59\xa\xcc\x0\x14\x26\x42\x41\x10\x36\x2\x40\xd4\x1c\x42\x19\x40\x73\x8\x65\xf0\xcc\x21\xa0\x81\x43\xd7\x1c\x4\xc3\x2c\xd5\x1c\x84\xa2\x2c\xd7\x1c\xc4\xb2\x2c\x17\x1d\x23\x0\x0\x23\x6\x8\x0\x82\x60\x90\xad\xc1\x43\x98\x1\x32\x62\x80\x0\x20\x8\x6\x19\x1b\x40\x44\x1a\x24\x23\x6\x8\x0\x82\x60\x90\xb5\x41\x44\xa4\x81\x32\x62\x70\x0\x20\x8\x6\xd5\x1b\x44\x82\x32\x62\x70\x0\x20\x8\x6\x15\x1c\x48\x82\x32\x62\x90\x0\x20\x8\x6\x85\x1d\x54\x6c\xc0\x6\x62\x0\x55\xf0\x6\x33\x62\x70\x0\x20\x8\x6\xd5\x1c\x54\x7\x33\x62\xa0\x0\x20\x8\x6\xcd\x1d\x50\x81\xf0\x6\x62\x80\x8d\x26\x4\xc0\x68\x82\x10\x8c\x18\x28\x0\x8\x82\x41\xa3\x7\x17\x51\x50\x65\xb0\x8d\x26\x4\xc0\x68\x82\x10\x54\x72\x7\x35\xcc\x33\x4\xc7\x12\x20\x2\x33\x44\x84\x18\x14\x60\x60\x8c\x18\x1c\x0\x8\x82\x1\xd4\x7\x62\xc0\xe4\xc1\x68\x42\x0\xcc\x12\x1c\x23\x6\x7\x0\x82\x60\x0\xfd\x1\x19\x38\x7b\x30\x9a\x10\x0\xb3\x4\xc7\x88\xc1\x1\x80\x20\x18\x40\xa1\x60\x6\x50\x1f\x8c\x26\x4\xc0\x2c\xc1\x31\x62\x70\x0\x20\x8\x6\xd0\x28\xa0\x81\xf4\x7\xa3\x9\x1\x30\x4b\x70\x8c\x18\x1c\x0\x8\x82\x1\x54\xa\x6a\x40\x85\xc2\x68\x42\x0\xcc\x12\x1c\x23\x6\x7\x0\x82\x60\x0\x9d\x2\x1b\x58\xa3\x30\x9a\x10\x0\xb3\x4\xc7\x40\xcf\x81\x7\x0\x19\x18\x5e\xe1\x11\x64\x30\x78\x2\x19\x4\x3\x3d\x87\x1e\x0\x84\x81\x14\xc\x1\xd\x94\x80\x5\x3\x3d\x7\x1f\x0\x60\x60\x98\x41\x1\x6\x4\x18\xc\x66\x20\x98\x41\x30\x62\x70\x0\x20\x8\x6\x10\x2b\xc4\x81\x56\xa\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x30\x9a\x40\xc\x23\x6\x7\x0\x82\x60\x0\xc5\x82\x1d\x7c\xad\x30\x9a\x10\x0\xa3\x9\x42\x30\x9a\x30\x8\xa3\x9\xc4\x30\x62\x70\x0\x20\x8\x6\x90\x2d\xec\x1\x19\xb8\xc2\x68\x42\x0\x8c\x26\x8\xc1\x68\xc2\x20\x8c\x26\x10\xc3\x88\xc1\x1\x80\x20\x18\x40\xbb\x0\xa\x69\xe0\xa\xa3\x9\x1\x30\x9a\x20\x4\xa3\x9\x83\x30\x9a\x40\xc\x36\x5d\xf2\x19\x31\x40\x0\x10\x4\x3\x9\x1c\x48\xe1\xb9\x82\x11\x3\x4\x0\x41\x30\x90\xc2\xa1\x14\x96\x2b\xb0\xe0\x80\x8e\x59\x9b\x7c\x46\xc\x10\x0\x4\xc1\x40\x22\x7\x54\x90\xb6\x60\xc4\x0\x1\x40\x10\xc\xa4\x72\x48\x5\x67\xb\x2c\x50\xa0\x63\xd9\x27\x9f\x11\x3\x4\x0\x41\x30\x90\xd0\x81\x15\xaa\x2f\x18\x31\x40\x0\x10\x4\x3\x29\x1d\x5a\x21\xfa\x2\xb\x1a\xe8\x18\x37\x6\xf2\x19\x31\x40\x0\x10\x4\x3\x89\x1d\x60\x1\x1b\x83\x60\xc4\x0\x1\x40\x10\xc\xa4\x76\x88\x5\x6a\xc\x2\xb\x20\xe8\x8c\x18\x24\x0\x8\x82\x1\x22\xf\xb4\x90\xe\xe9\xf0\xb\x67\x30\x62\x90\x0\x20\x8\x6\x88\x3c\xd0\x42\x3a\xa4\x3\x38\x98\xc1\x88\x41\x2\x80\x20\x18\x20\xf2\x40\xb\xe9\x90\xe\xbd\x50\x6\x23\x6\x9\x0\x82\x60\x80\xc8\x3\x2d\xb0\x43\x3a\xfc\x42\x33\x62\x90\x0\x20\x8\x6\x88\x3c\xd0\x2\x3b\xa4\x3\x38\x24\x23\x6\x9\x0\x82\x60\x80\xc8\x3\x2d\xb0\x43\x3a\xf4\x42\x31\x62\x90\x0\x20\x8\x6\x88\x3c\xd0\x2\x3b\xa4\x3\x2f\x4\x8\x0\x0\x0\x0\x0";

}