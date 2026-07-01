
#include "asset_libraries.hpp"
#include "asset_state.hpp"
#include "asset_traits.hpp"

#include "msh/flat_model.hpp"
#include "odf/definition.hpp"

#include "output_stream.hpp"

#include "async/thread_pool.hpp"

#include "io/error.hpp"
#include "io/path.hpp"

#include "os/show_in_explorer.hpp"

#include "utility/event_listener.hpp"
#include "utility/file_watcher.hpp"
#include "utility/stopwatch.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <string_view>

#include <absl/container/btree_set.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace std::literals;

namespace we::assets {

/// @brief How many times to try reloading an asset that failed loading because of a sharing violation.
constexpr int max_asset_load_attempts = 20;
constexpr std::chrono::milliseconds asset_retry_load_delay = 50ms;

namespace {

struct platform_filter {
   bool allow_pc : 1 = false;
   bool allow_ps2 : 1 = false;
   bool allow_xbox : 1 = false;
};

const absl::flat_hash_set<std::string_view> ignored_folders =
   {"_BUILD"sv, "_LVL_PC"sv, "_LVL_PS2"sv, "_LVL_PSP"sv,  "_LVL_XBOX"sv,

    ".git"sv,   ".svn"sv,    ".vscode"sv,  ".WorldEdit"sv};

const std::array common_world_category_priority_high_table = {
   category::world,
};

const std::array common_category_priority_high_table = {
   category::world,
   category::common_world,
};

const std::array sides_category_priority_high_table = {
   category::world,
   category::common_world,
   category::common,
};

const std::array project_category_priority_high_table = {
   category::world,
   category::common_world,
   category::common,
   category::project,
};

const container::enum_array<std::span<const category>, category> category_priority_high_tables =
   container::make_enum_array<std::span<const category>, category>({
      {category::world, std::span<const category>{}},
      {category::common_world, common_world_category_priority_high_table},
      {category::common, common_category_priority_high_table},
      {category::sides, sides_category_priority_high_table},
      {category::project, project_category_priority_high_table},
   });

const std::array world_category_priority_low_table = {
   category::common_world,
   category::common,
   category::sides,
   category::project,
};
const std::array common_world_category_priority_low_table = {
   category::common,
   category::sides,
   category::project,
};

const std::array common_category_priority_low_table = {
   category::project,
   category::sides,
};

const std::array sides_category_priority_low_table = {
   category::project,
};

const container::enum_array<std::span<const category>, category> category_priority_low_tables =
   container::make_enum_array<std::span<const category>, category>({
      {category::world, world_category_priority_low_table},
      {category::common_world, common_world_category_priority_low_table},
      {category::common, common_category_priority_low_table},
      {category::sides, sides_category_priority_low_table},
      {category::project, std::span<const category>{}},
   });

bool is_platform_directory_skipped(std::string_view directory, platform_filter filter)
{
   if (string::iequals(directory, "PC")) return not filter.allow_pc;
   if (string::iequals(directory, "PS2")) return not filter.allow_ps2;
   if (string::iequals(directory, "XBOX")) return not filter.allow_xbox;

   return false;
}

bool is_parent_path_skipped(std::string_view path_view, platform_filter filter) noexcept
{
   if (path_view.starts_with('\\')) path_view.remove_prefix(1);

   while (not path_view.empty()) {
      auto [directory, rest] = string::split_first_of_exclusive(path_view, "\\");

      if (ignored_folders.contains(directory)) return true;
      if (is_platform_directory_skipped(directory, filter)) return true;

      path_view = rest;
   }

   return false;
}

void show_imgui_tree(const library_tree_branch& branch)
{
   if (ImGui::TreeNode(branch.name.c_str())) {

      if (ImGui::TreeNode("Directories")) {
         for (const library_tree_branch& child : branch.directories) {
            show_imgui_tree(child);
         }

         ImGui::TreePop();
      }

      if (ImGui::TreeNode("Assets")) {
         for (const lowercase_string& asset : branch.assets) {
            ImGui::TextUnformatted(asset.c_str(), asset.c_str() + asset.size());
         }

         ImGui::TreePop();
      }

      ImGui::TreePop();
   }
}

void show_imgui_tree(const library_tree& tree)
{
   if (ImGui::TreeNode("Directories")) {
      for (const library_tree_branch& branch : tree.directories) {
         show_imgui_tree(branch);
      }

      ImGui::TreePop();
   }

   if (ImGui::TreeNode("Assets")) {
      for (const lowercase_string& asset : tree.assets) {
         ImGui::TextUnformatted(asset.c_str(), asset.c_str() + asset.size());
      }

      ImGui::TreePop();
   }
}

}

void library_tree::add(const io::path& asset_path) noexcept
{
   const std::string_view asset = asset_path.stem();
   std::string_view parent_path_view = asset_path.parent_path();

   if (parent_path_view == asset_path) {
      assets.emplace(std::lower_bound(assets.begin(), assets.end(), asset,
                                      [](const std::string_view left,
                                         const std::string_view right) {
                                         return string::iless_than(left, right);
                                      }),
                     asset);
   }

   library_tree_branch* branch = nullptr;

   for (std::string_view part : string::token_iterator{parent_path_view, '\\'}) {
      std::vector<library_tree_branch>& branches =
         branch ? branch->directories : directories;

      auto lower_it = std::lower_bound(branches.begin(), branches.end(), part,
                                       [](const library_tree_branch& left,
                                          const std::string_view right) {
                                          return string::iless_than(left.name, right);
                                       });

      if (lower_it == branches.end() or not string::iequals(lower_it->name, part)) {
         auto inserted =
            branches.insert(lower_it, library_tree_branch{.name = std::string{part}});

         branch = &(*inserted);
      }
      else {
         branch = &(*lower_it);
      }
   }

   if (branch) {
      branch->assets.emplace(std::lower_bound(branch->assets.begin(),
                                              branch->assets.end(), asset,
                                              [](const std::string_view left,
                                                 const std::string_view right) {
                                                 return string::iless_than(left, right);
                                              }),
                             asset);
   }
}

void library_tree::remove(const io::path& asset_path) noexcept
{
   const std::string_view asset = asset_path.stem();
   std::string_view parent_path_view = asset_path.parent_path();

   if (parent_path_view == asset_path) {
      auto asset_it = std::lower_bound(assets.begin(), assets.end(), asset,
                                       [](const std::string_view left,
                                          const std::string_view right) {
                                          return string::iless_than(left, right);
                                       });

      if (asset_it == assets.end()) return;

      if (string::iequals(asset, *asset_it)) assets.erase(asset_it);

      return;
   }

   library_tree_branch* branch = nullptr;
   std::size_t tree_depth = 0;

   for (std::string_view part : string::token_iterator{parent_path_view, '\\'}) {
      std::vector<library_tree_branch>& branches =
         branch ? branch->directories : directories;

      auto branch_it = std::lower_bound(branches.begin(), branches.end(), part,
                                        [](const library_tree_branch& left,
                                           const std::string_view right) {
                                           return string::iless_than(left.name, right);
                                        });

      if (branch_it == branches.end()) return;
      if (not string::iequals(branch_it->name, part)) return;

      branch = &(*branch_it);
   }

   if (branch) {
      auto asset_it =
         std::lower_bound(branch->assets.begin(), branch->assets.end(), asset,
                          [](const std::string_view left, const std::string_view right) {
                             return string::iless_than(left, right);
                          });

      if (asset_it == branch->assets.end()) return;

      branch->assets.erase(asset_it);

      // Trim empty branches.
      //
      // This code's a bit involved but we start by building a stack of
      // library_tree_branch* that make up the path to the removed asset.
      //
      // If the stack has more than 1 entry we iterate it in reverse order (so child to parents) and remove any empty children parents.
      // Stopping when we hit a non-empty child.
      //
      // Finally we handle removing the root entry in branch stack from the tree root, if needed.
      //
      // During this code we take advantage of the fact we know we have a valid path through the tree to skip some work.
      if (branch->directories.empty() and branch->assets.empty()) {
         std::vector<library_tree_branch*> branch_stack;
         branch_stack.reserve(tree_depth);

         for (std::string_view part : string::token_iterator{parent_path_view, '\\'}) {
            std::vector<library_tree_branch>& branches =
               branch_stack.empty() ? directories : branch_stack.back()->directories;

            auto branch_it =
               std::lower_bound(branches.begin(), branches.end(), part,
                                [](const library_tree_branch& left,
                                   const std::string_view right) {
                                   return string::iless_than(left.name, right);
                                });

            branch_stack.push_back(&(*branch_it));
         }

         if (branch_stack.size() > 1) {
            for (std::ptrdiff_t i = std::ssize(branch_stack) - 1; i != 0; --i) {
               library_tree_branch* child = branch_stack[i];
               library_tree_branch* parent = branch_stack[i - 1];

               if (child->directories.empty() and child->assets.empty()) {
                  parent->directories.erase(parent->directories.begin() +
                                            (child - parent->directories.data()));
               }
               else {
                  break;
               }
            }
         }

         if (branch_stack[0]->directories.empty() and branch_stack[0]->assets.empty()) {
            directories.erase(directories.begin() +
                              (branch_stack[0] - directories.data()));
         }
      }
   }
}

template<typename T>
struct library<T>::impl {
   impl(output_stream& stream, std::shared_ptr<async::thread_pool> thread_pool)
      : _output_stream{stream}, _thread_pool{std::move(thread_pool)}
   {
   }

