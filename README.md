# tr03

A matching decompilation of Tomb Raider III, the Steam Windows
release, built and verified on Linux through Wine.

The goal is C and C++ source that compiles into the original machine code.
During development, a DLL replaces recovered functions in a copy of the
original executable, so they can be exercised in the game.

![Progress](docs/progress.svg)

## Sources

This is not a clean-room project. The references are:

- [TRX](https://github.com/LostArtefacts/TRX): the target's function and
  global names, addresses and types in `docs/symbols/tr3.txt`.
- `refs/tomb3-legacy`: the hand decompilation, for structures and logic.
- `refs/tr02`: reusable code and module layout.
- `refs/core`: TOMB21, Core Design's working source tree.

IDA and Ghidra provide disassembly and pseudocode for the original binary.

## Target

`tomb3.exe`, linked 1998-12-02, 32-bit PE with image base `0x400000`.
SHA-256:

```text
9ffe2fada80c0917bb14dc01c6d1624a767da8743107d1f0fcea437258977cbf
```

Game files are not distributed. Supply a copy at `orig/tomb3`; see
[`orig/README.md`](orig/README.md).

## Toolchain

Visual C++ 5.0 SP3, run under Wine. Component versions matter:

| Component | Version |
| --- | --- |
| Compiler driver (`cl.exe`) | 11.00.7022 |
| C frontend (`c1.dll`) | 11.00.7113 |
| C++ frontend (`c1xx.dll`) | 11.00.7303 |
| Optimizer (`c2.exe`) | 11.00.7303 |
| Linker (`link.exe`) | 5.10.7303 |

The original has linker version 5.10 and a VC5 SP3 resource record
(build 1668). Its compiler objects have no version records, so the
exact original compiler revision cannot be recovered from this header.
The selected compiler is VC5 SP3.

The header evidence does not establish an exact original linker build.
The versions above identify the selected build tools; their file hashes
are recorded in `config/toolchain.sha256`. The Rich IDs are decoded using
[richprint's compiler table](https://github.com/dishather/richprint/blob/master/comp_id.txt).

The build uses the DirectX 5 SDK at `C:\dx5`. The exact SDK revision
used for the original executable is not established.

The compiler installation comes from Visual Studio 97, with Service
Pack 3 (X03-50158).

The release flags in `project.toml` use `/G5 /Zp1 /W3 /O2`: Pentium
scheduling, byte-packed structures and speed optimizations. Matching
additional functions may require adjustments to these flags.

## Setup

Clone with submodules, or run `git submodule update --init`.
Requirements: Python 3.11+, `just`, Wine, Ghidra and Java 21. Running the
game headlessly also needs Xvfb, xdotool and ImageMagick.

Place the reference checkouts at the `refs/` paths above. Directories or
local symlinks work; game files, references, `.wine/` and `build/` are
ignored by Git. Initialize the prefix with
`WINEPREFIX="$PWD/.wine" wineboot -i`, and supply the compiler's `bin`,
`include` and `lib` directories under `.wine/drive_c/msvc5sp3`.
Supply DirectX 5 headers and libraries under `.wine/drive_c/dx5`. Set `GHIDRA` if the installation is outside the search paths in `project.toml`.

## Workflow

| Command | Purpose |
| --- | --- |
| `just analyze` | Analyze the original and record function sizes |
| `just manifest` | Apply names and types from TRX's symbol manifest |
| `just decompile ADDRESS` | Export Ghidra pseudocode |
| `just fn NAME` | Look up a function in the reference source |
| `just compare [NAME]` | Build and check byte parity; optionally show a function diff |
| `just progress` | Rebuild the progress chart |
| `just game` | Prepare the game with the replacement DLL |
| `just run wait:10 shot:title` | Launch under Wine and capture a screenshot |

Write recovered functions in `src/`, following the original module layout.
Record function and global addresses in `config/symbols.csv`. The DLL's
hook table is generated from the functions defined in the build; other
symbols resolve to the original executable's addresses.

The build tools are a submodule of
[decomp-tools](https://github.com/LostArtefacts/decomp-tools), shared with
other Tomb Raider decompilations. `just` lists all available commands.
