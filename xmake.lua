---@diagnostic disable: undefined-global
add_rules("mode.debug", "mode.release")

add_requires("raylib")

local lib = {
    "src/graph.cpp",
    "src/graphtypes.cpp",
    "src/algo/hakimi.cpp"
}

local bin = {
    "src/main.cpp"
}

target("graphexia")
    set_kind("binary")
    set_languages("c++23")

    set_warnings("all", "extra")
    --set_policy("build.sanitizer.address", true)
    --set_policy("build.sanitizer.leak", true)
    --set_policy("build.sanitizer.memory", true)
    set_policy("build.sanitizer.undefined", true)

    add_includedirs("include", { public = true })

    add_files(lib)
    add_files(bin)

    add_packages("raylib")