   void add(const io::path& asset_path, uint64 last_write_time,
            const category category) noexcept
   {
      const lowercase_string name{asset_path.stem()};

      {
         std::scoped_lock lock{_assets_mutex};

         asset_category_state& asset_category_state =
            _asset_category_sets[category][name];

         asset_category_state.path = asset_path;

         if (not asset_category_state.in_use) {
            for (const assets::category priority_category :
                 category_priority_high_tables[category]) {

               const auto priority_asset_category_state_it =
                  _asset_category_sets[priority_category].find(name);

               if (priority_asset_category_state_it !=
                      _asset_category_sets[priority_category].end() and
                   priority_asset_category_state_it->second.in_use) {
                  return;
               }
            }

            asset_category_state.in_use = true;

            for (const assets::category priority_category :
                 category_priority_low_tables[category]) {
               const auto priority_asset_category_state_it =
                  _asset_category_sets[priority_category].find(name);

               if (priority_asset_category_state_it !=
                      _asset_category_sets[priority_category].end() and
                   priority_asset_category_state_it->second.in_use) {
                  priority_asset_category_state_it->second.in_use = false;
               }
            }
         }
      }

      auto new_state = make_asset_state(name, asset_path, last_write_time);

      auto [state_pair, inserted] = [&] {
         std::scoped_lock lock{_assets_mutex};

         return _assets.emplace(name, new_state);
      }();

      auto state = state_pair->second;

      if (not inserted) {
         std::scoped_lock state_lock{state->mutex, _existing_assets_mutex,
                                     _assets_tree_mutex};

         if (not state->exists) {
            _existing_assets.emplace_back(name);
            _existing_assets_sorted = false;

            _assets_tree.add(asset_path);
         }
         else if (state->path != asset_path) {
            _assets_tree.remove(state->path);
            _assets_tree.add(asset_path);
         }

         state->exists = true;
         state->load_failure = false;
         state->path = asset_path;
         state->start_load = [this, name] { enqueue_create_asset(name, false); };
         state->last_write_time.store(last_write_time, std::memory_order_relaxed);
      }
      else {
         std::scoped_lock lock{_existing_assets_mutex, _assets_tree_mutex};

         _existing_assets.emplace_back(name);
         _existing_assets_sorted = false;

         _assets_tree.add(asset_path);
      }

      if (state->ref_count.load(std::memory_order_relaxed) > 0) {
         enqueue_create_asset(name, true);
      }

      _change_event.broadcast(name);
   }

