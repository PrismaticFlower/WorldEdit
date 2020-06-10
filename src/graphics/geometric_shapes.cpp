
#include "geometric_shapes.hpp"
#include "math/align.hpp"

#include <array>

#include <d3dx12.h>

#pragma warning(disable : 4324) // structure was padded due to alignment specifier

namespace sk::graphics {

namespace {
struct geometric_shapes_buffer {

   alignas(D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT) const std::array<float3, 162> icosphere_vertices{
      {{0.000000f, -1.000000f, 0.000000f},
       {-0.203181f, -0.967950f, -0.147618f},
       {0.077607f, -0.967950f, -0.238853f},
       {-0.723607f, -0.447220f, -0.525725f},
       {-0.609547f, -0.657519f, -0.442856f},
       {-0.812729f, -0.502301f, -0.295238f},
       {0.251147f, -0.967949f, 0.000000f},
       {0.077607f, -0.967950f, 0.238853f},
       {-0.203181f, -0.967950f, 0.147618f},
       {-0.860698f, -0.251151f, -0.442858f},
       {0.276388f, -0.447220f, -0.850649f},
       {0.029639f, -0.502302f, -0.864184f},
       {0.155215f, -0.251152f, -0.955422f},
       {0.894426f, -0.447216f, 0.000000f},
       {0.831051f, -0.502299f, -0.238853f},
       {0.956626f, -0.251149f, -0.147618f},
       {0.276388f, -0.447220f, 0.850649f},
       {0.483971f, -0.502302f, 0.716565f},
       {0.436007f, -0.251152f, 0.864188f},
       {-0.723607f, -0.447220f, 0.525725f},
       {-0.531941f, -0.502302f, 0.681712f},
       {-0.687159f, -0.251152f, 0.681715f},
       {-0.687159f, -0.251152f, -0.681715f},
       {0.436007f, -0.251152f, -0.864188f},
       {0.956626f, -0.251149f, 0.147618f},
       {0.155215f, -0.251152f, 0.955422f},
       {-0.860698f, -0.251151f, 0.442858f},
       {-0.276388f, 0.447220f, -0.850649f},
       {-0.483971f, 0.502302f, -0.716565f},
       {-0.232822f, 0.657519f, -0.716563f},
       {0.723607f, 0.447220f, -0.525725f},
       {0.531941f, 0.502302f, -0.681712f},
       {0.609547f, 0.657519f, -0.442856f},
       {0.723607f, 0.447220f, 0.525725f},
       {0.812729f, 0.502301f, 0.295238f},
       {0.609547f, 0.657519f, 0.442856f},
       {-0.276388f, 0.447220f, 0.850649f},
       {-0.029639f, 0.502302f, 0.864184f},
       {-0.232822f, 0.657519f, 0.716563f},
       {-0.894426f, 0.447216f, 0.000000f},
       {-0.831051f, 0.502299f, 0.238853f},
       {-0.753442f, 0.657515f, 0.000000f},
       {-0.251147f, 0.967949f, 0.000000f},
       {-0.077607f, 0.967950f, 0.238853f},
       {0.000000f, 1.000000f, 0.000000f},
       {-0.525730f, 0.850652f, 0.000000f},
       {-0.361800f, 0.894429f, 0.262863f},
       {-0.638194f, 0.723610f, 0.262864f},
       {-0.162456f, 0.850654f, 0.499995f},
       {-0.447209f, 0.723612f, 0.525728f},
       {-0.688189f, 0.525736f, 0.499997f},
       {-0.483971f, 0.502302f, 0.716565f},
       {0.203181f, 0.967950f, 0.147618f},
       {0.138197f, 0.894430f, 0.425319f},
       {0.052790f, 0.723612f, 0.688185f},
       {0.425323f, 0.850654f, 0.309011f},
       {0.361804f, 0.723612f, 0.587778f},
       {0.262869f, 0.525738f, 0.809012f},
       {0.531941f, 0.502302f, 0.681712f},
       {0.203181f, 0.967950f, -0.147618f},
       {0.447210f, 0.894429f, 0.000000f},
       {0.670817f, 0.723611f, 0.162457f},
       {0.425323f, 0.850654f, -0.309011f},
       {0.670817f, 0.723611f, -0.162457f},
       {0.850648f, 0.525736f, 0.000000f},
       {0.812729f, 0.502301f, -0.295238f},
       {-0.077607f, 0.967950f, -0.238853f},
       {0.138197f, 0.894430f, -0.425319f},
       {0.361804f, 0.723612f, -0.587778f},
       {-0.162456f, 0.850654f, -0.499995f},
       {0.052790f, 0.723612f, -0.688185f},
       {0.262869f, 0.525738f, -0.809012f},
       {-0.029639f, 0.502302f, -0.864184f},
       {-0.361800f, 0.894429f, -0.262863f},
       {-0.447209f, 0.723612f, -0.525728f},
       {-0.638194f, 0.723610f, -0.262864f},
       {-0.688189f, 0.525736f, -0.499997f},
       {-0.831051f, 0.502299f, -0.238853f},
       {-0.956626f, 0.251149f, 0.147618f},
       {-0.951058f, 0.000000f, 0.309013f},
       {-0.861804f, 0.276396f, 0.425322f},
       {-0.809019f, 0.000000f, 0.587782f},
       {-0.670821f, 0.276397f, 0.688189f},
       {-0.587786f, -0.000000f, 0.809017f},
       {-0.436007f, 0.251152f, 0.864188f},
       {-0.155215f, 0.251152f, 0.955422f},
       {0.000000f, 0.000000f, 1.000000f},
       {0.138199f, 0.276397f, 0.951055f},
       {0.309016f, -0.000000f, 0.951057f},
       {0.447215f, 0.276397f, 0.850649f},
       {0.587786f, -0.000000f, 0.809017f},
       {0.687159f, 0.251152f, 0.681715f},
       {0.860698f, 0.251151f, 0.442858f},
       {0.951058f, 0.000000f, 0.309013f},
       {0.947213f, 0.276396f, 0.162458f},
       {1.000000f, 0.000001f, -0.000000f},
       {0.947213f, 0.276397f, -0.162458f},
       {0.951058f, -0.000000f, -0.309013f},
       {0.860698f, 0.251151f, -0.442858f},
       {0.687159f, 0.251152f, -0.681715f},
       {0.587786f, 0.000000f, -0.809017f},
       {0.447216f, 0.276397f, -0.850648f},
       {0.309017f, -0.000001f, -0.951056f},
       {0.138199f, 0.276397f, -0.951055f},
       {0.000000f, -0.000000f, -1.000000f},
       {-0.155215f, 0.251152f, -0.955422f},
       {-0.436007f, 0.251152f, -0.864188f},
       {-0.587786f, 0.000000f, -0.809017f},
       {-0.670820f, 0.276396f, -0.688190f},
       {-0.809019f, -0.000002f, -0.587783f},
       {-0.861804f, 0.276394f, -0.425323f},
       {-0.951058f, -0.000000f, -0.309013f},
       {-0.956626f, 0.251149f, -0.147618f},
       {-0.309017f, -0.000000f, 0.951056f},
       {-0.447216f, -0.276398f, 0.850648f},
       {-0.138199f, -0.276398f, 0.951055f},
       {-0.262869f, -0.525738f, 0.809012f},
       {0.029639f, -0.502302f, 0.864184f},
       {0.809018f, -0.000000f, 0.587783f},
       {0.670819f, -0.276397f, 0.688191f},
       {0.861803f, -0.276396f, 0.425324f},
       {0.688189f, -0.525736f, 0.499997f},
       {0.831051f, -0.502299f, 0.238853f},
       {0.809018f, 0.000000f, -0.587783f},
       {0.861803f, -0.276396f, -0.425324f},
       {0.670819f, -0.276397f, -0.688191f},
       {0.688189f, -0.525736f, -0.499997f},
       {0.483971f, -0.502302f, -0.716565f},
       {-0.309017f, 0.000000f, -0.951056f},
       {-0.138199f, -0.276398f, -0.951055f},
       {-0.447216f, -0.276398f, -0.850648f},
       {-0.262869f, -0.525738f, -0.809012f},
       {-0.531941f, -0.502302f, -0.681712f},
       {-1.000000f, 0.000000f, 0.000000f},
       {-0.947213f, -0.276396f, -0.162458f},
       {-0.947213f, -0.276396f, 0.162458f},
       {-0.850648f, -0.525736f, 0.000000f},
       {-0.812729f, -0.502301f, 0.295238f},
       {-0.609547f, -0.657519f, 0.442856f},
       {-0.425323f, -0.850654f, 0.309011f},
       {-0.361803f, -0.723612f, 0.587779f},
       {-0.138197f, -0.894429f, 0.425321f},
       {-0.052789f, -0.723611f, 0.688186f},
       {0.162456f, -0.850654f, 0.499995f},
       {0.232822f, -0.657519f, 0.716563f},
       {0.447211f, -0.723612f, 0.525727f},
       {0.361801f, -0.894429f, 0.262863f},
       {0.638195f, -0.723609f, 0.262863f},
       {0.525730f, -0.850652f, 0.000000f},
       {0.753442f, -0.657515f, 0.000000f},
       {0.638195f, -0.723609f, -0.262864f},
       {0.361801f, -0.894428f, -0.262864f},
       {0.447211f, -0.723610f, -0.525729f},
       {0.162456f, -0.850654f, -0.499995f},
       {0.232822f, -0.657519f, -0.716563f},
       {-0.670817f, -0.723611f, 0.162457f},
       {-0.670818f, -0.723610f, -0.162458f},
       {-0.447211f, -0.894428f, -0.000001f},
       {-0.425323f, -0.850654f, -0.309011f},
       {-0.052790f, -0.723612f, -0.688185f},
       {-0.138199f, -0.894429f, -0.425321f},
       {-0.361805f, -0.723611f, -0.587779f}}};

