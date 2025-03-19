#include "mesh_geometry.hpp"
namespace we::world {

namespace {

enum class block_box_surface { pos_x, neg_x, pos_y, neg_y, pos_z, neg_z };

}

const std::array<block_vertex, 24> block_cube_vertices = {{
   {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, (uint16)block_box_surface::pos_z},
   {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, (uint16)block_box_surface::pos_z},
   {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, (uint16)block_box_surface::pos_z},
   {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, (uint16)block_box_surface::neg_x},
   {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, (uint16)block_box_surface::neg_x},
   {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, (uint16)block_box_surface::neg_x},
   {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, (uint16)block_box_surface::neg_z},
   {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, (uint16)block_box_surface::neg_z},
   {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, (uint16)block_box_surface::neg_z},
   {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, (uint16)block_box_surface::pos_x},
   {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, (uint16)block_box_surface::pos_x},
   {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, (uint16)block_box_surface::pos_x},
   {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, (uint16)block_box_surface::neg_y},
   {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, (uint16)block_box_surface::neg_y},
   {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, (uint16)block_box_surface::neg_y},
   {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, (uint16)block_box_surface::pos_y},
   {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, (uint16)block_box_surface::pos_y},
   {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, (uint16)block_box_surface::pos_y},
   {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, (uint16)block_box_surface::pos_z},
   {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, (uint16)block_box_surface::neg_x},
   {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, (uint16)block_box_surface::neg_z},
   {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, (uint16)block_box_surface::pos_x},
   {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, (uint16)block_box_surface::neg_y},
   {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, (uint16)block_box_surface::pos_y},
}};

const std::array<std::array<uint16, 3>, 12> block_cube_triangles = {{
   {0, 1, 2},
   {3, 4, 5},
   {6, 7, 8},
   {9, 10, 11},
   {12, 13, 14},
   {15, 16, 17},
   {0, 18, 1},
   {3, 19, 4},
   {6, 20, 7},
   {9, 21, 10},
   {12, 22, 13},
   {15, 23, 16},
}};

}