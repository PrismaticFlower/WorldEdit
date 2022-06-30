#include "pch.h"

#include "assets/asset_ref.hpp"
#include "assets/odf/definition.hpp"

using namespace std::literals;

namespace we::assets::tests {

TEST_CASE("asset_ref use count", "[Assets][AssetRef]")
{
   asset_ref<odf::definition> null_ref;

   REQUIRE(null_ref.use_count() == 0);

   asset_ref<odf::definition> ref{
      std::make_shared<asset_state<odf::definition>>(std::weak_ptr<odf::definition>{}, false,
                                                     L"no such file.odf", [] {})};

   REQUIRE(ref.use_count() == 1);

   asset_ref ref_copy{ref}; // copy construction

   REQUIRE(ref.use_count() == 2);
   REQUIRE(ref_copy.use_count() == 2);

   asset_ref ref_move{std::move(ref)}; // move construction

   REQUIRE(ref.use_count() == 0);
   REQUIRE(ref_copy.use_count() == 2);
   REQUIRE(ref_move.use_count() == 2);

   ref = ref_copy; // copy assignment

   REQUIRE(ref.use_count() == 3);
   REQUIRE(ref_copy.use_count() == 3);
   REQUIRE(ref_move.use_count() == 3);

   ref_move = std::move(ref_copy); // move assignment

   REQUIRE(ref.use_count() == 2);
   REQUIRE(ref_copy.use_count() == 0);
   REQUIRE(ref_move.use_count() == 2);

   ref = asset_ref<odf::definition>{}; // empty assignment
   ref_copy = asset_ref<odf::definition>{};
   ref_move = asset_ref<odf::definition>{};

   REQUIRE(ref.use_count() == 0);
   REQUIRE(ref_copy.use_count() == 0);
   REQUIRE(ref_move.use_count() == 0);
}
}
