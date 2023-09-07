#pragma once

#include "io.hpp"
#include "assets/config/io.hpp"
#include "io/output_file.hpp"
#include "io/read_file.hpp"

namespace we::settings {

namespace {

void write(io::output_file& file, std::string_view name, float value)
{
   file.write_ln("\t{}({:f});", name, value);
}

void write(io::output_file& file, std::string_view name, float3 value)
{
   file.write_ln("\t{}({:f}, {:f}, {:f});", name, value.x, value.y, value.z);
}

void write(io::output_file& file, std::string_view name, float4 value)
{
   file.write_ln("\t{}({:f}, {:f}, {:f}, {:f});", name, value.x, value.y,
                 value.z, value.w);
}

void write(io::output_file& file, std::string_view name, const std::string& value)
{
   file.write_ln("\t{}(\"{}\");", name, value);
}

void read(assets::config::node& node, float& out)
{
   out = node.values.get<float>(0);
}

void read(assets::config::node& node, float3& out)
{
   out = {node.values.get<float>(0), node.values.get<float>(1),
          node.values.get<float>(2)};
}

void read(assets::config::node& node, float4& out)
{
   out = {node.values.get<float>(0), node.values.get<float>(1),
          node.values.get<float>(2), node.values.get<float>(3)};
}

void read(assets::config::node& node, std::string& out)
{
   out = node.values.get<std::string>(0);
}

}

// I'm hoping the use of macros in these functions reduces the likelihood of typos causing fun bugs.
// But maybe they're just making the functions ugly for no reason. Hopefully it's the former.

auto load(const std::string_view path) -> settings
{
   assets::config::node root =
      assets::config::read_config(io::read_file_to_string(path));

   settings settings{};

   for (auto& node : root) {
      if (node.key == "graphics") {
#define setting_entry(setting)                                                 \
   if (prop.key == #setting) {                                                 \
      read(prop, settings.graphics.setting);                                   \
      continue;                                                                \
   }
         for (auto& prop : node) {
            setting_entry(path_node_color);
            setting_entry(path_node_outline_color);
            setting_entry(path_node_connection_color);
            setting_entry(path_node_orientation_color);
            setting_entry(region_color);
            setting_entry(barrier_outline_color);
            setting_entry(barrier_overlay_color);
            setting_entry(planning_hub_outline_color);
            setting_entry(planning_hub_overlay_color);
            setting_entry(planning_connection_outline_color);
            setting_entry(planning_connection_overlay_color);
            setting_entry(sector_color);
            setting_entry(portal_color);
            setting_entry(hintnode_color);
            setting_entry(boundary_color);
            setting_entry(light_volume_alpha);
            setting_entry(hover_color);
            setting_entry(selected_color);
            setting_entry(creation_color);
            setting_entry(barrier_height);
            setting_entry(boundary_height);
            setting_entry(planning_hub_height);
            setting_entry(planning_connection_height);
            setting_entry(path_node_size);
            setting_entry(line_width);
         }
#undef setting_entry
      }
      else if (node.key == "camera") {
#define setting_entry(setting)                                                 \
   if (prop.key == #setting) {                                                 \
      read(prop, settings.camera.setting);                                     \
      continue;                                                                \
   }
         for (auto& prop : node) {
            setting_entry(move_speed);
            setting_entry(look_sensitivity);
            setting_entry(pan_sensitivity);
            setting_entry(sprint_power);
            setting_entry(step_size);
            setting_entry(fov);
            setting_entry(view_width);
         }
#undef setting_entry
      }
      else if (node.key == "ui") {
#define setting_entry(setting)                                                 \
   if (prop.key == #setting) {                                                 \
      read(prop, settings.ui.setting);                                         \
      continue;                                                                \
   }
         for (auto& prop : node) {
            setting_entry(extra_scaling);
         }
#undef setting_entry
      }
      else if (node.key == "preferences") {
#define setting_entry(setting)                                                 \
   if (prop.key == #setting) {                                                 \
      read(prop, settings.preferences.setting);                                \
      continue;                                                                \
   }
         for (auto& prop : node) {
            setting_entry(text_editor);
         }
#undef setting_entry
      }
   }

   return settings;
}

void save(const std::string_view path, const settings& settings) noexcept
{
   try {
      io::output_file file{path};

      file.write_ln("// WorldEdit Settings. Autosaved at app exit.\n");

      file.write_ln("graphics()");
      file.write_ln("{");

#define name_value(prop) #prop, settings.graphics.prop

      write(file, name_value(path_node_color));
      write(file, name_value(path_node_outline_color));
      write(file, name_value(path_node_connection_color));
      write(file, name_value(path_node_orientation_color));
      write(file, name_value(region_color));
      write(file, name_value(barrier_outline_color));
      write(file, name_value(barrier_overlay_color));
      write(file, name_value(planning_hub_outline_color));
      write(file, name_value(planning_hub_overlay_color));
      write(file, name_value(planning_connection_outline_color));
      write(file, name_value(planning_connection_overlay_color));
      write(file, name_value(sector_color));
      write(file, name_value(portal_color));
      write(file, name_value(hintnode_color));
      write(file, name_value(boundary_color));
      write(file, name_value(light_volume_alpha));
      write(file, name_value(hover_color));
      write(file, name_value(selected_color));
      write(file, name_value(creation_color));
      write(file, name_value(barrier_height));
      write(file, name_value(boundary_height));
      write(file, name_value(planning_hub_height));
      write(file, name_value(planning_connection_height));
      write(file, name_value(path_node_size));
      write(file, name_value(line_width));

#undef name_value

      file.write_ln("}\n");

      file.write_ln("camera()");
      file.write_ln("{");

#define name_value(prop) #prop, settings.camera.prop

      write(file, name_value(move_speed));
      write(file, name_value(look_sensitivity));
      write(file, name_value(pan_sensitivity));
      write(file, name_value(sprint_power));
      write(file, name_value(step_size));
      write(file, name_value(fov));
      write(file, name_value(view_width));

#undef name_value

      file.write_ln("}\n");

      file.write_ln("ui()");
      file.write_ln("{");

#define name_value(prop) #prop, settings.ui.prop

      write(file, name_value(extra_scaling));

#undef name_value

      file.write_ln("}\n");

      file.write_ln("preferences()");
      file.write_ln("{");

#define name_value(prop) #prop, settings.preferences.prop

      write(file, name_value(text_editor));

#undef name_value

      file.write_ln("}\n");
   }
   catch (std::exception&) {
      // Oh well...
   }
}

saver::~saver()
{
   save(path, settings);
}

}