   alignas(D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT)
      const std::array<std::array<uint16, 3>, 320> icosphere_indices = {
         {{0, 1, 2},       {3, 4, 5},       {0, 2, 6},       {0, 6, 7},
          {0, 7, 8},       {3, 5, 9},       {10, 11, 12},    {13, 14, 15},
          {16, 17, 18},    {19, 20, 21},    {3, 9, 22},      {10, 12, 23},
          {13, 15, 24},    {16, 18, 25},    {19, 21, 26},    {27, 28, 29},
          {30, 31, 32},    {33, 34, 35},    {36, 37, 38},    {39, 40, 41},
          {42, 43, 44},    {45, 46, 42},    {41, 47, 45},    {42, 46, 43},
          {46, 48, 43},    {45, 47, 46},    {47, 49, 46},    {46, 49, 48},
          {49, 38, 48},    {41, 40, 47},    {40, 50, 47},    {47, 50, 49},
          {50, 51, 49},    {49, 51, 38},    {51, 36, 38},    {43, 52, 44},
          {48, 53, 43},    {38, 54, 48},    {43, 53, 52},    {53, 55, 52},
          {48, 54, 53},    {54, 56, 53},    {53, 56, 55},    {56, 35, 55},
          {38, 37, 54},    {37, 57, 54},    {54, 57, 56},    {57, 58, 56},
          {56, 58, 35},    {58, 33, 35},    {52, 59, 44},    {55, 60, 52},
          {35, 61, 55},    {52, 60, 59},    {60, 62, 59},    {55, 61, 60},
          {61, 63, 60},    {60, 63, 62},    {63, 32, 62},    {35, 34, 61},
          {34, 64, 61},    {61, 64, 63},    {64, 65, 63},    {63, 65, 32},
          {65, 30, 32},    {59, 66, 44},    {62, 67, 59},    {32, 68, 62},
          {59, 67, 66},    {67, 69, 66},    {62, 68, 67},    {68, 70, 67},
          {67, 70, 69},    {70, 29, 69},    {32, 31, 68},    {31, 71, 68},
          {68, 71, 70},    {71, 72, 70},    {70, 72, 29},    {72, 27, 29},
          {66, 42, 44},    {69, 73, 66},    {29, 74, 69},    {66, 73, 42},
          {73, 45, 42},    {69, 74, 73},    {74, 75, 73},    {73, 75, 45},
          {75, 41, 45},    {29, 28, 74},    {28, 76, 74},    {74, 76, 75},
          {76, 77, 75},    {75, 77, 41},    {77, 39, 41},    {78, 40, 39},
          {79, 80, 78},    {26, 81, 79},    {78, 80, 40},    {80, 50, 40},
          {79, 81, 80},    {81, 82, 80},    {80, 82, 50},    {82, 51, 50},
          {26, 21, 81},    {21, 83, 81},    {81, 83, 82},    {83, 84, 82},
          {82, 84, 51},    {84, 36, 51},    {85, 37, 36},    {86, 87, 85},
          {25, 88, 86},    {85, 87, 37},    {87, 57, 37},    {86, 88, 87},
          {88, 89, 87},    {87, 89, 57},    {89, 58, 57},    {25, 18, 88},
          {18, 90, 88},    {88, 90, 89},    {90, 91, 89},    {89, 91, 58},
          {91, 33, 58},    {92, 34, 33},    {93, 94, 92},    {24, 95, 93},
          {92, 94, 34},    {94, 64, 34},    {93, 95, 94},    {95, 96, 94},
          {94, 96, 64},    {96, 65, 64},    {24, 15, 95},    {15, 97, 95},
          {95, 97, 96},    {97, 98, 96},    {96, 98, 65},    {98, 30, 65},
          {99, 31, 30},    {100, 101, 99},  {23, 102, 100},  {99, 101, 31},
          {101, 71, 31},   {100, 102, 101}, {102, 103, 101}, {101, 103, 71},
          {103, 72, 71},   {23, 12, 102},   {12, 104, 102},  {102, 104, 103},
          {104, 105, 103}, {103, 105, 72},  {105, 27, 72},   {106, 28, 27},
          {107, 108, 106}, {22, 109, 107},  {106, 108, 28},  {108, 76, 28},
          {107, 109, 108}, {109, 110, 108}, {108, 110, 76},  {110, 77, 76},
          {22, 9, 109},    {9, 111, 109},   {109, 111, 110}, {111, 112, 110},
          {110, 112, 77},  {112, 39, 77},   {84, 85, 36},    {83, 113, 84},
          {21, 114, 83},   {84, 113, 85},   {113, 86, 85},   {83, 114, 113},
          {114, 115, 113}, {113, 115, 86},  {115, 25, 86},   {21, 20, 114},
          {20, 116, 114},  {114, 116, 115}, {116, 117, 115}, {115, 117, 25},
          {117, 16, 25},   {91, 92, 33},    {90, 118, 91},   {18, 119, 90},
          {91, 118, 92},   {118, 93, 92},   {90, 119, 118},  {119, 120, 118},
          {118, 120, 93},  {120, 24, 93},   {18, 17, 119},   {17, 121, 119},
          {119, 121, 120}, {121, 122, 120}, {120, 122, 24},  {122, 13, 24},
          {98, 99, 30},    {97, 123, 98},   {15, 124, 97},   {98, 123, 99},
          {123, 100, 99},  {97, 124, 123},  {124, 125, 123}, {123, 125, 100},
          {125, 23, 100},  {15, 14, 124},   {14, 126, 124},  {124, 126, 125},
          {126, 127, 125}, {125, 127, 23},  {127, 10, 23},   {105, 106, 27},
          {104, 128, 105}, {12, 129, 104},  {105, 128, 106}, {128, 107, 106},
          {104, 129, 128}, {129, 130, 128}, {128, 130, 107}, {130, 22, 107},
          {12, 11, 129},   {11, 131, 129},  {129, 131, 130}, {131, 132, 130},
          {130, 132, 22},  {132, 3, 22},    {112, 78, 39},   {111, 133, 112},
          {9, 134, 111},   {112, 133, 78},  {133, 79, 78},   {111, 134, 133},
          {134, 135, 133}, {133, 135, 79},  {135, 26, 79},   {9, 5, 134},
          {5, 136, 134},   {134, 136, 135}, {136, 137, 135}, {135, 137, 26},
          {137, 19, 26},   {138, 20, 19},   {139, 140, 138}, {8, 141, 139},
          {138, 140, 20},  {140, 116, 20},  {139, 141, 140}, {141, 142, 140},
          {140, 142, 116}, {142, 117, 116}, {8, 7, 141},     {7, 143, 141},
          {141, 143, 142}, {143, 144, 142}, {142, 144, 117}, {144, 16, 117},
          {144, 17, 16},   {143, 145, 144}, {7, 146, 143},   {144, 145, 17},
          {145, 121, 17},  {143, 146, 145}, {146, 147, 145}, {145, 147, 121},
          {147, 122, 121}, {7, 6, 146},     {6, 148, 146},   {146, 148, 147},
          {148, 149, 147}, {147, 149, 122}, {149, 13, 122},  {149, 14, 13},
          {148, 150, 149}, {6, 151, 148},   {149, 150, 14},  {150, 126, 14},
          {148, 151, 150}, {151, 152, 150}, {150, 152, 126}, {152, 127, 126},
          {6, 2, 151},     {2, 153, 151},   {151, 153, 152}, {153, 154, 152},
          {152, 154, 127}, {154, 10, 127},  {137, 138, 19},  {136, 155, 137},
          {5, 156, 136},   {137, 155, 138}, {155, 139, 138}, {136, 156, 155},
          {156, 157, 155}, {155, 157, 139}, {157, 8, 139},   {5, 4, 156},
          {4, 158, 156},   {156, 158, 157}, {158, 1, 157},   {157, 1, 8},
          {1, 0, 8},       {154, 11, 10},   {153, 159, 154}, {2, 160, 153},
          {154, 159, 11},  {159, 131, 11},  {153, 160, 159}, {160, 161, 159},
          {159, 161, 131}, {161, 132, 131}, {2, 1, 160},     {1, 158, 160},
          {160, 158, 161}, {158, 4, 161},   {161, 4, 132},   {4, 3, 132}}};

