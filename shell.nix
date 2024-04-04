# usage: just install 'nix' packagemanager (nixos.org) and run 'nix-shell' to start developing 
{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
    # nativeBuildInputs is usually what you want -- tools you need to run
    nativeBuildInputs = with pkgs.buildPackages; [ 
     cmake gdb SDL2 alsa-lib libjack2 lhasa perl rtmidi zlib zziplib 
    ];
 
}
