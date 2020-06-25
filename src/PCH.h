#pragma once

#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/SKSE.h"

#include "AutoTOML.hpp"

#include <tortellini.hh>

#include <tbb/concurrent_hash_map.h>
#include <tbb/scalable_allocator.h>

#include <xbyak/xbyak.h>

#include <intrin.h>

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#define DLLEXPORT __declspec(dllexport)