   alignas(D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT) const std::array<float3, 66> cylinder_vertices{
      {{0.000000f, -1.000000f, 0.000000f},
       {0.000000f, -1.000000f, 1.000000f},
       {-0.195090f, -1.000000f, 0.980785f},
       {0.000000f, 1.000000f, 0.000000f},
       {-0.195090f, 1.000000f, 0.980785f},
       {0.000000f, 1.000000f, 1.000000f},
       {-0.382683f, -1.000000f, 0.923880f},
       {-0.382683f, 1.000000f, 0.923880f},
       {-0.555570f, -1.000000f, 0.831470f},
       {-0.555570f, 1.000000f, 0.831470f},
       {-0.707107f, -1.000000f, 0.707107f},
       {-0.707107f, 1.000000f, 0.707107f},
       {-0.831470f, -1.000000f, 0.555570f},
       {-0.831470f, 1.000000f, 0.555570f},
       {-0.923880f, -1.000000f, 0.382683f},
       {-0.923880f, 1.000000f, 0.382683f},
       {-0.980785f, -1.000000f, 0.195090f},
       {-0.980785f, 1.000000f, 0.195090f},
       {-1.000000f, -1.000000f, 0.000000f},
       {-1.000000f, 1.000000f, 0.000000f},
       {-0.980785f, -1.000000f, -0.195090f},
       {-0.980785f, 1.000000f, -0.195090f},
       {-0.923880f, -1.000000f, -0.382683f},
       {-0.923880f, 1.000000f, -0.382683f},
       {-0.831470f, -1.000000f, -0.555570f},
       {-0.831470f, 1.000000f, -0.555570f},
       {-0.707107f, -1.000000f, -0.707107f},
       {-0.707107f, 1.000000f, -0.707107f},
       {-0.555570f, -1.000000f, -0.831470f},
       {-0.555570f, 1.000000f, -0.831470f},
       {-0.382683f, -1.000000f, -0.923880f},
       {-0.382683f, 1.000000f, -0.923880f},
       {-0.195090f, -1.000000f, -0.980785f},
       {-0.195090f, 1.000000f, -0.980785f},
       {0.000000f, -1.000000f, -1.000000f},
       {0.000000f, 1.000000f, -1.000000f},
       {0.195091f, -1.000000f, -0.980785f},
       {0.195091f, 1.000000f, -0.980785f},
       {0.382684f, -1.000000f, -0.923879f},
       {0.382684f, 1.000000f, -0.923879f},
       {0.555571f, -1.000000f, -0.831469f},
       {0.555571f, 1.000000f, -0.831469f},
       {0.707107f, -1.000000f, -0.707106f},
       {0.707107f, 1.000000f, -0.707106f},
       {0.831470f, -1.000000f, -0.555570f},
       {0.831470f, 1.000000f, -0.555570f},
       {0.923880f, -1.000000f, -0.382683f},
       {0.923880f, 1.000000f, -0.382683f},
       {0.980785f, -1.000000f, -0.195089f},
       {0.980785f, 1.000000f, -0.195089f},
       {1.000000f, -1.000000f, 0.000001f},
       {1.000000f, 1.000000f, 0.000001f},
       {0.980785f, -1.000000f, 0.195091f},
       {0.980785f, 1.000000f, 0.195091f},
       {0.923879f, -1.000000f, 0.382684f},
       {0.923879f, 1.000000f, 0.382684f},
       {0.831469f, -1.000000f, 0.555571f},
       {0.831469f, 1.000000f, 0.555571f},
       {0.707106f, -1.000000f, 0.707108f},
       {0.707106f, 1.000000f, 0.707108f},
       {0.555569f, -1.000000f, 0.831470f},
       {0.555569f, 1.000000f, 0.831470f},
       {0.382682f, -1.000000f, 0.923880f},
       {0.382682f, 1.000000f, 0.923880f},
       {0.195089f, -1.000000f, 0.980786f},
       {0.195089f, 1.000000f, 0.980786f}}};

