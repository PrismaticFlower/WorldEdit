#include "world_edit.hpp"

#include "edits/insert_entity.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/sampling.hpp"
#include "math/vector_funcs.hpp"

#include "world/utility/barrier_construction.hpp"
#include "world/utility/object_properties.hpp"
#include "world/utility/raycast_terrain.hpp"
#include "world/utility/terrain_cut.hpp"
#include "world/utility/terrain_sample.hpp"
#include "world/utility/world_utilities.hpp"

#include <numbers>

#include <imgui.h>

namespace we {

namespace {

auto ground_painted_object(const quaternion& rotation, float3 positionWS,
                           const lowercase_string& object_class_name,
                           const world::world& world,
                           world::object_class_library& object_classes) -> float3
{
   const world::object_class_handle class_handle =
      object_classes.acquire(object_class_name);

   const asset_data<assets::msh::flat_model> model =
      object_classes[class_handle].model;

   object_classes.free(class_handle);

   const float3 object_dirWS = normalize(rotation * float3{0.0f, 1.0f, 0.0f});

   float terrain_distance = 0.0f;

   for (const float3& pointLS : model->ground_points) {
      const std::optional<float> hit =
         world::raycast(rotation * pointLS + positionWS, -object_dirWS, world.terrain);

      if (hit and *hit > terrain_distance) {
         terrain_distance = *hit;
      }
   }

   return positionWS - terrain_distance * object_dirWS;
}

}

void world_edit::ui_show_world_object_painter() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({512.0f * _display_scale, 512.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({400.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("Paint Objects on Terrain", &_world_object_paint_open)) {
      if (_object_paint_config.layer >= _world.layer_descriptions.size()) {
         _object_paint_config.layer = 0;
      }

      ImGui::DragFloat("Radius", &_object_paint_config.radius, 1.0f, 1.0f,
                       2048.0f, "%.1f");

      ImGui::DragFloat("Frequency", &_object_paint_config.frequency, 0.125f,
                       1.0f, 32.0f);

      if (ImGui::BeginTable("Randomness", 2)) {
         ImGui::TableNextRow();

         ImGui::TableNextColumn();

         if (ImGui::RadioButton("White Noise", not _object_paint_config.quasirandom)) {
            _object_paint_config.quasirandom = false;
         }

         ImGui::SetItemTooltip("Completely random placement without the brush. "
                               "Like what you want when painting strokes.");

         ImGui::TableNextColumn();

         if (ImGui::RadioButton("Low-discrepancy Sequence",
                                _object_paint_config.quasirandom)) {
            _object_paint_config.quasirandom = true;
         }

         ImGui::SetItemTooltip(
            "Useful if you want to evenly (but \"randomly\") filly an area and "
            "aren't moving the brush much.");

         ImGui::EndTable();
      }

      if (ImGui::BeginTable("Settings", 3)) {
         ImGui::TableNextRow();

         ImGui::TableNextColumn();

         ImGui::Checkbox("Align to Terrain", &_object_paint_config.align_to_terrain);

         ImGui::SetItemTooltip(
            "Align painted objects' orientation to the terrain underneath.");

         ImGui::TableNextColumn();

         ImGui::Checkbox("Randomize Rotations", &_object_paint_config.randomize_rotation);

         ImGui::SetItemTooltip(
            "Randomize painted objects' rotation around the Y axis.");

         ImGui::TableNextColumn();

         ImGui::Checkbox("Auto Barriers", &_object_paint_config.auto_barrier);

         ImGui::SetItemTooltip(
            "Add barriers for painted objects using their collision bounding "
            "boxes. Objects with no collision will not receive a barrier.");

         ImGui::EndTable();
      }

      if (ImGui::BeginCombo("Layer", _world
                                        .layer_descriptions[_object_paint_config.layer]
                                        .name.c_str())) {
         for (std::ptrdiff_t i = 0; i < std::ssize(_world.layer_descriptions); ++i) {
            if (ImGui::Selectable(_world.layer_descriptions[i].name.c_str(),
                                  i == _object_paint_config.layer)) {
               _object_paint_config.layer = static_cast<int8>(i);
            }
         }

         ImGui::EndCombo();
      }

      ImGui::SeparatorText("Object Pool");

      if (ImGui::BeginChild("Object Pool",
                            {0.0f, ImGui::GetContentRegionAvail().y -
                                      ImGui::GetFrameHeightWithSpacing() * 2.0f})) {
         const float thumbnail_size =
            graphics::renderer::thumbnail_base_length * 0.5f * _display_scale;
         const ImVec2 text_offset = {thumbnail_size +
                                        ImGui::GetStyle().ItemSpacing.x,
                                     floorf((thumbnail_size - ImGui::GetFontSize()) *
                                            0.5f)};
         const float button_width = ImGui::GetContentRegionAvail().x;

         for (std::size_t object_class_index = 0;
              object_class_index < _object_paint_config.object_pool.size();
              ++object_class_index) {
            const lowercase_string& class_name =
               _object_paint_config.object_pool[object_class_index];
            bool delete_entry = false;

            if (ImGui::IsRectVisible({thumbnail_size, thumbnail_size})) {
               const std::optional<graphics::object_class_thumbnail> thumbnail =
                  [&]() -> std::optional<graphics::object_class_thumbnail> {
                  try {
                     return _renderer->request_object_class_thumbnail(class_name);
                  }
                  catch (graphics::gpu::exception& e) {
                     handle_gpu_error(e);

                     return std::nullopt;
                  }
               }();

               if (not thumbnail) continue;

               const ImVec2 image_cursor_pos = ImGui::GetCursorScreenPos();

               ImGui::PushID((int)object_class_index);

               ImGui::Selectable("##pick", false, ImGuiSelectableFlags_None,
                                 {button_width, thumbnail_size});

               if (ImGui::BeginPopupContextItem()) {
                  if (ImGui::MenuItem("Delete")) delete_entry = true;

                  ImGui::EndPopup();
               }

               ImGui::PopID();

               ImGui::GetWindowDrawList()
                  ->AddImage(thumbnail->imgui_texture_id, image_cursor_pos,
                             {image_cursor_pos.x + thumbnail_size,
                              image_cursor_pos.y + thumbnail_size},
                             {thumbnail->uv_left, thumbnail->uv_top},
                             {thumbnail->uv_right, thumbnail->uv_bottom});

               if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                  ImGui::TextUnformatted(class_name.data(),
                                         class_name.data() + class_name.size());

                  ImGui::EndTooltip();
               }

               const ImVec2 text_cursor_pos = {image_cursor_pos.x + text_offset.x,
                                               image_cursor_pos.y + text_offset.y};

               ImGui::GetWindowDrawList()->AddText(text_cursor_pos,
                                                   ImGui::GetColorU32(ImGuiCol_Text),
                                                   class_name.data(),
                                                   class_name.data() +
                                                      class_name.size());
            }
            else {
               ImGui::Dummy({button_width, thumbnail_size});
            }

            if (delete_entry) {
               _object_paint_config.object_pool.erase(
                  _object_paint_config.object_pool.begin() + object_class_index);
            }
         }
      }

      ImGui::EndChild();

      ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_Button));
      ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,
                            ImGui::GetColorU32(ImGuiCol_ButtonHovered));
      ImGui::PushStyleColor(ImGuiCol_FrameBgActive,
                            ImGui::GetColorU32(ImGuiCol_ButtonActive));

