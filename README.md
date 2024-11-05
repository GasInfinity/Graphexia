# Graphexia
![Graphexia Logo](https://github.com/GasInfinity/Graphexia/blob/main/assets/logo.png?raw=true)
---
Manipulate graphs efficiently with all the eye candy.

## Building
You will need `xmake` and a C++ compiler.  
The only dependency right now is `raylib` but it will be replaced by `SDL3` in the near future.
  
To build it with the default xmake toolchain just run:
```
$ xmake
$ xmake run
```

If you want to change the toolchain used (for example, you want to compile with gcc strictly or your setup cannot compile Graphexia correctly), then run:
```
xmake f --toolchain=gcc # Or clang if you want to use clang (!warning!, with the current nix flake environment clang doesn't work...)
xmake run
```
