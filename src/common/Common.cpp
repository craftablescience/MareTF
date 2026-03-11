#include "Common.h"

#include <sourcepp/String.h>

using namespace sourcepp;

bool fileIsASupportedImageFileFormat(std::string_view extension) {
	static constexpr std::array<std::string_view, 15> SUPPORTED_EXTENSIONS{
		".apng",
		".bmp",
		".exr",
		".gif",
		".hdr",
		".jpeg",
		".jpg",
		".pic",
		".png",
		".pgm",
		".ppm",
		".psd",
		".qoi",
		".tga",
		".webp",
	};
	return std::ranges::find(SUPPORTED_EXTENSIONS, sourcepp::string::toLower(extension)) != SUPPORTED_EXTENSIONS.end();
}

std::string_view supportedImageFileFormatExtension(vtfpp::ImageConversion::FileFormat fileFormat) {
	switch (fileFormat) {
		using enum vtfpp::ImageConversion::FileFormat;
		case DEFAULT:
			// We should not be here!
			break;
		case PNG:  return ".png";
		case JPG:  return ".jpg";
		case BMP:  return ".bmp";
		case TGA:  return ".tga";
		case WEBP: return ".webp";
		case QOI:  return ".qoi";
		case HDR:  return ".hdr";
		case EXR:  return ".exr";
	}
	return "";
}

vtfpp::ImageConversion::FileFormat supportedImageFileFormatExtension(std::string_view fileFormatExtension) {
	using enum vtfpp::ImageConversion::FileFormat;
	if (string::iequals(fileFormatExtension, ".png")) return PNG;
	if (string::iequals(fileFormatExtension, ".jpg") || string::iequals(fileFormatExtension, ".jpeg")) return JPG;
	if (string::iequals(fileFormatExtension, ".bmp")) return BMP;
	if (string::iequals(fileFormatExtension, ".tga")) return TGA;
	if (string::iequals(fileFormatExtension, ".webp")) return WEBP;
	if (string::iequals(fileFormatExtension, ".qoi")) return QOI;
	if (string::iequals(fileFormatExtension, ".hdr")) return HDR;
	if (string::iequals(fileFormatExtension, ".exr")) return EXR;
	// We should not be here!
	return DEFAULT;
}

std::string_view supportedImageFileFormatsForSave() {
	return "Image Formats (*.png *.jpg *.jpeg *.bmp *.tga *.webp *.qoi *.hdr *.exr)";
}

std::array<std::string_view, 32> getPrettyFlagNamesFor(uint16_t minorVersion, vtfpp::VTF::Platform platform) {
	std::array<std::string_view, 32> flags{
		"Point Sample",
		"Trilinear",
		"Clamp S",
		"Clamp T",
		"Anisotropic",
		"(VTEX) Hint DXT5",
		"(VTEX) No Compress",
		"Normal",
		"No Mips",
		"No LOD",
		"Load Small Mips",
		"Procedural",
		"One-bit Alpha",
		"Multi-bit Alpha",
		"Envmap",
		"Unused (1<<15)",
		"Unused (1<<16)",
		"Unused (1<<17)",
		"Unused (1<<18)",
		"Unused (1<<19)",
		"Unused (1<<20)",
		"Unused (1<<21)",
		"Unused (1<<22)",
		"Unused (1<<23)",
		"Unused (1<<24)",
		"Unused (1<<25)",
		"Unused (1<<26)",
		"Unused (1<<27)",
		"Unused (1<<28)",
		"Unused (1<<29)",
		"Unused (1<<30)",
		"Unused (1<<31)",
	};
	if (minorVersion >= 1) {
		flags[15] = "Rendertarget";
		flags[16] = "Depth Rendertarget";
		flags[17] = "No Debug Override";
		flags[18] = "Single Copy";
		flags[19] = "(VTEX) One Over Mip Level in Alpha";
		flags[20] = "(VTEX) Premultiply Color by One Over Mip Level in Alpha";
		flags[21] = "(VTEX) Convert Normal to DUDV";
	}
	if (minorVersion >= 2) {
		flags[22] = "(VTEX) Alpha Test Mip Generation";
		flags[23] = "No Depth Buffer";
		flags[24] = "(VTEX) NICE Filtered";
		flags[25] = "Clamp U";
		if (platform == vtfpp::VTF::Platform::PLATFORM_XBOX) {
			flags[26] = "[XBOX] (VTEX) Preswizzled";
			flags[27] = "[XBOX] Cacheable";
			flags[28] = "[XBOX] Unfilterable OK";
		}
	}
	if (minorVersion >= 3) {
		flags[10] = "Load All Mips";
		flags[26] = "Vertex Texture";
		flags[27] = "SSBump";
		flags[28] = "Unused (1<<28)";
		flags[29] = "Border";
	}
	if (minorVersion >= 4) {
		flags[5] = "Unused (1<<5)";
		flags[6] = "sRGB";
		flags[19] = "[TF2] Staging Memory";
		flags[20] = "[TF2] Immediate Cleanup";
		flags[21] = "[TF2] Ignore mat_picmip";
		flags[22] = "Unused (1<<22)";
		flags[24] = "Unused (1<<24)";
		flags[30] = "[TF2] Internal (1<<30)";
		flags[31] = "[TF2] Internal (1<<31)";
	}
	if (minorVersion >= 5) {
		flags[6] = "PWL Corrected";
		flags[10] = "Unused (1<<10)";
		flags[18] = "Unused (1<<18)";
		flags[19] = "sRGB";
		flags[20] = "Default Pool";
		flags[21] = "[CS:GO] Combined";
		flags[22] = "[CS:GO] Async Download";
		flags[24] = "[CS:GO] Skip Initial Download";
		flags[28] = "Load Most Mips";
		flags[30] = "[CS:GO] YCoCg";
		flags[31] = "[CS:GO] Async Skip Initial Low Res";
	}
	if (minorVersion >= 6) {
		flags[18] = "Ignore mat_picmip";
		flags[21] = "Combined";
		flags[22] = "Async Download";
		flags[24] = "Skip Initial Download";
		flags[30] = "YCoCg";
		flags[31] = "Async Skip Initial Low Res";
	}
	return flags;
}
