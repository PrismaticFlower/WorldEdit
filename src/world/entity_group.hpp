#pragma once

#include "barrier.hpp"
#include "blocks.hpp"
#include "boundary.hpp"
#include "hintnode.hpp"
#include "light.hpp"
#include "measurement.hpp"
#include "object.hpp"
#include "path.hpp"
#include "planning.hpp"
#include "portal.hpp"
#include "region.hpp"
#include "sector.hpp"

#include <vector>

namespace we::world {

struct entity_group {
   quaternion rotation;
   float3 position;
   float rotation_angle = 0.0f;

   int8 layer = 0;

   std::vector<object> objects;
   std::vector<light> lights;
   std::vector<path> paths;
   std::vector<region> regions;
   std::vector<sector> sectors;
   std::vector<portal> portals;
   std::vector<hintnode> hintnodes;
   std::vector<barrier> barriers;
   std::vector<planning_hub> planning_hubs;
   std::vector<planning_connection> planning_connections;
   std::vector<boundary> boundaries;
   std::vector<measurement> measurements;

   struct blocks {
      std::vector<block_description_box> boxes;
      std::vector<block_description_ramp> ramps;
      std::vector<block_description_quad> quads;
      std::vector<block_description_cylinder> cylinders;

      struct stairways {
         std::vector<block_description_stairway> description;
         std::vector<block_custom_mesh_handle> mesh;

         bool operator==(const stairways&) const noexcept = default;
      } stairways;

      std::vector<block_description_cone> cones;
      std::vector<block_description_hemisphere> hemispheres;
      std::vector<block_description_pyramid> pyramids;

      std::vector<block_material> materials;

      bool operator==(const blocks&) const noexcept = default;
   } blocks;

   bool operator==(const entity_group&) const noexcept = default;
};

}
