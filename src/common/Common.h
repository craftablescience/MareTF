#pragma once

#include <array>
#include <string_view>

#include <vtfpp/VTF.h>

[[nodiscard]] std::string getOutputPathForInput(std::string_view inputPath, vtfpp::VTF::Platform outputPlatform);

[[nodiscard]] consteval std::array<std::string_view, 7> getOutputCubemapFacePathSuffixes() {
	return {"rt", "lf", "bk", "ft", "up", "dn", "sph"};
}

[[nodiscard]] std::array<std::string, 6> getOutputSkyboxPathsForInput(std::string_view inputPath, vtfpp::VTF::Platform outputPlatform);

[[nodiscard]] bool fileIsASupportedImageFileFormat(std::string_view extension);

[[nodiscard]] std::string_view supportedImageFileFormatExtension(vtfpp::ImageConversion::FileFormat fileFormat);

[[nodiscard]] vtfpp::ImageConversion::FileFormat supportedImageFileFormatExtension(std::string_view fileFormatExtension);

[[nodiscard]] std::string_view supportedImageFileFormatsForLoad();

[[nodiscard]] std::string_view supportedImageFileFormatsForSave();

[[nodiscard]] std::array<std::string_view, 32> getPrettyFlagNamesFor(uint16_t minorVersion, vtfpp::VTF::Platform platform);