   void remove(const io::path& asset_path, const category category) noexcept
   {
      if (not is_registered(asset_path, category)) return;

      const lowercase_string name{asset_path.stem()};

      const std::shared_ptr<asset_state<T>> asset_state = [&] {
         std::scoped_lock lock{_assets_mutex};

         auto it = _assets.find(name);

         return it != _assets.end() ? it->second : nullptr;
      }();

      if (asset_state) {
         std::scoped_lock lock{_assets_mutex, _load_tasks_mutex, _existing_assets_mutex,
                               _assets_tree_mutex, asset_state->mutex};

         if (asset_state->path == asset_path) {
            asset_state->exists = false;
            asset_state->load_failure = false;
            asset_state->path = {};
            asset_state->start_load = [this] {};
            asset_state->last_write_time.store(0, std::memory_order_relaxed);

            _load_tasks.erase(name);

            std::erase_if(_existing_assets, [&](const stable_string& asset) {
               return asset == name;
            });

            _assets_tree.remove(asset_path);
         }
      }

      {
         std::unique_lock lock{_assets_mutex};

         const auto asset_category_state_it =
            _asset_category_sets[category].find(name);

         if (asset_category_state_it != _asset_category_sets[category].end()) {
            if (asset_category_state_it->second.path == asset_path) {

               const bool in_use = asset_category_state_it->second.in_use;

               _asset_category_sets[category].erase(asset_category_state_it);

               if (in_use) {
                  for (const assets::category priority_category :
                       category_priority_low_tables[category]) {
                     const auto lower_asset_category_state_it =
                        _asset_category_sets[priority_category].find(name);

                     if (lower_asset_category_state_it !=
                         _asset_category_sets[priority_category].end()) {
                        const io::path new_path =
                           lower_asset_category_state_it->second.path;

                        lock.unlock();

                        return add(new_path, io::get_last_write_time(new_path),
                                   priority_category);
                     }
                  }
               }
            }
         }
      }

      _change_event.broadcast(name);
   }

   bool is_registered(const io::path& asset_path, const category category) noexcept
   {
      std::scoped_lock lock{_assets_mutex};

      const lowercase_string name{asset_path.stem()};

      const auto it = _asset_category_sets[category].find(name);

      if (it == _asset_category_sets[category].end()) return false;

      return it->second.path == asset_path;
   }

   auto operator[](const lowercase_string& name) noexcept -> asset_ref<T>
   {
      if (name.empty()) return asset_ref{_null_asset};

      // Take a shared_lock to try and get an already existing asset state.
      {
         std::shared_lock lock{_assets_mutex};

         if (auto asset = _assets.find(name); asset != _assets.end()) {
            return asset_ref{asset->second};
         }
      }

      auto placeholder_state = make_placeholder_asset_state();

      std::lock_guard lock{_assets_mutex};

      auto [state_pair, inserted] = _assets.emplace(name, placeholder_state);

      return state_pair->second;
   }

   auto listen_for_loads(
      std::move_only_function<void(const lowercase_string& name,
                                   asset_ref<T> asset, asset_data<T> data) const>
         callback) noexcept
      -> event_listener<void(const lowercase_string&, asset_ref<T>, asset_data<T>)>
   {
      return _load_event.listen(std::move(callback));
   }

   auto listen_for_load_failures(
      std::move_only_function<void(const lowercase_string& name, asset_ref<T> asset) const> callback) noexcept
      -> event_listener<void(const lowercase_string&, asset_ref<T>)>
   {
      return _load_failed_event.listen(std::move(callback));
   }

   auto listen_for_changes(
      std::move_only_function<void(const lowercase_string& name) const> callback) noexcept
      -> event_listener<void(const lowercase_string&)>
   {
      return _change_event.listen(std::move(callback));
   }

   void update_loaded() noexcept
   {
   restart_loop:
      std::unique_lock lock{_load_tasks_mutex};

      for (auto& [task_name, task] : _load_tasks) {
         if (not task.ready()) continue;

         asset_data<T> asset_data = task.get();
         lowercase_string name = task_name;

         _load_tasks.erase(name);
         lock.unlock(); // After thise line accessing _load_tasks is no longer safe.

         asset_ref<T> asset;
         std::shared_ptr<asset_state<T>> asset_state;

         // Get the asset_state.
         {
            std::scoped_lock assets_lock{_assets_mutex};

            asset_state = _assets[name];
         }

         // Update the asset data.
         {
            std::scoped_lock asset_lock{asset_state->mutex};

            asset_state->data = asset_data;
         }

         asset = asset_ref<T>{asset_state};

         if (asset_data != nullptr) {
            _load_event.broadcast(name, asset, asset_data);
         }
         else {
            asset_state->load_failure = true;
         }

         goto restart_loop;
      }
   }

   void clear() noexcept
   {
      std::scoped_lock lock{_assets_mutex, _load_tasks_mutex, _existing_assets_mutex};

      _load_tasks.clear();
      _assets.clear();
      _asset_category_sets = {};
      _existing_assets.clear();
      _existing_assets_sorted = true;
   }

   void view_existing(
      function_ptr<void(const std::span<const stable_string> assets) noexcept> callback) noexcept
   {
      if (not _existing_assets_sorted.exchange(true)) {
         std::scoped_lock lock{_existing_assets_mutex};

         std::sort(_existing_assets.begin(), _existing_assets.end());
      }

      std::shared_lock lock{_existing_assets_mutex};

      callback(_existing_assets);
   }

   void view_tree(function_ptr<void(const library_tree& tree) noexcept> callback) noexcept
   {
      std::shared_lock lock{_assets_tree_mutex};

      callback(_assets_tree);
   }