   alignas(D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT) const std::array<std::array<uint16, 3>, 128> cylinder_indices{
      {{0, 1, 2},    {3, 4, 5},    {5, 2, 1},    {0, 2, 6},    {3, 7, 4},
       {4, 6, 2},    {0, 6, 8},    {3, 9, 7},    {7, 8, 6},    {0, 8, 10},
       {3, 11, 9},   {9, 10, 8},   {0, 10, 12},  {3, 13, 11},  {11, 12, 10},
       {0, 12, 14},  {3, 15, 13},  {13, 14, 12}, {0, 14, 16},  {3, 17, 15},
       {15, 16, 14}, {0, 16, 18},  {3, 19, 17},  {17, 18, 16}, {0, 18, 20},
       {3, 21, 19},  {19, 20, 18}, {0, 20, 22},  {3, 23, 21},  {21, 22, 20},
       {0, 22, 24},  {3, 25, 23},  {23, 24, 22}, {0, 24, 26},  {3, 27, 25},
       {25, 26, 24}, {0, 26, 28},  {3, 29, 27},  {27, 28, 26}, {0, 28, 30},
       {3, 31, 29},  {29, 30, 28}, {0, 30, 32},  {3, 33, 31},  {31, 32, 30},
       {0, 32, 34},  {3, 35, 33},  {33, 34, 32}, {0, 34, 36},  {3, 37, 35},
       {35, 36, 34}, {0, 36, 38},  {3, 39, 37},  {37, 38, 36}, {0, 38, 40},
       {3, 41, 39},  {39, 40, 38}, {0, 40, 42},  {3, 43, 41},  {41, 42, 40},
       {0, 42, 44},  {3, 45, 43},  {43, 44, 42}, {0, 44, 46},  {3, 47, 45},
       {45, 46, 44}, {0, 46, 48},  {3, 49, 47},  {47, 48, 46}, {0, 48, 50},
       {3, 51, 49},  {49, 50, 48}, {0, 50, 52},  {3, 53, 51},  {51, 52, 50},
       {0, 52, 54},  {3, 55, 53},  {53, 54, 52}, {0, 54, 56},  {3, 57, 55},
       {55, 56, 54}, {0, 56, 58},  {3, 59, 57},  {57, 58, 56}, {0, 58, 60},
       {3, 61, 59},  {59, 60, 58}, {0, 60, 62},  {3, 63, 61},  {61, 62, 60},
       {0, 62, 64},  {3, 65, 63},  {63, 64, 62}, {0, 64, 1},   {3, 5, 65},
       {65, 1, 64},  {5, 4, 2},    {4, 7, 6},    {7, 9, 8},    {9, 11, 10},
       {11, 13, 12}, {13, 15, 14}, {15, 17, 16}, {17, 19, 18}, {19, 21, 20},
       {21, 23, 22}, {23, 25, 24}, {25, 27, 26}, {27, 29, 28}, {29, 31, 30},
       {31, 33, 32}, {33, 35, 34}, {35, 37, 36}, {37, 39, 38}, {39, 41, 40},
       {41, 43, 42}, {43, 45, 44}, {45, 47, 46}, {47, 49, 48}, {49, 51, 50},
       {51, 53, 52}, {53, 55, 54}, {55, 57, 56}, {57, 59, 58}, {59, 61, 60},
       {61, 63, 62}, {63, 65, 64}, {65, 5, 1}}};

