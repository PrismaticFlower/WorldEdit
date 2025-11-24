#include "blocks_munge.hpp"

#include "../shared.hpp"

#include "assets/req/io.hpp"

#include "io/error.hpp"
#include "io/output_file.hpp"
#include "io/read_file.hpp"

#include "ucfb/reader.hpp"
#include "ucfb/writer.hpp"

#include "utility/string_icompare.hpp"

#include "os/process.hpp"

#include "world/blocks.hpp"
#include "world/io/load_blocks.hpp"
#include "world/io/save_blocks_meshes.hpp"

#include <fmt/core.h>

#include <absl/container/btree_set.h>

using namespace we::ucfb::literals;

namespace we::munge {

namespace {

constexpr std::string_view temp_intermediate_prefix = "BLKTMP";
constexpr std::string_view temp_munged_prefix = "BLKMUN";

constexpr std::string_view blocks_layer_name = "_WE_blocks";

void munge_raw_files(const tool_context& context)
{
   // ODFs
   {
      os::process process = os::process_create_desc{
         .executable_path =
            io::compose_path(context.toolsfl_bin_path, "OdfMunge.exe"),
         .command_line =
            fmt::format("-inputfile *.odf -platform {} -sourcedir {} "
                        "-outputdir {}",
                        context.platform, context.source_path.string_view(),
                        context.output_path.string_view()),
         .working_directory = context.toolsfl_bin_path,
         .capture_stdout = true,
         .capture_stderr = true,
         .priority = munge_process_priority,
      };

      std::string standard_error = process.get_standard_error();

      context.feedback.print_output(process.get_standard_output());
      context.feedback.parse_error_string(standard_error, context.source_path);
      context.feedback.print_errors(std::move(standard_error));

      process.wait_for_exit();
   }

   // MSHs
   {
      os::process process = os::process_create_desc{
         .executable_path =
            io::compose_path(context.toolsfl_bin_path, "pc_ModelMunge.exe"),
         .command_line =
            fmt::format("-inputfile *.msh -platform {} -sourcedir {} "
                        "-outputdir {}",
                        context.platform, context.source_path.string_view(),
                        context.output_path.string_view()),
         .working_directory = context.toolsfl_bin_path,
         .capture_stdout = true,
         .capture_stderr = true,
         .priority = munge_process_priority,
      };

      std::string standard_error = process.get_standard_error();

      context.feedback.print_output(process.get_standard_output());
      context.feedback.parse_error_string(standard_error, context.source_path);
      context.feedback.print_errors(std::move(standard_error));

      process.wait_for_exit();
   }
}

void combine_and_edit_segm(ucfb::reader segm, ucfb::writer& out)
{
   while (segm) {
      ucfb::reader child = segm.read_child();

      if (child.id() == "VBUF"_id) {
         const auto [count, stride, format] =
            child.read_multi<uint32, uint32, uint32>();

         const uint32 compressed_texcoords_flags = 0x00008000;

         if ((format & compressed_texcoords_flags) != 0) continue;

         child.reset_head();

         out.write_child("VBUF"_id).write(child.read_bytes(child.size()));
      }
      else {
         out.write_child(child.id()).write(child.read_bytes(child.size()));
      }
   }
}

void combine_and_edit_modl(ucfb::reader modl, ucfb::writer& out)
{
   while (modl) {
      ucfb::reader child = modl.read_child();

      auto id = child.id();

      fmt::println("{}", std::string_view{(const char*)&id, 4});

      if (child.id() == "segm"_id) {
         ucfb::writer segm_out = out.write_child("segm"_id);

         combine_and_edit_segm(child, segm_out);
      }
      else {
         out.write_child(child.id()).write(child.read_bytes(child.size()));
      }
   }
}

void combine_and_edit_model(ucfb::reader reader, ucfb::writer& out)
{
   while (reader) {
      ucfb::reader child = reader.read_child();

      if (child.id() == "modl"_id) {
         ucfb::writer modl_out = out.write_child("modl"_id);

         combine_and_edit_modl(child, modl_out);
      }
      else {
         out.write_child(child.id()).write(child.read_bytes(child.size()));
      }
   }
}

void combine_munged_files(const io::path& output_file_path,
                          const io::path& source_dir, const std::string_view platform,
                          const std::string_view world_name,
                          const std::size_t object_count)
{
   io::output_file file{output_file_path};
   ucfb::writer ucfb_out{"ucfb"_id, file, ucfb::writer_options{}};

   absl::btree_set<std::string> needed_textures;

   for (std::size_t object_index = 0; object_index < object_count; ++object_index) {
      const std::string object_name =
         fmt::format("WE_{}_blocks{}", world_name, object_index);

      for (const assets::req::requirement_list& model_list :
           assets::req::read(io::read_file_to_string(
              io::compose_path(source_dir, object_name, ".model.req")))) {
         if (model_list.file_type != "texture") continue;

         for (const std::string& texture : model_list.entries) {
            needed_textures.emplace(texture);
         }
      }

      if (const std::vector<std::byte> bytes = io::read_file_to_bytes(
             io::compose_path(source_dir, object_name, ".model"));
          string::iequals(platform, "pc")) {
         combine_and_edit_model(ucfb::reader{bytes, {.aligned_children = true}},
                                ucfb_out);
      }
      else {
         ucfb::reader reader{bytes, {.aligned_children = true}};

         ucfb_out.write(reader.read_bytes(reader.size()));
      }

      // .class
      {
         const std::vector<std::byte> bytes = io::read_file_to_bytes(
            io::compose_path(source_dir, object_name, ".class"));

         ucfb::reader reader{bytes, {.aligned_children = true}};

         ucfb_out.write(reader.read_bytes(reader.size()));
      }
   }

   {
      ucfb::writer wrld = ucfb_out.write_child("wrld"_id);

      wrld.write_child("NAME"_id).write(blocks_layer_name);
      wrld.write_child("INFO"_id).write(uint32{0}, static_cast<uint32>(object_count));

      for (std::size_t object_index = 0; object_index < object_count; ++object_index) {
         ucfb::writer inst = wrld.write_child("inst"_id);

         // INFO
         {
            ucfb::writer info = inst.write_child("INFO"_id);

            info.write_child("TYPE"_id).write(
               fmt::format("WE_{}_blocks{}", world_name, object_index));
            info.write_child("NAME"_id).write("");

            // XFRM
            {
               ucfb::writer xfrm = info.write_child("XFRM"_id);

               xfrm.write(float3{1.0f, 0.0f, 0.0f});
               xfrm.write(float3{0.0f, 1.0f, 0.0f});
               xfrm.write(float3{0.0f, 0.0f, 1.0f});
               xfrm.write(float3{0.0f, 0.0f, 0.0f});
            }
         }
      }
   }

   assets::req::requirement_list requirements = {
      .file_type = "texture",
      .entries = {needed_textures.begin(), needed_textures.end()},
   };

   assets::req::save(io::make_path_with_new_extension(output_file_path,
                                                      ".blocks.req"),
                     std::span{&requirements, 1});
}

void execute_blocks_munge(const io::path& input_path, const tool_context& context)
{
   const io::path output_path =
      io::compose_path(context.output_path, input_path.stem(), ".blocks");
   const io::path intermediate_path =
      io::compose_path(context.output_path, input_path.filename());

   if (io::exists(intermediate_path) and (io::get_last_write_time(input_path) <=
                                          io::get_last_write_time(intermediate_path))) {
      return;
   }

   std::string blocks_string;

   try {
      blocks_string = io::read_file_to_string(input_path);
   }
   catch (io::error& e) {
      context.feedback.add_error(
         {.file = input_path,
          .tool = "BlocksMunge",
          .message = fmt::format("Unable to read file: {}", e.what())});

      return;
   }

   // Early out for unchanged blocks.
   try {
      if (blocks_string == io::read_file_to_string(intermediate_path)) {
         return;
      }
   }
   catch (io::error&) {
      // This is fine, just munge anyway
   }

   context.feedback.print_output(fmt::format("Munging {}", input_path.filename()));

   world::blocks blocks;

   try {
      null_output_stream output;

      blocks = world::load_blocks_from_string(blocks_string, {}, output);
   }
   catch (std::exception& e) {
      context.feedback.add_error(
         {.file = input_path,
          .tool = "BlocksMunge",
          .message = fmt::format("Failed to parse blocks: {}", e.what())});

      return;
   }

   const io::path temp_intermediate_path =
      io::compose_path(context.output_path, fmt::format("{}_{}", temp_intermediate_prefix,
                                                        input_path.stem()));

   if (not io::exists(temp_intermediate_path) and
       not io::create_directory(temp_intermediate_path)) {
      context.feedback.add_error(
         {.file = input_path,
          .tool = "BlocksMunge",
          .message =
             fmt::format("Failed to create temp directory ('{}') for munging.",
                         temp_intermediate_path.string_view())});

      return;
   }

   const io::path temp_munged_path =
      io::compose_path(context.output_path,
                       fmt::format("{}_{}", temp_munged_prefix, input_path.stem()));

   if (not io::exists(temp_munged_path) and not io::create_directory(temp_munged_path)) {
      context.feedback.add_error(
         {.file = input_path,
          .tool = "BlocksMunge",
          .message =
             fmt::format("Failed to create temp directory ('{}') for munging.",
                         temp_munged_path.string_view())});

      return;
   }

   const std::string_view world_name = input_path.stem();
   std::size_t saved_meshes = 0;

   try {
      saved_meshes =
         world::save_blocks_meshes(temp_intermediate_path, world_name, blocks);
   }
   catch (io::error& e) {
      context.feedback.add_error(
         {.file = input_path,
          .tool = "BlocksMunge",
          .message =
             fmt::format("Failed to save intermediate files for munging: {}",
                         e.what())});

      return;
   }

   {
      tool_context child_context = context;

      child_context.source_path = temp_intermediate_path;
      child_context.output_path = temp_munged_path;

      munge_raw_files(child_context);
   }

   try {
      combine_munged_files(io::compose_path(context.output_path, world_name, ".blocks"),
                           temp_munged_path, context.platform, world_name,
                           saved_meshes);
   }
   catch (io::error& e) {
      context.feedback.add_error(
         {.file = input_path,
          .tool = "BlocksMunge",
          .message = fmt::format("Failed to save combine munged files: {}", e.what())});

      return;
   }

   if (not io::copy_file(input_path, intermediate_path)) {
      context.feedback.add_warning(
         {.file = input_path, .tool = "BlocksMunge", .message = "Failed to create cached intermediate file. Munging will still work but checks for if it's safe to skip the munge will always fail."});
   }

   for (const io::directory_entry& entry :
        io::directory_iterator{temp_intermediate_path, false}) {
      if (not entry.is_file) continue;

      if (not io::remove(entry.path)) {
         context.feedback.add_warning(
            {.file = entry.path, .tool = "BlocksMunge", .message = "Failed to remove temporary file. Munging should still succeed."});
      }
   }

   for (const io::directory_entry& entry :
        io::directory_iterator{temp_munged_path, false}) {
      if (not entry.is_file) continue;

      if (not io::remove(entry.path)) {
         context.feedback.add_warning(
            {.file = entry.path, .tool = "BlocksMunge", .message = "Failed to remove temporary file. Munging should still succeed."});
      }
   }
}

}

void execute_blocks_munge(const tool_context& context) noexcept
{
   for (const io::directory_entry& entry : io::directory_iterator{context.source_path}) {
      if (not string::iequals(entry.path.extension(), ".blk")) continue;

      try {
         execute_blocks_munge(entry.path, context);
      }
      catch (std::exception& e) {
         context.feedback.add_error(
            {.file = entry.path,
             .tool = "BlocksMunge",
             .message = fmt::format("Unexpected Error Occured: {}", e.what())});
      }
   }
}

}