   auto query_path(const lowercase_string& name) noexcept -> io::path
   {
      std::shared_lock lock{_assets_mutex};

      auto it = _assets.find(name);

      if (it == _assets.end()) return {};

      asset_state<T>& state = *it->second;

      std::shared_lock state_lock{state.mutex};

      return state.path;
   }

   auto query_last_write_time(const lowercase_string& name) noexcept -> uint64
   {
      std::shared_lock lock{_assets_mutex};

      auto it = _assets.find(name);

      if (it == _assets.end()) return 0;

      asset_state<T>& state = *it->second;

      return state.last_write_time.load(std::memory_order_relaxed);
   }

   auto errors() -> std::vector<error>
   {
      std::vector<error> current_errors;

      {
         std::scoped_lock lock{_errors_mutex};

         current_errors.swap(_errors);
      }

      return current_errors;
   }

   void show_imgui_child() noexcept
   {
      std::scoped_lock lock{_assets_mutex, _load_tasks_mutex, _existing_assets_mutex,
                            _assets_tree_mutex, _errors_mutex};

      ImGui::PushFont(nullptr, ImGui::GetFontSize() * 2.0f);
      ImGui::Text("This window is expensive to keep opened!");
      ImGui::PopFont();

      static std::string filter;

      if (ImGui::BeginChild("Library")) {
         if (ImGui::BeginTabBar("Library")) {
            if (ImGui::BeginTabItem("Assets")) {
               ImGui::InputText("Filter", &filter);

               if (ImGui::BeginTable("Assets", 7,
                                     ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
                                        ImGuiTableFlags_Borders |
                                        ImGuiTableFlags_SizingStretchProp)) {

                  ImGui::TableSetupColumn("Name");
                  ImGui::TableSetupColumn("Data");
                  ImGui::TableSetupColumn("Exists");
                  ImGui::TableSetupColumn("Load Failure");
                  ImGui::TableSetupColumn("Path");
                  ImGui::TableSetupColumn("Ref Count");
                  ImGui::TableSetupColumn("Last Write Time");
                  ImGui::TableHeadersRow();

                  int id = 0;

                  for (const auto& [name, state] : _assets) {
                     if (not string::icontains(name, filter)) continue;

                     std::scoped_lock state_lock{state->mutex};

                     ImGui::PushID(id++);

                     ImGui::TableNextRow();

                     ImGui::TableNextColumn();
                     ImGui::TextUnformatted(name.c_str(), name.c_str() + name.size());

                     if (ImGui::BeginItemTooltip()) {
                        ImGui::TextUnformatted(name.c_str(),
                                               name.c_str() + name.size());

                        ImGui::EndTooltip();
                     }

                     ImGui::TableNextColumn();
                     ImGui::Text(state->data.expired() ? "-" : "X");

                     ImGui::TableNextColumn();
                     ImGui::Text(state->exists ? "X" : "-");

                     ImGui::TableNextColumn();
                     ImGui::Text(state->load_failure.load(std::memory_order_relaxed)
                                    ? "X"
                                    : "-");

                     ImGui::TableNextColumn();
                     if (ImGui::TextLink(state->path.c_str())) {
                        os::try_show_in_explorer(state->path);
                     }

                     if (ImGui::IsItemHovered()) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                     }

                     if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                        ImGui::TextUnformatted(state->path.c_str(),
                                               state->path.c_str() +
                                                  state->path.string_view().size());

                        ImGui::EndTooltip();
                     }

                     ImGui::TableNextColumn();
                     ImGui::Text("%zu", state->ref_count.load(std::memory_order_relaxed));

                     ImGui::TableNextColumn();
                     ImGui::Text("%zu", state->last_write_time.load(
                                           std::memory_order_relaxed));

                     ImGui::PopID();
                  }

                  ImGui::EndTable();
               }

               ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Asset Category Sets")) {
               ImGui::InputText("Filter", &filter);

               const container::enum_array<const char*, category> category_names =
                  container::make_enum_array<const char*, category>({
                     {category::world, "World"},
                     {category::common_world, "Common World"},
                     {category::common, "Common"},
                     {category::sides, "Sides"},
                     {category::project, "Project"},
                  });

               for (const category category :
                    {category::world, category::common_world, category::common,
                     category::sides, category::project}) {
                  if (ImGui::TreeNode(category_names[category])) {
                     if (ImGui::BeginTable("Assets", 3,
                                           ImGuiTableFlags_Resizable |
                                              ImGuiTableFlags_Reorderable |
                                              ImGuiTableFlags_Borders |
                                              ImGuiTableFlags_SizingStretchProp)) {

                        ImGui::TableSetupColumn("Name");
                        ImGui::TableSetupColumn("In Use");
                        ImGui::TableSetupColumn("Path");
                        ImGui::TableHeadersRow();

                        int id = 0;

                        for (const auto& [name, state] :
                             _asset_category_sets[category]) {
                           if (not string::icontains(name, filter)) continue;

                           ImGui::PushID(id++);

                           ImGui::TableNextRow();

                           ImGui::TableNextColumn();
                           ImGui::TextUnformatted(name.c_str(),
                                                  name.c_str() + name.size());

                           if (ImGui::BeginItemTooltip()) {
                              ImGui::TextUnformatted(name.c_str(),
                                                     name.c_str() + name.size());

                              ImGui::EndTooltip();
                           }

                           ImGui::TableNextColumn();
                           ImGui::Text(state.in_use ? "X" : "-");

                           ImGui::TableNextColumn();
                           if (ImGui::TextLink(state.path.c_str())) {
                              os::try_show_in_explorer(state.path);
                           }

                           if (ImGui::IsItemHovered()) {
                              ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                           }

                           if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                              ImGui::TextUnformatted(state.path.c_str(),
                                                     state.path.c_str() +
                                                        state.path.string_view().size());

                              ImGui::EndTooltip();
                           }

                           ImGui::PopID();
                        }

                        ImGui::EndTable();
                     }

                     ImGui::TreePop();
                  }
               }

               ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Loading Assets")) {
               for (const auto& [name, state] : _load_tasks) {
                  ImGui::TextUnformatted(name.c_str(), name.c_str() + name.size());
               }

               ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Existing Assets")) {
               bool existing_assets_sorted =
                  _existing_assets_sorted.load(std::memory_order_relaxed);

               if (ImGui::Checkbox("Existing Assets Sorted", &existing_assets_sorted)) {
                  _existing_assets_sorted.store(existing_assets_sorted,
                                                std::memory_order_relaxed);
               }

               ImGui::Separator();

               for (const std::string_view name : _existing_assets) {
                  ImGui::TextUnformatted(name.data(), name.data() + name.size());
               }

               ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Assets Tree")) {
               show_imgui_tree(_assets_tree);

               ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Pending Errors")) {
               if (ImGui::BeginTable("Messages", 3,
                                     ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
                                        ImGuiTableFlags_Borders |
                                        ImGuiTableFlags_SizingStretchProp)) {

                  ImGui::TableSetupColumn("Asset", ImGuiTableColumnFlags_None, 0.125f);
                  ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_None, 0.375f);
                  ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_None, 0.5f);
                  ImGui::TableHeadersRow();

