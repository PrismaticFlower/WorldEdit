
#include <cassert>
#include <vector>

namespace we::container {

template<typename T, std::size_t page_size = 8192>
struct paged_stack {
   using value_type = T;
   using size_type = std::size_t;
   using reference = T&;
   using const_reference = const T&;

   paged_stack() = default;

   paged_stack(const paged_stack&) noexcept = default;

   paged_stack(paged_stack&& other) noexcept
   {
      paged_stack discard;

      discard.swap(other);
      this->swap(discard);
      *this = std::move(discard);
   }

   auto operator=(const paged_stack&) noexcept -> paged_stack& = default;

   auto operator=(paged_stack&& other) noexcept -> paged_stack&
   {
      paged_stack discard;

      discard.swap(other);
      this->swap(discard);

      return *this;
   }

   [[nodiscard]] auto top() noexcept -> T&
   {
      assert(not empty());

      return get_page(_size - 1).back();
   }

   [[nodiscard]] auto top() const noexcept -> const T&
   {
      assert(not empty());

      return get_page(_size - 1).back();
   }

   [[nodiscard]] bool empty() const noexcept
   {
      return _size == 0;
   }

   [[nodiscard]] auto size() const noexcept -> std::size_t
   {
      return _size;
   }

   auto push(const T& value) noexcept -> T&
   {
      const std::size_t new_item_index = _size++;

      std::vector<T>& page = get_page(new_item_index);

      page.push_back(value);

      return page.back();
   }

   auto push(T&& value) noexcept -> T&
   {
      const std::size_t new_item_index = _size++;

      std::vector<T>& page = get_page(new_item_index);

      page.push_back(std::move(value));

      return page.back();
   }

   template<typename... Args>
   auto emplace(Args&&... args) noexcept -> T&
   {
      const std::size_t new_item_index = _size++;

      std::vector<T>& page = get_page(new_item_index);

      page.emplace_back(std::forward<Args>(args)...);

      return page.back();
   }

   auto pop() noexcept -> T
   {
      assert(not empty());

      const std::size_t item_index = --_size;

      std::vector<T>& page = get_page(item_index);

      T value = std::move(page.back());

      page.pop_back();

      return value;
   }

   void swap(paged_stack& other) noexcept
   {
      using std::swap;

      swap(this->_size, other._size);
      swap(this->_pages, other._pages);
   }

private:
   auto get_page(const std::size_t item_index) noexcept -> std::vector<T>&
   {
      const std::size_t page_index = item_index / page_size;

      assert(page_index <= _pages.size());

      if (page_index == _pages.size()) {
         _pages.emplace_back().reserve(page_size);
      }

      return _pages[page_index];
   }

   auto get_page(const std::size_t item_index) const noexcept
      -> const std::vector<T>&
   {
      const std::size_t page_index = item_index / page_size;

      assert(page_index < _pages.size());

      return _pages[page_index];
   }

   std::size_t _size = 0;
   std::vector<std::vector<T>> _pages;
};

}