      if (std::optional<lowercase_string> new_object =
             ui_object_class_pick_widget_untracked(lowercase_string{}, "",
                                                   "##pick_object_class");
          new_object) {
         _object_paint_config.object_pool.push_back(*new_object);
      }

      ImGui::PopStyleColor(3);

      const ImVec2 add_min = ImGui::GetItemRectMin();
      const ImVec2 add_text_size = ImGui::CalcTextSize("Add New Object");

      const ImVec2 add_text_offset =
         {roundf((ImGui::CalcItemWidth() - add_text_size.x) *
                 ImGui::GetStyle().ButtonTextAlign.x),
          roundf((ImGui::GetFrameHeight() - add_text_size.y) *
                 ImGui::GetStyle().ButtonTextAlign.y)};

      ImGui::GetWindowDrawList()->AddText({add_min.x + add_text_offset.x,
                                           add_min.y + add_text_offset.y},
                                          ImGui::GetColorU32(ImGuiCol_Text),
                                          "Add New Object");

      if (ImGui::BeginCombo("Object Pool History", "<previous object pools>",
                            ImGuiComboFlags_HeightLargest)) {
         for (std::size_t i = 0;
              i < _world.configuration.untracked_paint_object_pool_history.size();
              ++i) {
            const std::vector<lowercase_string>& pool =
               _world.configuration.untracked_paint_object_pool_history[i];

            const float thumbnail_size =
               graphics::renderer::thumbnail_base_length * 0.5f * _display_scale;
            const ImVec2 text_offset =
               {thumbnail_size + ImGui::GetStyle().ItemSpacing.x,
                floorf((thumbnail_size - ImGui::GetFontSize()) * 0.5f)};
            const float button_width = ImGui::GetContentRegionAvail().x;

            const ImVec2 base_cursor_pos = ImGui::GetCursorScreenPos();

            ImGui::PushID((int)i);

            if (ImGui::Selectable("##pick", _object_paint_config.object_pool == pool,
                                  ImGuiSelectableFlags_None,
                                  {button_width, thumbnail_size * pool.size()})) {
               _object_paint_config.object_pool = pool;
            }

            if (ImGui::BeginItemTooltip()) {
               for (const lowercase_string& class_name : pool) {
                  ImGui::TextUnformatted(class_name.data(),
                                         class_name.data() + class_name.size());
               }

               ImGui::EndTooltip();
            }

            ImGui::PopID();

            for (std::size_t object_class_index = 0;
                 object_class_index < pool.size(); ++object_class_index) {
               const lowercase_string& class_name = pool[object_class_index];

               const ImVec2 image_min = {base_cursor_pos.x,
                                         base_cursor_pos.y +
                                            thumbnail_size * object_class_index};
               const ImVec2 image_max = {image_min.x + thumbnail_size,
                                         image_min.y + thumbnail_size};

               if (ImGui::IsRectVisible(image_min, image_max)) {
                  const std::optional<graphics::object_class_thumbnail> thumbnail =
                     [&]() -> std::optional<graphics::object_class_thumbnail> {
                     try {
                        return _renderer->request_object_class_thumbnail(class_name);
                     }
                     catch (graphics::gpu::exception& e) {
                        handle_gpu_error(e);

                        return std::nullopt;
                     }
                  }();

                  if (not thumbnail) continue;

                  ImGui::GetWindowDrawList()
                     ->AddImage(thumbnail->imgui_texture_id, image_min, image_max,
                                {thumbnail->uv_left, thumbnail->uv_top},
                                {thumbnail->uv_right, thumbnail->uv_bottom});

                  const ImVec2 text_cursor_pos = {image_min.x + text_offset.x,
                                                  image_min.y + text_offset.y};

                  ImGui::GetWindowDrawList()->AddText(text_cursor_pos,
                                                      ImGui::GetColorU32(ImGuiCol_Text),
                                                      class_name.data(),
                                                      class_name.data() +
                                                         class_name.size());
               }
            }

            if (i != _world.configuration.untracked_paint_object_pool_history.size() - 1) {
               ImGui::Separator();
            }
         }

         ImGui::EndCombo();
      }