                  int id = 0;

                  for (const assets::error& error : _errors) {
                     ImGui::PushID(id++);

                     ImGui::TableNextRow();

                     ImGui::TableNextColumn();
                     ImGui::TextUnformatted(error.name.c_str(),
                                            error.name.c_str() + error.name.size());

                     if (ImGui::BeginItemTooltip()) {
                        ImGui::TextUnformatted(error.name.c_str(),
                                               error.name.c_str() + error.name.size());

                        ImGui::EndTooltip();
                     }

                     ImGui::TableNextColumn();
                     if (ImGui::TextLink(error.path.c_str())) {
                        os::try_show_in_explorer(error.path);
                     }

                     if (ImGui::IsItemHovered()) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                     }

                     if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                        ImGui::TextUnformatted(error.path.c_str(),
                                               error.path.c_str() +
                                                  error.path.string_view().size());

                        ImGui::EndTooltip();
                     }

                     ImGui::TableNextColumn();
                     ImGui::PushTextWrapPos();
                     ImGui::TextUnformatted(error.message.c_str(),
                                            error.message.c_str() +
                                               error.message.size());
                     ImGui::PopTextWrapPos();

                     if (ImGui::BeginPopupContextItem("##message_context")) {
                        if (ImGui::MenuItem("Copy")) {
                           ImGui::SetClipboardText(error.message.c_str());
                        }

                        ImGui::EndPopup();
                     }

                     ImGui::PopID();
                  }

                  ImGui::EndTable();
               }

               ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
         }
      }

      ImGui::EndChild();
   }

private:
   auto make_asset_state(const lowercase_string& name, const io::path& asset_path,
                         uint64 last_write_time) -> std::shared_ptr<asset_state<T>>
   {
      return std::make_shared<asset_state<T>>(
         std::weak_ptr<T>{}, not asset_path.empty(), asset_path,
         [this, name = name] { enqueue_create_asset(name, false); }, last_write_time);
   }

   auto make_placeholder_asset_state() -> std::shared_ptr<asset_state<T>>
   {
      return std::make_shared<asset_state<T>>(std::weak_ptr<T>{}, false, io::path{}, [] {}, 0);
   }

   void enqueue_create_asset(lowercase_string name, bool preempt_current_load) noexcept
   {
      using namespace std::literals;

      auto asset = [&] {
         std::scoped_lock lock{_assets_mutex};

         return _assets.at(name);
      }();

      // Do not try reload assets that previously failed loading.
      if (asset->load_failure) return;

      io::path asset_path = [&] {
         std::shared_lock asset_lock{asset->mutex};

         return asset->path;
      }();

      std::scoped_lock tasks_lock{_load_tasks_mutex};

      if (preempt_current_load) {
         if (auto inprogress_load = _load_tasks.find(name);
             inprogress_load != _load_tasks.end()) {
            auto& [_, load_task] = *inprogress_load;

            load_task.cancel();

            _load_tasks.erase(inprogress_load);
         }
      }
      else if (_load_tasks.contains(name)) {
         return;
      }

      _load_tasks[name] = _thread_pool->exec(
         async::task_priority::low,
         [this, asset_path = std::move(asset_path), asset, name]() -> asset_data<T> {
            try {
               for (int load_attempt = 0;; ++load_attempt) {
                  try {
                     utility::stopwatch load_timer;

                     auto asset_data =
                        std::make_shared<const T>(asset_traits<T>::load(asset_path));

                     _output_stream.write("Loaded asset '{}'\n   Time Taken: {:f}ms\n"sv,
                                          asset_path.string_view(),
                                          load_timer.elapsed_ms());

                     return asset_data;
                  }
                  catch (io::open_error& e) {
                     if (e.code() == io::open_error_code::sharing_violation) {
                        if (load_attempt >= max_asset_load_attempts) throw;

                        std::this_thread::sleep_for(asset_retry_load_delay);
                     }
                     else {
                        throw;
                     }
                  }
               }
            }
            catch (std::exception& e) {
               _output_stream.write("Error while loading asset:\n   File: {}\n   Message: \n{}\n"sv,
                                    asset_path.string_view(),
                                    string::indent(2, e.what()));

               {
                  std::scoped_lock lock{_errors_mutex};

                  _errors.push_back(
                     {.name = name, .path = asset_path, .message = e.what()});
               }

               _load_failed_event.broadcast(name, asset);

               return nullptr;
            }
         });
   }

   struct asset_category_state {
      bool in_use = false;
      io::path path;
   };

   we::output_stream& _output_stream;

   std::shared_mutex _assets_mutex;
   absl::flat_hash_map<lowercase_string, std::shared_ptr<asset_state<T>>> _assets; // guarded by _assets_mutex
   container::enum_array<absl::flat_hash_map<lowercase_string, asset_category_state>, category>
      _asset_category_sets; // guarded by _assets_mutex

   std::shared_mutex _load_tasks_mutex;
   absl::flat_hash_map<lowercase_string, async::task<asset_data<T>>> _load_tasks; // guarded by _load_tasks_mutex

   std::shared_mutex _existing_assets_mutex;
   std::vector<stable_string> _existing_assets;
   std::atomic_bool _existing_assets_sorted = true;

   std::shared_mutex _assets_tree_mutex;
   library_tree _assets_tree;

   std::shared_mutex _errors_mutex;
   std::vector<error> _errors;

   std::shared_ptr<async::thread_pool> _thread_pool;

   const std::shared_ptr<asset_state<T>> _null_asset = make_placeholder_asset_state();

   utility::event<void(const lowercase_string&, asset_ref<T>, asset_data<T>)> _load_event;
   utility::event<void(const lowercase_string&, asset_ref<T>)> _load_failed_event;
   utility::event<void(const lowercase_string&)> _change_event;
};

