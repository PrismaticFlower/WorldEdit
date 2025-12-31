#include "pch.h"

#include "munge/builtin/utility/bf_fnv_1a_hash.hpp"

namespace we::munge::tests {

TEST_CASE("bf_fnv_1a_hash", "[Munge]")
{
   CHECK(bf_fnv_1a_hash("hello") == 1335831723);
   CHECK(bf_fnv_1a_hash("{hEllO!}") == bf_fnv_1a_hash("[HeLLo!]"));
}

}