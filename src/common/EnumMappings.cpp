#include "EnumMappings.h"

namespace not_magic_enum::detail {

const std::vector<vtfpp::HOT::Rect::Flags> HOT_RECT_FLAGS_E{
	vtfpp::HOT::Rect::FLAG_RANDOM_ROTATION,
	vtfpp::HOT::Rect::FLAG_RANDOM_REFLECTION,
	vtfpp::HOT::Rect::FLAG_IS_ALTERNATE,
};
const std::vector<std::string_view> HOT_RECT_FLAGS_S{
	"RANDOM_ROTATION",
	"RANDOM_REFLECTION",
	"IS_ALTERNATE",
};
const std::vector<std::string_view> HOT_RECT_FLAGS_P{
	"Random Rotation",
	"Random Reflection",
	"Is Alternate",
};

const std::vector<vtfpp::ImageFormat> IMAGE_FORMAT_E{
	vtfpp::VTF::FORMAT_UNCHANGED,
	vtfpp::VTF::FORMAT_DEFAULT,
	vtfpp::ImageFormat::RGBA8888,
	vtfpp::ImageFormat::ABGR8888,
	vtfpp::ImageFormat::RGB888,
	vtfpp::ImageFormat::BGR888,
	vtfpp::ImageFormat::RGB565,
	vtfpp::ImageFormat::I8,
	vtfpp::ImageFormat::IA88,
	vtfpp::ImageFormat::P8,
	vtfpp::ImageFormat::A8,
	vtfpp::ImageFormat::RGB888_BLUESCREEN,
	vtfpp::ImageFormat::BGR888_BLUESCREEN,
	vtfpp::ImageFormat::ARGB8888,
	vtfpp::ImageFormat::BGRA8888,
	vtfpp::ImageFormat::DXT1,
	vtfpp::ImageFormat::DXT3,
	vtfpp::ImageFormat::DXT5,
	vtfpp::ImageFormat::BGRX8888,
	vtfpp::ImageFormat::BGR565,
	vtfpp::ImageFormat::BGRX5551,
	vtfpp::ImageFormat::BGRA4444,
	vtfpp::ImageFormat::DXT1_ONE_BIT_ALPHA,
	vtfpp::ImageFormat::BGRA5551,
	vtfpp::ImageFormat::UV88,
	vtfpp::ImageFormat::UVWQ8888,
	vtfpp::ImageFormat::RGBA16161616F,
	vtfpp::ImageFormat::RGBA16161616,
	vtfpp::ImageFormat::UVLX8888,
	vtfpp::ImageFormat::R32F,
	vtfpp::ImageFormat::RGB323232F,
	vtfpp::ImageFormat::RGBA32323232F,
	vtfpp::ImageFormat::RG1616F,
	vtfpp::ImageFormat::RG3232F,
	vtfpp::ImageFormat::RGBX8888,
	vtfpp::ImageFormat::EMPTY,
	vtfpp::ImageFormat::ATI2N,
	vtfpp::ImageFormat::ATI1N,
	vtfpp::ImageFormat::RGBA1010102,
	vtfpp::ImageFormat::BGRA1010102,
	vtfpp::ImageFormat::R16F,
	vtfpp::ImageFormat::CONSOLE_BGRX8888_LINEAR,
	vtfpp::ImageFormat::CONSOLE_RGBA8888_LINEAR,
	vtfpp::ImageFormat::CONSOLE_ABGR8888_LINEAR,
	vtfpp::ImageFormat::CONSOLE_ARGB8888_LINEAR,
	vtfpp::ImageFormat::CONSOLE_BGRA8888_LINEAR,
	vtfpp::ImageFormat::CONSOLE_RGB888_LINEAR,
	vtfpp::ImageFormat::CONSOLE_BGR888_LINEAR,
	vtfpp::ImageFormat::CONSOLE_BGRX5551_LINEAR,
	vtfpp::ImageFormat::CONSOLE_I8_LINEAR,
	vtfpp::ImageFormat::CONSOLE_RGBA16161616_LINEAR,
	vtfpp::ImageFormat::CONSOLE_BGRX8888_LE,
	vtfpp::ImageFormat::CONSOLE_BGRA8888_LE,
	vtfpp::ImageFormat::R8,
	vtfpp::ImageFormat::BC7,
	vtfpp::ImageFormat::BC6H,
};
const std::vector<std::string_view> IMAGE_FORMAT_S{
	"UNCHANGED",
	"DEFAULT",
	"RGBA8888",
	"ABGR8888",
	"RGB888",
	"BGR888",
	"RGB565",
	"I8",
	"IA88",
	"P8",
	"A8",
	"RGB888_BLUESCREEN",
	"BGR888_BLUESCREEN",
	"ARGB8888",
	"BGRA8888",
	"DXT1",
	"DXT3",
	"DXT5",
	"BGRX8888",
	"BGR565",
	"BGRX5551",
	"BGRA4444",
	"DXT1_ONE_BIT_ALPHA",
	"BGRA5551",
	"UV88",
	"UVWQ8888",
	"RGBA16161616F",
	"RGBA16161616",
	"UVLX8888",
	"R32F",
	"RGB323232F",
	"RGBA32323232F",
	"RG1616F",
	"RG3232F",
	"RGBX8888",
	"EMPTY",
	"ATI2N",
	"ATI1N",
	"RGBA1010102",
	"BGRA1010102",
	"R16F",
	"CONSOLE_BGRX8888_LINEAR",
	"CONSOLE_RGBA8888_LINEAR",
	"CONSOLE_ABGR8888_LINEAR",
	"CONSOLE_ARGB8888_LINEAR",
	"CONSOLE_BGRA8888_LINEAR",
	"CONSOLE_RGB888_LINEAR",
	"CONSOLE_BGR888_LINEAR",
	"CONSOLE_BGRX5551_LINEAR",
	"CONSOLE_I8_LINEAR",
	"CONSOLE_RGBA16161616_LINEAR",
	"CONSOLE_BGRX8888_LE",
	"CONSOLE_BGRA8888_LE",
	"R8",
	"BC7",
	"BC6H",
};
const std::vector<std::string_view> IMAGE_FORMAT_P = IMAGE_FORMAT_S;

const std::vector<vtfpp::ImageConversion::FileFormat> IMAGE_CONVERSION_FILE_FORMAT_E{
	vtfpp::ImageConversion::FileFormat::DEFAULT,
	vtfpp::ImageConversion::FileFormat::PNG,
	vtfpp::ImageConversion::FileFormat::JPG,
	vtfpp::ImageConversion::FileFormat::JPG,
	vtfpp::ImageConversion::FileFormat::BMP,
	vtfpp::ImageConversion::FileFormat::TGA,
	vtfpp::ImageConversion::FileFormat::WEBP,
	vtfpp::ImageConversion::FileFormat::QOI,
	vtfpp::ImageConversion::FileFormat::HDR,
	vtfpp::ImageConversion::FileFormat::EXR,
};
const std::vector<std::string_view> IMAGE_CONVERSION_FILE_FORMAT_S{
	"DEFAULT",
	"PNG",
	"JPG",
	"JPEG",
	"BMP",
	"TGA",
	"WEBP",
	"QOI",
	"HDR",
	"EXR",
};
const std::vector<std::string_view> IMAGE_CONVERSION_FILE_FORMAT_P = IMAGE_CONVERSION_FILE_FORMAT_S;

const std::vector<vtfpp::ImageConversion::ResizeFilter> IMAGE_CONVERSION_RESIZE_FILTER_E{
	vtfpp::ImageConversion::ResizeFilter::DEFAULT,
	vtfpp::ImageConversion::ResizeFilter::BOX,
	vtfpp::ImageConversion::ResizeFilter::BILINEAR,
	vtfpp::ImageConversion::ResizeFilter::CUBIC_BSPLINE,
	vtfpp::ImageConversion::ResizeFilter::CATMULL_ROM,
	vtfpp::ImageConversion::ResizeFilter::MITCHELL,
	vtfpp::ImageConversion::ResizeFilter::POINT_SAMPLE,
	vtfpp::ImageConversion::ResizeFilter::KAISER,
	vtfpp::ImageConversion::ResizeFilter::NICE,
};
const std::vector<std::string_view> IMAGE_CONVERSION_RESIZE_FILTER_S{
	"DEFAULT",
	"BOX",
	"BILINEAR",
	"CUBIC_BSPLINE",
	"CATMULL_ROM",
	"MITCHELL",
	"POINT_SAMPLE",
	"KAISER",
	"NICE",
};
const std::vector<std::string_view> IMAGE_CONVERSION_RESIZE_FILTER_P{
	"Default",
	"Box",
	"Bilinear",
	"Cubic BSpline",
	"Catmull-Rom",
	"Mitchell",
	"Point Sample",
	"Kaiser",
	"NICE",
};

const std::vector<vtfpp::ImageConversion::ResizeMethod> IMAGE_CONVERSION_RESIZE_METHOD_E{
	vtfpp::ImageConversion::ResizeMethod::NONE,
	vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_BIGGER,
	vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_SMALLER,
	vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_NEAREST,
};
const std::vector<std::string_view> IMAGE_CONVERSION_RESIZE_METHOD_S{
	"NONE",
	"BIGGER",
	"SMALLER",
	"NEAREST",
};
const std::vector<std::string_view> IMAGE_CONVERSION_RESIZE_METHOD_P{
	"None",
	"Power of 2 (Bigger)",
	"Power of 2 (Smaller)",
	"Power of 2 (Nearest)",
};

const std::vector<vtfpp::CompressionMethod> COMPRESSION_METHOD_E{
	vtfpp::CompressionMethod::DEFLATE,
	vtfpp::CompressionMethod::ZSTD,
	vtfpp::CompressionMethod::CONSOLE_LZMA,
};
const std::vector<std::string_view> COMPRESSION_METHOD_S{
	"DEFLATE",
	"ZSTD",
	"CONSOLE_LZMA",
};
const std::vector<std::string_view> COMPRESSION_METHOD_P{
	"Deflate",
	"Zstd",
	"LZMA (Console)",
};

const std::vector<vtfpp::Resource::Type> RESOURCE_TYPE_E{
	vtfpp::Resource::TYPE_THUMBNAIL_DATA,
	vtfpp::Resource::TYPE_PALETTE_DATA,
	vtfpp::Resource::TYPE_FALLBACK_DATA,
	vtfpp::Resource::TYPE_IMAGE_DATA,
	vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA,
	vtfpp::Resource::TYPE_CRC,
	vtfpp::Resource::TYPE_LOD_CONTROL_INFO,
	vtfpp::Resource::TYPE_EXTENDED_FLAGS,
	vtfpp::Resource::TYPE_KEYVALUES_DATA,
	vtfpp::Resource::TYPE_HOTSPOT_DATA,
	vtfpp::Resource::TYPE_AUX_COMPRESSION,
};
const std::vector<std::string_view> RESOURCE_TYPE_S{
	"THUMBNAIL",
	"PALETTE",
	"FALLBACK",
	"IMAGE",
	"PARTICLE_SHEET",
	"CRC",
	"LOD_CONTROL_INFO",
	"EXTENDED_FLAGS",
	"KEYVALUES_DATA",
	"HOTSPOT_DATA",
	"AUX_COMPRESSION",
};
const std::vector<std::string_view> RESOURCE_TYPE_P{
	"Thumbnail",
	"Palette",
	"Fallback",
	"Image",
	"Particle Sheet",
	"CRC",
	"LOD Control Info",
	"Extended Flags",
	"KeyValues Data",
	"Hotspot Data",
	"Compression Info",
};

const std::vector<vtfpp::VTF::Flags> VTF_FLAGS_E{
	// v0
	vtfpp::VTF::FLAG_V0_POINT_SAMPLE,
	vtfpp::VTF::FLAG_V0_TRILINEAR,
	vtfpp::VTF::FLAG_V0_CLAMP_S,
	vtfpp::VTF::FLAG_V0_CLAMP_T,
	vtfpp::VTF::FLAG_V0_ANISOTROPIC,
	vtfpp::VTF::FLAG_V0_NORMAL,
	vtfpp::VTF::FLAG_V0_NO_LOD,
	vtfpp::VTF::FLAG_V0_LOAD_SMALL_MIPS,
	vtfpp::VTF::FLAG_V0_PROCEDURAL,
	vtfpp::VTF::FLAG_V0_ONE_BIT_ALPHA,
	vtfpp::VTF::FLAG_V0_MULTI_BIT_ALPHA,
	// v1
	vtfpp::VTF::FLAG_V1_RENDERTARGET,
	vtfpp::VTF::FLAG_V1_DEPTH_RENDERTARGET,
	vtfpp::VTF::FLAG_V1_NO_DEBUG_OVERRIDE,
	vtfpp::VTF::FLAG_V1_SINGLE_COPY,
	// v2
	vtfpp::VTF::FLAG_V2_NO_DEPTH_BUFFER,
	vtfpp::VTF::FLAG_V2_CLAMP_U,
	// XBOX
	vtfpp::VTF::FLAG_XBOX_CACHEABLE,
	vtfpp::VTF::FLAG_XBOX_UNFILTERABLE_OK,
	// v3
	vtfpp::VTF::FLAG_V3_LOAD_ALL_MIPS,
	vtfpp::VTF::FLAG_V3_VERTEX_TEXTURE,
	vtfpp::VTF::FLAG_V3_SSBUMP,
	vtfpp::VTF::FLAG_V3_BORDER,
	// v4
	vtfpp::VTF::FLAG_V4_SRGB,
	// v4 (TF2)
	vtfpp::VTF::FLAG_V4_TF2_STAGING_MEMORY,
	vtfpp::VTF::FLAG_V4_TF2_IMMEDIATE_CLEANUP,
	vtfpp::VTF::FLAG_V4_TF2_IGNORE_PICMIP,
	vtfpp::VTF::FLAG_V4_TF2_STREAMABLE_COARSE,
	vtfpp::VTF::FLAG_V4_TF2_STREAMABLE_FINE,
	// v5
	vtfpp::VTF::FLAG_V5_PWL_CORRECTED,
	vtfpp::VTF::FLAG_V5_SRGB,
	vtfpp::VTF::FLAG_V5_DEFAULT_POOL,
	vtfpp::VTF::FLAG_V5_LOAD_MOST_MIPS,
	// v5 (CS:GO)
	vtfpp::VTF::FLAG_V5_CSGO_COMBINED,
	vtfpp::VTF::FLAG_V5_CSGO_ASYNC_DOWNLOAD,
	vtfpp::VTF::FLAG_V5_CSGO_SKIP_INITIAL_DOWNLOAD,
	vtfpp::VTF::FLAG_V5_CSGO_YCOCG,
	vtfpp::VTF::FLAG_V5_CSGO_ASYNC_SKIP_INITIAL_LOW_RES,
};
const std::vector<std::string_view> VTF_FLAGS_S{
	// v0
	"POINT_SAMPLE",
	"TRILINEAR",
	"CLAMP_S",
	"CLAMP_T",
	"ANISOTROPIC",
	"NORMAL",
	"NO_LOD",
	"LOAD_SMALL_MIPS",
	"PROCEDURAL",
	"ONE_BIT_ALPHA",
	"MULTI_BIT_ALPHA",
	// v1
	"RENDERTARGET",
	"DEPTH_RENDERTARGET",
	"NO_DEBUG_OVERRIDE",
	"SINGLE_COPY",
	// v2
	"NO_DEPTH_BUFFER",
	"CLAMP_U",
	// XBOX
	"XBOX_CACHEABLE",
	"XBOX_UNFILTERABLE_OK",
	// v3
	"LOAD_ALL_MIPS",
	"VERTEX_TEXTURE",
	"SSBUMP",
	"BORDER",
	// v4
	"SRGB_V4",
	// v4 (TF2)
	"TF2_STAGING_MEMORY",
	"TF2_IMMEDIATE_CLEANUP",
	"TF2_IGNORE_PICMIP",
	"TF2_STREAMABLE_COARSE",
	"TF2_STREAMABLE_FINE",
	// v5
	"PWL_CORRECTED",
	"SRGB_V5",
	"DEFAULT_POOL",
	"LOAD_MOST_MIPS",
	// v5 (CS:GO)
	"CSGO_COMBINED",
	"CSGO_ASYNC_DOWNLOAD",
	"CSGO_SKIP_INITIAL_DOWNLOAD",
	"CSGO_YCOCG",
	"CSGO_ASYNC_SKIP_INITIAL_LOW_RES",
};
const std::vector<std::string_view> VTF_FLAGS_P = VTF_FLAGS_S;

const std::vector<vtfpp::VTF::Platform> VTF_PLATFORM_E{
	vtfpp::VTF::PLATFORM_PC,
	vtfpp::VTF::PLATFORM_XBOX,
	vtfpp::VTF::PLATFORM_X360,
	vtfpp::VTF::PLATFORM_PS3_ORANGEBOX,
	vtfpp::VTF::PLATFORM_PS3_PORTAL2,
};
const std::vector<std::string_view> VTF_PLATFORM_S{
	"PC",
	"XBOX",
	"X360",
	"PS3_ORANGEBOX",
	"PS3_PORTAL2",
};
const std::vector<std::string_view> VTF_PLATFORM_P{
	"PC",
	"XBOX",
	"X360",
	"PS3 (Orange Box)",
	"PS3 (P2, CS:GO)",
};

} // namespace not_magic_enum::detail
