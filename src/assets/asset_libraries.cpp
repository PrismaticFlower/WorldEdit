
#include "asset_libraries.hpp"
#include "utility/file_watcher.hpp"

#include <string_view>
#include <unordered_set>

using namespace std::literals;

namespace we::assets {

namespace {

const std::unordered_set ignored_folders = {L"_BUILD"sv,    L"_LVL_PC"sv,
                                            L"_LVL_PS2"sv,  L"_LVL_PSP"sv,
                                            L"_LVL_XBOX"sv,

                                            L".git"sv,      L".svn"sv,
                                            L".vscode"sv};

}

libraries_manager::libraries_manager(output_stream& stream,
                                     std::shared_ptr<async::thread_pool> thread_pool) noexcept
   : odfs{stream, thread_pool}, models{stream, thread_pool}, textures{stream, thread_pool}
{
}

libraries_manager::~libraries_manager() = default;

void libraries_manager::source_directory(const std::filesystem::path& source_directory) noexcept
{
   clear();

   for (auto entry =
           std::filesystem::recursive_directory_iterator{
              source_directory,
              std::filesystem::directory_options::follow_directory_symlink |
                 std::filesystem::directory_options::skip_permission_denied};
        entry != std::filesystem::end(entry); ++entry) {
      const auto& path = entry->path();

      if (ignored_folders.contains((--path.end())->native())) {
         entry.disable_recursion_pending();

         continue;
      }

      register_asset(path);
   }

   _file_watcher = std::make_unique<utility::file_watcher>(source_directory);
}

void libraries_manager::update_modified() noexcept
{
   if (not _file_watcher) return;

   _file_watcher->evaluate_modified_files([this](const std::filesystem::path& path) {
      // TODO: Skip path if parent path is ignored.

      register_asset(path);
   });

   if (_file_watcher->unknown_files_changed()) {
      // TODO: manual scan here
   }
}

void libraries_manager::clear() noexcept
{
   odfs.clear();
   models.clear();
   textures.clear();
}

void libraries_manager::register_asset(const std::filesystem::path& path) noexcept
{
   if (auto extension = path.extension(); extension == L".odf"sv) {
      odfs.add(path);
   }
   else if (extension == L".msh"sv) {
      models.add(path);
   }
   else if (extension == L".tga"sv) {
      textures.add(path);
   }
}

}
