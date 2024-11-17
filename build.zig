const std = @import("std");
const builtin = @import("builtin");

const GraphicsBackend = enum { gles3, gl, d3d11, metal, wgpu };
const LinuxGLInterface = enum { glx, egl };

const windows_graphics_backends = &.{ GraphicsBackend.gl, GraphicsBackend.d3d11 };
const linux_graphics_backends = &.{ GraphicsBackend.gl, GraphicsBackend.gles3 };
const wasm_graphics_backends = &.{ GraphicsBackend.gles3, GraphicsBackend.wgpu };

const define_for_backend = std.EnumArray(GraphicsBackend, []const u8).init(.{ .gles3 = "SOKOL_GLES3", .gl = "SOKOL_GLCORE", .d3d11 = "SOKOL_D3D11", .metal = "SOKOL_METAL", .wgpu = "SOKOL_WGPU" });

const known_shaderlang_for_linux = std.EnumMap(GraphicsBackend, []const u8).init(.{ .gles3 = "glsl300es", .gl = "glsl430" });
const known_shaderlang_for_windows = std.EnumMap(GraphicsBackend, []const u8).init(.{ .d3d11 = "hlsl4", .gl = "glsl430" });
const known_shaderlang_for_wasm = std.EnumMap(GraphicsBackend, []const u8).init(.{ .gles3 = "glsl300es", .wgpu = "wgsl" });

const lib_sources = &[_][]const u8{
    "lib/Graph.cpp",
    "lib/GraphMatrix.cpp",
    "lib/GraphTypes.cpp",
    "lib/Algo/Hakimi.cpp",
};

const app_c_sources = &[_][]const u8{"src/vendor.c"};
const app_cxx_sources = &[_][]const u8{
    "src/main.cpp",
    "src/GraphView.cpp",
    "src/Graphexia.cpp",
    "src/GPXRenderer.cpp",
    "src/GPXFontRenderer.cpp",
    "src/Render/StaticTextureBatch.cpp",
    "src/Util/BMFont.cpp",
};

const app_shaders_dir = "assets/shaders/";
const app_shaders = &[_][]const u8{
    "Graph.glsl",
    "BMFont.glsl",
};

const lib_flags = &[_][]const u8{
    "-std=c++23",
};
const app_flags = &[_][]const u8{
    "",
};
const app_c_flags = &[_][]const u8{
    "-std=c99",
};
const app_cxx_flags = &[_][]const u8{
    "-std=c++23",
};
const app_defines = &[_][]const u8{
    "NK_INCLUDE_FIXED_TYPES",
    "NK_INCLUDE_STANDARD_IO",
    "NK_INCLUDE_DEFAULT_ALLOCATOR",
    "NK_INCLUDE_VERTEX_BUFFER_OUTPUT",
    "NK_INCLUDE_FONT_BAKING",
    "NK_INCLUDE_DEFAULT_FONT",
    "NK_INCLUDE_STANDARD_VARARGS",
};

const Options = struct {
    graphics_backend: ?GraphicsBackend = null,
    linux_gl_interface: LinuxGLInterface = LinuxGLInterface.egl,

    const defaults = Options{};

    pub fn getOptions(b: *std.Build) Options {
        return .{
            .graphics_backend = b.option(GraphicsBackend, "graphics_backend", "Graphics backend to use at runtime") orelse defaults.graphics_backend,
            .linux_gl_interface = b.option(LinuxGLInterface, "linux_gl_interface", "The linux OpenGL interface to use") orelse defaults.linux_gl_interface,
        };
    }
};

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addStaticLibrary(.{
        .name = "lgraphexia",
        .target = target,
        .optimize = optimize,
    });
    lib.addCSourceFiles(.{
        .files = lib_sources,
        .flags = lib_flags,
    });
    lib.addIncludePath(b.path("include/"));
    b.installArtifact(lib);

    const options = Options.getOptions(b);
    if (target.result.isWasm())
        try buildWeb(b, lib, target, optimize, options)
    else
        try buildNative(b, lib, target, optimize, options);
}

