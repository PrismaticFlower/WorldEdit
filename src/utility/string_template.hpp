#pragma once

#include <initializer_list>
#include <span>
#include <string>
#include <string_view>

namespace we::string {

struct template_string_var {
   std::string_view key;
   std::string_view value;
};

/// @brief Resolve a string template. Effectively basic environment variable expansion.
///
/// "%var%" with a template_string_var of {.key = "var", .value = "flower"} will produce "flower"
/// "%var%" with no matching template string will produce "%var%"
/// "%%" will produce '%'
/// "%unclosed" will produce "%unclosed"
/// "unopened%" will produce "unopened%"
///
/// @param str The template string to resolve.
/// @param variables The varibale to use during resolution.
/// @return The resolved string.
auto resolve_template(const std::string_view str,
                      const std::span<const template_string_var> variables) noexcept
   -> std::string;

/// @brief Resolve a string template. Effectively basic environment variable expansion.
///
/// "%var%" with a template_string_var of {.key = "var", .value = "flower"} will produce "flower"
/// "%var%" with no matching template string will produce "%var%"
/// "%%" will produce '%'
/// "%unclosed" will produce "%unclosed"
/// "unopened%" will produce "unopened%"
///
/// @param str The template string to resolve.
/// @param variables The varibale to use during resolution.
/// @return The resolved string.
auto resolve_template(const std::string_view str,
                      const std::initializer_list<template_string_var> variables) noexcept
   -> std::string;

}