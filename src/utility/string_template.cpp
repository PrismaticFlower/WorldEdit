#include "string_template.hpp"
#include "string_icompare.hpp"

#include <cassert>

namespace we::string {

auto resolve_template(const std::string_view str,
                      const std::initializer_list<template_string_var> variables) noexcept
   -> std::string
{
   std::size_t result_size = 0;

   for (std::size_t offset = 0; offset < str.size();) {
      const std::size_t var_start = str.find('%', offset);

      if (var_start == str.npos) {
         result_size += str.substr(offset).size();

         break;
      }
      else {
         result_size += str.substr(offset, var_start - offset).size();
      }

      offset = var_start + 1;

      if (offset >= str.size()) {
         result_size += 1;

         break;
      }

      const std::size_t var_end = str.find('%', offset);

      if (var_end == str.npos) {
         result_size += 1;
         result_size += str.substr(offset).size();

         break;
      }
      else if (var_end - var_start == 1) {
         result_size += 1;
      }
      else {
         const std::string_view var_name =
            str.substr(var_start + 1, var_end - var_start - 1);

         bool found = false;

         for (const template_string_var& possible_var : variables) {
            if (iequals(possible_var.key, var_name)) {
               result_size += possible_var.value.size();

               found = true;

               break;
            }
         }

         if (not found) result_size += var_name.size() + 2;
      }

      offset = var_end + 1;
   }

   std::string result;
   result.reserve(result_size);

   for (std::size_t offset = 0; offset < str.size();) {
      const std::size_t var_start = str.find('%', offset);

      if (var_start == str.npos) {
         result += str.substr(offset);

         break;
      }
      else {
         result += str.substr(offset, var_start - offset);
      }

      offset = var_start + 1;

      if (offset >= str.size()) {
         result += '%';

         break;
      }

      const std::size_t var_end = str.find('%', offset);

      if (var_end == str.npos) {
         result += '%';
         result += str.substr(offset);

         break;
      }
      else if (var_end - var_start == 1) {
         result += '%';
      }
      else {
         const std::string_view var_name =
            str.substr(var_start + 1, var_end - var_start - 1);

         bool found = false;

         for (const template_string_var& possible_var : variables) {
            if (iequals(possible_var.key, var_name)) {
               result += possible_var.value;

               found = true;

               break;
            }
         }

         if (not found) {
            result += '%';
            result += var_name;
            result += '%';
         }
      }

      offset = var_end + 1;
   }

   assert(result_size == result.size());

   return result;
}

}