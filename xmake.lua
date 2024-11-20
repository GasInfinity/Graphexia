---@diagnostic disable: undefined-global
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build/", lsp = "clangd"})

local lib = {
    "lib/Graph.cpp",
    "lib/GraphMatrix.cpp",
    "lib/GraphTypes.cpp",
    "lib/Algo/Hakimi.cpp",
    "lib/Algo/Kruskal.cpp",
    "lib/Algo/BFS.cpp",
    "lib/Algo/DFS.cpp",
}

local app = {
    "src/vendor.c",
    "src/main.cpp",
    "src/GraphView.cpp",
    "src/Graphexia.cpp",
    "src/GPXRenderer.cpp",
    "src/GPXFontRenderer.cpp",
    "src/Render/StaticTextureBatch.cpp",
    "src/Util/BMFont.cpp",

    "assets/shaders/Graph.glsl",
    "assets/shaders/BMFont.glsl"
}

if is_plat("linux") then
    add_requires("X11", "Xi", "Xcursor", "dl", "pthread", "m", { system = true })
    add_requires("EGL", "GL", "GLESv2", { system = true, optional = true })
end

option("wgpu", { default = false, description = "Use WebGPU instead of WebGL on emscripten" })
option("gl", { default = false, description = "Force GL backend on MacOS and Windows" })
option("use_egl", { default = false, description = "Use EGL on Linux instead of GLX" })
option("gles")
    add_deps("use_egl")
    set_default(false)
    set_description("Force GLES on Linux instead of OpenGL (Requires EGL)")
    after_check(function(option)
        if not option:dep("use_egl"):enabled() then
            option:enable(false)
        end
    end)

rule("sokol-shdc")
    set_extensions(".glsl")
    on_load(function (target)
        local headerdir = path.join(target:autogendir(), "rules", "sokol-shdc")
        if not os.isdir(headerdir) then os.mkdir(headerdir) end
        target:add("includedirs", headerdir)
    end)
    before_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        local headerdir = path.join(target:autogendir(), "rules", "sokol-shdc")
        local headerfile = path.join(headerdir, path.filename(sourcefile) .. ".h")
        target:add("includedirs", headerdir)

        local targetSlang
        if target:is_plat("wasm") then
            if has_config("wgpu") then
                targetSlang = "wgsl"
            else
                targetSlang = "glsl300es"
            end
        elseif target:is_plat("linux") then
            if has_config("gles") then
                targetSlang = "glsl300es"
            else
                targetSlang = "glsl430"
            end
        end

        batchcmds:show_progress(opt.progress, "${color.build.object}generating.sokol-shdc %s", sourcefile)
        batchcmds:mkdir(headerdir)

        local argv = {"-i", sourcefile, "-o", headerfile, "--slang", targetSlang}
        batchcmds:vrunv("$(projectdir)/tools/sokol-shdc", argv, {envs = {XMAKE_SKIP_HISTORY = "y"}})

        batchcmds:add_depfiles(sourcefile)
        batchcmds:set_depmtime(os.mtime(headerfile))
        batchcmds:set_depcache(target:dependfile(headerfile))
    end)


rule("copy_assets")
    after_build(function (target, batchcmds)
        local runAssets = path.join(target:rundir(), "assets")
        if not os.isdir(runAssets) then os.mkdir(runAssets) end

        print("Copying assets: 'fonts/'")
        os.cp("$(projectdir)/assets/fonts", runAssets)
    end)

target("graphexia")
    if is_plat("wasm") then
        add_options("wgpu")
    elseif is_plat("windows") or is_plat("macos") then
        add_options("gl")
    elseif is_plat("linux") then
        add_options("use_egl", "gles")
    end

    set_kind("binary")
    set_languages("c99", "c++23")
    add_rules("sokol-shdc")

    set_warnings("all", "extra")
    set_policy("build.warning", true)
    --set_policy("build.sanitizer.address", true)
    --set_policy("build.sanitizer.leak", true)
    --set_policy("build.sanitizer.memory", true)
    set_policy("build.sanitizer.undefined", true)

    add_includedirs("include", { public = true })
    add_includedirs("src")
    add_includedirs("vendor/include")

    add_files(lib)
    add_files(app)

    add_defines("NK_INCLUDE_FIXED_TYPES",
                "NK_INCLUDE_STANDARD_IO",
                "NK_INCLUDE_DEFAULT_ALLOCATOR",
                "NK_INCLUDE_VERTEX_BUFFER_OUTPUT",
                "NK_INCLUDE_FONT_BAKING",
                "NK_INCLUDE_DEFAULT_FONT",
                "NK_INCLUDE_STANDARD_VARARGS")

    if is_plat("wasm") then
        add_ldflags("--shell-file ./assets/web/shell.html")
        add_ldflags("--embed-file ./assets/fonts/")
        add_ldflags("-sINITIAL_MEMORY=67108864") -- 64 MB. TODO: Make it a config option...

        if has_config("wgpu") then
            add_defines("SOKOL_WGPU")
        else
            add_ldflags("-sMAX_WEBGL_VERSION=2")
            add_defines("SOKOL_GLES3")
        end
    else
        add_rules("copy_assets")

        if is_plat("linux") then
            add_packages("X11", "Xi", "Xcursor", "dl", "pthread", "m")

            if has_config("use_egl") then
                add_packages("EGL")
                add_defines("SOKOL_FORCE_EGL")
            end

            if has_config("gles") then
                add_packages("GLESv2")
                add_defines("SOKOL_GLES3")
            else
                add_packages("GL")
                add_defines("SOKOL_GLCORE")
            end
        end
    end
