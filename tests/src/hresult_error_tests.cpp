#include "pch.h"

#include "hresult_error.hpp"

namespace sk::tests {

TEST_CASE("throw if failed tests", "[Error]")
{
   REQUIRE_THROWS_AS(throw_if_failed(E_FAIL), hresult_exception);
   REQUIRE_NOTHROW(throw_if_failed(S_OK));
}

}