template<typename T>
library<T>::library(output_stream& stream, std::shared_ptr<async::thread_pool> thread_pool)
   : self{stream, std::move(thread_pool)}
{
}

template<typename T>
void library<T>::add(const io::path& asset_path, uint64 last_write_time,
                     const category category) noexcept
{
   self->add(asset_path, last_write_time, category);
}

template<typename T>
void library<T>::remove(const io::path& asset_path, const category category) noexcept
{
   self->remove(asset_path, category);
}

template<typename T>
bool library<T>::is_registered(const io::path& asset_path, const category category) noexcept
{
   return self->is_registered(asset_path, category);
}

template<typename T>
auto library<T>::operator[](const lowercase_string& name) noexcept -> asset_ref<T>
{
   return self.get()[name];
}

template<typename T>
auto library<T>::listen_for_loads(
   std::move_only_function<void(const lowercase_string& name, asset_ref<T> asset, asset_data<T> data) const>
      callback) noexcept
   -> event_listener<void(const lowercase_string&, asset_ref<T>, asset_data<T>)>
{
   return self->listen_for_loads(std::move(callback));
}

template<typename T>
auto library<T>::listen_for_load_failures(
   std::move_only_function<void(const lowercase_string& name, asset_ref<T> asset) const> callback) noexcept
   -> event_listener<void(const lowercase_string&, asset_ref<T>)>
{
   return self->listen_for_load_failures(std::move(callback));
}
template<typename T>
auto library<T>::listen_for_changes(
   std::move_only_function<void(const lowercase_string& name) const> callback) noexcept
   -> event_listener<void(const lowercase_string&)>
{
   return self->listen_for_changes(std::move(callback));
}

template<typename T>
void library<T>::update_loaded() noexcept
{
   self->update_loaded();
}

template<typename T>
void library<T>::clear() noexcept
{
   self->clear();
}

template<typename T>
void library<T>::view_existing(
   function_ptr<void(const std::span<const stable_string> assets) noexcept> callback) noexcept
{
   self->view_existing(callback);
}

template<typename T>
void library<T>::view_tree(function_ptr<void(const library_tree& tree) noexcept> callback) noexcept
{
   self->view_tree(callback);
}

template<typename T>
auto library<T>::query_path(const lowercase_string& name) noexcept -> io::path
{
   return self->query_path(name);
}

template<typename T>
auto library<T>::query_last_write_time(const lowercase_string& name) noexcept -> uint64
{
   return self->query_last_write_time(name);
}

template<typename T>
auto library<T>::errors() -> std::vector<error>
{
   return self->errors();
}

template<typename T>
void library<T>::show_imgui_child() noexcept
{
   return self->show_imgui_child();
}

template struct library<odf::definition>;
template struct library<msh::flat_model>;
template struct library<texture::texture>;
template struct library<sky::config>;

struct directory::impl {
   void add(const io::path& asset_path) noexcept
   {
      std::scoped_lock lock{_mutex};

      auto [state_pair, inserted] =
         _assets.emplace(lowercase_string{asset_path.stem()}, asset_path);

      if (not inserted) {
         state_pair->second = asset_path;
      }
      else {
         _existing_assets_sorted = false;
         _existing_assets.push_back(stable_string{state_pair->first});
      }
   }

   void remove(const io::path& asset_path) noexcept
   {
      std::scoped_lock lock{_mutex};

      lowercase_string name{asset_path.stem()};

      _assets.erase(name);

      if (auto it = std::find(_existing_assets.begin(), _existing_assets.end(),
                              std::string_view{name});
          it != _existing_assets.end()) {
      }
   }

   bool is_registered(const io::path& asset_path) noexcept
   {
      std::scoped_lock lock{_mutex};

      lowercase_string name{asset_path.stem()};

      const auto it = _assets.find(name);

      if (it == _assets.end()) return false;

      return string::iequals(it->second.string_view(), asset_path.string_view());
   }

   void clear() noexcept
   {
      std::scoped_lock lock{_mutex};

      _assets.clear();
      _existing_assets.clear();
      _existing_assets_sorted = true;
   }

   void view_existing(
      function_ptr<void(const std::span<const stable_string> assets) noexcept> callback) noexcept
   {
      std::shared_lock lock{_mutex};

      if (not _existing_assets_sorted) {
         std::sort(_existing_assets.begin(), _existing_assets.end());

         _existing_assets_sorted = true;
      }

      callback(_existing_assets);
   }

   auto query_path(const lowercase_string& name) noexcept -> io::path
   {
      std::scoped_lock lock{_mutex};

      if (auto it = _assets.find(name); it != _assets.end()) return it->second;

      return {};
   }

