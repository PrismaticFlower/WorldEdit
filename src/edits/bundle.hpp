#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

#include <absl/container/inlined_vector.h>

namespace we::edits {

using bundle_vector =
   absl::InlinedVector<std::unique_ptr<edit<world::edit_context>>, 12>;

/// @brief Make an edit from a vector of other edits. The resulting edit will behave as though it was one edit. is_coalescable and coalesce will be called for the edits inside the bundle.
/// @param edits The edits that make up the bundle.
/// @return The edit.
auto make_bundle(bundle_vector edits) -> std::unique_ptr<edit<world::edit_context>>;

}
