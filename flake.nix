{
  description = "Graphexia nix flake";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { nixpkgs, ... }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forEachSupportedSystem = f: nixpkgs.lib.genAttrs supportedSystems (system: f {
        pkgs = import nixpkgs { inherit system; };
      });
    in
    {
      devShells = forEachSupportedSystem ({ pkgs }: {
        default = pkgs.mkShell.override {
          stdenv = pkgs.stdenv;
        } {
          buildInputs = with pkgs; [
            xorg.libX11.dev
            xorg.libXi.dev
            xorg.libXcursor.dev
            libGL
            eglexternalplatform
          ];

          nativeBuildInputs = with pkgs; [
            xmake
            emscripten
            valgrind
          ] ++ (if system == "aarch64-darwin" then [ ] else [ gdb ]);
        };
      });
    };
}
