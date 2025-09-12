#include "pch.h"

#include "io/path.hpp"

using namespace std::literals;

namespace we::io::tests {

TEST_CASE("io path core tests", "[IO]")
{
   io::path path{R"(C:\What\A\Nice\Path\cat.picture)"};

   CHECK(path.parent_path() == R"(C:\What\A\Nice\Path)");
   CHECK(path.filename() == R"(cat.picture)");
   CHECK(path.stem() == R"(cat)");
   CHECK(path.extension() == R"(.picture)");

   CHECK(path.c_str() == R"(C:\What\A\Nice\Path\cat.picture)"sv);
   CHECK(path.string_view() == R"(C:\What\A\Nice\Path\cat.picture)"sv);
}

TEST_CASE("io path swap", "[IO]")
{
   io::path a{"cat.picture"};
   io::path b{"dog.picture"};

   a.swap(b);

   CHECK(a.string_view() == "dog.picture");
   CHECK(b.string_view() == "cat.picture");
}

TEST_CASE("io path stem directory", "[IO]")
{
   io::path path{R"(C:\What\A\Nice\Path\Cat)"};

   CHECK(path.stem() == "Cat");
}

TEST_CASE("io path file operator+=", "[IO]")
{
   io::path path{R"(C:\A\Nice\Long\Path\It\Goes\On\And\On)"};

   path += R"(\Over\The\Hill\And\Back\Around)";

   CHECK(path.string_view() ==
         R"(C:\A\Nice\Long\Path\It\Goes\On\And\On\Over\The\Hill\And\Back\Around)");
}

TEST_CASE("io path file operator==", "[IO]")
{
   io::path a{R"(C:\A\Lovely\Path)"};
   io::path b{R"(C:\A\Lovely\Path)"};

   CHECK(a == b);
}

TEST_CASE("io path compose_path", "[IO]")
{
   io::path path = compose_path(R"(C:\Directory)", "Other");

   CHECK(path.string_view() == R"(C:\Directory\Other)");
}

TEST_CASE("io path compose_path trailing seperator", "[IO]")
{
   io::path path = compose_path(R"(C:\Directory\)", "Other");

   CHECK(path.string_view() == R"(C:\Directory\Other)");
}

TEST_CASE("io path compose_path extension", "[IO]")
{
   io::path path = compose_path(R"(C:\Directory)", "readme", ".md");

   CHECK(path.string_view() == R"(C:\Directory\readme.md)");
}

TEST_CASE("io path compose_path extension trailing seperator", "[IO]")
{
   io::path path = compose_path(R"(C:\Directory\)", "readme", ".md");

   CHECK(path.string_view() == R"(C:\Directory\readme.md)");
}

TEST_CASE("io path make_path_with_new_extension", "[IO]")
{
   io::path path = make_path_with_new_extension(R"(C:\readme.txt)", ".md");

   CHECK(path.string_view() == R"(C:\readme.md)");
}

TEST_CASE("io path make_path_from_wide_cstring", "[IO]")
{
   CHECK(make_path_from_wide_cstring(LR"(C:\🙂.md)").string_view() == R"(C:\🙂.md)");
}

TEST_CASE("io path seperator conversion constructor", "[IO]")
{
   io::path path{R"(C:/What\A/Nice\Path/cat.picture)"};

   CHECK(path.string_view() == R"(C:\What\A\Nice\Path\cat.picture)"sv);
}

TEST_CASE("io path seperator conversion operator+=", "[IO]")
{
   io::path path{R"(C:\A\Nice\Long\Path\It\Goes\On\And\On)"};

   path += R"(\Over/The/Hill\And\Back/Around)";

   CHECK(path.string_view() ==
         R"(C:\A\Nice\Long\Path\It\Goes\On\And\On\Over\The\Hill\And\Back\Around)");
}

TEST_CASE("io wide_path tests", "[IO]")
{
   io::wide_path short_path{io::path{R"(C:\What\A\Nice\Path\cat.picture)"}};

   CHECK(short_path.c_str() == LR"(C:\What\A\Nice\Path\cat.picture)"sv);

   io::wide_path long_path{io::path{
      R"(C:\ALongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongPath\readme.md)"}};

   CHECK(long_path.c_str() ==
         LR"(C:\ALongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongLongPath\readme.md)"sv);
}

TEST_CASE("io exists tests", "[IO]")
{
   CHECK(io::exists("data/io_exists_file.txt"));
   CHECK(io::exists("data"));
   CHECK(not io::exists("data/io_missing_file.txt"));
   CHECK(not io::exists("flowers"));
}

TEST_CASE("io remove tests", "[IO]")
{
   FILE* file = nullptr;

   REQUIRE(fopen_s(&file, "temp/remove_file.txt", "wb") == 0);

   if (file) fclose(file);

   CHECK(io::remove("temp/remove_file.txt"));
   CHECK(not io::exists("temp/remove_file.txt"));
}

TEST_CASE("io create_directory and remove tests", "[IO]")
{
   CHECK(io::create_directory("temp/create_directory"));
   CHECK(io::remove("temp/create_directory"));
   CHECK(not io::exists("temp/create_directory"));
}

TEST_CASE("io create_directories and remove tests", "[IO]")
{
   CHECK(io::create_directories("temp/create_directories/hello/wow"));
   CHECK(io::remove("temp/create_directories/hello/wow"));
   CHECK(io::remove("temp/create_directories/hello"));
   CHECK(io::remove("temp/create_directories"));
   CHECK(not io::exists("temp/create_directories"));
}

// TODO: directory_iterator tests

}
