# VSBHDASF
Sound blaster emulation for HDA (and AC97/SBLive); a fork of Japheth's VSBHDA.

This fork adds support for using soundfonts for MPU-401 emulation using TinySoundFont, and properly fixes it to make it work with games like Duke Nukem 3D.

Requirements remain the exact same as upstream VSBHDA, except it now requires potentially 128MB of XMS memory for soundfont functionality.

Requirements, supported sound cards and emulated Sound Blaster variants remain the same for this fork.

# Compiling
Only DJGPP makefiles are supported. DJGPP must be properly installed, and JWasm v2.17 (or later) must be available.

Run `make -f djgpp.mak` for normal i386 version, and `make -f djgpp-p4.mak` to create a variant optimized for SSE2. DOSLFN **must** be loaded or else it will fail.

# Usage
Same as upstream VSBHDA. However, soundfonts require setting the `SOUNDFONT` environment variable to the path to soundfont (e.g. `set SOUNDFONT=\path\to\soundfont`) before launching. Alternatively, you may put a soundfont file named `sfont.sf2` in the directory where the executable resides and launch it in the exact same directory.

The MPU-401 options from the BLASTER environment variable is respected, but can be overriden with the `/P` switch, immediately followed by the I/O address of the emulated MPU-401 without any spaces (e.g. `/P330`).
