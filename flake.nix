{
  description = "A minimal C project starter using the Clang toolchain";

  # 1. Inputs: We only need nixpkgs.
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  # 2. Outputs: We define what this flake provides.
  # You can find your system by running: nix eval --raw nixpkgs#system
  outputs = { self, nixpkgs }: let
    # We explicitly target one system to keep it minimal.
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
    appName = "solbot-lsp";

    # Get the latest LLVM/Clang package set from nixpkgs.
    # This gives us access to clang, clangd, lldb, etc.
    llvm = pkgs.llvmPackages_latest;
  in {

    # 3. Package: How to build your program using Clang.
    packages.${system}.default = pkgs.clangStdenv.mkDerivation {
      pname = appName;
      version = "0.1.0";
      src = ./.; # The source code is the current directory.

      # buildInputs are not needed here, because clangStdenv automatically
      # provides the clang compiler.

      # Build phase: A single command to compile main.c
      buildPhase = ''
        runHook preBuild
        # $CC is now the clang compiler because we are using clangStdenv
        $CC main.c -o ${appName} -g # -g flag adds debug symbols
        runHook postBuild
      '';

      # Install phase: Copy the compiled program to the output.
      installPhase = ''
        runHook preInstall
        mkdir -p $out/bin
        cp ./${appName} $out/bin/
        runHook postInstall
      '';
    };

    # 4. Development Shell: The environment you'll work in.
    devShells.${system}.default = pkgs.mkShell {
      # These packages will be available in your shell for development.
      packages = [
        # This provides the `clang` compiler, `clangd` LSP, `clang-format`, etc.
        llvm.clang-tools
        # This provides the `lldb` debugger.
        llvm.lldb
      ];
    };

    # 5. App: Makes the package runnable with `nix run`.
    apps.${system}.default = {
      type = "app";
      program = "${self.packages.${system}.default}/bin/${appName}";
    };
  };
}
