#pragma once

namespace we::assets {

struct libraries_manager;

}

namespace we::edits::tests {

/// @brief Return an empty assets::libraries_manager that can be used for testing.
/// @return The assets::libraries_manager. It is the same across calls.
auto null_asset_libraries() noexcept -> assets::libraries_manager&;

}