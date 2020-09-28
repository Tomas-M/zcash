# Common definitions used across Zcash derivations:
rec {
  # High-level Zcash parameters:
  zcname = "zcash";
  zcversion = "FIXME";
  zcsrc = ./../../..;

  # Frequently used libraries:
  inherit (builtins) attrNames;
  
  pkgs = import ./../pkgs-pinned.nix;
  inherit (pkgs) lib buildPlatform fetchurl stdenv;
  inherit (stdenv) mkDerivation;

  # Our own utilities:
  fnCompose = f: g: arg: f (g arg);
  importTOML = with builtins; fnCompose fromTOML readFile;

  # Useful builder script utilities:
  zcbuildutil = ./zc-build-util.sh;
}
