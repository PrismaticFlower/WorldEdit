#include "mesh_geometry.hpp"
namespace we::world {

// Blender (4.4) Python script for "exporting" shapes. Shape geometry could also be authored any other way but this works for me.
#if 0
import bpy

SHAPE_NAME = "cube"

me = bpy.context.object.data
uv_layer = me.uv_layers.active.data

me.calc_loop_triangles()
me.calc_tangents()

index_remap = list(range(len(me.loops)))
index_remap.sort(key=lambda i: [me.loops[i].normal.x, me.loops[i].normal.z, me.loops[i].normal.y])

print(f'const std::array<block_vertex, {len(index_remap)}> block_{SHAPE_NAME}_vertices = {{{{')

for index in index_remap:
    loop = me.loops[index]
    uv_loop = uv_layer[index]
    
    position = me.vertices[loop.vertex_index].co
    normal = loop.normal
    tangent = loop.tangent
    bitangent_sign = loop.bitangent_sign
    uv = uv_loop.uv
    
    print(f'   {{ {{ {position[0]}, {position[2]}, {position[1]} }}, {{ {tangent[0]}, {tangent[2]}, {tangent[1]} }}, {{ {bitangent_sign} }}, {{ {normal[0]}, {normal[2]}, {normal[1]} }}, {{ {uv[0]}, {uv[1]} }},  SURFACE_INDEX}},')

print(f'}}}};')
print('')

loop_triangles = [[index_remap[index] for index in loop_triangles.loops] for loop_triangles in me.loop_triangles]
loop_triangles.sort();

print(f'const std::array<std::array<uint16, 3>, {len(loop_triangles)}> block_{SHAPE_NAME}_triangles = {{{{')

for triangle in loop_triangles:
    print(f'   {{ {triangle[0]}, {triangle[2]}, {triangle[1]}  }} ,')
 
print(f'}}}};')
print('')

unique_vertices = {v.co.copy().freeze() for v in me.vertices}
indexed_unique_vertices = {v: i for i, v in enumerate(unique_vertices)}

print(f'const std::array<float3, {len(unique_vertices)}> block_{SHAPE_NAME}_points = {{{{') 
 
for v in unique_vertices:
    print(f'   {{ {v[0]}, {v[2]}, {v[1]} }},')
    
print(f'}}}};')
print('')

unique_edges = set()   
    
for edge in me.edges:
    i0 = indexed_unique_vertices[me.vertices[edge.vertices[0]].co.copy().freeze()]
    i1 = indexed_unique_vertices[me.vertices[edge.vertices[1]].co.copy().freeze()]
    