   alignas(D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT) const std::array<float3, 8> cube_vertices{
      {{1.000000f, 1.000000f, 1.000000f},
       {-1.000000f, 1.000000f, -1.000000f},
       {-1.000000f, 1.000000f, 1.000000f},
       {1.000000f, -1.000000f, -1.000000f},
       {-1.000000f, -1.000000f, -1.000000f},
       {1.000000f, 1.000000f, -1.000000f},
       {1.000000f, -1.000000f, 1.000000f},
       {-1.000000f, -1.000000f, 1.000000f}}};

   alignas(D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT)
      const std::array<std::array<uint16, 3>, 12> cube_indices{{{0, 1, 2},
                                                                {1, 3, 4},
                                                                {5, 6, 3},
                                                                {7, 3, 6},
                                                                {2, 4, 7},
                                                                {0, 7, 6},
                                                                {0, 5, 1},
                                                                {1, 5, 3},
                                                                {5, 0, 6},
                                                                {7, 4, 3},
                                                                {2, 1, 4},
                                                                {0, 2, 7}}};

   alignas(D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT) const std::array<float3, 6> octahedron_vertices{
      {{1.000000f, 0.000000f, 1.000000f},
       {0.000000f, 1.000000f, 0.000000f},
       {-1.000000f, 0.000000f, 1.000000f},
       {-1.000000f, 0.000000f, -1.000000f},
       {1.000000f, 0.000000f, -1.000000f},
       {0.000000f, -1.000000f, 0.000000f}}};

