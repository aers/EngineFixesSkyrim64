#pragma once

#define SKSE_CUSTOM_WINDEF

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
//#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
//#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
//#define NOUSER
//#define NONLS
//#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
//#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

#include "RE/Skyrim.h"

#include <xbyak/xbyak.h>  // must be between these two

#include "SKSE/SKSE.h"

#include "AutoTOML.hpp"

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

#include <intrin.h>

#include <boost/regex.hpp>
#include <nonstd/span.hpp>
#include <tbb/concurrent_hash_map.h>
#include <tbb/scalable_allocator.h>

#ifndef NDEBUG
#include <spdlog/sinks/msvc_sink.h>
#else
#include <spdlog/sinks/basic_file_sink.h>
#endif

using namespace std::literals;

using SKSE::util::adjust_pointer;
using SKSE::util::not_null;
using SKSE::util::unrestricted_cast;

namespace logger = SKSE::log;

namespace stl
{
    using nonstd::span;
    using SKSE::stl::report_and_fail;
}

#define DLLEXPORT __declspec(dllexport)
