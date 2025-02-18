<div>
  <img align="left" width="40px" src="https://github.com/craftablescience/MareTF/blob/mane/res/logo.png?raw=true" alt="MareTF Logo" />
  <h1>MareTF</h1>
</div>

<div>
  <a href="https://github.com/craftablescience/MareTF/blob/mane/LICENSE" target="_blank" rel="noopener noreferrer"><img src="https://img.shields.io/github/license/craftablescience/MareTF?branch=mane&label=license" alt="License" /></a>
  <a href="https://github.com/craftablescience/MareTF/actions" target="_blank" rel="noopener noreferrer"><img src="https://img.shields.io/github/actions/workflow/status/craftablescience/MareTF/build.yml?branch=mane&label=builds" alt="Workflow Status" /></a>
  <a href="https://discord.gg/ASgHFkX" target="_blank" rel="noopener noreferrer"><img src="https://img.shields.io/discord/678074864346857482?label=discord&logo=Discord&logoColor=%23FFFFFF" alt="Discord" /></a>
  <a href="https://ko-fi.com/craftablescience" target="_blank" rel="noopener noreferrer"><img src="https://img.shields.io/badge/donate-006dae?label=ko-fi&logo=ko-fi" alt="Ko-Fi" /></a>
</div>

A work in progress command-line utility to work with VTF files.

## Features

- Create Mode
  - Directly convert animated images (APNG/GIF)
  - Directly convert floating point images (EXR/HDR)
  - Kaiser mipmap filtering selected by default
  - Create non-power of two textures
  - Create console VTFs
    - Xbox 360
    - PlayStation 3
  - Supports new Strata Source VTF version
    - New formats (BC7 / BC6H)
    - New CPU compression (Deflate / Zstandard)
- Edit Mode
  - Edit existing VTFs
  - Change VTF version, format, platform, etc.
  - Recompute mipmaps, thumbnail, reflectivity vector
  - Add, overwrite, or remove resources
- Info Mode
  - Print out all VTF metadata and non-image resource data
  - Parse compiled particle sheet resource to plaintext
  - Print data as colored human-readable text or as plain KeyValues
- More to come!

<img src="https://github.com/craftablescience/MareTF/blob/mane/res/vtf_kirin_wink.png?raw=true" alt="The same green-skinned and red-haired kirin as from the MareTF logo, winking." />

## Example Usage

```shell
maretf create input.png --version 7.4 --format UNCHANGED --filter KAISER
```
- Creates a new VTF from `input.png`...
- ...with version 7.4 (supported by most Source engine games)...
- ...with the same format as the input file...
- ...and mipmaps will be computed with the Kaiser filtering algorithm.

---

```shell
maretf edit input.360.vtf -o input.vtf --set-platform PC --set-version 7.6 --recompute-mips
```
- Edits an existing VTF at `input.360.vtf`...
- ...sets the output file to `input.vtf`...
- ...sets the VTF platform to PC...
- ...changes the version to 7.6 (supported by Strata Source engine games)...
- ...and mipmaps will be recomputed with the default filtering algorithm (Kaiser).

---

```shell
maretf info input.vtf
```
- Prints the VTF's metadata to the console.

## Credits

