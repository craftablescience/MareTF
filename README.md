<div>
  <img align="left" width="44px" src="https://github.com/craftablescience/MareTF/blob/mane/res/logo.png?raw=true" alt="MareTF Logo" />
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

<img src="https://github.com/craftablescience/MareTF/blob/mane/res/vtf_kirin_sleep.png?raw=true" alt="The same green-skinned and red-haired kirin as from the MareTF logo, sleeping." />