   alignas(D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT) const std::array<std::array<uint16, 3>, 8> octahedron_indices{
      {{0, 1, 2}, {2, 1, 3}, {3, 1, 4}, {4, 1, 0}, {4, 5, 3}, {0, 5, 4}, {2, 5, 0}, {3, 5, 2}}};

   alignas(D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT) const std::array<float3, 33> cone_vertices{
      {{0.000000f, -1.000000f, -1.000000f},
       {0.000000f, 1.000000f, 0.000000f},
       {0.195090f, -1.000000f, -0.980785f},
       {0.382683f, -1.000000f, -0.923880f},
       {0.555570f, -1.000000f, -0.831470f},
       {0.707107f, -1.000000f, -0.707107f},
       {0.831470f, -1.000000f, -0.555570f},
       {0.923880f, -1.000000f, -0.382683f},
       {0.980785f, -1.000000f, -0.195090f},
       {1.000000f, -1.000000f, -0.000000f},
       {0.980785f, -1.000000f, 0.195090f},
       {0.923880f, -1.000000f, 0.382683f},
       {0.831470f, -1.000000f, 0.555570f},
       {0.707107f, -1.000000f, 0.707107f},
       {0.555570f, -1.000000f, 0.831470f},
       {0.382683f, -1.000000f, 0.923880f},
       {0.195090f, -1.000000f, 0.980785f},
       {-0.000000f, -1.000000f, 1.000000f},
       {-0.195091f, -1.000000f, 0.980785f},
       {-0.382684f, -1.000000f, 0.923879f},
       {-0.555571f, -1.000000f, 0.831469f},
       {-0.707107f, -1.000000f, 0.707106f},
       {-0.831470f, -1.000000f, 0.555570f},
       {-0.923880f, -1.000000f, 0.382683f},
       {-0.980785f, -1.000000f, 0.195089f},
       {-1.000000f, -1.000000f, -0.000001f},
       {-0.980785f, -1.000000f, -0.195091f},
       {-0.923879f, -1.000000f, -0.382684f},
       {-0.831469f, -1.000000f, -0.555571f},
       {-0.707106f, -1.000000f, -0.707108f},
       {-0.555569f, -1.000000f, -0.831470f},
       {-0.382682f, -1.000000f, -0.923880f},
       {-0.195089f, -1.000000f, -0.980786f}}};

