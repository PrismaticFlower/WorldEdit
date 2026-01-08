#include "odf_munge.hpp"

#include "utility/bf_fnv_1a_hash.hpp"

#include "assets/odf/definition_io.hpp"
#include "assets/req/builder.hpp"
#include "assets/req/io.hpp"
#include "assets/req/requirement_list.hpp"

#include "io/error.hpp"
#include "io/output_file.hpp"
#include "io/read_file.hpp"

#include "odf_munge/error.hpp"

#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include "ucfb/writer.hpp"

#include <fmt/format.h>

#include <cstring>

using namespace we::string;
using namespace we::ucfb::literals;

namespace we::munge {

namespace {

struct queued_munge {
   io::path path;
   async::task<void> task;
};

auto load_definition(const io::path& input_file_path) -> assets::odf::definition
{
   std::vector<char> odf_contents;

   try {
      odf_contents = io::read_file_to_chars(input_file_path);
   }
   catch (io::error& e) {
      throw odf_error{e.what(), odf_ec::odf_load_io_error};
   }

   try {
      return assets::odf::read_definition(std::move(odf_contents));
   }
   catch (std::exception& e) {
      throw odf_error{e.what(), odf_ec::odf_load_parse_error};
   }
}

struct req_lists {
   std::vector<std::string> texture;
   std::vector<std::string> sprite;
   std::vector<std::string> animbank;
   std::vector<std::string> anim;
   std::vector<std::string> animset;
   std::vector<std::string> model;
   std::vector<std::string> class_;
   std::vector<std::string> config;
};

auto build_req_lists(const assets::odf::definition& definition) -> req_lists
{
   req_lists requirements;

   using assets::req::add_to;

   if (not definition.header.class_parent.empty()) {
      add_to(requirements.class_, definition.header.class_parent);
   }

   for (const assets::odf::property& prop : definition.properties) {
      if (prop.value.empty()) continue;
      if (not std::isalpha(prop.value[0]) and prop.value[0] != '_') continue;

      const std::string_view value = string::trim_trailing_whitespace(prop.value);

      if (iequals(prop.key, "AnimationAddon") or //
          iequals(prop.key, "AnimationName") or  //
          iequals(prop.key, "AnimationLowres")) {
         add_to(requirements.animbank, value);
      }
      else if (iequals(prop.key, "PilotAnimation") or
               iequals(prop.key, "SoldierAnimation")) {
         add_to(requirements.anim, fmt::format("human_{}", value));
      }
      else if (iequals(prop.key, "ComboAnimationBank")) {
         auto [animset, id_combo] = split_first_of_exclusive(value, " ");

         id_combo = trim_leading_whitespace(id_combo);

         auto [id, combo] = split_first_of_exclusive(id_combo, " ");

         if (animset.contains('_')) {
            add_to(requirements.animset, animset);
         }
         else {
            add_to(requirements.animset, fmt::format("human_{}", animset));
         }

         add_to(requirements.config, combo);
      }
      else if (iequals(prop.key, "AnimationBank") or //
               iequals(prop.key, "CustomAnimationBank")) {
         if (value.contains('_')) {
            add_to(requirements.animset, value);
         }
         else {
            add_to(requirements.animset, fmt::format("human_{}", value));
         }
      }
      else if (iequals(prop.key, "AttachTrigger")) {
         auto [hp, args6] = split_first_of_exclusive(value, " ");

         args6 = trim_leading_whitespace(args6);

         auto [time0, args5] = split_first_of_exclusive(args6, " ");

         args5 = trim_leading_whitespace(args5);

         auto [anim0, args4] = split_first_of_exclusive(args5, " ");

         args4 = trim_leading_whitespace(args4);

         auto [anim1, args3] = split_first_of_exclusive(args4, " ");

         if (not anim0.empty()) add_to(requirements.anim, anim0);
         if (not anim1.empty()) {
            add_to(requirements.anim, fmt::format("human_{}", anim1));
         }
      }
      else if (iequals(prop.key, "AnimationTrigger") or
               iequals(prop.key, "OrdnanceCollision")) {
         // These create no references.
      }
      else if (icontains(prop.key, "Animation")) {
         add_to(requirements.anim, value);
      }
      else if (icontains(prop.key, "Effect") or
               icontains(prop.key, "Emitter")) {
         add_to(requirements.config, value);
      }
      else if (icontains(prop.key, "Explosion") or //
               icontains(prop.key, "Odf") or       //
               icontains(prop.key, "Ordnance") or  //
               icontains(prop.key, "Weapon")) {
         add_to(requirements.class_, value);
      }
      else if (icontains(prop.key, "Geometry") or //
               icontains(prop.key, "Model")) {
         add_to(requirements.model, value);
      }
      else if (icontains(prop.key, "Skeleton")) {
         add_to(requirements.animbank, value);
      }
      else if (icontains(prop.key, "Sprite")) {
         add_to(requirements.sprite, value);
      }
      else if (icontains(prop.key, "Texture")) {
         add_to(requirements.texture, value);
      }
   }

   return requirements;
}

void write_req(const io::path& output_file_path, req_lists lists)
{
   std::vector<assets::req::requirement_list> requirements;
   requirements.reserve(not lists.texture.empty() +  //
                        not lists.sprite.empty() +   //
                        not lists.animbank.empty() + //
                        not lists.anim.empty() +     //
                        not lists.animset.empty() +  //
                        not lists.model.empty() +    //
                        not lists.class_.empty() +   //
                        not lists.config.empty());

   if (not lists.texture.empty()) {
      requirements.push_back(
         {.file_type = "texture", .entries = std::move(lists.texture)});
   }

   if (not lists.sprite.empty()) {
      requirements.push_back(
         {.file_type = "sprite", .entries = std::move(lists.sprite)});
   }

   if (not lists.animbank.empty()) {
      requirements.push_back(
         {.file_type = "animbank", .entries = std::move(lists.animbank)});
   }

   if (not lists.anim.empty()) {
      requirements.push_back({.file_type = "anim", .entries = std::move(lists.anim)});
   }

   if (not lists.animset.empty()) {
      requirements.push_back(
         {.file_type = "animset", .entries = std::move(lists.animset)});
   }

   if (not lists.model.empty()) {
      requirements.push_back({.file_type = "model", .entries = std::move(lists.model)});
   }

   if (not lists.class_.empty()) {
      requirements.push_back({.file_type = "class", .entries = std::move(lists.class_)});
   }

   if (not lists.config.empty()) {
      requirements.push_back(
         {.file_type = "config", .entries = std::move(lists.config)});
   }

   try {
      assets::req::save(output_file_path, requirements);
   }
   catch (io::open_error& e) {
      throw odf_error{e.what(), odf_ec::req_write_io_open_error};
   }
   catch (io::error& e) {
      throw odf_error{e.what(), odf_ec::req_write_io_generic_error};
   }
}

void write_class(const io::path& output_file_path, const assets::odf::definition& definition)
{
   try {
      io::output_file out{output_file_path};

      ucfb::writer ucfb{"ucfb"_id, out, {}};

      // expc/ordc/wpnc/entc
      {
         const ucfb::chunk_id root_id = [&] {
            switch (definition.type) {
            case assets::odf::type::explosion_class:
               return "expc"_id;
            case assets::odf::type::ordnance_class:
               return "ordc"_id;
            case assets::odf::type::weapon_class:
               return "wpnc"_id;
            case assets::odf::type::game_object_class:
            default:
               return "entc"_id;
            }
         }();

         ucfb::writer cls = ucfb.write_child(root_id);

         const std::string_view class_base = definition.header.class_parent.empty()
                                                ? definition.header.class_label
                                                : definition.header.class_parent;

         cls.write_child("BASE"_id).write(class_base);
         cls.write_child("TYPE"_id).write(output_file_path.stem());

         for (const assets::odf::property& odf_prop : definition.properties) {
            if (odf_prop.value.empty()) continue;

            ucfb::writer prop = cls.write_child("PROP"_id);

            prop.write(bf_fnv_1a_hash(odf_prop.key));
            prop.write(odf_prop.value);
         }
      }
   }
   catch (io::open_error& error) {
      throw odf_error{error.what(), odf_ec::write_io_generic_error};
   }
   catch (io::error& error) {
      throw odf_error{error.what(), odf_ec::write_io_generic_error};
   }
}

}

void execute_odf_munge(const tool_context& context) noexcept
{
   std::vector<queued_munge> munge_tasks;

   try {
      for (io::directory_iterator it = io::directory_iterator{context.source_path};
           it != it.end(); ++it) {
         const io::directory_entry& entry = *it;

         if (entry.is_file and iequals(entry.path.extension(), ".odf")) {
            if (entry.last_write_time <
                io::get_last_write_time(
                   io::compose_path(context.output_path, entry.path.stem(), ".class"))) {
               continue;
            }

            munge_tasks.emplace_back(
               entry.path,
               context.thread_pool.exec(async::task_priority::low,
                                        [input_file_path = entry.path, &context] {
                                           execute_odf_munge(input_file_path, context);
                                        }));
         }
         else if (entry.is_directory) {
            if (iequals(entry.path.stem(), "PC") or iequals(entry.path.stem(), "PS2") or
                iequals(entry.path.stem(), "XBOX")) {
               if (not iequals(entry.path.stem(), context.platform)) {
                  it.skip_directory();
               }
            }
         }
      }
   }
   catch (std::exception& e) {
      context.feedback.add_error(
         {.tool = "OdfMunge", .message = fmt::format("Unknown error occured while enumerating ODFs. Unhelpful Message: {}", e.what())});
   }

   for (std::ptrdiff_t i = std::ssize(munge_tasks) - 1; i >= 0; --i) {
      try {
         munge_tasks[i].task.get();
      }
      catch (odf_error& e) {
         context.feedback.add_error({.file = munge_tasks[i].path,
                                     .tool = "OdfMunge",
                                     .message = get_descriptive_message(e)});
      }
      catch (std::exception& e) {
         context.feedback.add_error(
            {.file = munge_tasks[i].path,
             .tool = "OdfMunge",
             .message = fmt::format("Unknown error occured while munging "
                                    "ODF. Unhelpful Message: {}",
                                    e.what())});
      }
   }
}

void execute_odf_munge(const io::path& input_file_path, const tool_context& context)
{
   context.feedback.print_output(fmt::format("Munging {}", input_file_path.filename()));

   const assets::odf::definition definition = load_definition(input_file_path);

   write_req(io::compose_path(context.output_path, input_file_path.stem(), ".class.req"),
             build_req_lists(definition));

   write_class(io::compose_path(context.output_path, input_file_path.stem(), ".class"),
               definition);
}

}