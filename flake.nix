{
  description = "A C-based Solidity Language Server";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};
        appName = "solbot-lsp";
        version = "0.0.1";

        # ----------------------------------------------------------------------
        #  Development Compiler Flags (Strict + Sanitizers)
        # ----------------------------------------------------------------------
        # These flags are used in the `devShell` for catching errors early.
        devBuildFlags = [
          "-std=c99" # Use the C99 standard
          "-Wall" # Enable all standard warnings
          "-Wextra" # Enable extra warnings
          "-Wpedantic" # Enforce strict ISO C compliance
          "-Wshadow" # Warn when a variable shadows another
          "-Wformat=2" # Stricter printf/scanf format checks
          "-Wconversion" # Warn about potentially problematic type conversions
          # "-Werror"          # Uncomment to treat all warnings as errors
          "-I." # Look for include files in the project root. E.g. you can then use libs/foundation.h anywhere
          "-Ilibs"

          # --- Memory Debugging ---
          "-g" # Generate debug symbols
          "-fsanitize=address" # Enable AddressSanitizer (ASan) for memory errors
          "-fno-omit-frame-pointer" # Improves stack traces with sanitizers
        ];

        # ----------------------------------------------------------------------
        #  Production Compiler Flags
        # ----------------------------------------------------------------------
        prodBuildFlags = [
          "-std=c99"
          "-Wall"
          "-Wextra"
          "-Wpedantic"
          "-Werror" # Treat warnings as errors for production
          "-O2" # Enable optimizations
          "-I." # Look for include files in the project root. E.g. you can then use libs/foundation.h anywhere
          "-Ilibs"
        ];
      in {
        # ----------------------------------------------------------------------
        #  Package Definition (for `nix build`)
        # ----------------------------------------------------------------------
        # Note that clangStdenv is used here, not the stdenv. This makes `clang`
        # available when `make install` is called.
        packages.default = pkgs.clangStdenv.mkDerivation {
          pname = appName;
          version = version;
          src = pkgs.lib.cleanSource ./.;
          # Set build flags for this package build
          NIX_CFLAGS_COMPILE = builtins.toString prodBuildFlags;

          # We use a Makefile, so we need `gnumake`
          nativeBuildInputs = [pkgs.gnumake];

          # The default phases will run `make` and `make install`
          # so we don't need to specify `buildPhase` or `installPhase`
          # as long as our Makefile has `install` target.
          preBuild = "make clean";
        };

        # ----------------------------------------------------------------------
        #  Development Shell (for `nix develop`)
        # ----------------------------------------------------------------------
        devShells.default = pkgs.mkShell {
          # These packages will be available in the shell
          buildInputs = with pkgs; [
            llvmPackages_latest.clang
            llvmPackages_latest.lldb
            gnumake

            # Runtime dependency for AddressSanitizer
            llvmPackages_latest.compiler-rt
            
            # Clangd needs to know the build flags used, otherwise we will have
            # missing #include errors. Bear is the tool to generate the compile_commands.json
            # file which provdies clangd with this info.
            bear 
          ];

          NIX_CFLAGS_COMPILE = builtins.toString devBuildFlags;

          # Let the shell know where to find the ASan runtime library
          LD_LIBRARY_PATH = "${pkgs.llvmPackages_latest.compiler-rt}/lib";

          # This environment variable tells clangd: "It is safe to ask 
          # the clang compiler in the Nix store where the headers are."
          # This removes the need for a .clangd config file.
          env.CLANGD_FLAGS = "--query-driver=${pkgs.lib.getExe pkgs.llvmPackages_latest.clang}";

          shellHook = ''
            # 1. Define the shortcut for later use
            alias gen-lsp="make clean && bear -- make"
            
            # 2. Run the command explicitly right now
            echo "⚙️  Auto-generating LSP config..."
            make clean && bear -- make
          '';
        };

        # ----------------------------------------------------------------------
        #  App Runner (for `nix run`)
        # ----------------------------------------------------------------------
        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/${appName}";
        };
      }
    );
}