   alignas(D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT) const std::array<std::array<uint16, 3>, 62> cone_indices{
      {{0, 1, 2},    {2, 1, 3},    {3, 1, 4},    {4, 1, 5},    {5, 1, 6},
       {6, 1, 7},    {7, 1, 8},    {8, 1, 9},    {9, 1, 10},   {10, 1, 11},
       {11, 1, 12},  {12, 1, 13},  {13, 1, 14},  {14, 1, 15},  {15, 1, 16},
       {16, 1, 17},  {17, 1, 18},  {18, 1, 19},  {19, 1, 20},  {20, 1, 21},
       {21, 1, 22},  {22, 1, 23},  {23, 1, 24},  {24, 1, 25},  {25, 1, 26},
       {26, 1, 27},  {27, 1, 28},  {28, 1, 29},  {29, 1, 30},  {30, 1, 31},
       {31, 1, 32},  {32, 1, 0},   {8, 16, 24},  {32, 0, 2},   {2, 3, 4},
       {4, 5, 6},    {6, 7, 4},    {7, 8, 4},    {8, 9, 10},   {10, 11, 8},
       {11, 12, 8},  {12, 13, 16}, {13, 14, 16}, {14, 15, 16}, {16, 17, 18},
       {18, 19, 20}, {20, 21, 22}, {22, 23, 24}, {24, 25, 26}, {26, 27, 28},
       {28, 29, 32}, {29, 30, 32}, {30, 31, 32}, {32, 2, 4},   {16, 18, 24},
       {18, 20, 24}, {20, 22, 24}, {24, 26, 32}, {26, 28, 32}, {32, 4, 8},
       {8, 12, 16},  {32, 8, 24}}};
};

constexpr geometric_shapes_buffer shapes_buffer;

}

geometric_shapes::geometric_shapes(gpu::device& device)
{
   init_gpu_buffer(device);
   init_shapes();
}

void geometric_shapes::init_gpu_buffer(gpu::device& device)
{
   _gpu_buffer =
      device.create_buffer({.size = sizeof(shapes_buffer)},
                           D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);

   auto copy_context = device.copy_manager.aquire_context();

   ID3D12Resource& upload_buffer = copy_context.create_upload_resource(
      CD3DX12_RESOURCE_DESC::Buffer(sizeof(shapes_buffer)));

   std::byte* const upload_buffer_ptr = [&] {
      const D3D12_RANGE read_range{};
      void* map_void_ptr = nullptr;

      throw_if_failed(upload_buffer.Map(0, &read_range, &map_void_ptr));

      return static_cast<std::byte*>(map_void_ptr);
   }();

   std::memcpy(upload_buffer_ptr, &shapes_buffer, sizeof(shapes_buffer));

   const D3D12_RANGE write_range{0, sizeof(shapes_buffer)};
   upload_buffer.Unmap(0, &write_range);

   copy_context.command_list.CopyResource(_gpu_buffer.resource(), &upload_buffer);

   (void)device.copy_manager.close_and_execute(copy_context);
}

void geometric_shapes::init_shapes()
{
   const D3D12_GPU_VIRTUAL_ADDRESS gpu_address =
      _gpu_buffer.resource()->GetGPUVirtualAddress();

   _icosphere =
      {.index_count = static_cast<uint32>(shapes_buffer.icosphere_indices.size()) * 3,
       .index_buffer_view = {.BufferLocation =
                                gpu_address + offsetof(geometric_shapes_buffer,
                                                       icosphere_indices),
                             .SizeInBytes = sizeof(geometric_shapes_buffer::icosphere_indices),
                             .Format = DXGI_FORMAT_R16_UINT},
       .position_vertex_buffer_view =
          {.BufferLocation =
              gpu_address + offsetof(geometric_shapes_buffer, icosphere_vertices),
           .SizeInBytes = sizeof(geometric_shapes_buffer::icosphere_vertices),
           .StrideInBytes = sizeof(float3)}};

   _cylinder =
      {.index_count = static_cast<uint32>(shapes_buffer.cylinder_indices.size()) * 3,
       .index_buffer_view = {.BufferLocation =
                                gpu_address + offsetof(geometric_shapes_buffer,
                                                       cylinder_indices),
                             .SizeInBytes = sizeof(geometric_shapes_buffer::cylinder_indices),
                             .Format = DXGI_FORMAT_R16_UINT},
       .position_vertex_buffer_view =
          {.BufferLocation =
              gpu_address + offsetof(geometric_shapes_buffer, cylinder_vertices),
           .SizeInBytes = sizeof(geometric_shapes_buffer::cylinder_vertices),
           .StrideInBytes = sizeof(float3)}};

   _cube = {.index_count = static_cast<uint32>(shapes_buffer.cube_indices.size()) * 3,
            .index_buffer_view = {.BufferLocation =
                                     gpu_address + offsetof(geometric_shapes_buffer,
                                                            cube_indices),
                                  .SizeInBytes = sizeof(geometric_shapes_buffer::cube_indices),
                                  .Format = DXGI_FORMAT_R16_UINT},
            .position_vertex_buffer_view =
               {.BufferLocation =
                   gpu_address + offsetof(geometric_shapes_buffer, cube_vertices),
                .SizeInBytes = sizeof(geometric_shapes_buffer::cube_vertices),
                .StrideInBytes = sizeof(float3)}};

   _octahedron =
      {.index_count = static_cast<uint32>(shapes_buffer.octahedron_indices.size()) * 3,
       .index_buffer_view = {.BufferLocation =
                                gpu_address + offsetof(geometric_shapes_buffer,
                                                       octahedron_indices),
                             .SizeInBytes = sizeof(geometric_shapes_buffer::octahedron_indices),
                             .Format = DXGI_FORMAT_R16_UINT},
       .position_vertex_buffer_view =
          {.BufferLocation =
              gpu_address + offsetof(geometric_shapes_buffer, octahedron_vertices),
           .SizeInBytes = sizeof(geometric_shapes_buffer::octahedron_vertices),
           .StrideInBytes = sizeof(float3)}};

   _cone = {.index_count = static_cast<uint32>(shapes_buffer.cone_indices.size()) * 3,
            .index_buffer_view = {.BufferLocation =
                                     gpu_address + offsetof(geometric_shapes_buffer,
                                                            cone_indices),
                                  .SizeInBytes = sizeof(geometric_shapes_buffer::cone_indices),
                                  .Format = DXGI_FORMAT_R16_UINT},
            .position_vertex_buffer_view =
               {.BufferLocation =
                   gpu_address + offsetof(geometric_shapes_buffer, cone_vertices),
                .SizeInBytes = sizeof(geometric_shapes_buffer::cone_vertices),
                .StrideInBytes = sizeof(float3)}};
}

}
