#include "drag_select.hpp"

namespace we::world {

void drag_select(const blocks& blocks, const frustum& frustumWS,
                 block_drag_select_op op, selection& selection) noexcept
{
   for (uint32 block_index = 0; block_index < blocks.boxes.size(); ++block_index) {
      if (blocks.boxes.hidden[block_index]) continue;

      if (intersects(frustumWS, {.min =
                                    {
                                       blocks.boxes.bbox.min_x[block_index],
                                       blocks.boxes.bbox.min_y[block_index],
                                       blocks.boxes.bbox.min_z[block_index],
                                    },
                                 .max = {
                                    blocks.boxes.bbox.max_x[block_index],
                                    blocks.boxes.bbox.max_y[block_index],
                                    blocks.boxes.bbox.max_z[block_index],
                                 }})) {
         if (op == block_drag_select_op::add) {
            selection.add(block_id{blocks.boxes.ids[block_index]});
         }
         else if (op == block_drag_select_op::remove) {
            selection.remove(block_id{blocks.boxes.ids[block_index]});
         }
      }
   }

   for (uint32 block_index = 0; block_index < blocks.ramps.size(); ++block_index) {
      if (blocks.ramps.hidden[block_index]) continue;

      if (intersects(frustumWS, {.min =
                                    {
                                       blocks.ramps.bbox.min_x[block_index],
                                       blocks.ramps.bbox.min_y[block_index],
                                       blocks.ramps.bbox.min_z[block_index],
                                    },
                                 .max = {
                                    blocks.ramps.bbox.max_x[block_index],
                                    blocks.ramps.bbox.max_y[block_index],
                                    blocks.ramps.bbox.max_z[block_index],
                                 }})) {
         if (op == block_drag_select_op::add) {
            selection.add(block_id{blocks.ramps.ids[block_index]});
         }
         else if (op == block_drag_select_op::remove) {
            selection.remove(block_id{blocks.ramps.ids[block_index]});
         }
      }
   }

   for (uint32 block_index = 0; block_index < blocks.quads.size(); ++block_index) {
      if (blocks.quads.hidden[block_index]) continue;

      if (intersects(frustumWS, {.min =
                                    {
                                       blocks.quads.bbox.min_x[block_index],
                                       blocks.quads.bbox.min_y[block_index],
                                       blocks.quads.bbox.min_z[block_index],
                                    },
                                 .max = {
                                    blocks.quads.bbox.max_x[block_index],
                                    blocks.quads.bbox.max_y[block_index],
                                    blocks.quads.bbox.max_z[block_index],
                                 }})) {
         if (op == block_drag_select_op::add) {
            selection.add(block_id{blocks.quads.ids[block_index]});
         }
         else if (op == block_drag_select_op::remove) {
            selection.remove(block_id{blocks.quads.ids[block_index]});
         }
      }
   }

   for (uint32 block_index = 0; block_index < blocks.cylinders.size(); ++block_index) {
      if (blocks.cylinders.hidden[block_index]) continue;

      if (intersects(frustumWS, {.min =
                                    {
                                       blocks.cylinders.bbox.min_x[block_index],
                                       blocks.cylinders.bbox.min_y[block_index],
                                       blocks.cylinders.bbox.min_z[block_index],
                                    },
                                 .max = {
                                    blocks.cylinders.bbox.max_x[block_index],
                                    blocks.cylinders.bbox.max_y[block_index],
                                    blocks.cylinders.bbox.max_z[block_index],
                                 }})) {
         if (op == block_drag_select_op::add) {
            selection.add(block_id{blocks.cylinders.ids[block_index]});
         }
         else if (op == block_drag_select_op::remove) {
            selection.remove(block_id{blocks.cylinders.ids[block_index]});
         }
      }
   }

   for (uint32 block_index = 0; block_index < blocks.stairways.size(); ++block_index) {
      if (blocks.stairways.hidden[block_index]) continue;

      if (intersects(frustumWS, {.min =
                                    {
                                       blocks.stairways.bbox.min_x[block_index],
                                       blocks.stairways.bbox.min_y[block_index],
                                       blocks.stairways.bbox.min_z[block_index],
                                    },
                                 .max = {
                                    blocks.stairways.bbox.max_x[block_index],
                                    blocks.stairways.bbox.max_y[block_index],
                                    blocks.stairways.bbox.max_z[block_index],
                                 }})) {
         if (op == block_drag_select_op::add) {
            selection.add(block_id{blocks.stairways.ids[block_index]});
         }
         else if (op == block_drag_select_op::remove) {
            selection.remove(block_id{blocks.stairways.ids[block_index]});
         }
      }
   }

   for (uint32 block_index = 0; block_index < blocks.cones.size(); ++block_index) {
      if (blocks.cones.hidden[block_index]) continue;

      if (intersects(frustumWS, {.min =
                                    {
                                       blocks.cones.bbox.min_x[block_index],
                                       blocks.cones.bbox.min_y[block_index],
                                       blocks.cones.bbox.min_z[block_index],
                                    },
                                 .max = {
                                    blocks.cones.bbox.max_x[block_index],
                                    blocks.cones.bbox.max_y[block_index],
                                    blocks.cones.bbox.max_z[block_index],
                                 }})) {
         if (op == block_drag_select_op::add) {
            selection.add(block_id{blocks.cones.ids[block_index]});
         }
         else if (op == block_drag_select_op::remove) {
            selection.remove(block_id{blocks.cones.ids[block_index]});
         }
      }
   }
}

}