fn buildNative(b: *std.Build, lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, options: Options) !void {
    lib.linkLibC();
    lib.linkLibCpp();
    const app = b.addExecutable(.{
        .name = "graphexia",
        .target = target,
        .optimize = optimize,
    });
    app.linkLibC();
    app.linkLibCpp();

    switch (target.result.os.tag) {
        .linux => {
            app.linkSystemLibrary("pthread");
            app.linkSystemLibrary("dl");
            app.linkSystemLibrary("m");
            app.linkSystemLibrary("X11");
            app.linkSystemLibrary("Xi");
            app.linkSystemLibrary("Xcursor");

            switch (options.linux_gl_interface) {
                .glx => {},
                .egl => {
                    app.linkSystemLibrary("EGL");
                    app.root_module.addCMacro("SOKOL_FORCE_EGL", "");
                },
            }

            const backend = options.graphics_backend orelse GraphicsBackend.gl;
            try checkGraphicsBackend(backend, linux_graphics_backends);
            app.root_module.addCMacro(define_for_backend.get(backend), "");

            switch (backend) {
                .gl => app.linkSystemLibrary("GL"),
                .gles3 => {
                    if (options.linux_gl_interface != .egl) {
                        std.log.err("Please, enable EGL to use GLES in linux.", .{});
                        return error.LinuxEGLExpected;
                    }

                    app.linkSystemLibrary("GLESv2");
                },
                else => unreachable,
            }

            compileAddShaders(b, app, known_shaderlang_for_linux.get(backend).?);
        },
        .windows => {
            app.linkSystemLibrary("kernel32");
            app.linkSystemLibrary("user32");
            app.linkSystemLibrary("gdi32");
            app.linkSystemLibrary("ole32");

            const backend = options.graphics_backend orelse GraphicsBackend.d3d11;
            try checkGraphicsBackend(backend, windows_graphics_backends);
            app.root_module.addCMacro(define_for_backend.get(backend), "");

            if (backend == .d3d11) {
                app.linkSystemLibrary("dxgi");
                app.linkSystemLibrary("d3d11");
            }

            compileAddShaders(b, app, known_shaderlang_for_windows.get(backend).?);
        },
        else => @panic("OS not implemented, sorry!"),
    }

    const run_step = b.addRunArtifact(app);
    buildShared(b, app, lib, run_step);
}

fn buildWeb(b: *std.Build, lib: *std.Build.Step.Compile, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, options: Options) !void {
    if (target.result.os.tag != .emscripten) {
        std.log.err("Please, compile graphexia with -Dtarget=wasm32-emscripten", .{});
        return error.WasmEmscriptenExpected;
    }

    const app = b.addStaticLibrary(.{
        .name = "graphexia",
        .target = target,
        .optimize = optimize,
    });

    const emsdk = b.dependency("emsdk", .{});
    const emsdk_setup = emsdkSetupStep(b, emsdk);

    if (emsdk_setup) |setup| {
        lib.step.dependOn(&setup.step);
        app.step.dependOn(&setup.step);
    }

    const emsdk_c_includes = emsdkPath(b, emsdk, &.{ "upstream", "emscripten", "cache", "sysroot", "include" });
    const emsdk_cxx_includes = emsdkPath(b, emsdk, &.{ "upstream", "emscripten", "cache", "sysroot", "include", "c++", "v1" });
    lib.addIncludePath(emsdk_cxx_includes);
    lib.addIncludePath(emsdk_c_includes);
    app.addIncludePath(emsdk_cxx_includes);
    app.addIncludePath(emsdk_c_includes);

    const backend = options.graphics_backend orelse GraphicsBackend.gles3;

    try checkGraphicsBackend(backend, wasm_graphics_backends);
    app.root_module.addCMacro(define_for_backend.get(backend), "");
    compileAddShaders(b, app, known_shaderlang_for_wasm.get(backend).?);

    const installed_artifact = emsdkLink(b, emsdk, app, lib, optimize, options);
    const run_step = emsdkRun(b, emsdk);
    run_step.step.dependOn(&installed_artifact.step);

    buildShared(b, app, lib, run_step);
}

fn buildShared(b: *std.Build, app: *std.Build.Step.Compile, lib: *std.Build.Step.Compile, run: *std.Build.Step.Run) void {
    inline for (app_defines) |define| {
        app.root_module.addCMacro(define, "");
    }

    app.addCSourceFiles(.{
        .files = app_c_sources,
        .flags = app_c_flags ++ app_flags,
    });
    app.addCSourceFiles(.{
        .files = app_cxx_sources,
        .flags = app_cxx_flags ++ app_flags,
    });
    app.linkLibrary(lib);
    b.installArtifact(app);
    app.addIncludePath(b.path("vendor/include/"));
    app.addIncludePath(b.path("include/"));
    app.addIncludePath(b.path("src/"));

    run.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run.step);
}

fn compileAddShaders(b: *std.Build, app: *std.Build.Step.Compile, shaderlang: []const u8) void {
    app.addIncludePath(b.path(app_shaders_dir));

    inline for (app_shaders) |shader| {
        const shader_file = app_shaders_dir ++ shader;
        const run_sokolsdhc = b.addSystemCommand(&[_][]const u8{"./tools/sokol-shdc"});
        run_sokolsdhc.addArg("-l");
        run_sokolsdhc.addArg(shaderlang);
        run_sokolsdhc.addArg("-i");
        run_sokolsdhc.addFileArg(b.path(shader_file));
        run_sokolsdhc.addArg("-o");
        run_sokolsdhc.addFileArg(b.path(shader_file ++ ".h"));

        app.step.dependOn(&run_sokolsdhc.step);
    }
}