- The kirin in the program logo was created with [pony.town](https://pony.town)'s character creator
- Dependencies:
  - [argparse](https://github.com/p-ranav/argparse)
  - [sourcepp](https://github.com/craftablescience/sourcepp)

## Full Help Text

```
Usage: maretf [--help] [--output PATH] [-y] [--no-pretty-formatting] [--version X.Y] [--format IMAGE_FORMAT] [--filter R
ESIZE_FILTER] [--flag FLAG]... [--no-automatic-transparency-flags] [--no-mips] [--no-thumbnail] [--platform PLATFORM] [-
-compression-method COMPRESSION_METHOD] [--compression-level LEVEL] [--start-frame FRAME_INDEX] [--bumpmap-scale SCALE]
[--width-resize-method RESIZE_METHOD] [--height-resize-method RESIZE_METHOD] [--set-version X.Y] [--set-format IMAGE_FOR
MAT] [--set-width WIDTH] [--set-height HEIGHT] [--edit-filter RESIZE_FILTER] [--add-flag FLAG]... [--remove-flag FLAG]..
. [--recompute-transparency-flags] [--recompute-mips] [--remove-mips] [--recompute-thumbnail] [--remove-thumbnail] [--re
compute-reflectivity] [--set-platform PLATFORM] [--set-compression-method COMPRESSION_METHOD] [--set-compression-level L
EVEL] [--set-start-frame FRAME_INDEX] [--set-bumpmap-scale SCALE] [--set-particle-sheet-resource PATH] [--remove-particl
e-sheet-resource] [--set-crc-resource CRC] [--remove-crc-resource] [--set-lod-resource U.V] [--remove-lod-resource] [--s
et-tso-resource COMBINED_FLAGS] [--remove-tso-resource] [--set-kvd-resource PATH] [--remove-kvd-resource] [--info-output
-mode VAR] MODE PATH

Positional arguments:
  MODE                                         The mode to run the program in. This determines what arguments are proces
sed. Valid options: "create", "edit", and "info". [required]
  PATH                                         The path to the input file. [required]

Optional arguments:
  -h, --help                                   shows help message and exits
  -o, --output PATH                            The path to the output file, if the current mode outputs a file.
  -y                                           Automatically say yes to any prompts.
  --no-pretty-formatting                       Disables printing ANSI color codes and emojis.

"create" mode (detailed usage):
  -v, --version                                Major and minor version, split by a period. Ignored if platform is specif
ied as anything other than PC. [nargs=0..1] [default: "7.4"]
  -f, --format                                 Output format. [nargs=0..1] [default: "DEFAULT"]
  -r, --filter                                 The resize filter used to generate mipmaps and when resizing the base tex
ture to match a power of 2 (if necessary). [nargs=0..1] [default: "KAISER"]
  --flag FLAG                                  Extra flags to add. ENVMAP, ONE_BIT_ALPHA, MULTI_BIT_ALPHA, NO_MIP, and N
O_LOD flags are applied automatically based on the VTF properties. [may be repeated]
  --no-automatic-transparency-flags            Disable adding ONE_BIT_ALPHA and MULTI_BIT_ALPHA flags by default dependi
ng on the output image format.
  --no-mips                                    Disable mipmap generation.
  --no-thumbnail                               Disable thumbnail generation.
  -p, --platform                               Set the platform (PC/console) to build for. [nargs=0..1] [default: "PC"]
  -m, --compression-method                     Set the compression method. Deflate is supported on all Strata Source gam
es for VTF v7.6. Zstd is supported on all Strata Source games for VTF v7.6 besides Portal: Revolution. LZMA is supported
 for console VTFs. [nargs=0..1] [default: "ZSTD"]
  -c, --compression-level                      Set the compression level. -1 to 9 for Deflate and LZMA, -1 to 22 for Zst
d. [nargs=0..1] [default: 6]
  --start-frame                                The start frame used in animations, counting from zero. Ignored when crea
ting console VTFs. [nargs=0..1] [default: 0]
  --bumpmap-scale                              The bumpmap scale. It can have a decimal point. [nargs=0..1] [default: 1]

  --width-resize-method                        How to resize the texture's width to match a power of 2. [nargs=0..1] [de
fault: "BIGGER"]
  --height-resize-method                       How to resize the texture's height to match a power of 2. [nargs=0..1] [d
efault: "BIGGER"]

"edit" mode (detailed usage):
  --set-version X.Y                            Set the version.
  --set-format IMAGE_FORMAT                    Set the image format. Keep in mind converting to a lossy format like DXTn
 means irreversibly losing information. Recommended to pair this with the recompute transparency flags argument.
  --set-width WIDTH                            Set the lowest mip's width. Ignores power of two resize rule.
  --set-height HEIGHT                          Set the lowest mip's height. Ignores power of two resize rule.
  --edit-filter                                Use this resize filter for all resizing operations that accept a filter p
arameter, including mipmap generation. [nargs=0..1] [default: "KAISER"]
  --add-flag FLAG                              Flags to add. ENVMAP and NO_MIP flags are ignored. [may be repeated]
  --remove-flag FLAG                           Flags to remove. ENVMAP and NO_MIP flags are ignored. [may be repeated]
  --recompute-transparency-flags               Recomputes transparency flags based on the image format.
  --recompute-mips                             Recomputes mipmaps with the specified edit resize filter.
  --remove-mips                                Remove mipmaps. If recompute mips is specified, this argument is ignored.

  --recompute-thumbnail                        Recompute the thumbnail.
  --remove-thumbnail                           Remove the thumbnail. If recompute thumbnail is specified, this argument
is ignored.
  --recompute-reflectivity                     Recompute the reflectivity vector.
  --set-platform PLATFORM                      Set the VTF platform.
  --set-compression-method COMPRESSION_METHOD  Set the compression method. Deflate is supported on all Strata Source gam
es for VTF v7.6. Zstd is supported on all Strata Source games for VTF v7.6 besides Portal: Revolution. LZMA is supported
 for console VTFs.
  --set-compression-level LEVEL                Set the compression level. -1 to 9 for Deflate and LZMA, -1 to 22 for Zst
d.
  --set-start-frame FRAME_INDEX                Set the start frame.
  --set-bumpmap-scale SCALE                    Set the bumpmap scale. It can have a decimal point.
  --set-particle-sheet-resource PATH           Set the particle sheet resource. Path should point to a valid particle sh
eet file.
  --remove-particle-sheet-resource             Remove the particle sheet resource. If set particle sheet resource is spe
cified, this argument is ignored.
  --set-crc-resource CRC                       Set the CRC resource.
  --remove-crc-resource                        Remove the CRC resource. If set CRC resource is specified, this argument
is ignored.
  --set-lod-resource U.V                       Set the LOD resource. U and V values should be separated by a period.
  --remove-lod-resource                        Remove the LOD resource. If set LOD resource is specified, this argument
is ignored.
  --set-tso-resource COMBINED_FLAGS            Set the TSO (extended flags) resource. You'll have to do the math to comb
ine the flags into one integer yourself.
  --remove-tso-resource                        Remove the TSO (extended flags) resource. If set TSO resource is specifie
d, this argument is ignored.
  --set-kvd-resource PATH                      Set the nonstandard KVD (KeyValues Data) resource. Path should point to a
 text file.
  --remove-kvd-resource                        Remove the nonstandard KVD (KeyValues Data) resource. If set KVD resource
 is specified, this argument is ignored.

"info" mode (detailed usage):
  --info-output-mode                           The mode to output information in. Can be "human" or "kv1". [nargs=0..1]
[default: "human"]

Enumerations:

IMAGE_FORMAT
 • UNCHANGED
 • DEFAULT
 • RGBA8888
 • ABGR8888
 • RGB888
 • BGR888
 • RGB565
 • I8
 • IA88
 • P8
 • A8
 • RGB888_BLUESCREEN
 • BGR888_BLUESCREEN
 • ARGB8888
 • BGRA8888
 • DXT1
 • DXT3
 • DXT5
 • BGRX8888
 • BGR565
 • BGRX5551
 • BGRA4444
 • DXT1_ONE_BIT_ALPHA
 • BGRA5551
 • UV88
 • UVWQ8888
 • RGBA16161616F
 • RGBA16161616
 • UVLX8888
 • R32F
 • RGB323232F
 • RGBA32323232F
 • RG1616F
 • RG3232F
 • RGBX8888
 • EMPTY
 • ATI2N
 • ATI1N
 • RGBA1010102
 • BGRA1010102
 • R16F
 • CONSOLE_BGRX8888_LINEAR
 • CONSOLE_RGBA8888_LINEAR
 • CONSOLE_ABGR8888_LINEAR
 • CONSOLE_ARGB8888_LINEAR
 • CONSOLE_BGRA8888_LINEAR
 • CONSOLE_RGB888_LINEAR
 • CONSOLE_BGR888_LINEAR
 • CONSOLE_BGRX5551_LINEAR
 • CONSOLE_I8_LINEAR
 • CONSOLE_RGBA16161616_LINEAR
 • CONSOLE_BGRX8888_LE
 • CONSOLE_BGRA8888_LE
 • R8
 • BC7
 • BC6H

RESIZE_FILTER
 • DEFAULT
 • BOX
 • BILINEAR
 • CUBIC_BSPLINE
 • CATMULL_ROM
 • MITCHELL
 • POINT_SAMPLE
 • KAISER

FLAG
 • POINT_SAMPLE
 • TRILINEAR
 • CLAMP_S
 • CLAMP_T
 • ANISOTROPIC
 • HINT_DXT5
 • PWL_CORRECTED
 • NORMAL
 • NO_MIP
 • NO_LOD
 • LOAD_ALL_MIPS
 • PROCEDURAL
 • ONE_BIT_ALPHA
 • MULTI_BIT_ALPHA
 • ENVMAP
 • RENDERTARGET
 • DEPTH_RENDERTARGET
 • NO_DEBUG_OVERRIDE
 • SINGLE_COPY
 • SRGB
 • DEFAULT_POOL
 • COMBINED
 • ASYNC_DOWNLOAD
 • NO_DEPTH_BUFFER
 • SKIP_INITIAL_DOWNLOAD
 • CLAMP_U
 • XBOX_PRESWIZZLED
 • VERTEX_TEXTURE
 • XBOX_CACHEABLE
 • SSBUMP
 • LOAD_MOST_MIPS
 • BORDER
 • YCOCG
 • ASYNC_SKIP_INITIAL_LOW_RES

PLATFORM
 • PC
 • PS3
 • X360

COMPRESSION_METHOD
 • DEFLATE
 • ZSTD
 • CONSOLE_LZMA

RESIZE_METHOD
 • NONE
 • BIGGER
 • SMALLER
 • NEAREST
```

<img src="https://github.com/craftablescience/MareTF/blob/mane/res/vtf_kirin_sleep.png?raw=true" alt="The same green-skinned and red-haired kirin as from the MareTF logo, sleeping." />