    unique_edges.add((i0, i1))

print(f'const std::array<std::array<uint16, 2>, {len(unique_edges)}> block_{SHAPE_NAME}_edges = {{{{')

for edge in sorted(list(unique_edges)): 
    print(f'   {{ {edge[0]}, {edge[1]} }},')
        
print(f'}}}};')
print('')

print(f'extern const std::array<block_vertex, {len(index_remap)}> block_{SHAPE_NAME}_vertices;')
print(f'extern const std::array<std::array<uint16, 3>, {len(loop_triangles)}> block_{SHAPE_NAME}_triangles;')
print(f'extern const std::array<float3, {len(unique_vertices)}> block_{SHAPE_NAME}_points;')
print(f'extern const std::array<std::array<uint16, 2>, {len(unique_edges)}> block_{SHAPE_NAME}_edges;')
#endif

namespace {

enum class block_box_surface { pos_x, neg_x, pos_y, neg_y, pos_z, neg_z };

}

const std::array<block_vertex, 24> block_cube_vertices = {{
   {{-1.0, -1.0, -1.0}, {0.0, 0.0, 1.0}, {1.0}, {-1.0, 0.0, 0.0}, {0.0, 1.0}, (uint32)block_box_surface::neg_x},
   {{-1.0, 1.0, -1.0}, {0.0, 0.0, 1.0}, {1.0}, {-1.0, 0.0, 0.0}, {0.0, 0.0}, (uint32)block_box_surface::neg_x},
   {{-1.0, 1.0, 1.0}, {0.0, 0.0, 1.0}, {1.0}, {-1.0, 0.0, 0.0}, {1.0, 0.0}, (uint32)block_box_surface::neg_x},
   {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}, {1.0}, {-1.0, 0.0, 0.0}, {1.0, 1.0}, (uint32)block_box_surface::neg_x},
   {{-1.0, -1.0, 1.0}, {0.0, 0.0, -1.0}, {1.0}, {0.0, -1.0, 0.0}, {0.0, 1.0}, (uint32)block_box_surface::neg_y},
   {{1.0, -1.0, 1.0}, {0.0, 0.0, -1.0}, {1.0}, {0.0, -1.0, 0.0}, {0.0, 0.0}, (uint32)block_box_surface::neg_y},
   {{1.0, -1.0, -1.0}, {0.0, 0.0, -1.0}, {1.0}, {0.0, -1.0, 0.0}, {1.0, 0.0}, (uint32)block_box_surface::neg_y},
   {{-1.0, -1.0, -1.0}, {0.0, 0.0, -1.0}, {1.0}, {0.0, -1.0, 0.0}, {1.0, 1.0}, (uint32)block_box_surface::neg_y},
   {{1.0, -1.0, -1.0}, {-1.0, 0.0, 0.0}, {1.0}, {0.0, 0.0, -1.0}, {0.0, 1.0}, (uint32)block_box_surface::neg_z},
   {{1.0, 1.0, -1.0}, {-1.0, 0.0, 0.0}, {1.0}, {0.0, 0.0, -1.0}, {0.0, 0.0}, (uint32)block_box_surface::neg_z},
   {{-1.0, 1.0, -1.0}, {-1.0, 0.0, 0.0}, {1.0}, {0.0, 0.0, -1.0}, {1.0, 0.0}, (uint32)block_box_surface::neg_z},
   {{-1.0, -1.0, -1.0}, {-1.0, 0.0, 0.0}, {1.0}, {0.0, 0.0, -1.0}, {1.0, 1.0}, (uint32)block_box_surface::neg_z},
   {{-1.0, -1.0, 1.0}, {1.0, 0.0, 0.0}, {1.0}, {0.0, 0.0, 1.0}, {0.0, 1.0}, (uint32)block_box_surface::pos_x},
   {{-1.0, 1.0, 1.0}, {1.0, 0.0, 0.0}, {1.0}, {0.0, 0.0, 1.0}, {0.0, 0.0}, (uint32)block_box_surface::pos_x},
   {{1.0, 1.0, 1.0}, {1.0, 0.0, 0.0}, {1.0}, {0.0, 0.0, 1.0}, {1.0, 0.0}, (uint32)block_box_surface::pos_x},
   {{1.0, -1.0, 1.0}, {1.0, 0.0, 0.0}, {1.0}, {0.0, 0.0, 1.0}, {1.0, 1.0}, (uint32)block_box_surface::pos_x},
   {{1.0, 1.0, 1.0}, {0.0, 0.0, -1.0}, {1.0}, {0.0, 1.0, 0.0}, {0.0, 1.0}, (uint32)block_box_surface::pos_y},
   {{-1.0, 1.0, 1.0}, {0.0, 0.0, -1.0}, {1.0}, {0.0, 1.0, 0.0}, {0.0, 0.0}, (uint32)block_box_surface::pos_y},
   {{-1.0, 1.0, -1.0}, {0.0, 0.0, -1.0}, {1.0}, {0.0, 1.0, 0.0}, {1.0, 0.0}, (uint32)block_box_surface::pos_y},
   {{1.0, 1.0, -1.0}, {0.0, 0.0, -1.0}, {1.0}, {0.0, 1.0, 0.0}, {1.0, 1.0}, (uint32)block_box_surface::pos_y},
   {{1.0, -1.0, 1.0}, {0.0, 0.0, -1.0}, {1.0}, {1.0, 0.0, 0.0}, {0.0, 1.0}, (uint32)block_box_surface::pos_z},
   {{1.0, 1.0, 1.0}, {0.0, 0.0, -1.0}, {1.0}, {1.0, 0.0, 0.0}, {0.0, 0.0}, (uint32)block_box_surface::pos_z},
   {{1.0, 1.0, -1.0}, {0.0, 0.0, -1.0}, {1.0}, {1.0, 0.0, 0.0}, {1.0, 0.0}, (uint32)block_box_surface::pos_z},
   {{1.0, -1.0, -1.0}, {0.0, 0.0, -1.0}, {1.0}, {1.0, 0.0, 0.0}, {1.0, 1.0}, (uint32)block_box_surface::pos_z},
}};

const std::array<std::array<uint16, 3>, 12> block_cube_triangles = {{
   {0, 2, 1},
   {0, 3, 2},
   {4, 6, 5},
   {4, 7, 6},
   {8, 10, 9},
   {8, 11, 10},
   {12, 14, 13},
   {12, 15, 14},
   {16, 18, 17},
   {16, 19, 18},
   {20, 22, 21},
   {20, 23, 22},
}};

const std::array<float3, 8> block_cube_points = {{
   {1.0, -1.0, 1.0},
   {-1.0, 1.0, 1.0},
   {1.0, -1.0, -1.0},
   {-1.0, 1.0, -1.0},
   {-1.0, -1.0, -1.0},
   {-1.0, -1.0, 1.0},
   {1.0, 1.0, -1.0},
   {1.0, 1.0, 1.0},
}};

const std::array<std::array<uint16, 2>, 12> block_cube_edges = {{
   {0, 5},
   {1, 5},
   {1, 7},
   {2, 0},
   {3, 1},
   {4, 2},
   {4, 3},
   {5, 4},
   {6, 2},
   {6, 3},
   {7, 0},
   {7, 6},
}};

}