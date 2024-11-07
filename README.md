# Graphexia
![Graphexia Logo](https://github.com/GasInfinity/Graphexia/blob/main/assets/logo.png?raw=true)
---
Manipulate graphs efficiently with all the eye candy.

## Building
You will need `xmake` and a C++ compiler. Currently we depend on `sokol` and `nuklear`, as they are single header libraries you won't need to do anything. 
The supported config options are:
- `force_egl` to use `EGL` in linux instead of `GLX`
- `force_gles` to use `GLES` in linux instead of `OpenGL`
- ~~`gl` to use `OpenGL` instead of `D3D11` or `Metal`~~
- ~~`wgpu` to use `WebGPU` instead of `WebGL` on emscripten~~

It is recommended to build with a specific toolchain, the recommended one for linux is gcc:
```
xmake f --toolchain=gcc # Or clang if you want to use clang (!warning!, with the current nix flake environment clang doesn't work...)
xmake
xmake run
```

### Graphexia also runs on the web!
To compile Graphexia with emscripten, you only need to change the toolchain and platform:
```
xmake f --toolchain=emcc -p wasm
xmake
# The generated html should be on the build directory. Try to run it with your browser or emrun
```

### Help wanted
Compiling for windows will be a breeze but I don't have a MacOS system to build and test against it...

