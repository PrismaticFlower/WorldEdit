#include "output.hpp"

#include "utility/string_ops.hpp"

#include <forward_list>
#include <shared_mutex>
#include <vector>

namespace we::munge {

struct output::impl {
   void write(std::string str) noexcept
   {
      if (str.empty()) return;

      add_lines(store_string(std::move(str)));
   }

   auto view_lines() noexcept -> std::span<const std::string_view>
   {
      {
         std::unique_lock lock{_new_lines_front_mutex, std::defer_lock};

         if (lock.try_lock()) {
            std::swap(_new_lines_front, _new_lines_back);

            _lines.append_range(_new_lines_back);

            _new_lines_back.clear();
         }
      }

      return _lines;
   }

   void clear() noexcept
   {
      std::scoped_lock lock{_storage_mutex, _new_lines_front_mutex};

      _storage.clear();
      _new_lines_front.clear();
      _new_lines_back.clear();
      _lines.clear();
   }

private:
   std::shared_mutex _storage_mutex;
   std::forward_list<std::string> _storage;

   std::shared_mutex _new_lines_front_mutex;
   std::vector<std::string_view> _new_lines_front;

   std::vector<std::string_view> _new_lines_back;
   std::vector<std::string_view> _lines;

   auto store_string(std::string str) noexcept -> const std::string_view
   {
      std::scoped_lock lock{_storage_mutex};

      return _storage.emplace_front(std::move(str));
   }

   void add_lines(std::string_view str) noexcept
   {
      std::scoped_lock lock{_new_lines_front_mutex};

      for (const auto [i, line] : string::lines_iterator{str}) {
         _new_lines_front.push_back(line);
      }
   }
};

output::output() = default;

output::~output() = default;

void output::write(std::string str) noexcept
{
   return impl->write(std::move(str));
}

auto output::view_lines() noexcept -> std::span<const std::string_view>
{
   return impl->view_lines();
}

void output::clear() noexcept
{
   return impl->clear();
}

}
