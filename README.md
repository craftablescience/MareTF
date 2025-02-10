<div>
  <img align="left" width="44px" src="https://github.com/craftablescience/MareTF/blob/main/res/logo.png?raw=true" alt="MareTF Logo" />
  <h1>MareTF</h1>
</div>

<div>
  <a href="https://github.com/craftablescience/MareTF/blob/main/LICENSE" target="_blank" rel="noopener noreferrer"><img src="https://img.shields.io/github/license/craftablescience/MareTF?label=license" alt="License" /></a>
  <a href="https://github.com/craftablescience/MareTF/actions" target="_blank" rel="noopener noreferrer"><img src="https://img.shields.io/github/actions/workflow/status/craftablescience/MareTF/build.yml?branch=mane&label=builds" alt="Workflow Status" /></a>
  <a href="https://discord.gg/ASgHFkX" target="_blank" rel="noopener noreferrer"><img src="https://img.shields.io/discord/678074864346857482?label=discord&logo=Discord&logoColor=%23FFFFFF" alt="Discord" /></a>
  <a href="https://ko-fi.com/craftablescience" target="_blank" rel="noopener noreferrer"><img src="https://img.shields.io/badge/donate-006dae?label=ko-fi&logo=ko-fi" alt="Ko-Fi" /></a>
</div>

A work in progress command-line utility to work with VTF files.

- Creation Features
  - Directly convert animated images (APNG/GIF)
  - Directly convert floating point images (EXR/HDR)
  - Kaiser mipmap filtering selected by default
  - Create non-power of two textures
  - Create console (Orange Box) VTFs
  - Supports new Strata Source VTF version
    - New formats (BC7/BC6H)
    - New CPU compression (Deflate/Zstandard)
- More to come!

## Backend

This tool is powered by a collection of open-source C++20 Source engine parsers called [sourcepp](https://github.com/craftablescience/sourcepp).
