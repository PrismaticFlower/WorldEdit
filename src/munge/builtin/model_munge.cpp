#include "model_munge.hpp"

#include "model_munge/error.hpp"
#include "model_munge/load_model.hpp"
#include "model_munge/model.hpp"
#include "model_munge/write_model.hpp"

#include "assets/req/builder.hpp"
#include "assets/req/io.hpp"
#include "assets/req/requirement_list.hpp"

#include "executor.hpp"

#include "io/error.hpp"

#include <fmt/format.h>

namespace we::munge {

namespace {

void write_req(const io::path& output_file_path, const model_container& model_container)
{
   assets::req::requirement_list textures{.file_type = "texture"};
   textures.entries.reserve(8);

   for (const model& model : model_container.models) {
      for (const model_segment& segment : model.segments) {
         for (const std::string& texture : segment.material.textures) {
            if (texture.empty()) continue;

            assets::req::add_to(textures.entries, texture);
         }
      }
   }

   for (const cloth& cloth : model_container.cloths) {
      if (cloth.texture_name.empty()) continue;

      assets::req::add_to(textures.entries, cloth.texture_name);
   }

   if (textures.entries.empty()) return;

   try {
      assets::req::save(output_file_path, std::span{&textures, 1});
   }
   catch (io::open_error& e) {
      throw model_error{e.what(), model_ec::req_write_io_open_error};
   }
   catch (io::error& e) {
      throw model_error{e.what(), model_ec::req_write_io_generic_error};
   }
}

}

void execute_model_munge(const tool_context& context) noexcept
{
   execute_builtin_munge({.input_extension = "msh",
                          .output_extension = "model",
                          .tool_name = "ModelMunge",
                          .execute_munge = execute_model_munge},
                         context);
}

void execute_model_munge(const io::path& input_file_path,
                         const std::vector<assets::option>& directory_options,
                         const tool_context& context)
{
   context.feedback.print_output(fmt::format("Munging {}", input_file_path.filename()));

   const model_container model =
      load_model(input_file_path, directory_options, context.feedback);

   write_req(io::compose_path(context.output_path, input_file_path.stem(), ".model.req"),
             model);

   write_model(io::compose_path(context.output_path, input_file_path.stem(), ".model"),
               model);
}

}