/// *==============================================================*
///  common.hpp
///
///  Includes commonly used standard library headers and defines
///  some utility macros for optional types.
/// *==============================================================*
#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdint>
#include <cstdbool>
#include <cstdlib>

#include <string>
#include <iostream>
#include <memory>
#include <variant>

#include <vector>
#include <unordered_map>

#include <optional>
#define some(value) std::optional(value);
#define none(_)     std::nullopt;

#endif /* COMMON_HPP */