   void show_imgui_child() noexcept
   {
      std::scoped_lock lock{_mutex};

      ImGui::PushFont(nullptr, ImGui::GetFontSize() * 2.0f);
      ImGui::Text("This window is expensive to keep opened!");
      ImGui::PopFont();

      if (ImGui::BeginChild("Directory")) {
         if (ImGui::TreeNode("Assets")) {
            if (ImGui::BeginTable("Assets", 2,
                                  ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
                                     ImGuiTableFlags_Borders |
                                     ImGuiTableFlags_SizingStretchProp)) {

               ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 0.25f);
               ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_None, 0.75f);
               ImGui::TableHeadersRow();

               int id = 0;

               for (const auto& [name, path] : _assets) {
                  ImGui::PushID(id++);

                  ImGui::TableNextRow();

                  ImGui::TableNextColumn();
                  ImGui::TextUnformatted(name.c_str(), name.c_str() + name.size());

                  if (ImGui::BeginItemTooltip()) {
                     ImGui::TextUnformatted(name.c_str(), name.c_str() + name.size());

                     ImGui::EndTooltip();
                  }

                  ImGui::TableNextColumn();
                  if (ImGui::TextLink(path.c_str())) {
                     os::try_show_in_explorer(path);
                  }

                  if (ImGui::IsItemHovered()) {
                     ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                  }

                  if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                     ImGui::TextUnformatted(path.c_str(),
                                            path.c_str() + path.string_view().size());

                     ImGui::EndTooltip();
                  }

                  ImGui::PopID();
               }

               ImGui::EndTable();
            }

            ImGui::TreePop();
         }

         if (ImGui::TreeNode("Existing Assets")) {
            ImGui::Checkbox("Existing Assets Sorted", &_existing_assets_sorted);

            ImGui::Separator();

            for (const std::string_view name : _existing_assets) {
               ImGui::TextUnformatted(name.data(), name.data() + name.size());
            }

            ImGui::TreePop();
         }
      }

      ImGui::EndChild();
   }

private:
   std::shared_mutex _mutex;
   absl::flat_hash_map<lowercase_string, io::path> _assets; // guarded by _mutex
   std::vector<stable_string> _existing_assets;
   bool _existing_assets_sorted = true;
};

directory::directory() noexcept = default;

void directory::add(const io::path& asset_path) noexcept
{
   return self->add(asset_path);
}

void directory::remove(const io::path& asset_path) noexcept
{
   return self->remove(asset_path);
}
bool directory::is_registered(const io::path& asset_path) noexcept
{
   return self->is_registered(asset_path);
}

void directory::clear() noexcept
{
   return self->clear();
}

void directory::view_existing(
   function_ptr<void(const std::span<const stable_string> assets) noexcept> callback) noexcept
{
   return self->view_existing(callback);
}

auto directory::query_path(const lowercase_string& name) noexcept -> io::path
{
   return self->query_path(name);
}

void directory::show_imgui_child() noexcept
{
   return self->show_imgui_child();
}

libraries_manager::libraries_manager(output_stream& stream,
                                     std::shared_ptr<async::thread_pool> thread_pool) noexcept
   : odfs{stream, thread_pool},
     models{stream, thread_pool},
     textures{stream, thread_pool},
     skies{stream, thread_pool},
     _thread_pool{thread_pool}
{
}

libraries_manager::~libraries_manager() = default;

void libraries_manager::set_source_directory(const io::path& source_directory,
                                             const std::string_view world_name) noexcept
{
   clear();

   _source_directory = source_directory;

   if (not io::exists(_source_directory)) return;

   _category_relative_paths = container::make_enum_array<std::string, category>({
      {category::world, fmt::format("\\Worlds\\{}\\", world_name)},
      {category::common_world, "\\Worlds\\Common\\"},
      {category::common, "\\Common\\"},
      {category::sides, "\\Sides\\"},
      {category::project, ""},
   });

   struct asset_entry {
      io::path path;
      uint64 last_write_time = 0;
   };

   std::vector<asset_entry> path_list;
   path_list.reserve(4096);

   for (auto entry = io::directory_iterator{_source_directory};
        entry != entry.end(); ++entry) {
      const io::path& path = entry->path;

      if (entry->is_directory) {
         if (is_platform_directory_skipped(path.stem(), {.allow_pc = true}) or
             ignored_folders.contains(path.stem())) {
            entry.skip_directory();
         }

         continue;
      }

      path_list.emplace_back(path, entry->last_write_time);
   }

   using path_comparator =
      decltype([](const std::string_view l, const std::string_view r) {
         return string::iless_than(l, r);
      });

   absl::btree_set<std::string_view, path_comparator> path_set;

   for (const asset_entry& entry : path_list) {
      const io::path platform_path{
         fmt::format("{}\\{}\\{}", entry.path.parent_path(), _current_platform,
                     entry.path.filename())};

      if (not path_set.contains(platform_path.string_view())) {
         register_asset(entry.path, entry.last_write_time);
      }
   }

   _file_watcher =
      std::make_unique<utility::file_watcher>(source_directory.string_view());
   _file_changed_event = _file_watcher->listen_file_changed(
      [this](const io::path& path) { update_asset(path); });
   _file_removed_event = _file_watcher->listen_file_removed(
      [this](const io::path& path) { forget_asset(path); });
   _unknown_files_changed = _file_watcher->listen_unknown_files_changed([this]() {
      // TODO: manual scan here.
   });
}

void libraries_manager::update_loaded() noexcept
{
   odfs.update_loaded();
   models.update_loaded();
   textures.update_loaded();
   skies.update_loaded();
}

bool libraries_manager::gather_errors(std::vector<error>& out) noexcept
{
   const std::size_t start_size = out.size();

   out.append_range(odfs.errors());
   out.append_range(models.errors());
   out.append_range(textures.errors());
   out.append_range(skies.errors());

   return start_size != out.size();
}

void libraries_manager::clear() noexcept
{
   _category_relative_paths = {};

   odfs.clear();
   models.clear();
   textures.clear();
   skies.clear();
   entity_groups.clear();
}

