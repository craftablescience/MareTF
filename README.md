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

A utility to create, edit, and display every type of VTF file ever made.

###### *See also: [VTF Thumbnailer](https://github.com/craftablescience/vtf-thumbnailer) and [QVTF++](https://github.com/craftablescience/qvtfpp)*

![A preview of the application's main windows.](https://github.com/craftablescience/MareTF/blob/mane/res/brand/screenshot1.png?raw=true)

## Features

- Create Mode
  - Directly convert a wide range of input image formats
    - Animated images (APNG/GIF)
    - Floating point images (EXR/HDR)
    - Standard images (PNG/JPG/TGA/WebP)
    - Esoteric images (QOI/PSD/PGM/PPM/PIC/BMP)
  - Kaiser mipmap filtering selected by default
  - Create non-power of two textures
  - Create cubemaps from HDRIs
  - Create console VTFs
    - Original Xbox
    - Xbox 360
    - PlayStation 3
  - Supports new formats in Alien Swarm and beyond
  - Supports new Strata Source VTF version
    - New formats (BC7 / BC6H)
    - New CPU compression (Deflate / Zstandard)
  - Watch input file or directory for changes and recreate the VTF(s)
- Edit Mode
  - Edit existing VTFs
  - Change VTF version, format, platform, etc.
  - Recompute mipmaps, thumbnail, reflectivity vector
  - Add, overwrite, or remove resources
- Extract Mode
  - Save the image data contained within VTFs as image files
    - Save as PNG/JPEG/BMP/TGA/WebP/QOI/HDR/EXR
    - Defaults to PNG or EXR based on the image format
- Info Mode
  - Print out all VTF metadata and non-image resource data
  - Parse compiled particle sheet resource to plaintext
  - Print data as colored human-readable text or as plain KeyValues
- More to come!

<img src="https://github.com/craftablescience/MareTF/blob/mane/res/olive_shade_wink.png?raw=true" alt="The same olive-skinned and red-haired kirin as from the MareTF logo, winking." />

## Credits

- Assets:
  - The kirin in the program logo (Olive Shade) was created with [pony.town](https://pony.town)'s character creator
  - The lovely GUI splash screen art is by [@pastacrylic](https://linktr.ee/pastacrylic)
- CLI Dependencies:
  - [argparse](https://github.com/p-ranav/argparse)
  - [efsw](https://github.com/SpartanJ/efsw)
  - [indicators](https://github.com/p-ranav/indicators)
  - [sourcepp](https://github.com/craftablescience/sourcepp)
- GUI Dependencies:
  - [sourcepp](https://github.com/craftablescience/sourcepp)
  - [Qt](https://www.qt.io)

## Example CLI Usage

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

## Full CLI Help Text

```
Usage: maretf [--help] [--output PATH] [--yes] [--no] [--quiet] [--verbose] [--no-recurse]
              [--no-pretty-formatting] [--watch] [--version X.Y] [--format IMAGE_FORMAT]
              [--quality COMPRESSION_QUALITY] [--filter RESIZE_FILTER] [--flag FLAG]...
              [--no-automatic-transparency-flags] [--no-mips] [--animated-frames]
              [--no-thumbnail] [--platform PLATFORM] [--compression-method COMPRESSION_METHOD]
              [--compression-level LEVEL] [--start-frame FRAME_INDEX]
              [--bumpscale BUMPMAP_SCALE] [--invert-green] [--opengl] [--hdri]
              [--hdri-no-filter] [--resize-method RESIZE_METHOD]
              [--width-resize-method RESIZE_METHOD] [--height-resize-method RESIZE_METHOD]
              [--console-mip-scale SCALE] [--gamma-correct] [--gamma-correct-amount GAMMA]
              [--srgb] [--clamps] [--clampt] [--clampu] [--pointsample] [--trilinear]
              [--aniso] [--normal] [--ssbump] [--particle-sheet-resource PATH]
              [--crc-resource CRC] [--lod-resource U.V] [--ts0-resource COMBINED_FLAGS]
              [--kvd-resource PATH] [--hotspot-data-resource PATH]
              [--hotspot-rect X1 Y1 X2 Y2 HOTSPOT_RECT_FLAGS...]... [--set-version X.Y]
              [--set-format IMAGE_FORMAT] [--set-width WIDTH] [--set-height HEIGHT]
              [--edit-filter RESIZE_FILTER] [--add-flag FLAG]... [--remove-flag FLAG]...
              [--recompute-transparency-flags] [--recompute-mips] [--remove-mips]
              [--recompute-thumbnail] [--remove-thumbnail] [--recompute-reflectivity]
              [--set-platform PLATFORM] [--set-compression-method COMPRESSION_METHOD]
              [--set-compression-level LEVEL] [--set-start-frame FRAME_INDEX]
              [--set-bumpmap-scale SCALE] [--set-console-mip-scale SCALE]
              [--set-particle-sheet-resource PATH] [--remove-particle-sheet-resource]
              [--set-crc-resource CRC] [--remove-crc-resource] [--set-lod-resource U.V]
              [--remove-lod-resource] [--set-ts0-resource COMBINED_FLAGS]
              [--remove-ts0-resource] [--set-kvd-resource PATH] [--remove-kvd-resource]
              [--set-hotspot-data-resource PATH] [--remove-hotspot-data-resource]
              [--add-hotspot-rect X1 Y1 X2 Y2 HOTSPOT_RECT_FLAGS...]...
              [--info-output-mode VAR] [--info-skip-resources] [--extract-format FILE_FORMAT]
              [--extract-mip MIP] [--extract-all-mips] [--extract-frame FRAME]
              [--extract-all-frames] [--extract-face FACE] [--extract-all-faces]
              [--extract-slices SLICE] [--extract-all-slices] [--extract-all] MODE PATH

Positional arguments:
  MODE                                         The mode to run the program in. This
                                               determines what arguments are processed. Valid
                                               options: "create", "edit", "extract", and
                                               "info". "convert" is also permissible and is
                                               an alias of "create" for vtex2 compatibility.
                                               [required]
  PATH                                         The path to the input file or directory.
                                               [required]

Optional arguments:
  -h, --help                                   shows help message and exits
  -o, --output PATH                            The path to the output file (if the current
                                               mode outputs a file). Ignored if the input
                                               path is a directory.
  -y, --yes                                    Automatically say yes to any prompts. Enabled
                                               by default if no TTY is detected.
  --no                                         Automatically say no to any prompts. Overrides
                                               --yes.
  --quiet                                      Don't print anything to stdout or stderr
                                               (assuming program arguments are parsed
                                               successfully). Enabled by default if no TTY is
                                               detected.
  --verbose                                    Allow printing to stdout or stderr, even when
                                               no TTY is detected (assuming program arguments
                                               are parsed successfully).
  --no-recurse                                 If the input path is a directory, do not enter
                                               subdirectories when scanning for files.
  --no-pretty-formatting                       Disables printing ANSI color codes and emojis.
                                               Pretty formatting is disabled by default if no
                                               TTY is detected.

"create" mode (detailed usage):
  --watch                                      After creation is complete, watch the input
                                               file or directory for any changes and re-TF
                                               the VTF(s). --no is implied on the first
                                               creation pass. --yes is implied after the
                                               first creation pass.
  -v, --version                                Major and minor version, split by a period.
                                               Ignored if platform is specified as anything
                                               other than PC. Note that older branches of the
                                               Source engine will not load VTF versions made
                                               for newer branches. VTF v7.6 is only loadable
                                               by games running on Strata Source.
                                               [nargs=0..1] [default: "7.4"]
  -f, --format                                 Output format. [nargs=0..1]
                                               [default: "DEFAULT"]
  -q, --quality                                The quality of DXTn/BCn format compression,
                                               between 0.0 and 1.0. Higher quality will take
                                               significantly longer to create the texture. If
                                               quality is below 0.0, default compression
                                               values will be used (0.1 for BC7, BC6H, and
                                               1.0 for all others). Ignored if output format
                                               is uncompressed. [nargs=0..1] [default: -1]
  -r, --filter                                 The resize filter used to generate mipmaps and
                                               when resizing the base texture to match a
                                               power of 2 (if necessary). [nargs=0..1]
                                               [default: "KAISER"]
  --flag FLAG                                  Extra flags to add. ENVMAP, ONE_BIT_ALPHA,
                                               MULTI_BIT_ALPHA, and NO_MIP flags are applied
                                               automatically based on the VTF properties.
                                               [may be repeated]
  --no-automatic-transparency-flags            Disable adding ONE_BIT_ALPHA and
                                               MULTI_BIT_ALPHA flags by default depending on
                                               the output image format.
  --no-mips                                    Disable mipmap generation.
  -a, --animated-frames                        If input texture filename ends in two or more
                                               numbers, check for consecutive numbered files
                                               and add as animation frames if found.
  --no-thumbnail                               Disable thumbnail generation.
  -p, --platform                               Set the platform (PC/console) to build for.
                                               [nargs=0..1] [default: "PC"]
  -m, --compression-method                     Set the compression method. Deflate is
                                               supported on all Strata Source games for VTF
                                               v7.6. Zstd is supported on all Strata Source
                                               games for VTF v7.6 besides Portal: Revolution.
                                               LZMA is supported for console VTFs.
                                               [nargs=0..1] [default: "ZSTD"]
  -c, --compression-level                      Set the compression level. -1 to 9 for Deflate
                                               and LZMA, -1 to 22 for Zstd. [nargs=0..1]
                                               [default: 6]
  --start-frame                                The start frame used in animations, counting
                                               from zero. Ignored when creating console VTFs.
                                               [nargs=0..1] [default: 0]
  --bumpscale                                  The bumpmap scale. It can have a decimal
                                               point. [nargs=0..1] [default: 1]
  --invert-green                               Invert the green channel of the input image.
                                               This converts OpenGL normal maps into DirectX
                                               normal maps.
  --opengl                                     Alias of --invert-green, added for vtex2
                                               compatibility.
  --hdri                                       Interpret the given image as an
                                               equirectangular HDRI and create a cubemap.
  --hdri-no-filter                             When creating a cubemap from an input HDRI, do
                                               not perform bilinear filtering.
  --resize-method                              How to resize the texture's width and height
                                               to match a power of 2. Overridden by
                                               --width-resize-method and
                                               --height-resize-method. [nargs=0..1]
                                               [default: "BIGGER"]
  --width-resize-method                        How to resize the texture's width to match a
                                               power of 2. [nargs=0..1] [default: "BIGGER"]
  --height-resize-method                       How to resize the texture's height to match a
                                               power of 2. [nargs=0..1] [default: "BIGGER"]
  --console-mip-scale                          On console platforms, expands the perceived
                                               size of the texture when applied to map
                                               geometry and models. For example, given a
                                               256x256 texture, setting a mip scale of 1 will
                                               cause it to be perceived as 512x512 without
                                               actually increasing memory requirements.
                                               Ignored on PC. [nargs=0..1] [default: 0]
  --gamma-correct                              Perform gamma correction on the input image. 
  --gamma-correct-amount                       The gamma to use in gamma correction. A value
                                               of 1/2.2 is assumed by a good deal of code in
                                               Source engine, change this if you know what
                                               you're doing. [nargs=0..1] [default: 0.454545]
  --srgb                                       Adds PWL_CORRECTED flag before version 7.4,
                                               adds SRGB flag otherwise.
  --clamps                                     Alias of --flag CLAMP_S, added for vtex2
                                               compatibility.
  --clampt                                     Alias of --flag CLAMP_T, added for vtex2
                                               compatibility.
  --clampu                                     Alias of --flag CLAMP_U, added for vtex2
                                               compatibility.
  --pointsample                                Alias of --flag POINT_SAMPLE, added for vtex2
                                               compatibility.
  --trilinear                                  Alias of --flag TRILINEAR, added for vtex2
                                               compatibility.
  --aniso                                      Alias of --flag ANISOTROPIC, added for vtex2
                                               compatibility.
  --normal                                     Alias of --flag NORMAL, added for vtex2
                                               compatibility.
  --ssbump                                     Alias of --flag SSBUMP, added for vtex2
                                               compatibility.
  --particle-sheet-resource PATH               Set the particle sheet resource. Path should
                                               point to a valid particle sheet file.
  --crc-resource CRC                           Set the CRC resource.
  --lod-resource U.V                           Set the LOD resource. U and V values should be
                                               separated by a period.
  --ts0-resource COMBINED_FLAGS                Set the TS0 (extended flags) resource. You'll
                                               have to do the math to combine the flags into
                                               one integer yourself.
  --kvd-resource PATH                          Set the nonstandard KVD (KeyValues Data)
                                               resource. Path should point to a text file.
  --hotspot-data-resource PATH                 Set the hotspot data resource. Path should
                                               point to a valid HOT file.
  --hotspot-rect                               Adds a rect to the hotspot data resource. The
                                               4 input values are in pixel coordinates, and
                                               should not have a decimal point or be less
                                               than zero. Flags should be separated by a
                                               comma with no spaces (or use NONE if no flags
                                               are present). The resource is added and
                                               initialized to default values if not present
                                               beforehand. [nargs: 5] [may be repeated]

"edit" mode (detailed usage):
  --set-version X.Y                            Set the version.
  --set-format IMAGE_FORMAT                    Set the image format. Keep in mind converting
                                               to a lossy format like DXTn means irreversibly
                                               losing information. Recommended to pair this
                                               with the recompute transparency flags
                                               argument.
  --set-width WIDTH                            Set the largest mip's width. Ignores power of
                                               two resize rule. Keep in mind this operation
                                               will result in information loss, especially if
                                               the texture is using a lossy format.
                                               Recommended to pair this with the recompute
                                               mips argument if the input texture is using a
                                               lossless format.
  --set-height HEIGHT                          Set the largest mip's height. Ignores power of
                                               two resize rule. Keep in mind this operation
                                               will result in information loss, especially if
                                               the texture is using a lossy format.
                                               Recommended to pair this with the recompute
                                               mips argument if the input texture is using a
                                               lossless format.
  --edit-filter                                Use this resize filter for all resizing
                                               operations that accept a filter parameter,
                                               including mipmap generation. [nargs=0..1]
                                               [default: "KAISER"]
  --add-flag FLAG                              Flags to add. ENVMAP and NO_MIP flags are
                                               ignored. [may be repeated]
  --remove-flag FLAG                           Flags to remove. ENVMAP and NO_MIP flags are
                                               ignored. [may be repeated]
  --recompute-transparency-flags               Recomputes transparency flags based on the
                                               image format.
  --recompute-mips                             Recomputes mipmaps with the specified edit
                                               resize filter.
  --remove-mips                                Remove mipmaps. If recompute mips is
                                               specified, this argument is ignored.
  --recompute-thumbnail                        Recompute the thumbnail.
  --remove-thumbnail                           Remove the thumbnail. If recompute thumbnail
                                               is specified, this argument is ignored.
  --recompute-reflectivity                     Recompute the reflectivity vector.
  --set-platform PLATFORM                      Set the VTF platform.
  --set-compression-method COMPRESSION_METHOD  Set the compression method. Deflate is
                                               supported on all Strata Source games for VTF
                                               v7.6. Zstd is supported on all Strata Source
                                               games for VTF v7.6 besides Portal: Revolution.
                                               LZMA is supported for console VTFs.
  --set-compression-level LEVEL                Set the compression level. -1 to 9 for Deflate
                                               and LZMA, -1 to 22 for Zstd.
  --set-start-frame FRAME_INDEX                Set the start frame.
  --set-bumpmap-scale SCALE                    Set the bumpmap scale. It can have a decimal
                                               point.
  --set-console-mip-scale                      Set the mip scale. Only has effect on console
                                               platforms. See --console-mip-scale for more
                                               information. [nargs=0..1] [default: 0]
  --set-particle-sheet-resource PATH           Set the particle sheet resource. Path should
                                               point to a valid particle sheet file.
  --remove-particle-sheet-resource             Remove the particle sheet resource. If set
                                               particle sheet resource is specified, this
                                               argument is ignored.
  --set-crc-resource CRC                       Set the CRC resource.
  --remove-crc-resource                        Remove the CRC resource. If set CRC resource
                                               is specified, this argument is ignored.
  --set-lod-resource U.V                       Set the LOD resource. U and V values should be
                                               separated by a period.
  --remove-lod-resource                        Remove the LOD resource. If set LOD resource
                                               is specified, this argument is ignored.
  --set-ts0-resource COMBINED_FLAGS            Set the TS0 (extended flags) resource. You'll
                                               have to do the math to combine the flags into
                                               one integer yourself.
  --remove-ts0-resource                        Remove the TS0 (extended flags) resource. If
                                               set TS0 resource is specified, this argument
                                               is ignored.
  --set-kvd-resource PATH                      Set the nonstandard KVD (KeyValues Data)
                                               resource. Path should point to a text file.
  --remove-kvd-resource                        Remove the nonstandard KVD (KeyValues Data)
                                               resource. If set KVD resource is specified,
                                               this argument is ignored.
  --set-hotspot-data-resource PATH             Set the hotspot data resource. Path should
                                               point to a valid HOT file.
  --remove-hotspot-data-resource               Remove the hotspot data resource. If set HOT
                                               resource is specified, this argument is
                                               ignored.
  --add-hotspot-rect                           Adds a rect to the hotspot data resource. The
                                               4 input values are in pixel coordinates, and
                                               should not have a decimal point or be less
                                               than zero. Flags should be separated by a
                                               comma with no spaces (or use NONE if no flags
                                               are present). The resource is added and
                                               initialized to default values if not present
                                               beforehand. [nargs: 5] [may be repeated]

"info" mode (detailed usage):
  --info-output-mode                           The mode to output information in. Can be
                                               "human" or "kv1". [nargs=0..1]
                                               [default: "human"]
  --info-skip-resources                        Do not print resource internals.

"extract" mode (detailed usage):
  --extract-format                             Output file format. [nargs=0..1]
                                               [default: "DEFAULT"]
  --extract-mip                                Set the mip to extract. Overridden by
                                               --extract-all-mips. [nargs=0..1] [default: 0]
  --extract-all-mips                           Extract all mips. Overridden by --extract-all.
  --extract-frame                              Set the frame to extract. Overridden by
                                               --extract-all-frames. [nargs=0..1]
                                               [default: 0]
  --extract-all-frames                         Extract all frames. Overridden by
                                               --extract-all.
  --extract-face                               Set the face to extract. Overridden by
                                               --extract-all-faces. [nargs=0..1] [default: 0]
  --extract-all-faces                          Extract all faces. Overridden by
                                               --extract-all.
  --extract-slices                             Set the slice to extract. Overridden by
                                               --extract-all-slices. [nargs=0..1]
                                               [default: 0]
  --extract-all-slices                         Extract all slices. Overridden by
                                               --extract-all.
  --extract-all                                Extract all mips, frames, faces, and slices.

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

FLAG
 • POINT_SAMPLE
 • TRILINEAR
 • CLAMP_S
 • CLAMP_T
 • ANISOTROPIC
 • NORMAL
 • NO_LOD
 • LOAD_SMALL_MIPS
 • PROCEDURAL
 • ONE_BIT_ALPHA
 • MULTI_BIT_ALPHA
 • RENDERTARGET
 • DEPTH_RENDERTARGET
 • NO_DEBUG_OVERRIDE
 • SINGLE_COPY
 • NO_DEPTH_BUFFER
 • CLAMP_U
 • XBOX_CACHEABLE
 • XBOX_UNFILTERABLE_OK
 • LOAD_ALL_MIPS
 • VERTEX_TEXTURE
 • SSBUMP
 • BORDER
 • SRGB_V4
 • TF2_STAGING_MEMORY
 • TF2_IMMEDIATE_CLEANUP
 • TF2_IGNORE_PICMIP
 • TF2_STREAMABLE_COARSE
 • TF2_STREAMABLE_FINE
 • PWL_CORRECTED
 • SRGB_V5
 • DEFAULT_POOL
 • LOAD_MOST_MIPS
 • CSGO_COMBINED
 • CSGO_ASYNC_DOWNLOAD
 • CSGO_SKIP_INITIAL_DOWNLOAD
 • CSGO_YCOCG
 • CSGO_ASYNC_SKIP_INITIAL_LOW_RES

HOTSPOT_RECT_FLAGS
 • RANDOM_ROTATION
 • RANDOM_REFLECTION
 • IS_ALTERNATE

PLATFORM
 • PC
 • XBOX
 • X360
 • PS3_ORANGEBOX
 • PS3_PORTAL2

FILE_FORMAT
 • DEFAULT
 • PNG
 • JPG
 • JPEG
 • BMP
 • TGA
 • WEBP
 • QOI
 • HDR
 • EXR

RESIZE_FILTER
 • DEFAULT
 • BOX
 • BILINEAR
 • CUBIC_BSPLINE
 • CATMULL_ROM
 • MITCHELL
 • POINT_SAMPLE
 • KAISER
 • NICE

RESIZE_METHOD
 • NONE
 • BIGGER
 • SMALLER
 • NEAREST

COMPRESSION_METHOD
 • DEFLATE
 • ZSTD
 • CONSOLE_LZMA
```

<img src="https://github.com/craftablescience/MareTF/blob/mane/res/olive_shade_sleep.png?raw=true" alt="The same olive-skinned and red-haired kirin as from the MareTF logo, sleeping." />
