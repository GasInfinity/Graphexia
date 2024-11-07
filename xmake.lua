---@diagnostic disable: undefined-global
add_rules("mode.debug", "mode.release")

local lib = {
    "src/graphview.cpp",
    "src/graphmatrix.cpp",
    "src/graphtypes.cpp",
    "src/algo/hakimi.cpp"
}

local app = {
    "app/sokol.c",
    "app/graphexia.cpp",
    "app/main.cpp"
}

add_requires("sokol", "nuklear")

if is_plat("linux") then
    add_requires("X11", "Xi", "Xcursor", "dl", "pthread", "m", { system = true })
    add_requires("EGL", "GL", "GLESv2", { system = true, optional = true })
end

option("wgpu", { default = false, description = "Use WebGPU instead of WebGL on emscripten" })
option("gl", { default = false, description = "Force GL backend on MacOS and Windows" })
option("force_egl", { default = false, description = "Force EGL on Linux instead of GLX" })
option("force_gles")
    add_deps("force_egl")
    set_default(false)
    set_description("Force GLES on Linux instead of OpenGL (Requires EGL)")
    after_check(function(option)
        if not option:dep("force_egl"):enabled() then
            option:enable(false)
        end
    end)

target("graphexia")
    if is_plat("wasm") then
        add_options("wgpu")
    elseif is_plat("windows") or is_plat("macos") then
        add_options("gl")
    elseif is_plat("linux") then
        add_options("force_egl", "force_gles")
    end

    set_kind("binary")
    set_languages("c99", "c++23")

    set_warnings("all", "extra")
    --set_policy("build.sanitizer.address", true)
    --set_policy("build.sanitizer.leak", true)
    --set_policy("build.sanitizer.memory", true)
    set_policy("build.sanitizer.undefined", true)

    add_includedirs("include", { public = true })

    add_files(lib)
    add_files(app)

    add_defines("NK_INCLUDE_FIXED_TYPES", "NK_INCLUDE_STANDARD_IO", "NK_INCLUDE_DEFAULT_ALLOCATOR", "NK_INCLUDE_VERTEX_BUFFER_OUTPUT", "NK_INCLUDE_FONT_BAKING", "NK_INCLUDE_DEFAULT_FONT", "NK_INCLUDE_STANDARD_VARARGS")
    add_packages("sokol", "nuklear")

    if is_plat("wasm") then
        add_ldflags("--shell-file assets/shell.html")

        if has_config("wgpu") then
            add_defines("SOKOL_WGPU")
        else
            add_ldflags("-sMAX_WEBGL_VERSION=2")
            add_defines("SOKOL_GLES3")
        end
    elseif is_plat("linux") then
        add_packages("X11", "Xi", "Xcursor", "dl", "pthread", "m")

        if has_config("force_egl") then
            add_packages("EGL")
            add_defines("SOKOL_FORCE_EGL")
        end

        if has_config("force_gles") then
            add_packages("GLESv2")
            add_defines("SOKOL_GLES3")
        else
            add_packages("GL")
            add_defines("SOKOL_GLCORE")
        end
    end
