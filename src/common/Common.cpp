#include "Common.h"

#include <sourcepp/String.h>

using namespace sourcepp;

[[nodiscard]] std::string getOutputPathForInput(std::string_view inputPath, vtfpp::VTF::Platform outputPlatform) {
	std::string inputLowercase{inputPath};
	sourcepp::string::toLower(inputLowercase);

	const bool lowercaseExtension = std::filesystem::path{inputPath}.extension() == std::filesystem::path{inputLowercase}.extension();

	std::string out;
	if (inputLowercase.ends_with(".360.vtf") || inputLowercase.ends_with(".ps3.vtf")) {
		out = inputPath.substr(0, inputPath.size() - 8);
	} else if (inputLowercase.ends_with(".vtf") || inputLowercase.ends_with(".xtf")) {
		out = inputPath.substr(0, inputPath.size() - 4);
	} else {
		out = inputPath.substr(0, inputPath.size() - std::filesystem::path{inputPath}.extension().string().size());
	}

	switch (outputPlatform) {
	case vtfpp::VTF::PLATFORM_UNKNOWN:
		break;
	case vtfpp::VTF::PLATFORM_PC:
		return out + (lowercaseExtension ? ".vtf" : ".VTF");
	case vtfpp::VTF::PLATFORM_XBOX:
		return out + (lowercaseExtension ? ".xtf" : ".XTF");
	case vtfpp::VTF::PLATFORM_X360:
		return out + (lowercaseExtension ? ".360.vtf" : ".360.VTF");
	case vtfpp::VTF::PLATFORM_PS3_ORANGEBOX:
	case vtfpp::VTF::PLATFORM_PS3_PORTAL2:
		return out + (lowercaseExtension ? ".ps3.vtf" : ".PS3.VTF");
	}
	return "";
}

[[nodiscard]] std::array<std::string, 6> getOutputSkyboxPathsForInput(std::string_view inputPath, vtfpp::VTF::Platform outputPlatform) {
	const std::filesystem::path inputPathPath{inputPath};
	const auto inputPathStart = (inputPathPath.parent_path() / inputPathPath.stem()).string();
	const auto inputPathEnd = inputPathPath.extension().string();
	return {
		::getOutputPathForInput(inputPathStart + ::getOutputCubemapFacePathSuffixes()[0].data() + inputPathEnd, outputPlatform),
		::getOutputPathForInput(inputPathStart + ::getOutputCubemapFacePathSuffixes()[1].data() + inputPathEnd, outputPlatform),
		::getOutputPathForInput(inputPathStart + ::getOutputCubemapFacePathSuffixes()[2].data() + inputPathEnd, outputPlatform),
		::getOutputPathForInput(inputPathStart + ::getOutputCubemapFacePathSuffixes()[3].data() + inputPathEnd, outputPlatform),
		::getOutputPathForInput(inputPathStart + ::getOutputCubemapFacePathSuffixes()[4].data() + inputPathEnd, outputPlatform),
		::getOutputPathForInput(inputPathStart + ::getOutputCubemapFacePathSuffixes()[5].data() + inputPathEnd, outputPlatform),
	};
}

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

std::string_view supportedImageFileFormatsForLoad() {
	return "Image Formats (*.apng *.bmp *.exr *.gif *.hdr *.jpg *.jpeg *.pic *.png *.pgm *.ppm *.psd *.qoi *.tga *.webp)";
}

std::string_view supportedImageFileFormatsForSave() {
	return "Image Formats (*.bmp *.exr *.hdr *.jpg *.jpeg *.png *.qoi *.tga *.webp)";
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
		"No Mipmaps",
		"No LOD",
		"Load Small Mipmaps",
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
		flags[19] = "(VTEX) One Over Mipmap Level in Alpha";
		flags[20] = "(VTEX) Premultiply Color by One Over Mipmap Level in Alpha";
		flags[21] = "(VTEX) Convert Normal to DUDV";
	}
	if (minorVersion >= 2) {
		flags[22] = "(VTEX) Alpha Test Mipmap Generation";
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
		flags[10] = "Load All Mipmaps";
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
		flags[28] = "Load Most Mipmaps";
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
