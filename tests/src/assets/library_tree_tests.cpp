#include "pch.h"

#include "assets/asset_libraries.hpp"
#include "io/path.hpp"

using namespace std::literals;

namespace we::assets::tests {

TEST_CASE("assets library_tree", "[Assets]")
{
   library_tree tree;

   tree.add(R"(D:\Worlds\odf\sphere.odf)");

   {
      library_tree_branch& disk = tree.directories[0];

      REQUIRE(disk.directories.size() == 1);
      CHECK(disk.name == "D:");

      {
         library_tree_branch& worlds = disk.directories[0];

         REQUIRE(worlds.directories.size() == 1);
         CHECK(worlds.name == "Worlds");

         {
            library_tree_branch& odf = worlds.directories[0];

            REQUIRE(odf.directories.size() == 0);
            CHECK(odf.name == "odf");

            REQUIRE(odf.assets.size() == 1);
            CHECK(odf.assets[0] == "sphere");
         }
      }
   }

   tree.add(R"(D:\Worlds\odf\box.odf)");

   {
      library_tree_branch& disk = tree.directories[0];

      REQUIRE(disk.directories.size() == 1);
      CHECK(disk.name == "D:");

      {
         library_tree_branch& worlds = disk.directories[0];

         REQUIRE(worlds.directories.size() == 1);
         CHECK(worlds.name == "Worlds");

         {
            library_tree_branch& odf = worlds.directories[0];

            REQUIRE(odf.directories.size() == 0);
            CHECK(odf.name == "odf");

            REQUIRE(odf.assets.size() == 2);
            CHECK(odf.assets[0] == "box");
            CHECK(odf.assets[1] == "sphere");
         }
      }
   }

   tree.add(R"(D:\Worlds\props\lamp.odf)");

   {
      library_tree_branch& disk = tree.directories[0];

      REQUIRE(disk.directories.size() == 1);
      CHECK(disk.name == "D:");

      {
         library_tree_branch& worlds = disk.directories[0];

         REQUIRE(worlds.directories.size() == 2);
         CHECK(worlds.name == "Worlds");

         {
            library_tree_branch& odf = worlds.directories[0];

            REQUIRE(odf.directories.size() == 0);
            CHECK(odf.name == "odf");

            REQUIRE(odf.assets.size() == 2);
            CHECK(odf.assets[0] == "box");
            CHECK(odf.assets[1] == "sphere");
         }

         {
            library_tree_branch& props = worlds.directories[1];

            REQUIRE(props.directories.size() == 0);
            CHECK(props.name == "props");

            REQUIRE(props.assets.size() == 1);
            CHECK(props.assets[0] == "lamp");
         }
      }
   }

   tree.remove(R"(D:\Worlds\odf\box.odf)");

   {
      library_tree_branch& disk = tree.directories[0];

      REQUIRE(disk.directories.size() == 1);
      CHECK(disk.name == "D:");

      {
         library_tree_branch& worlds = disk.directories[0];

         REQUIRE(worlds.directories.size() == 2);
         CHECK(worlds.name == "Worlds");

         {
            library_tree_branch& odf = worlds.directories[0];

            REQUIRE(odf.directories.size() == 0);
            CHECK(odf.name == "odf");

            REQUIRE(odf.assets.size() == 1);
            CHECK(odf.assets[0] == "sphere");
         }

         {
            library_tree_branch& props = worlds.directories[1];

            REQUIRE(props.directories.size() == 0);
            CHECK(props.name == "props");

            REQUIRE(props.assets.size() == 1);
            CHECK(props.assets[0] == "lamp");
         }
      }
   }

   tree.remove(R"(D:\Worlds\props\lamp.odf)");

   {
      library_tree_branch& disk = tree.directories[0];

      REQUIRE(disk.directories.size() == 1);
      CHECK(disk.name == "D:");

      {
         library_tree_branch& worlds = disk.directories[0];

         REQUIRE(worlds.directories.size() == 1);
         CHECK(worlds.name == "Worlds");

         {
            library_tree_branch& odf = worlds.directories[0];

            REQUIRE(odf.directories.size() == 0);
            CHECK(odf.name == "odf");

            REQUIRE(odf.assets.size() == 1);
            CHECK(odf.assets[0] == "sphere");
         }
      }
   }

   tree.remove(R"(D:\Worlds\odf\sphere.odf)");

   CHECK(tree.directories.empty());
}

TEST_CASE("assets library_tree root", "[Assets]")
{
   library_tree tree;

   tree.add(R"(sphere.odf)");

   REQUIRE(tree.assets.size() == 1);
   CHECK(tree.assets[0] == "sphere");

   tree.add(R"(box.odf)");

   REQUIRE(tree.assets.size() == 2);

   CHECK(tree.assets[0] == "box");
   CHECK(tree.assets[1] == "sphere");

   tree.remove(R"(box.odf)");

   REQUIRE(tree.assets.size() == 1);

   CHECK(tree.assets[0] == "sphere");

   tree.remove(R"(sphere.odf)");

   REQUIRE(tree.assets.size() == 0);
}

}
