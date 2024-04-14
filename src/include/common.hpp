/// *==============================================================*
///  common.hpp
///
///  Includes commonly used standard library headers and defines
///  some utility macros.
/// *==============================================================*
#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdint>
#include <cstdbool>
#include <cstdlib>

#include <string>
#include <iostream>
#include <memory>

#include <stack>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#define some(value) value
#define none(_) nullptr

#define min_of(a, b) (a < b ? a : b)
#define max_of(a, b) (a > b ? a : b)

#endif /* COMMON_HPP */