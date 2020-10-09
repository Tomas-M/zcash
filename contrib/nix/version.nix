let
  inherit (import ./util) nixpkgs config zcstdenv;
  inherit (nixpkgs) lib writeScript;
  inherit (config.zcash) pname;

  versionDerivation = zcstdenv.mkDerivation {
    name = "${pname}-version";
    src = ../..;

    nativeBuildInputs = [
      nixpkgs.git
    ];

    builder = writeScript "calc-${pname}-version.sh" ''
      source "$stdenv/setup"
      cd "$src"
      git describe --tags --dirty > "$out"
    '';
  };
in
  lib.strings.fileContents versionDerivation
