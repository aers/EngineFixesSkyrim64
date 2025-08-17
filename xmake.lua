-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibsse")

-- set project
set_project("EngineFixes")
set_version("7.0.0")
set_license("MIT")

-- set defaults
set_languages("c++23")
set_warnings("allextra")

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- set policies
set_policy("package.requires_lock", true)

-- set configs
set_config("skyrim_ae", true)

set_config("commonlib_ini", true)
set_config("commonlib_toml", true)
set_config("commonlib_xbyak", true)

-- third parties
add_requires("vcpkg::safetyhook", {alias="safetyhook"})
add_requires("vcpkg::zydis", {alias="zydis"})
add_requires("vcpkg::mimalloc", {alias="mimalloc"})
-- add_requires("vcpkg::tbb", {alias="tbb"})
add_requires("vcpkg::gtl", {alias="gtl"})
add_requires("vcpkg::boost-regex", {alias="boost-regex"})

-- targets
target("EngineFixes")
    -- add dependencies to target
    add_deps("commonlibsse")
    add_packages("safetyhook", "zydis", "mimalloc", "gtl", "boost-regex")

    -- add commonlibsse plugin
    add_rules("commonlibsse.plugin", {
        name = "EngineFixes",
        author = "aers",
        description = "General bug fix SKSE64 plugin"
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")

    add_installfiles("EngineFixes.toml", {prefixdir = "SKSE/Plugins"})
    add_installfiles("EngineFixes_SNCT.ini", {prefixdir = "SKSE/Plugins"})
    add_installfiles("EngineFixes_preload.txt", {prefixdir = "SKSE/Plugins"})