void libraries_manager::show_imgui(bool* open) noexcept
{
   if (ImGui::Begin("Asset Libraries Debugger", open)) {
      if (ImGui::BeginTabBar("Library Tabs")) {
         if (ImGui::BeginTabItem("Manager")) {
            ImGui::LabelText("Source Directory", "%s", _source_directory.c_str());
            ImGui::LabelText("Current Platform", "%s", _current_platform.c_str());

            ImGui::SeparatorText("Category Relative Paths");

            ImGui::LabelText("World", "%s",
                             _category_relative_paths[category::world].c_str());
            ImGui::LabelText("Common World", "%s",
                             _category_relative_paths[category::common_world].c_str());
            ImGui::LabelText("Common", "%s",
                             _category_relative_paths[category::common].c_str());
            ImGui::LabelText("Sides", "%s",
                             _category_relative_paths[category::sides].c_str());
            ImGui::LabelText("Project", "%s",
                             _category_relative_paths[category::project].c_str());

            ImGui::EndTabItem();
         }

         if (ImGui::BeginTabItem("ODFs")) {
            odfs.show_imgui_child();

            ImGui::EndTabItem();
         }

         if (ImGui::BeginTabItem("Models")) {
            models.show_imgui_child();

            ImGui::EndTabItem();
         }

         if (ImGui::BeginTabItem("Textures")) {
            textures.show_imgui_child();

            ImGui::EndTabItem();
         }

         if (ImGui::BeginTabItem("Skies")) {
            skies.show_imgui_child();

            ImGui::EndTabItem();
         }

         if (ImGui::BeginTabItem("Entity Groups")) {
            entity_groups.show_imgui_child();

            ImGui::EndTabItem();
         }

         ImGui::EndTabBar();
      }
   }

   ImGui::End();
}

void libraries_manager::register_asset(const io::path& path, uint64 last_write_time) noexcept
{
   const category category = categorize(path);

   if (const auto extension = path.extension(); string::iequals(extension, ".odf"sv)) {
      odfs.add(path, last_write_time, category);
   }
   else if (string::iequals(extension, ".msh"sv)) {
      models.add(path, last_write_time, category);
   }
   else if (string::iequals(extension, ".tga"sv)) {
      textures.add(path, last_write_time, category);
   }
   else if (string::iequals(extension, ".sky"sv)) {
      skies.add(path, last_write_time, category);
   }
   else if (string::iequals(extension, ".eng"sv) or
            string::iequals(extension, ".obg"sv)) {
      entity_groups.add(path);
   }
}

void libraries_manager::forget_asset(const io::path& path) noexcept
{
   const category category = categorize(path);

   if (const auto extension = path.extension(); string::iequals(extension, ".odf"sv)) {
      odfs.remove(path, category);
   }
   else if (string::iequals(extension, ".msh"sv)) {
      models.remove(path, category);
   }
   else if (string::iequals(extension, ".tga"sv)) {
      textures.remove(path, category);
   }
   else if (string::iequals(extension, ".sky"sv)) {
      skies.remove(path, category);
   }
   else if (string::iequals(extension, ".eng"sv) or
            string::iequals(extension, ".obg"sv)) {
      entity_groups.remove(path);
   }
}

void libraries_manager::update_asset(const io::path& path) noexcept
{
   // 1. Updating registered asset with same path
   // 2. Replacing registered asset with a new platform path.
   // 3. Attempting to replace registered asset with a platform path with an updated common path. (Reject)
   // 4. New Asset

   if (is_registered_asset(path)) {
      register_asset(path, io::get_last_write_time(path));

      return;
   }

   if (string::iends_with(path.parent_path(), _current_platform)) {
      std::string_view parent_path_view = path.parent_path();

      parent_path_view.remove_suffix(_current_platform.size() + 1); // + 1 for seperator

      const io::path common_path{
         fmt::format("{}\\{}", parent_path_view, path.filename())};

      if (is_registered_asset(common_path)) {
         forget_asset(common_path);
         register_asset(path, io::get_last_write_time(path));

         return;
      }
   }

   const io::path platform_path{fmt::format("{}\\{}\\{}", path.parent_path(),
                                            _current_platform, path.filename())};

   if (io::exists(platform_path)) return;

   if (path.string_view().size() < _source_directory.string_view().size()) {
      return;
   }

   std::string_view asset_relative_path = path.string_view();
   asset_relative_path.remove_prefix(_source_directory.string_view().size());

   if (is_parent_path_skipped(asset_relative_path, {.allow_pc = true})) return;

   register_asset(path, io::get_last_write_time(path));
}

bool libraries_manager::is_registered_asset(const io::path& path) noexcept
{
   const category category = categorize(path);

   if (const auto extension = path.extension(); string::iequals(extension, ".odf"sv)) {
      return odfs.is_registered(path, category);
   }
   else if (string::iequals(extension, ".msh"sv)) {
      return models.is_registered(path, category);
   }
   else if (string::iequals(extension, ".tga"sv)) {
      return textures.is_registered(path, category);
   }
   else if (string::iequals(extension, ".sky"sv)) {
      return skies.is_registered(path, category);
   }
   else if (string::iequals(extension, ".eng"sv) or
            string::iequals(extension, ".obg"sv)) {
      return entity_groups.is_registered(path);
   }

   return false;
}

auto libraries_manager::categorize(const io::path& path) const noexcept -> category
{
   if (not string::istarts_with(path.string_view(), _source_directory.string_view())) {
      return category::project;
   }

   std::string_view asset_relative_path = path.string_view();
   asset_relative_path.remove_prefix(_source_directory.string_view().size());

   for (const category category : {category::world, category::common_world,
                                   category::common, category::sides}) {
      if (string::istarts_with(asset_relative_path, _category_relative_paths[category])) {
         return category;
      }
   }

   return category::project;
}

}
