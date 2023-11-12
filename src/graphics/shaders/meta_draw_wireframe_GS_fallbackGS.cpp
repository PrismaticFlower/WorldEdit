#include "shader_def.hpp"

namespace we::graphics::shaders {

extern const char meta_draw_wireframe_GS_fallbackGS_dxil_bytes[2673];

auto meta_draw_wireframe_GS_fallbackGS() noexcept -> shader_def
{
   return {
      .name = "meta_draw_wireframe_GS_fallbackGS",
      .entrypoint = L"main",
      .target = L"gs_6_6",
      .file = L"meta_draw_wireframe_GS_fallbackGS.hlsl",
      .dxil = {reinterpret_cast<const std::byte*>(meta_draw_wireframe_GS_fallbackGS_dxil_bytes),
               sizeof(meta_draw_wireframe_GS_fallbackGS_dxil_bytes) - 1},
   };
}

const char meta_draw_wireframe_GS_fallbackGS_dxil_bytes[2673] = "\x44\x58\x42\x43\x90\xf9\x18\x6c\xb8\x1d\x5\x4b\x75\xb4\xb3\xf6\x66\x4b\x2\x63\x1\x0\x0\x0\x70\xa\x0\x0\x7\x0\x0\x0\x3c\x0\x0\x0\x4c\x0\x0\x0\xe0\x0\x0\x0\xb4\x1\x0\x0\xd0\x2\x0\x0\x10\x3\x0\x0\x2c\x3\x0\x0\x53\x46\x49\x30\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x49\x53\x47\x31\x8c\x0\x0\x0\x3\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\x68\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x6e\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x7a\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\xf\xf\x0\x0\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x4f\x53\x47\x31\xcc\x0\x0\x0\x5\x0\x0\x0\x8\x0\x0\x0\x0\x0\x0\x0\xa8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x0\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xae\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x3\x0\x0\x0\x1\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xba\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x2\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xba\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x3\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xba\x0\x0\x0\x2\x0\x0\x0\x0\x0\x0\x0\x3\x0\x0\x0\x4\x0\x0\x0\xf\x0\x0\x0\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x53\x56\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x50\x53\x56\x30\x14\x1\x0\x0\x30\x0\x0\x0\x3\x0\x0\x0\x5\x0\x0\x0\x1\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\xff\xff\xff\xff\x2\x0\x3\x0\x3\x3\x0\x3\x5\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x30\x0\x0\x0\x0\x43\x4f\x4c\x4f\x52\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x43\x4f\x4c\x4f\x52\x0\x46\x4c\x41\x54\x5f\x50\x4f\x53\x49\x54\x49\x4f\x4e\x50\x53\x0\x0\x0\x0\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x10\x0\x0\x0\x1\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x3\x3\x4\x0\x0\x7\x0\x0\x0\x0\x0\x0\x0\x1\x2\x44\x0\x3\x1\x0\x0\x17\x0\x0\x0\x0\x0\x0\x0\x1\x0\x44\x0\x3\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1\x44\x3\x3\x4\x0\x0\x1d\x0\x0\x0\x1\x0\x0\x0\x3\x2\x44\x0\x3\x1\x0\x0\x1\x0\x0\x0\x2\x0\x0\x0\x4\x0\x0\x0\x8\x0\x0\x0\x10\x0\x0\x0\x20\x0\x0\x0\x40\x0\x0\x0\x80\x0\x0\x0\x0\x11\x1\x0\x0\x22\x2\x0\x0\x44\x4\x0\x0\x88\x8\x0\x49\x4c\x44\x4e\x38\x0\x0\x0\x0\x0\x33\x0\x73\x68\x61\x64\x65\x72\x73\x5c\x62\x75\x69\x6c\x64\x5c\x6d\x65\x74\x61\x5f\x64\x72\x61\x77\x5f\x77\x69\x72\x65\x66\x72\x61\x6d\x65\x5f\x47\x53\x5f\x66\x61\x6c\x6c\x62\x61\x63\x6b\x47\x53\x2e\x70\x64\x62\x0\x48\x41\x53\x48\x14\x0\x0\x0\x0\x0\x0\x0\xa1\x5e\x0\xff\x71\x9a\xbe\x22\x1a\x1a\xb9\x8c\x2a\x68\x8f\x35\x44\x58\x49\x4c\x3c\x7\x0\x0\x66\x0\x2\x0\xcf\x1\x0\x0\x44\x58\x49\x4c\x6\x1\x0\x0\x10\x0\x0\x0\x24\x7\x0\x0\x42\x43\xc0\xde\x21\xc\x0\x0\xc6\x1\x0\x0\xb\x82\x20\x0\x2\x0\x0\x0\x13\x0\x0\x0\x7\x81\x23\x91\x41\xc8\x4\x49\x6\x10\x32\x39\x92\x1\x84\xc\x25\x5\x8\x19\x1e\x4\x8b\x62\x80\x14\x45\x2\x42\x92\xb\x42\xa4\x10\x32\x14\x38\x8\x18\x4b\xa\x32\x52\x88\x48\x90\x14\x20\x43\x46\x88\xa5\x0\x19\x32\x42\xe4\x48\xe\x90\x91\x22\xc4\x50\x41\x51\x81\x8c\xe1\x83\xe5\x8a\x4\x29\x46\x6\x51\x18\x0\x0\x6\x0\x0\x0\x1b\x8c\xe0\xff\xff\xff\xff\x7\x40\x2\xa8\xd\x84\xf0\xff\xff\xff\xff\x3\x20\x1\x0\x0\x0\x49\x18\x0\x0\x2\x0\x0\x0\x13\x82\x60\x42\x20\x0\x0\x0\x89\x20\x0\x0\x12\x0\x0\x0\x32\x22\x48\x9\x20\x64\x85\x4\x93\x22\xa4\x84\x4\x93\x22\xe3\x84\xa1\x90\x14\x12\x4c\x8a\x8c\xb\x84\xa4\x4c\x10\x40\x23\x0\x25\x0\x14\x66\x0\xe6\x8\xc0\x60\x8e\x0\x29\xc6\x20\x84\x14\x42\xa6\x18\x80\x10\x52\x6\xa1\x32\x0\x52\x48\xd\x4\x64\x4\x99\x2\x98\x23\x8\x0\x0\x0\x13\x14\x72\xc0\x87\x74\x60\x87\x36\x68\x87\x79\x68\x3\x72\xc0\x87\xd\xaf\x50\xe\x6d\xd0\xe\x7a\x50\xe\x6d\x0\xf\x7a\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\xa0\x7\x73\x20\x7\x6d\x90\xe\x78\xa0\x7\x73\x20\x7\x6d\x90\xe\x71\x60\x7\x7a\x30\x7\x72\xd0\x6\xe9\x30\x7\x72\xa0\x7\x73\x20\x7\x6d\x90\xe\x76\x40\x7\x7a\x60\x7\x74\xd0\x6\xe6\x10\x7\x76\xa0\x7\x73\x20\x7\x6d\x60\xe\x73\x20\x7\x7a\x30\x7\x72\xd0\x6\xe6\x60\x7\x74\xa0\x7\x76\x40\x7\x6d\xe0\xe\x78\xa0\x7\x71\x60\x7\x7a\x30\x7\x72\xa0\x7\x76\x40\x7\x43\x9e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x86\x3c\x6\x10\x0\x1\x0\x0\x0\x0\x0\x0\x0\xc\x79\x10\x20\x0\x4\x0\x0\x0\x0\x0\x0\x0\x18\xf2\x28\x40\x0\x8\x0\x0\x0\x0\x0\x0\x0\x90\x5\x2\x0\x0\x0\x11\x0\x0\x0\x32\x1e\x98\x14\x19\x11\x4c\x90\x8c\x9\x26\x47\xc6\x4\x43\x22\x25\x50\xc\xe5\x30\x2\x50\x6\xe5\x51\x4\xa5\x40\xa5\x24\x46\x0\x8a\xa0\x10\xca\xa0\x4\xa8\x8d\x15\x31\x54\x20\x10\x8\x4\x6\xa0\x0\x12\x20\x4a\xa\x20\x8c\x12\x20\x18\x23\x20\x28\x46\x0\x0\x0\x0\x79\x18\x0\x0\x53\x0\x0\x0\x1a\x3\x4c\x90\x46\x2\x13\xc4\x8e\xc\x6f\xec\xed\x4d\xc\x24\xc6\xe5\xc6\x45\x66\x6\x6\xc7\xe5\x6\x4\xc5\x26\xa7\xac\x86\xa6\x4c\x26\x7\x26\x65\x43\x10\x4c\x10\x8\x62\x82\x40\x14\x1b\x84\x81\x98\x20\x10\xc6\x6\x61\x30\x28\x9c\xcd\x6d\x18\x10\x82\x98\x20\x34\xd2\x86\x40\x99\x20\x8\x0\x89\xb6\xb0\x34\xb7\x9\x2\x71\xb0\x18\x7a\x62\x7a\x92\x9a\x20\x14\xcc\x4\xa1\x68\x36\x4\xce\x4\xa1\x70\x26\x8\xc5\x33\x41\x20\x90\x9\x2\x91\x6c\x10\x2a\x6b\xc3\xe2\x3c\x50\x24\x4d\x3\xe5\x44\x17\x97\x29\xab\x2f\xa8\xb7\xb9\x34\xba\xb4\x37\xb7\x9\x42\x1\x6d\x58\x86\xc\xd2\x24\x6a\xa0\x86\xe8\x9a\x20\x10\xa\x1f\x23\xa6\x20\xaa\x2f\xa8\xa7\xa9\x24\xaa\xa4\x27\x27\xa8\xa9\x9\x42\x11\x6d\x58\xb8\xe\x8a\x24\x6f\xa0\xb8\xe8\xda\x30\x60\xdb\xb7\x61\x70\x6\x6e\xc3\xc2\x75\x50\x14\x6\x5e\x45\x71\xd1\xb5\x61\xc0\x36\x31\xd8\x30\x80\xc1\x18\x0\x13\x4\x62\xd9\x50\x54\xd5\x50\x6\xc3\x6\x61\x30\x83\xd\x5\xd3\x90\x1\x70\x6\x55\xd8\xd8\xec\xda\x5c\xd2\xc8\xca\xdc\xe8\xa6\x4\x41\x15\x32\x3c\x17\xbb\x32\xb9\xb9\xb4\x37\xb7\x29\x1\xd1\x84\xc\xcf\xc5\x2e\x8c\xcd\xae\x4c\x6e\x4a\x60\xd4\x21\xc3\x73\x99\x43\xb\x23\x2b\x93\x6b\x7a\x23\x2b\x63\x9b\x12\x20\x75\xc8\xf0\x5c\xec\xd2\xca\xee\x92\xc8\xa6\xe8\xc2\xe8\xca\xa6\x4\x4a\x1d\x32\x3c\x97\x32\x37\x3a\xb9\x3c\xa8\xb7\x34\x37\xba\xb9\x29\xc1\x19\x0\x0\x0\x0\x79\x18\x0\x0\x51\x0\x0\x0\x33\x8\x80\x1c\xc4\xe1\x1c\x66\x14\x1\x3d\x88\x43\x38\x84\xc3\x8c\x42\x80\x7\x79\x78\x7\x73\x98\x71\xc\xe6\x0\xf\xed\x10\xe\xf4\x80\xe\x33\xc\x42\x1e\xc2\xc1\x1d\xce\xa1\x1c\x66\x30\x5\x3d\x88\x43\x38\x84\x83\x1b\xcc\x3\x3d\xc8\x43\x3d\x8c\x3\x3d\xcc\x78\x8c\x74\x70\x7\x7b\x8\x7\x79\x48\x87\x70\x70\x7\x7a\x70\x3\x76\x78\x87\x70\x20\x87\x19\xcc\x11\xe\xec\x90\xe\xe1\x30\xf\x6e\x30\xf\xe3\xf0\xe\xf0\x50\xe\x33\x10\xc4\x1d\xde\x21\x1c\xd8\x21\x1d\xc2\x61\x1e\x66\x30\x89\x3b\xbc\x83\x3b\xd0\x43\x39\xb4\x3\x3c\xbc\x83\x3c\x84\x3\x3b\xcc\xf0\x14\x76\x60\x7\x7b\x68\x7\x37\x68\x87\x72\x68\x7\x37\x80\x87\x70\x90\x87\x70\x60\x7\x76\x28\x7\x76\xf8\x5\x76\x78\x87\x77\x80\x87\x5f\x8\x87\x71\x18\x87\x72\x98\x87\x79\x98\x81\x2c\xee\xf0\xe\xee\xe0\xe\xf5\xc0\xe\xec\x30\x3\x62\xc8\xa1\x1c\xe4\xa1\x1c\xcc\xa1\x1c\xe4\xa1\x1c\xdc\x61\x1c\xca\x21\x1c\xc4\x81\x1d\xca\x61\x6\xd6\x90\x43\x39\xc8\x43\x39\x98\x43\x39\xc8\x43\x39\xb8\xc3\x38\x94\x43\x38\x88\x3\x3b\x94\xc3\x2f\xbc\x83\x3c\xfc\x82\x3b\xd4\x3\x3b\xb0\xc3\x8c\xc8\x21\x7\x7c\x70\x3\x72\x10\x87\x73\x70\x3\x7b\x8\x7\x79\x60\x87\x70\xc8\x87\x77\xa8\x7\x7a\x98\x81\x3c\xe4\x80\xf\x6e\x40\xf\xe5\xd0\xe\xf0\x30\x83\x81\xc8\x1\x1f\xdc\x40\x1c\xe4\xa1\x1c\xc2\x61\x1d\xdc\x40\x1c\xe4\x1\x0\x0\x0\x71\x20\x0\x0\xe\x0\x0\x0\x36\x0\xd\x97\xef\x3c\x3e\xc1\x20\x13\x3b\x45\x4\xc0\x58\xc0\x34\x5c\xbe\xf3\xf8\x8b\x3\xc\x62\xf3\x50\x93\x5f\xdc\xb6\x9\x54\xc3\xe5\x3b\x8f\x2f\x4d\x4e\x44\xa0\xd4\xf4\x50\x93\x5f\xdc\xb6\x1\x10\xc\x80\x34\x0\x0\x61\x20\x0\x0\x92\x0\x0\x0\x13\x4\x43\x2c\x10\x0\x0\x0\x2\x0\x0\x0\x44\xa\xa1\x84\x3\x0\x0\x0\x23\x6\x9\x0\x82\x60\x60\x50\xc2\xe2\x20\xce\x88\x41\x2\x80\x20\x18\x18\xd5\xc0\x3c\xc5\x33\x62\x90\x0\x20\x8\x6\x86\x45\x34\x50\x2\x8d\x18\x24\x0\x8\x82\x81\x71\x15\x4e\x84\x44\x23\x6\x9\x0\x82\x60\x60\x60\xc6\x23\x31\xd5\x88\x41\x2\x80\x20\x18\x18\xd9\x1\x4d\x89\x35\x62\x90\x0\x20\x8\x6\x86\x86\x44\x54\x73\x8d\x18\x24\x0\x8\x82\x81\xb1\x25\x52\xc5\x60\x23\x6\x9\x0\x82\x60\x60\x70\xca\x64\x41\xd3\x88\x41\x2\x80\x20\x18\x18\xdd\x42\x5d\xd\x35\x62\x90\x0\x20\x8\x6\x86\xc7\x54\x58\x54\x8d\x18\x24\x0\x8\x82\x81\xf1\x35\x56\x6\x59\xb3\x4\x2\xe\x54\x10\x74\x0\x4c\xc2\x88\x41\x2\x80\x20\x18\x18\x61\xf0\x6c\x5b\x15\x8c\x18\x24\x0\x8\x82\x81\x21\x6\x10\xc7\x49\xc2\x88\x41\x2\x80\x20\x18\x18\x63\x10\x75\x9d\x35\x8c\x18\x24\x0\x8\x82\x81\x41\x6\x92\xe7\x55\xc4\x88\x41\x2\x80\x20\x18\x18\x65\x30\x89\xc1\x97\x15\x23\x6\x9\x0\x82\x60\x60\x98\x1\x35\x6\x60\x60\x19\x23\x6\x9\x0\x82\x60\x60\x9c\x41\x45\x6\x61\xa0\x1d\x23\x6\x9\x0\x82\x60\x60\xa0\x81\x55\x6\x62\x90\x21\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x37\x6\x63\xd0\x21\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x37\x6\x63\x90\x1d\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x37\x6\x63\xc0\x19\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x37\x6\x63\xa0\x15\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x67\x6\x63\xd0\x11\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x67\x6\x63\x90\xd\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x67\x6\x63\xc0\x9\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x67\x6\x63\xa0\x5\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x63\xd0\x55\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x63\x90\x51\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x63\xc0\x4d\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x63\xa0\x49\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x66\xd0\x45\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x66\x90\x41\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x66\xc0\x3d\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x66\xa0\x39\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x60\xd0\x35\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x60\x90\x31\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x60\xc0\x2d\x23\x6\x9\x0\x82\x60\x80\xa0\xc1\x7\x6\x60\xa0\x29\x23\x6\x6\x0\x82\x60\xa0\x9c\x81\xd5\x59\x62\x6\x30\x18\x6e\x8\xc6\x0\xc\x66\x19\x2\x21\x0\x0\x0\x0\x0\x0\x0\x0";

}