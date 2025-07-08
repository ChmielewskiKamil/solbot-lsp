{
  description = "A C-based Solidity Language Server";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        appName = "solbot-lsp";

        # ----------------------------------------------------------------------
        #  Development Compiler Flags (Strict + Sanitizers)
        # ----------------------------------------------------------------------
        # These flags are used in the `devShell` for catching errors early.
        devBuildFlags = [
          "-std=c99"           # Use the C99 standard
          "-Wall"              # Enable all standard warnings
          "-Wextra"            # Enable extra warnings
          "-Wpedantic"         # Enforce strict ISO C compliance
          "-Wshadow"           # Warn when a variable shadows another
          "-Wformat=2"         # Stricter printf/scanf format checks
          "-Wconversion"       # Warn about potentially problematic type conversions
          # "-Werror"          # Uncomment to treat all warnings as errors

          # --- Memory Debugging ---
          "-g"                           # Generate debug symbols
          "-fsanitize=address"           # Enable AddressSanitizer (ASan) for memory errors
          "-fno-omit-frame-pointer"      # Improves stack traces with sanitizers
        ];

        # ----------------------------------------------------------------------
        #  Production Compiler Flags
        # ----------------------------------------------------------------------
        # These flags are for the final, optimized package build.
        # No sanitizers here unless you specifically need them for release debugging.
        prodBuildFlags = [
          "-std=c99"
          "-Wall"
          "-Wextra"
          "-Wpedantic"
          "-Werror" # Treat warnings as errors for production
          "-O2" # Enable optimizations
        ];
      in
      {
        # ----------------------------------------------------------------------
        #  Package Definition (for `nix build`)
        # ----------------------------------------------------------------------
        packages.default = pkgs.clangStdenv.mkDerivation {
          pname = appName;
          version = "0.1.0";
          src = ./.;

          # Set build flags for this package build
          NIX_CFLAGS_COMPILE = builtins.toString prodBuildFlags;

          # We use a Makefile, so we need `gnumake`
          nativeBuildInputs = [ pkgs.gnumake ];
          
          # The default phases will run `make` and `make install`
          # so we don't need to specify `buildPhase` or `installPhase`
          # as long as our Makefile has `install` target.
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
          ];

          # Set the strict compiler flags for our interactive development session
          # This is the key for a fast feedback loop.
          NIX_CFLAGS_COMPILE = builtins.toString devBuildFlags;

          # Let the shell know where to find the ASan runtime library
          LD_LIBRARY_PATH = "${pkgs.llvmPackages_latest.compiler-rt}/lib";
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