fn checkGraphicsBackend(backend: GraphicsBackend, supported: []const GraphicsBackend) !void {
    if (!std.mem.containsAtLeast(GraphicsBackend, supported, 1, &[_]GraphicsBackend{backend})) {
        std.log.err("Graphics Backend '{}' not supported for the specified target. Supported backends: {any}", .{ backend, supported });
        return error.UnsupportedBackend;
    }
}

fn emsdkRun(b: *std.Build, emsdk: *std.Build.Dependency) *std.Build.Step.Run {
    const emrun_path = b.findProgram(&.{"emrun"}, &.{}) catch emsdkPath(b, emsdk, &.{ "upstream", "emscripten", "emrun" }).getPath(b);
    const emrun = b.addSystemCommand(&.{ emrun_path, b.fmt("{s}/web/{s}.html", .{ b.install_path, "graphexia" }) });
    return emrun;
}

fn emsdkLink(b: *std.Build, emsdk: *std.Build.Dependency, app: *std.Build.Step.Compile, lib: *std.Build.Step.Compile, optimize: std.builtin.OptimizeMode, options: Options) *std.Build.Step.InstallDir {
    const emcc_path = emsdkPath(b, emsdk, &.{ "upstream", "emscripten", "em++" }).getPath(b);
    const emcc = b.addSystemCommand(&.{emcc_path});
    emcc.setName("em++");
    emcc.addArg("-sINITIAL_MEMORY=67108864"); // FIXME: Drop this
    emcc.addArgs(&.{ "--shell-file", "./assets/web/shell.html" });
    emcc.addArgs(&.{ "--embed-file", "./assets/fonts/" });

    if (optimize == .Debug) {
        emcc.addArgs(&.{ "-Og", "-sSAFE_HEAP=1", "-sSTACK_OVERFLOW_CHECK=1", "-g3" });
    } else {
        emcc.addArg("-sASSERTIONS=0");
        if (optimize == .ReleaseSmall) {
            emcc.addArg("-Oz");
        } else {
            emcc.addArg("-O3");
        }
    }

    const backend = options.graphics_backend orelse GraphicsBackend.gles3;
    switch (backend) {
        .gles3 => emcc.addArgs(&.{ "-sMIN_WEBGL_VERSION=2", "-sMAX_WEBGL_VERSION=2" }),
        .wgpu => emcc.addArg("-sUSE_WEBGPU=1"),
        else => unreachable,
    }

    emcc.addArtifactArg(lib);
    emcc.addArtifactArg(app);
    emcc.addArg("-o");
    const output = emcc.addOutputFileArg(b.fmt("{s}.html", .{"graphexia"}));

    const install = b.addInstallDirectory(.{
        .source_dir = output.dirname(),
        .install_dir = .prefix,
        .install_subdir = "web",
    });

    b.getInstallStep().dependOn(&install.step);
    return install;
}

fn emsdkStep(b: *std.Build, emsdk: *std.Build.Dependency) *std.Build.Step.Run {
    if (builtin.target.os.tag == .windows) {
        return b.addSystemCommand(&.{emsdkPath(b, emsdk, &.{"emsdk.bat"}).getPath(b)});
    } else {
        const bash = b.addSystemCommand(&.{"bash"});
        bash.addArg(emsdkPath(b, emsdk, &.{"emsdk"}).getPath(b));
        return bash;
    }
}

fn emsdkSetupStep(b: *std.Build, emsdk: *std.Build.Dependency) ?*std.Build.Step.Run {
    const emscripten_path = emsdkPath(b, emsdk, &.{".emscripten"}).getPath(b);
    const emscripten_path_exists = !std.meta.isError(std.fs.accessAbsolute(emscripten_path, .{}));

    if (!emscripten_path_exists) {
        const emsdk_install = emsdkStep(b, emsdk);
        emsdk_install.addArgs(&.{ "install", "latest" });
        const emsdk_activate = emsdkStep(b, emsdk);
        emsdk_activate.addArgs(&.{ "activate", "latest" });
        emsdk_activate.step.dependOn(&emsdk_install.step);
        return emsdk_activate;
    } else {
        return null;
    }
}

fn emsdkPath(b: *std.Build, emsdk: *std.Build.Dependency, paths: []const []const u8) std.Build.LazyPath {
    return emsdk.path(b.pathJoin(paths));
}
