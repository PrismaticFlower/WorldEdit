#include "pch.h"

#include "utility/string_template.hpp"

namespace we::string {

TEST_CASE("string resolve_template", "[Utility]")
{
   CHECK(resolve_template("hello_world", {{"hello", "goodbye"}}) ==
         "hello_world");

   CHECK(resolve_template("Well, %hello%_%world%",
                          {{"hello", "goodbye"}, {"world", "bird"}}) ==
         "Well, goodbye_bird");

   CHECK(resolve_template("%hello%_%missing%!",
                          {{"hello", "goodbye"}, {"world", "bird"}}) ==
         "goodbye_%missing%!");

   CHECK(resolve_template("%hello%_%unclosed",
                          {{"hello", "goodbye"}, {"world", "bird"}}) ==
         "goodbye_%unclosed");

   CHECK(resolve_template("%hello%_unopened%",
                          {{"hello", "goodbye"}, {"world", "bird"}}) ==
         "goodbye_unopened%");

   CHECK(resolve_template("%hello%_unopened%!",
                          {{"hello", "goodbye"}, {"world", "bird"}}) ==
         "goodbye_unopened%!");

   CHECK(resolve_template("%%hello%%_%unclosed",
                          {{"hello", "goodbye"}, {"world", "bird"}}) ==
         "%hello%_%unclosed");
}

}