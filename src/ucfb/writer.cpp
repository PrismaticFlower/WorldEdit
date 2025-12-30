#include "writer.hpp"

#include "io/output_file.hpp"

#include <vector>

#include <Windows.h>

namespace we::ucfb {

struct writer_output {
   writer_output(const io::path& file, const writer_options options)
      : _file{CreateFileW(io::wide_path{file}.c_str(), GENERIC_WRITE, 0x0,
                          nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr)},
        options{options}
   {
   }

   ~writer_output()
   {
      for (std::unique_ptr<page>& page : _pages) {
         DWORD written_bytes = 0;

         WriteFile(_file, page.get(), _page_size, &written_bytes, nullptr);
      }

      const LARGE_INTEGER distance_to_move{.QuadPart =
                                              static_cast<int64>(_written_bytes)};
      LARGE_INTEGER new_file_pointer{.QuadPart = 0};

      SetFilePointerEx(_file, distance_to_move, &new_file_pointer, FILE_BEGIN);
      SetEndOfFile(_file);
      CloseHandle(_file);
   }

   void write(std::span<const std::byte> bytes)
   {
      if (bytes.size() < _cur_page.size()) {
         std::memcpy(_cur_page.data(), bytes.data(), bytes.size());

         _cur_page = _cur_page.subspan(bytes.size());

         _written_bytes += bytes.size();

         return;
      }

      while (not bytes.empty()) {
         if (_cur_page.empty()) {
            _cur_page = *_pages.emplace_back(std::make_unique<page>());

            continue;
         }

         const std::size_t bytes_to_write = std::min(bytes.size(), _cur_page.size());

         std::memcpy(_cur_page.data(), bytes.data(), bytes_to_write);

         _cur_page = _cur_page.subspan(bytes_to_write);
         bytes = bytes.subspan(bytes_to_write);

         _written_bytes += bytes_to_write;
      }
   }

   auto write_child(chunk_id id, writer* parent) -> writer
   {
      const uint32 zero = 0;

      write(std::as_bytes(std::span{&id, 1}));
      write(std::as_bytes(std::span{&zero, 1}));

      uint32& child_size = *new (_cur_page.data() - sizeof(uint32)) uint32{};

      return {*this, child_size, parent};
   }

   writer_output(const writer_output&) = delete;
   auto operator=(const writer_output&) -> writer_output& = delete;

   writer_output(writer_output&&) noexcept = delete;
   auto operator=(writer_output&&) noexcept -> writer_output& = delete;

   const writer_options options;

private:
   constexpr static std::size_t _page_size = 0x10000;
   using page = std::array<std::byte, _page_size>;

   std::span<std::byte> _cur_page;
   uint64 _written_bytes = 0;

   HANDLE _file;

   std::vector<std::unique_ptr<page>> _pages;
};

writer::writer(writer_output& out, uint32& size_out, writer* parent)
   : _out{out}, _size_out{size_out}, _parent{parent}
{
}

writer::~writer()
{
   if (_out.options.child_trail_alignment_padding_included) {
      while (_written_bytes % 4 != 0) write('\0');
   }

   _size_out = static_cast<uint32>(_written_bytes);

   if (not _out.options.child_trail_alignment_padding_included) {
      while (_written_bytes % 4 != 0) write('\0');
   }

   if (_parent) _parent->_written_bytes += this->_written_bytes;
}

void writer::write(const std::string_view str)
{
   write(std::as_bytes(std::span{str}));
   write('\0');
}

void writer::write(const char* str)
{
   write(std::string_view{str});
}

void writer::write(std::span<const std::byte> bytes)
{
   _written_bytes += bytes.size();

   _out.write(bytes);
}

auto writer::write_child(chunk_id id) -> writer
{
   while (_written_bytes % 4 != 0) write('\0');

   _written_bytes += 8;

   return _out.write_child(id, this);
}

writer_root::writer_root(chunk_id id, const io::path& file, const writer_options options)
   : _out{file, options}, _writer{_out->write_child(id, nullptr)}
{
}

writer_root::~writer_root() = default;

void writer_root::write(const std::string_view str)
{
   _writer.write(str);
}

void writer_root::write(const char* str)
{
   _writer.write(str);
}

void writer_root::write(std::span<const std::byte> bytes)
{
   _writer.write(bytes);
}

auto writer_root::write_child(chunk_id id) -> writer
{
   return _writer.write_child(id);
}

}