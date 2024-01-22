
#include "string.hpp"

#include <assert.h>
#include <string.h>

namespace we {

string::string(const string& str) noexcept : string{str.data(), str.size()} {}

string::string(string&& str) noexcept
{
   this->swap(str);
}

string::string(const char* string_null_terminated) noexcept
   : string{string_null_terminated, strlen(string_null_terminated)}
{
}

string::string(const char* string, size_type size) noexcept
{
   if (size >= (sizeof(_local_buffer) - 1)) {
      _data = new char[size + 1];
      _allocated_capacity = size;
   }
   else {
      _data = &_local_buffer[0];
   }

   memcpy(_data, string, size);

   _data[size] = '\0';
   _size = size;
}

string::~string()
{
   if (not local_buffer_active()) delete[] _data;
}

auto string::operator=(const string& other) noexcept -> string&
{
   if (&other == this) return *this;

   if (local_buffer_active()) {
      if (other._size <= (sizeof(_local_buffer) - 1)) {
         _size = other._size;
      }
      else {
         _data = new char[other._size + 1];
         _size = other._size;
         _allocated_capacity = other._size;
      }
   }
   else {
      if (other._size <= this->_allocated_capacity) {
         _size = other._size;
      }
      else {
         delete[] _data;

         _data = new char[other._size + 1];
         _size = other._size;
         _allocated_capacity = other._size;
      }
   }

   memcpy(_data, other._data, other._size);

   _data[_size] = '\0';

   return *this;
}

auto string::operator=(string&& other) noexcept -> string&
{
   string discard{static_cast<string&&>(other)};

   this->swap(discard);

   return *this;
}

auto string::operator=(const char* string_null_terminated) noexcept -> string&
{
   const size_type new_size = strlen(string_null_terminated);

   if (local_buffer_active()) {
      if (new_size <= (sizeof(_local_buffer) - 1)) {
         _size = new_size;
      }
      else {
         _data = new char[new_size + 1];
         _size = new_size;
         _allocated_capacity = new_size;
      }
   }
   else {
      if (new_size <= this->_allocated_capacity) {
         _size = new_size;
      }
      else {
         delete[] _data;

         _data = new char[new_size + 1];
         _size = new_size;
         _allocated_capacity = new_size;
      }
   }

   memcpy(_data, string_null_terminated, new_size);

   _data[_size] = '\0';

   return *this;
}

auto string::begin() noexcept -> char*
{
   return _data;
}

auto string::begin() const noexcept -> const char*
{
   return _data;
}

auto string::end() noexcept -> char*
{
   return _data + _size;
}

auto string::end() const noexcept -> const char*
{
   return _data + _size;
}

auto string::cbegin() const noexcept -> const char*
{
   return begin();
}

auto string::cend() const noexcept -> const char*
{
   return end();
}

auto string::size() const noexcept -> size_type
{
   return _size;
}

auto string::max_size() const noexcept -> size_type
{
   // I mean, we allocate from the heap. What are we meant to put here?
   return static_cast<size_type>(-1) / 2;
}

void string::resize(size_type new_size)
{
   if (new_size <= _size) {
      _size = new_size;
      _data[new_size] = '\0';
   }
   else if (local_buffer_active()) {
      if (new_size < (sizeof(_local_buffer) - 1)) {
         memset(&_data[_size], '\0', new_size - _size + 1);

         _size = new_size;
      }
      else {
         char* new_data = new char[new_size + 1];

         memcpy(new_data, _data, _size);
         memset(&new_data[_size], '\0', new_size - _size + 1);

         _data = new_data;
         _size = new_size;
         _allocated_capacity = new_size;
      }
   }
   else if (_allocated_capacity >= new_size) {
      memset(&_data[_size], '\0', new_size - _size + 1);

      _size = new_size;
   }
   else {
      char* new_data = new char[new_size + 1];

      memcpy(new_data, _data, _size);
      memset(&new_data[_size], '\0', new_size - _size + 1);

      delete[] _data;

      _data = new_data;
      _size = new_size;
      _allocated_capacity = new_size;
   }
}

auto string::capacity() const noexcept -> size_type
{
   return local_buffer_active() ? sizeof(_local_buffer) - 1 : _allocated_capacity;
}

void string::reserve(size_type new_min_capacity)
{
   if (local_buffer_active()) {
      if (new_min_capacity <= sizeof(_local_buffer) - 1) {
         return;
      }

      char* new_data = new char[new_min_capacity + 1];

      memcpy(new_data, _data, _size);
      new_data[_size] = '\0';

      _data = new_data;
      _allocated_capacity = new_min_capacity;
   }
   else if (new_min_capacity > _allocated_capacity) {
      char* new_data = new char[new_min_capacity + 1];

      memcpy(new_data, _data, _size);
      new_data[_size] = '\0';

      delete[] _data;

      _data = new_data;
      _allocated_capacity = new_min_capacity;
   }
}

void string::shrink_to_fit()
{
   if (local_buffer_active()) return;
   if (_size == _allocated_capacity) return;

   string shrunk{*this};

   shrunk.swap(*this);
}

void string::clear() noexcept
{
   _data[0] = '\0';
   _size = 0;
}

bool string::empty() const noexcept
{
   return _size == 0;
}

auto string::operator[](size_type pos) const noexcept -> const char&
{
   assert(pos < _size);

   return _data[pos];
}

auto string::operator[](size_type pos) noexcept -> char&
{
   assert(pos < _size);

   return _data[pos];
}

auto string::c_str() const noexcept -> const char*
{
   return _data;
}

auto string::data() const noexcept -> const char*
{
   return _data;
}

auto string::data() noexcept -> char*
{
   return _data;
}

void string::swap(string& other) noexcept
{
   if (this->local_buffer_active() and other.local_buffer_active()) {
      char temp_buffer[sizeof(_local_buffer)] = {};

      memcpy(temp_buffer, this->_local_buffer, sizeof(_local_buffer));
      memcpy(this->_local_buffer, other._local_buffer, sizeof(_local_buffer));
      memcpy(other._local_buffer, temp_buffer, sizeof(_local_buffer));

      const size_type temp_size = this->_size;

      this->_size = other._size;
      other._size = temp_size;
   }
   else if (this->local_buffer_active()) {
      char temp_buffer[sizeof(_local_buffer)] = {};
      memcpy(temp_buffer, this->_local_buffer, sizeof(_local_buffer));
      const size_type temp_size = this->_size;

      this->_data = other._data;
      this->_size = other._size;
      this->_allocated_capacity = other._allocated_capacity;

      other._local_buffer[0] = '\0';
      other._data = &other._local_buffer[0];
      other._size = temp_size;

      memcpy(other._local_buffer, temp_buffer, sizeof(_local_buffer));
   }
   else if (other.local_buffer_active()) {
      char temp_buffer[sizeof(_local_buffer)] = {};
      memcpy(temp_buffer, other._local_buffer, sizeof(_local_buffer));
      const size_type temp_size = other._size;

      other._data = this->_data;
      other._size = this->_size;
      other._allocated_capacity = this->_allocated_capacity;

      this->_local_buffer[0] = '\0';
      this->_data = &this->_local_buffer[0];
      this->_size = temp_size;

      memcpy(this->_local_buffer, temp_buffer, sizeof(_local_buffer));
   }
   else {
      char* const temp_data = other._data;
      const size_type temp_size = other._size;
      const size_type temp_allocated_capacity = other._allocated_capacity;

      other._data = this->_data;
      other._size = this->_size;
      other._allocated_capacity = this->_allocated_capacity;

      this->_data = temp_data;
      this->_size = temp_size;
      this->_allocated_capacity = temp_allocated_capacity;
   }
}

bool string::local_buffer_active() const noexcept
{
   return _data == _local_buffer;
}

}
