#include "writer.hpp"

namespace we::ucfb {

writer::writer(chunk_id id, io::output_file& file, const writer_options options,
               writer* parent)
   : _out{file}, _options{options}, _parent{parent}
{
   _out.write_object(id);

   _size_offset = _out.get_write_offset();

   _out.write_object(uint32{0});
}

writer::~writer()
{
   if (_options.child_trail_alignment_padding_included) {
      while (_written_bytes % 4 != 0) write('\0');
   }

   const io::file_write_offset current_write_offset = _out.get_write_offset();

   _out.set_write_offset(_size_offset);

   _out.write_object(static_cast<uint32>(_written_bytes));

   _out.set_write_offset(current_write_offset);

   if (not _options.child_trail_alignment_padding_included) {
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

   return {id, _out, _options, this};
}

}