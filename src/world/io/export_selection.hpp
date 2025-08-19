#pragma once

#include "../interaction_context.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"

#include "assets/asset_libraries.hpp"

#include "io/path.hpp"

#include "output_stream.hpp"

#include <span>

namespace we::world {

struct export_selection_options {
   bool copy_textures = true;
   bool include_terrain = false;
};

void export_selection_to_obj(const io::path& path,
                             const export_selection_options& options,
                             const selection& selection, const world& world,
                             const object_class_library& object_classes,
                             assets::libraries_manager& assets,
                             output_stream& warning_output);

}