      ImGui::End();
   }

   if (_hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      ImGui::SeparatorText("Terrain Object Painting");

      ImGui::Text("Increase Radius");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Object Painting", "Increase Radius")));

      ImGui::Text("Decrease Radius");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Object Painting", "Decrease Radius")));

      ImGui::End();
   }

   if (not _world_object_paint_open) return;

   for (const lowercase_string& object_class : _object_paint_config.object_pool) {
      _temporary_object_classes.add(object_class, _object_classes);
   }

   graphics::camera_ray rayWS =
      make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                      {ImGui::GetMainViewport()->Size.x,
                       ImGui::GetMainViewport()->Size.y});

   float hit_distance = FLT_MAX;

   if (auto hit = world::raycast(rayWS.origin, rayWS.direction, _world.terrain); hit) {
      hit_distance = *hit;
   }
   else {
      return;
   }

   const float3 cursor_positionWS = rayWS.origin + rayWS.direction * hit_distance;

   // Draw Brush
   {
      const float radius = _object_paint_config.radius;
      const int brush_segments = static_cast<int>(radius + 0.5f);
      const float pi2 = std::numbers::pi_v<float> * 2.0f;

      const float3 startLS = {1.0f, 0.0f, 0.0f};

      const float3 startWS = startLS * radius + cursor_positionWS;
      float3 lastWS = startWS;

      for (int i = 0; i < brush_segments; ++i) {
         const float t = i / static_cast<float>(brush_segments);
         const float segment = t * pi2;

         const float3 currentLS = {cosf(segment), 0.0f, sinf(segment)};
         const float3 currentWS = currentLS * radius + cursor_positionWS;

         _tool_visualizers.add_line_overlay(lastWS, currentWS, 0xff'ff'ff'ff);

         lastWS = currentWS;
      }

      _tool_visualizers.add_line_overlay(lastWS, startWS, 0xff'ff'ff'ff);
   }

   if (_object_paint_context.mouse_held != _object_paint_context.painting) {
      _object_paint_context.started.restart();
      _object_paint_context.painted_objects = 0;

      _object_paint_context.painting = _object_paint_context.mouse_held;
   }

   if (_object_paint_context.painting and
       not _object_paint_config.object_pool.empty()) {
      const float add_time_slice = 1.0f / _object_paint_config.frequency;
      const int objects_to_paint =
         std::max(static_cast<int>((_object_paint_context.started.elapsed() + add_time_slice) /
                                   add_time_slice) -
                     _object_paint_context.painted_objects,
                  0);

      for (int i = 0; i < objects_to_paint; ++i) {
         const int32 index = i + _object_paint_context.painted_objects;

         const lowercase_string& object_class_name =
            _object_paint_config.object_pool[_object_paint_context.random.generate_bounded(
               static_cast<uint32>(_object_paint_config.object_pool.size()))];

         const float2 sample =
            _object_paint_config.quasirandom
               ? R2(index)
               : float2{_object_paint_context.random.generate_unorm_float(),
                        _object_paint_context.random.generate_unorm_float()};
         const float2 disk_sample = concentric_sample_disk(sample);

         float3 positionWS =
            cursor_positionWS + float3{disk_sample.x, 0.0f, disk_sample.y} *
                                   _object_paint_config.radius;

         if (const std::optional<float> hit =
                world::raycast(float3{positionWS.x,
                                      _world.terrain.height_scale * (INT16_MAX + 1),
                                      positionWS.z},
                               {0.0f, -1.0f, 0.0f}, _world.terrain);
             hit) {
            positionWS.y = _world.terrain.height_scale * (INT16_MAX + 1) - *hit;
         }

         const float3 terrain_normalWS =
            world::sample_terrain_normal(_world.terrain, positionWS);
         quaternion rotation = rotation_between({0.0f, 1.0f, 0.0f}, terrain_normalWS);

         if (_object_paint_config.randomize_rotation) {
            const float y_rotation =
               _object_paint_context.random.generate_unorm_float() * 2.0f - 1.0f;

            rotation = rotation * quaternion{sqrt(1.0f - y_rotation * y_rotation),
                                             0.0f, y_rotation, 0.0f};
         }

         positionWS = ground_painted_object(rotation, positionWS, object_class_name,
                                            _world, _object_classes);

         world::object object = {.rotation = rotation,
                                 .position = positionWS,
                                 .class_name = object_class_name,

                                 .id = _world.next_id.objects.aquire()};

         if (_world.objects.size() == _world.objects.max_size()) {
            MessageBoxA(_window,
                        fmt::format("Max Objects ({}) Reached", _world.objects.max_size())
                           .c_str(),
                        "Limit Reached", MB_OK);
            break;
         }

         _edit_stack_world.apply(edits::make_insert_entity(std::move(object),
                                                           _object_classes),
                                 _edit_context, {.transparent = index != 0});

         if (_object_paint_config.auto_barrier) {
            const world::object_class& object_class =
               _object_classes[_world.objects.back().class_handle];

            if (object_class.model->collision_bounding_box.min != float3{} or
                object_class.model->collision_bounding_box.max != float3{}) {
               if (_world.barriers.size() == _world.barriers.max_size()) {
                  MessageBoxA(_window,
                              fmt::format("Max Barriers ({}) Reached",
                                          _world.barriers.max_size())
                                 .c_str(),
                              "Limit Reached", MB_OK);
                  break;
               }

               const world::barrier_metrics metrics =
                  world::barrier_from_object(_world.objects.back(), _object_classes);

               _edit_stack_world
                  .apply(edits::make_insert_entity(world::barrier{
                            .name = world::create_unique_name(_world.barriers, "Barrier"),
                            .position = metrics.position,
                            .size = metrics.size,
                            .rotation_angle = metrics.rotation_angle,
                            .id = _world.next_id.barriers.aquire(),
                         }),
                         _edit_context, {.transparent = true});
            }
         }
      }

      if (_object_paint_context.painted_objects == 0 and objects_to_paint > 0) {
         std::erase(_world.configuration.untracked_paint_object_pool_history,
                    _object_paint_config.object_pool);

         _world.configuration.untracked_paint_object_pool_history.push_back(
            _object_paint_config.object_pool);

         while (_world.configuration.untracked_paint_object_pool_history.size() > 10) {
            _world.configuration.untracked_paint_object_pool_history.erase(
               _world.configuration.untracked_paint_object_pool_history.begin());
         }
      }

      _object_paint_context.painted_objects += objects_to_paint;
   }
}
}