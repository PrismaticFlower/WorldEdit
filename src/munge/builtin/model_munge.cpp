#include "model_munge.hpp"

#include "model_munge/error.hpp"
#include "model_munge/load_model.hpp"
#include "model_munge/model.hpp"
#include "model_munge/write_model.hpp"

#include "assets/req/builder.hpp"
#include "assets/req/io.hpp"
#include "assets/req/requirement_list.hpp"

#include "io/error.hpp"

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

void execute_model_munge(const tool_context& context) noexcept;

void execute_model_munge(const io::path& input_file_path,
                         const std::vector<assets::option>& directory_options,
                         const tool_context& context)
{
   const model_container model =
      load_model(input_file_path, directory_options, context.feedback);

   write_req(io::compose_path(context.output_path, input_file_path.stem(), ".model.req"),
             model);

   write_model(io::compose_path(context.output_path, input_file_path.stem(), ".model"),
               model);
}

}