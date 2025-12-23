// ReSharper disable CppDFATimeOver
// ReSharper disable CppUseStructuredBinding

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <functional>
#include <exception>
#include <memory>
#include <mutex>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>

#if defined(_WIN32)
#include <io.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#include <argparse/argparse.hpp>
#include <efsw/efsw.hpp>
#include <indicators/progress_bar.hpp>
#include <kvpp/KV1Writer.h>
#include <sourcepp/FS.h>
#include <sourcepp/String.h>
#include <vtfpp/vtfpp.h>

#include "../common/Common.h"
#include "../common/Config.h"
#include "../common/EnumMappings.h"

using namespace std::literals;

namespace {

[[nodiscard]] bool runningInTTY() {
#if defined(_WIN32)
	static const bool check = _isatty(_fileno(stdout)) && _isatty(_fileno(stderr));
#elif defined(__linux__)
	static const bool check = isatty(STDOUT_FILENO) && isatty(STDERR_FILENO);
#else
	static constexpr bool check = true;
#endif
	return check;
}

[[nodiscard]] std::string_view randomDeviantArtTFTrope() {
	static constexpr std::array<std::string_view, 23> DEVIANTART_TF_TROPES{
		"Splicing DNA",
		"Drinking a potion stolen from a wizard neighbor",
		"Tampering with a sacred equine relic",
		"Downloading a mysterious app",
		"Irritating a magic-wielding farmhand",
		"Getting isekai'd into Ponyville",
		"Falling into the contraption",
		"Entering the light of the full moon on Nightmare Night",
		"Wearing a saddle on a dare",
		"Drawing a pony with a magic marker",
		"Injecting pony HRT",
		// The following are from @BethesdaCakeDelivery, thanks!
		"Abandoning humanity",
		"Initiating headbonk",
		"Turning hands into hooves",
		"Beginning pastelization",
		"Rearranging bone structure",
		"De-evolving into a four-legged creature",
		"Turning unguligrade",
		"Handing out hooves",
		"Hooving out hands",
		"Trotting through green fields and apple trees",
		"Buying an orbitouch",
		"Ponifying splines",
	};
	static std::random_device device;
	static std::mt19937 generator(device());
	std::uniform_int_distribution<> dist{0, DEVIANTART_TF_TROPES.size() - 1};
	return DEVIANTART_TF_TROPES.at(dist(generator));
}

[[nodiscard]] bool fileIsASupportedImageFileFormat(std::string_view extension) {
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

[[nodiscard]] std::string_view supportedImageFileFormatExtension(vtfpp::ImageConversion::FileFormat fileFormat) {
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

template<not_magic_enum::SupportedEnum E>
void enumValueValidityCheck(std::string_view enumName, const std::string& arg) {
	if (!not_magic_enum::enum_cast<E>(arg)) {
		throw std::runtime_error{"Invalid " + std::string{enumName} + " enum value: " + arg};
	}
}

template<typename duration = std::chrono::milliseconds>
class ElapsedTime {
	using clock = std::chrono::steady_clock;

public:
	ElapsedTime() : start{std::chrono::time_point_cast<duration>(clock::now())} {}

	[[nodiscard]] duration get() const noexcept {
		return std::chrono::duration_cast<duration>(std::chrono::time_point_cast<duration>(clock::now()) - this->start);
	}

protected:
	std::chrono::time_point<clock, duration> start;
};

class MareTFFileWatchListener : public efsw::FileWatchListener {
public:
	using Callback = std::function<void(efsw::WatchID, const std::string&, const std::string&, efsw::Action, std::string)>;

	explicit MareTFFileWatchListener(Callback callback_) : efsw::FileWatchListener{}, callback{std::move(callback_)} {}

	void handleFileAction(efsw::WatchID watchID, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) override {
		this->callback(watchID, dir, filename, action, oldFilename);
	}

private:
	Callback callback;
};

struct tfout_t {
	static inline bool QUIET = false;
	tfout_t& operator<<(const auto& x) {
		if (!QUIET) std::cout << x;
		return *this;
	}
} tfout;

struct tferr_t {
	static inline bool QUIET = false;
	tferr_t& operator<<(const auto& x) {
		if (!QUIET) std::cerr << x;
		return *this;
	}
} tferr;

struct tfendl_t {} tfendl;

// ReSharper disable once CppDeclaratorNeverUsed
template<> tfout_t& tfout_t::operator<<<tfendl_t>(const tfendl_t&) {
	if (!QUIET) std::cout << std::endl;
	return *this;
}

// ReSharper disable once CppDeclaratorNeverUsed
template<> tferr_t& tferr_t::operator<<<tfendl_t>(const tfendl_t&) {
	if (!QUIET) std::cerr << std::endl;
	return *this;
}

} // namespace

int main(int argc, const char* const argv[]) {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8); // Set up console to show UTF-8 characters
	setvbuf(stdout, nullptr, _IOFBF, 1000); // Enable buffering so VS terminal won't chop up UTF-8 byte sequences
#endif

	argparse::ArgumentParser cli{PROJECT_NAME, PROJECT_VERSION, argparse::default_arguments::help};

#ifdef _WIN32
	// Add the Windows-specific ones because why not
	cli.set_prefix_chars("-/");
	cli.set_assign_chars("=:");
#endif

	//region General Arguments

	std::string mode;
	cli
		.add_argument("mode")
		.metavar("MODE")
		.help("The mode to run the program in. This determines what arguments are processed. Valid options:"
		      R"( "create", "edit", "extract", and "info". "convert" is also permissible and is an alias of "create")"
		      " for vtex2 compatibility.")
		.choices("create", "convert", "edit", "extract", "info")
		.required()
		.store_into(mode);

	std::string inputPath;
	cli
		.add_argument("path")
		.metavar("PATH")
		.help("The path to the input file or directory.")
		.required()
		.store_into(inputPath);

	std::string outputPath;
	cli
		.add_argument("-o", "--output")
		.metavar("PATH")
		.help("The path to the output file (if the current mode outputs a file). Ignored if the input path is a directory.")
		.store_into(outputPath);

	bool overwrite;
	cli
		.add_argument("-y", "--yes")
		.help("Automatically say yes to any prompts. Enabled by default if no TTY is detected.")
		.flag()
		.store_into(overwrite);

	bool noOverwrite;
	cli
		.add_argument("--no")
		.help("Automatically say no to any prompts. Overrides --yes.")
		.flag()
		.store_into(noOverwrite);

	bool quiet;
	cli
		.add_argument("--quiet")
		.help("Don't print anything to stdout or stderr (assuming program arguments are parsed successfully)."
		      " Enabled by default if no TTY is detected.")
		.flag()
		.store_into(quiet);

	bool verbose;
	cli
		.add_argument("--verbose")
		.help("Allow printing to stdout or stderr, even when no TTY is detected (assuming program arguments"
		      " are parsed successfully).")
		.flag()
		.store_into(verbose);

	bool noRecurse;
	cli
		.add_argument("--no-recurse")
		.help("If the input path is a directory, do not enter subdirectories when scanning for files.")
		.flag()
		.store_into(noRecurse);

	bool noPrettyFormatting;
	cli
		.add_argument("--no-pretty-formatting")
		.help("Disables printing ANSI color codes and emojis. Pretty formatting is disabled by default"
		      " if no TTY is detected.")
		.flag()
		.store_into(noPrettyFormatting);

	//endregion

	//region Create Mode Arguments

	auto& createCLI = cli.add_group(R"("create" mode)");

	bool watch;
	createCLI
		.add_argument("--watch")
		.help("After creation is complete, watch the input file or directory for any changes and re-TF the VTF(s)."
		      " --no is implied on the first creation pass. --yes is implied after the first creation pass.")
		.flag()
		.store_into(watch);

	std::string version{"7.4"};
	createCLI
		.add_argument("-v", "--version")
		.metavar("X.Y")
		.help("Major and minor version, split by a period. Ignored if platform is specified as anything other than"
		      " PC. Note that older branches of the Source engine will not load VTF versions made for newer branches."
		      " VTF v7.6 is only loadable by games running on Strata Source.")
		.choices("7.0", "7.1", "7.2", "7.3", "7.4", "7.5", "7.6")
		.default_value(version).store_into(version);

	std::string format{not_magic_enum::enum_name(vtfpp::VTF::FORMAT_DEFAULT)};
	createCLI
		.add_argument("-f", "--format")
		.metavar("IMAGE_FORMAT")
		.help("Output format.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::ImageFormat>, "IMAGE_FORMAT"))
		.default_value(format).store_into(format);

	float compressedFormatQuality = vtfpp::ImageConversion::DEFAULT_COMPRESSED_QUALITY;
	createCLI
		.add_argument("-q", "--quality")
		.metavar("COMPRESSION_QUALITY")
		.help("The quality of DXTn/BCn format compression, between 0.0 and 1.0. Higher quality will take"
		      " significantly longer to create the texture. If quality is below 0.0, default compression values will be"
		      " used (0.1 for BC7, BC6H, and 1.0 for all others). Ignored if output format is uncompressed.")
		.scan<'g', float>()
		.default_value(compressedFormatQuality).store_into(compressedFormatQuality);

	std::string filter{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeFilter::KAISER)};
	createCLI
		.add_argument("-r", "--filter")
		.metavar("RESIZE_FILTER")
		.help("The resize filter used to generate mipmaps and when resizing the base texture to match a power of 2"
		      " (if necessary).")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::ImageConversion::ResizeFilter>, "RESIZE_FILTER"))
		.default_value(filter).store_into(filter);

	std::vector<std::string> flags;
	createCLI
		.add_argument("--flag")
		.metavar("FLAG")
		.help("Extra flags to add. ENVMAP, ONE_BIT_ALPHA, MULTI_BIT_ALPHA, and NO_MIP flags are applied"
		      " automatically based on the VTF properties.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::VTF::Flags>, "FLAG"))
		.append()
		.store_into(flags);

	bool noTransparencyFlags;
	createCLI
		.add_argument("--no-automatic-transparency-flags")
		.help("Disable adding ONE_BIT_ALPHA and MULTI_BIT_ALPHA flags by default depending on the output image format.")
		.flag()
		.store_into(noTransparencyFlags);

	bool noMips;
	createCLI
		.add_argument("--no-mips")
		.help("Disable mipmap generation.")
		.flag()
		.store_into(noMips);

	bool animatedFrames;
	createCLI
		.add_argument("-a", "--animated-frames")
		.help("If input texture filename ends in two or more numbers, check for consecutive numbered files and add as animation frames if found.")
		.flag()
		.store_into(animatedFrames);

	bool noThumbnail;
	createCLI
		.add_argument("--no-thumbnail")
		.help("Disable thumbnail generation.")
		.flag()
		.store_into(noThumbnail);

	std::string platform{not_magic_enum::enum_name(vtfpp::VTF::Platform::PLATFORM_PC)};
	createCLI
		.add_argument("-p", "--platform")
		.metavar("PLATFORM")
		.help("Set the platform (PC/console) to build for.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::VTF::Platform>, "PLATFORM"))
		.default_value(platform).store_into(platform);

	std::string compressionMethod{not_magic_enum::enum_name(vtfpp::CompressionMethod::ZSTD)};
	createCLI
		.add_argument("-m", "--compression-method")
		.metavar("COMPRESSION_METHOD")
		.help("Set the compression method. Deflate is supported on all Strata Source games for VTF v7.6."
		      " Zstd is supported on all Strata Source games for VTF v7.6 besides Portal: Revolution."
		      " LZMA is supported for console VTFs.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::CompressionMethod>, "COMPRESSION_METHOD"))
		.default_value(compressionMethod).store_into(compressionMethod);

	int compressionLevel = 6;
	createCLI
		.add_argument("-c", "--compression-level")
		.metavar("LEVEL")
		.help("Set the compression level. -1 to 9 for Deflate and LZMA, -1 to 22 for Zstd.")
		.scan<'d', int>()
		.default_value(compressionLevel).store_into(compressionLevel);

	int startFrame = 0;
	createCLI
		.add_argument("--start-frame")
		.metavar("FRAME_INDEX")
		.help("The start frame used in animations, counting from zero. Ignored when creating console VTFs.")
		.scan<'d', int>()
		.default_value(startFrame).store_into(startFrame);

	float bumpMapScale = 1.f;
	createCLI
		.add_argument("--bumpscale")
		.metavar("BUMPMAP_SCALE")
		.help("The bumpmap scale. It can have a decimal point.")
		.scan<'g', float>()
		.default_value(bumpMapScale).store_into(bumpMapScale);

	bool invertGreenChannel;
	createCLI
		.add_argument("--invert-green")
		.help("Invert the green channel of the input image. This converts OpenGL normal maps into DirectX normal maps.")
		.flag()
		.store_into(invertGreenChannel);

	bool invertGreenChannelAlt;
	createCLI
		.add_argument("--opengl")
		.help("Alias of --invert-green, added for vtex2 compatibility.")
		.flag()
		.store_into(invertGreenChannelAlt);

	bool hdri;
	createCLI
		.add_argument("--hdri")
		.help("Interpret the given image as an equirectangular HDRI and create a cubemap.")
		.flag()
		.store_into(hdri);

	bool hdriNoFilter;
	createCLI
		.add_argument("--hdri-no-filter")
		.help("When creating a cubemap from an input HDRI, do not perform bilinear filtering.")
		.flag()
		.store_into(hdriNoFilter);

	std::string resizeMethod{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_BIGGER)};
	createCLI
		.add_argument("--resize-method")
		.metavar("RESIZE_METHOD")
		.help("How to resize the texture's width and height to match a power of 2. Overridden by"
			" --width-resize-method and --height-resize-method.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::ImageConversion::ResizeMethod>, "RESIZE_METHOD"))
		.default_value(resizeMethod).store_into(resizeMethod);

	std::string widthResizeMethod{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_BIGGER)};
	createCLI
		.add_argument("--width-resize-method")
		.metavar("RESIZE_METHOD")
		.help("How to resize the texture's width to match a power of 2.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::ImageConversion::ResizeMethod>, "RESIZE_METHOD"))
		.default_value(widthResizeMethod).store_into(widthResizeMethod);

	std::string heightResizeMethod{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_BIGGER)};
	createCLI
		.add_argument("--height-resize-method")
		.metavar("RESIZE_METHOD")
		.help("How to resize the texture's height to match a power of 2.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::ImageConversion::ResizeMethod>, "RESIZE_METHOD"))
		.default_value(heightResizeMethod).store_into(heightResizeMethod);

	int consoleMipScale = 0;
	createCLI
		.add_argument("--console-mip-scale")
		.metavar("SCALE")
		.help("On console platforms, expands the perceived size of the texture when applied to map geometry and"
		      " models. For example, given a 256x256 texture, setting a mip scale of 1 will cause it to be perceived"
		      " as 512x512 without actually increasing memory requirements. Ignored on PC.")
		.scan<'d', int>()
		.default_value(consoleMipScale).store_into(consoleMipScale);

	bool gammaCorrection;
	createCLI
		.add_argument("--gamma-correct")
		.help("Perform gamma correction on the input image.")
		.flag()
		.store_into(gammaCorrection);

	float gammaCorrectionAmount = 1.f / 2.2f;
	createCLI
		.add_argument("--gamma-correct-amount")
		.metavar("GAMMA")
		.help("The gamma to use in gamma correction. A value of 1/2.2 is assumed by a good deal of code in Source"
		      " engine, change this if you know what you're doing.")
		.scan<'g', float>()
		.default_value(gammaCorrectionAmount).store_into(gammaCorrectionAmount);

	bool srgb;
	createCLI
		.add_argument("--srgb")
		.help("Adds PWL_CORRECTED flag before version 7.4, adds SRGB flag otherwise.")
		.flag()
		.store_into(srgb);

	bool clamp_s;
	createCLI
		.add_argument("--clamps")
		.help("Alias of --flag CLAMP_S, added for vtex2 compatibility.")
		.flag()
		.store_into(clamp_s);

	bool clamp_t;
	createCLI
		.add_argument("--clampt")
		.help("Alias of --flag CLAMP_T, added for vtex2 compatibility.")
		.flag()
		.store_into(clamp_t);

	bool clamp_u;
	createCLI
		.add_argument("--clampu")
		.help("Alias of --flag CLAMP_U, added for vtex2 compatibility.")
		.flag()
		.store_into(clamp_u);

	bool point_sample;
	createCLI
		.add_argument("--pointsample")
		.help("Alias of --flag POINT_SAMPLE, added for vtex2 compatibility.")
		.flag()
		.store_into(point_sample);

	bool trilinear;
	createCLI
		.add_argument("--trilinear")
		.help("Alias of --flag TRILINEAR, added for vtex2 compatibility.")
		.flag()
		.store_into(trilinear);

	bool anisotropic;
	createCLI
		.add_argument("--aniso")
		.help("Alias of --flag ANISOTROPIC, added for vtex2 compatibility.")
		.flag()
		.store_into(anisotropic);

	bool normal;
	createCLI
		.add_argument("--normal")
		.help("Alias of --flag NORMAL, added for vtex2 compatibility.")
		.flag()
		.store_into(normal);

	bool ssbump;
	createCLI
		.add_argument("--ssbump")
		.help("Alias of --flag SSBUMP, added for vtex2 compatibility.")
		.flag()
		.store_into(ssbump);

	std::string particleSheetResource;
	createCLI
		.add_argument("--particle-sheet-resource")
		.metavar("PATH")
		.help("Set the particle sheet resource. Path should point to a valid particle sheet file.")
		.store_into(particleSheetResource);

	int crcResource;
	createCLI
		.add_argument("--crc-resource")
		.metavar("CRC")
		.help("Set the CRC resource.")
		.scan<'d', int>()
		.store_into(crcResource);

	std::string lodResource;
	createCLI
		.add_argument("--lod-resource")
		.metavar("U.V")
		.help("Set the LOD resource. U and V values should be separated by a period.")
		.store_into(lodResource);

	int ts0Resource;
	createCLI
		.add_argument("--ts0-resource")
		.metavar("COMBINED_FLAGS")
		.help("Set the TS0 (extended flags) resource. You'll have to do the math to combine the flags"
		      " into one integer yourself.")
		.scan<'d', int>()
		.store_into(ts0Resource);

	std::string kvdResource;
	createCLI
		.add_argument("--kvd-resource")
		.metavar("PATH")
		.help("Set the nonstandard KVD (KeyValues Data) resource. Path should point to a text file.")
		.store_into(kvdResource);

	std::string hotspotDataResource;
	createCLI
		.add_argument("--hotspot-data-resource")
		.metavar("PATH")
		.help("Set the hotspot data resource. Path should point to a valid HOT file.")
		.store_into(hotspotDataResource);

	std::vector<std::string> hotspotRect;
	createCLI
		.add_argument("--hotspot-rect")
		.metavar("X1 Y1 X2 Y2 HOTSPOT_RECT_FLAGS")
		.help("Adds a rect to the hotspot data resource. The 4 input values are in pixel coordinates, and should"
		      " not have a decimal point or be less than zero. Flags should be separated by a comma with no spaces"
		      " (or use NONE if no flags are present). The resource is added and initialized to default values if"
		      " not present beforehand.")
		.nargs(5)
		.append()
		.store_into(hotspotRect);

	//endregion

	//region Edit Mode Arguments

	auto& editCLI = cli.add_group(R"("edit" mode)");

	std::string setVersion;
	editCLI
		.add_argument("--set-version")
		.metavar("X.Y")
		.help("Set the version.")
		.choices("7.0", "7.1", "7.2", "7.3", "7.4", "7.5", "7.6")
		.store_into(setVersion);

	std::string setFormat;
	editCLI
		.add_argument("--set-format")
		.metavar("IMAGE_FORMAT")
		.help("Set the image format. Keep in mind converting to a lossy format like DXTn means irreversibly losing"
		      " information. Recommended to pair this with the recompute transparency flags argument.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::ImageFormat>, "IMAGE_FORMAT"))
		.store_into(setFormat);

	int setWidth = 0;
	editCLI
		.add_argument("--set-width")
		.metavar("WIDTH")
		.help("Set the largest mip's width. Ignores power of two resize rule. Keep in mind this operation will"
		      " result in information loss, especially if the texture is using a lossy format. Recommended to pair this"
		      " with the recompute mips argument if the input texture is using a lossless format.")
		.scan<'d', int>()
		.store_into(setWidth);

	int setHeight = 0;
	editCLI
		.add_argument("--set-height")
		.metavar("HEIGHT")
		.help("Set the largest mip's height. Ignores power of two resize rule. Keep in mind this operation will"
		      " result in information loss, especially if the texture is using a lossy format. Recommended to pair this"
		      " with the recompute mips argument if the input texture is using a lossless format.")
		.scan<'d', int>()
		.store_into(setHeight);

	std::string editFilter{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeFilter::KAISER)};
	editCLI
		.add_argument("--edit-filter")
		.metavar("RESIZE_FILTER")
		.help("Use this resize filter for all resizing operations that accept a filter parameter,"
		      " including mipmap generation.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::ImageConversion::ResizeFilter>, "RESIZE_FILTER"))
		.default_value(editFilter).store_into(editFilter);

	std::vector<std::string> addFlags;
	editCLI
		.add_argument("--add-flag")
		.metavar("FLAG")
		.help("Flags to add. ENVMAP and NO_MIP flags are ignored.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::VTF::Flags>, "FLAG"))
		.append()
		.store_into(addFlags);

	std::vector<std::string> removeFlags;
	editCLI
		.add_argument("--remove-flag")
		.metavar("FLAG")
		.help("Flags to remove. ENVMAP and NO_MIP flags are ignored.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::VTF::Flags>, "FLAG"))
		.append()
		.store_into(removeFlags);

	bool recomputeTransparencyFlags;
	editCLI
		.add_argument("--recompute-transparency-flags")
		.help("Recomputes transparency flags based on the image format.")
		.flag()
		.store_into(recomputeTransparencyFlags);

	bool recomputeMips;
	editCLI
		.add_argument("--recompute-mips")
		.help("Recomputes mipmaps with the specified edit resize filter.")
		.flag()
		.store_into(recomputeMips);

	bool removeMips;
	editCLI
		.add_argument("--remove-mips")
		.help("Remove mipmaps. If recompute mips is specified, this argument is ignored.")
		.flag()
		.store_into(removeMips);

	bool recomputeThumbnail;
	editCLI
		.add_argument("--recompute-thumbnail")
		.help("Recompute the thumbnail.")
		.flag()
		.store_into(recomputeThumbnail);

	bool removeThumbnail;
	editCLI
		.add_argument("--remove-thumbnail")
		.help("Remove the thumbnail. If recompute thumbnail is specified, this argument is ignored.")
		.flag()
		.store_into(removeThumbnail);

	bool recomputeReflectivity;
	editCLI
		.add_argument("--recompute-reflectivity")
		.help("Recompute the reflectivity vector.")
		.flag()
		.store_into(recomputeReflectivity);

	std::string setPlatform;
	editCLI
		.add_argument("--set-platform")
		.metavar("PLATFORM")
		.help("Set the VTF platform.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::VTF::Platform>, "PLATFORM"))
		.store_into(setPlatform);

	std::string setCompressionMethod;
	editCLI
		.add_argument("--set-compression-method")
		.metavar("COMPRESSION_METHOD")
		.help("Set the compression method. Deflate is supported on all Strata Source games for VTF v7.6."
		      " Zstd is supported on all Strata Source games for VTF v7.6 besides Portal: Revolution."
		      " LZMA is supported for console VTFs.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::CompressionMethod>, "COMPRESSION_METHOD"))
		.store_into(setCompressionMethod);

	int setCompressionLevel;
	editCLI
		.add_argument("--set-compression-level")
		.metavar("LEVEL")
		.help("Set the compression level. -1 to 9 for Deflate and LZMA, -1 to 22 for Zstd.")
		.scan<'d', int>()
		.store_into(setCompressionLevel);

	int setStartFrame;
	editCLI
		.add_argument("--set-start-frame")
		.metavar("FRAME_INDEX")
		.help("Set the start frame.")
		.scan<'d', int>()
		.store_into(setStartFrame);

	float setBumpMapScale;
	editCLI
		.add_argument("--set-bumpmap-scale")
		.metavar("SCALE")
		.help("Set the bumpmap scale. It can have a decimal point.")
		.scan<'g', float>()
		.store_into(setBumpMapScale);

	int setConsoleMipScale = 0;
	createCLI
		.add_argument("--set-console-mip-scale")
		.metavar("SCALE")
		.help("Set the mip scale. Only has effect on console platforms. See --console-mip-scale for more information.")
		.scan<'d', int>()
		.default_value(setConsoleMipScale).store_into(setConsoleMipScale);

	std::string setParticleSheetResource;
	editCLI
		.add_argument("--set-particle-sheet-resource")
		.metavar("PATH")
		.help("Set the particle sheet resource. Path should point to a valid particle sheet file.")
		.store_into(setParticleSheetResource);

	bool removeParticleSheetResource;
	editCLI
		.add_argument("--remove-particle-sheet-resource")
		.help("Remove the particle sheet resource. If set particle sheet resource is specified,"
		      " this argument is ignored.")
		.flag()
		.store_into(removeParticleSheetResource);

	int setCRCResource;
	editCLI
		.add_argument("--set-crc-resource")
		.metavar("CRC")
		.help("Set the CRC resource.")
		.scan<'d', int>()
		.store_into(setCRCResource);

	bool removeCRCResource;
	editCLI
		.add_argument("--remove-crc-resource")
		.help("Remove the CRC resource. If set CRC resource is specified, this argument is ignored.")
		.flag()
		.store_into(removeCRCResource);

	std::string setLODResource;
	editCLI
		.add_argument("--set-lod-resource")
		.metavar("U.V")
		.help("Set the LOD resource. U and V values should be separated by a period.")
		.store_into(setLODResource);

	bool removeLODResource;
	editCLI
		.add_argument("--remove-lod-resource")
		.help("Remove the LOD resource. If set LOD resource is specified, this argument is ignored.")
		.flag()
		.store_into(removeLODResource);

	int setTS0Resource;
	editCLI
		.add_argument("--set-ts0-resource")
		.metavar("COMBINED_FLAGS")
		.help("Set the TS0 (extended flags) resource. You'll have to do the math to combine the flags"
		      " into one integer yourself.")
		.scan<'d', int>()
		.store_into(setTS0Resource);

	bool removeTS0Resource;
	editCLI
		.add_argument("--remove-ts0-resource")
		.help("Remove the TS0 (extended flags) resource. If set TS0 resource is specified, this argument is ignored.")
		.flag()
		.store_into(removeTS0Resource);

	std::string setKVDResource;
	editCLI
		.add_argument("--set-kvd-resource")
		.metavar("PATH")
		.help("Set the nonstandard KVD (KeyValues Data) resource. Path should point to a text file.")
		.store_into(setKVDResource);

	bool removeKVDResource;
	editCLI
		.add_argument("--remove-kvd-resource")
		.help("Remove the nonstandard KVD (KeyValues Data) resource. If set KVD resource is specified,"
		      " this argument is ignored.")
		.flag()
		.store_into(removeKVDResource);

	std::string setHotspotDataResource;
	editCLI
		.add_argument("--set-hotspot-data-resource")
		.metavar("PATH")
		.help("Set the hotspot data resource. Path should point to a valid HOT file.")
		.store_into(setHotspotDataResource);

	bool removeHotspotDataResource;
	editCLI
		.add_argument("--remove-hotspot-data-resource")
		.help("Remove the hotspot data resource. If set HOT resource is specified, this argument is ignored.")
		.flag()
		.store_into(removeHotspotDataResource);

	std::vector<std::string> addHotspotRect;
	editCLI
		.add_argument("--add-hotspot-rect")
		.metavar("X1 Y1 X2 Y2 HOTSPOT_RECT_FLAGS")
		.help("Adds a rect to the hotspot data resource. The 4 input values are in pixel coordinates, and should"
		      " not have a decimal point or be less than zero. Flags should be separated by a comma with no spaces"
		      " (or use NONE if no flags are present). The resource is added and initialized to default values if"
		      " not present beforehand.")
		.nargs(5)
		.append()
		.store_into(addHotspotRect);

	//endregion

	//region Info Mode Arguments

	auto& infoCLI = cli.add_group(R"("info" mode)");

	std::string infoOutputMode{"human"};
	infoCLI
		.add_argument("--info-output-mode")
		.help(R"(The mode to output information in. Can be "human" or "kv1".)")
		.choices("human", "kv1")
		.default_value(infoOutputMode).store_into(infoOutputMode);

	bool infoSkipResources;
	infoCLI
		.add_argument("--info-skip-resources")
		.help("Do not print resource internals.")
		.flag()
		.store_into(infoSkipResources);

	//endregion

	//region Extract Mode Arguments

	auto& extractCLI = cli.add_group(R"("extract" mode)");

	std::string extractFormat{not_magic_enum::enum_name(vtfpp::ImageConversion::FileFormat::DEFAULT)};
	extractCLI
		.add_argument("--extract-format")
		.metavar("FILE_FORMAT")
		.help("Output file format.")
		.action(std::bind_front(&::enumValueValidityCheck<vtfpp::ImageConversion::FileFormat>, "FILE_FORMAT"))
		.default_value(extractFormat).store_into(extractFormat);

	int extractMip = 0;
	editCLI
		.add_argument("--extract-mip")
		.metavar("MIP")
		.help("Set the mip to extract. Overridden by --extract-all-mips.")
		.scan<'d', int>()
		.default_value(extractMip).store_into(extractMip);

	bool extractAllMips;
	extractCLI
		.add_argument("--extract-all-mips")
		.help("Extract all mips. Overridden by --extract-all.")
		.flag()
		.store_into(extractAllMips);

	int extractFrame = 0;
	editCLI
		.add_argument("--extract-frame")
		.metavar("FRAME")
		.help("Set the frame to extract. Overridden by --extract-all-frames.")
		.scan<'d', int>()
		.default_value(0).store_into(extractFrame);

	bool extractAllFrames;
	extractCLI
		.add_argument("--extract-all-frames")
		.help("Extract all frames. Overridden by --extract-all.")
		.flag()
		.store_into(extractAllFrames);

	int extractFace = 0;
	editCLI
		.add_argument("--extract-face")
		.metavar("FACE")
		.help("Set the face to extract. Overridden by --extract-all-faces.")
		.scan<'d', int>()
		.default_value(extractFace).store_into(extractFace);

	bool extractAllFaces;
	extractCLI
		.add_argument("--extract-all-faces")
		.help("Extract all faces. Overridden by --extract-all.")
		.flag()
		.store_into(extractAllFaces);

	int extractSlice = 0;
	editCLI
		.add_argument("--extract-slices")
		.metavar("SLICE")
		.help("Set the slice to extract. Overridden by --extract-all-slices.")
		.scan<'d', int>()
		.default_value(extractSlice).store_into(extractSlice);

	bool extractAllSlices;
	extractCLI
		.add_argument("--extract-all-slices")
		.help("Extract all slices. Overridden by --extract-all.")
		.flag()
		.store_into(extractAllSlices);

	bool extractAll;
	extractCLI
		.add_argument("--extract-all")
		.help("Extract all mips, frames, faces, and slices.")
		.flag()
		.store_into(extractAll);

	//endregion

	//region Program Info

	std::string enumInfo = "Enumerations:\n\n";
	const auto addEnumInfo = [&enumInfo]<typename E>(std::string_view enumName) {
		enumInfo += enumName;
		enumInfo += "\n";
		for (auto name : not_magic_enum::enum_names<E>()) {
			enumInfo += " • ";
			enumInfo += name;
			enumInfo += '\n';
		}
		enumInfo += '\n';
	};
	addEnumInfo.operator()<vtfpp::ImageFormat>("IMAGE_FORMAT");
	addEnumInfo.operator()<vtfpp::VTF::Flags>("FLAG");
	addEnumInfo.operator()<vtfpp::HOT::Rect::Flags>("HOTSPOT_RECT_FLAGS");
	addEnumInfo.operator()<vtfpp::VTF::Platform>("PLATFORM");
	addEnumInfo.operator()<vtfpp::ImageConversion::FileFormat>("FILE_FORMAT");
	addEnumInfo.operator()<vtfpp::ImageConversion::ResizeFilter>("RESIZE_FILTER");
	addEnumInfo.operator()<vtfpp::ImageConversion::ResizeMethod>("RESIZE_METHOD");
	addEnumInfo.operator()<vtfpp::CompressionMethod>("COMPRESSION_METHOD");

	static constexpr std::string_view PROGRAM_DETAILS{
		"Program details:\n\n"
		PROJECT_NAME_PRETTY " — version v" PROJECT_VERSION " — created by " PROJECT_ORGANIZATION_NAME " — licensed under MIT\n\n"
		"Sample usage:\n"
		" • maretf create input.png --version 7.4 --format UNCHANGED --filter KAISER\n"
		" • maretf edit input.360.vtf -o input.vtf --set-platform PC --set-version 7.6 --recompute-mips\n"
		" • maretf info input.vtf\n"
		"See the project README for more information.\n\n"
		"Want to report a bug or request a feature? Make an issue at " PROJECT_HOMEPAGE_URL "/issues"
	};

	cli.add_epilog(enumInfo + std::string{PROGRAM_DETAILS});

	//endregion

	try {
		cli.parse_args(argc, argv);

		if (!::runningInTTY()) {
			overwrite = true;
			quiet = true;
			noPrettyFormatting = true;
		}

		if (quiet && !verbose) {
			tfout_t::QUIET = true;
			tferr_t::QUIET = true;
		} else {
			quiet = false;
		}

		// Pretty formatting colors
		std::string_view END;
		std::string_view RED;
		std::string_view GREEN;
		std::string_view CYAN;
		std::string_view BOLD;
		if (!noPrettyFormatting) {
			END   = "\033[0m";
			RED   = "\033[0;31m";
			GREEN = "\033[0;32m";
			CYAN  = "\033[0;36m";
			BOLD  = "\033[1m";

#ifdef _WIN32
			// Enable ANSI color codes on older Windows builds
			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			DWORD dwMode = 0;
			GetConsoleMode(hStdOut, &dwMode);
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode(hStdOut, dwMode);
#endif
		}

		static constexpr auto DIR_OPTIONS = std::filesystem::directory_options::follow_directory_symlink | std::filesystem::directory_options::skip_permission_denied;

		const auto checkFileDoesntExist = [&](const std::string& currentOutputPath, bool& shouldRet) -> int {
			shouldRet = true;
			const bool exists = std::filesystem::exists(currentOutputPath);
			if (exists && !std::filesystem::is_regular_file(currentOutputPath)) {
				tferr << "Output path at " << BOLD << currentOutputPath << END << " must not be a directory!" << tfendl;
				return EXIT_FAILURE;
			}
			if (exists && noOverwrite) {
				tfout << "Output file at " << BOLD << currentOutputPath << END << " already exists, leaving unmodified." << tfendl;
				return EXIT_SUCCESS;
			}
			if (exists && !overwrite) {
				std::string in;
				while (in.empty() || (!in.starts_with('y') && !in.starts_with('Y') && !in.starts_with('n') && !in.starts_with('N'))) {
					tfout << "Output file at " << BOLD << currentOutputPath << END << " already exists.\nOverwrite? (" << RED << 'y' << END << '/' << GREEN << 'N' << END << ") ";
					std::cin >> in;
				}
				if (in.empty() || in.starts_with('n') || in.starts_with('N')) {
					tfout << "Output file at " << BOLD << currentOutputPath << END << " already exists, leaving unmodified." << tfendl;
					return EXIT_SUCCESS;
				}
			} else if (exists) {
				tfout << "Output file at " << BOLD << currentOutputPath << END << " already exists, overwriting..." << tfendl;
			}
			shouldRet = false;
			return EXIT_SUCCESS;
		};

		const auto handleSettingResourcesForVTF = [&](vtfpp::VTF& vtf, bool editMode) {
			// Modify particle sheet resource
			if ((editMode && cli.is_used("--set-particle-sheet-resource")) || (!editMode && cli.is_used("--particle-sheet-resource"))) {
				try {
					const vtfpp::SHT sht{editMode ? setParticleSheetResource : particleSheetResource};
					if (!sht) {
						throw std::overflow_error{""};
					}
					vtf.setParticleSheetResource(sht);
				} catch (const std::overflow_error&) {
					tferr << "Failed to parse specified file at " << BOLD << (editMode ? setParticleSheetResource : particleSheetResource) << END << " for particle sheet resource! Check the file exists and has a .sht extension." << tfendl;
				}
			} else if (editMode && removeParticleSheetResource) {
				vtf.removeParticleSheetResource();
			}

			// Modify CRC resource
			if ((editMode && cli.is_used("--set-crc-resource")) || (!editMode && cli.is_used("--crc-resource"))) {
				vtf.setCRCResource(static_cast<uint32_t>(editMode ? setCRCResource : crcResource));
			} else if (editMode && removeCRCResource) {
				vtf.removeCRCResource();
			}

			// Modify LOD resource
			if ((editMode && cli.is_used("--set-lod-resource")) || (!editMode && cli.is_used("--lod-resource"))) {
				uint8_t setU, setV;
				const auto uv = sourcepp::string::split(editMode ? setLODResource : lodResource, '.');
				sourcepp::string::toInt(uv[0], setU);
				sourcepp::string::toInt(uv[1], setV);
				vtf.setLODResource(setU, setV);
			} else if (editMode && removeLODResource) {
				vtf.removeLODResource();
			}

			// Modify TS0 resource
			if ((editMode && cli.is_used("--set-ts0-resource")) || (!editMode && cli.is_used("--ts0-resource"))) {
				vtf.setExtendedFlagsResource(static_cast<uint32_t>(editMode ? setTS0Resource : ts0Resource));
			} else if (editMode && removeTS0Resource) {
				vtf.removeExtendedFlagsResource();
			}

			// Modify KVD resource
			if ((editMode && cli.is_used("--set-kvd-resource")) || (!editMode && cli.is_used("--kvd-resource"))) {
				if (const auto txt = sourcepp::fs::readFileText(editMode ? setKVDResource : kvdResource); txt.empty()) {
					tferr << "Failed to read contents of specified file at " << BOLD << (editMode ? setKVDResource : kvdResource) << END << " for KVD (KeyValues Data) resource! Check the file exists and is not empty." << tfendl;
				} else {
					vtf.setKeyValuesDataResource(txt);
				}
			} else if (editMode && removeKVDResource) {
				vtf.removeKeyValuesDataResource();
			}

			// Modify HOT resource
			if ((editMode && cli.is_used("--set-hotspot-data-resource")) || (!editMode && cli.is_used("--hotspot-data-resource"))) {
				try {
					const vtfpp::HOT hot{editMode ? setHotspotDataResource : hotspotDataResource};
					if (!hot) {
						throw std::overflow_error{""};
					}
					vtf.setHotspotDataResource(hot);
				} catch (const std::overflow_error&) {
					tferr << "Failed to parse specified file at " << BOLD << (editMode ? setHotspotDataResource : hotspotDataResource) << END << " for hotspot data resource! Check the file exists." << tfendl;
				}
			} else if (editMode && removeHotspotDataResource) {
				vtf.removeHotspotDataResource();
			} else if ((editMode && !addHotspotRect.empty()) || (!editMode && !hotspotRect.empty())) {
				vtfpp::HOT hot;
				if (editMode) {
					if (const auto hotspotDataResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_HOTSPOT_DATA)) {
						hot = hotspotDataResourcePtr->getDataAsHotspotData();
					}
				}
				const auto& rects = editMode ? addHotspotRect : hotspotRect;
				for (int i = 0; i < rects.size(); i += 5) {
					vtfpp::HOT::Rect rect{};

					sourcepp::string::toInt(rects[i + 0], rect.x1);
					rect.x1 = std::clamp<uint16_t>(rect.x1, 0, vtf.getWidth());
					sourcepp::string::toInt(rects[i + 2], rect.x2);
					rect.x2 = std::clamp<uint16_t>(rect.x2, 0, vtf.getWidth());
					if (rect.x1 > rect.x2) std::swap(rect.x1, rect.x2);

					sourcepp::string::toInt(rects[i + 1], rect.y1);
					rect.y1 = std::clamp<uint16_t>(rect.y1, 0, vtf.getHeight());
					sourcepp::string::toInt(rects[i + 3], rect.y2);
					rect.y2 = std::clamp<uint16_t>(rect.y2, 0, vtf.getHeight());
					if (rect.y1 > rect.y2) std::swap(rect.y1, rect.y2);

					if (!sourcepp::string::iequals(rects[i + 4], "NONE")) {
						for (const auto& hotspotFlagStr : sourcepp::string::split(rects[i + 4], ',')) {
							if (auto value = not_magic_enum::enum_cast<vtfpp::HOT::Rect::Flags>(sourcepp::string::trim(hotspotFlagStr))) {
								rect.flags |= *value;
							}
						}
					}

					hot.getRects().push_back(rect);
				}
				vtf.setHotspotDataResource(hot);
			}
		};

		if (mode == "create" || mode == "convert") {
			const auto create = [&](const std::string& currentInputPath) {
				// Check output path
				if (outputPath.empty()) {
					outputPath = ::getOutputPathForInput(currentInputPath, *not_magic_enum::enum_cast<vtfpp::VTF::Platform>(platform));
				}
				{
					bool checkFileShouldRet;
					int out = checkFileDoesntExist(outputPath, checkFileShouldRet);
					if (checkFileShouldRet) {
						return out;
					}
				}

				// Start to set up options
				vtfpp::VTF::CreationOptions options;

				// Set version
				if (version.size() != 3) {
					throw std::runtime_error{"Invalid version!"};
				}
				uint32_t majorVersion = 0;
				sourcepp::string::toInt(std::string_view{&version[0], 1}, majorVersion);
				if (majorVersion != 7) {
					throw std::runtime_error{"Invalid version!"};
				}
				sourcepp::string::toInt(std::string_view{&version[2], 1}, options.version);

				// Set format
				if (format == "UNCHANGED") {
					options.outputFormat = vtfpp::VTF::FORMAT_UNCHANGED;
				} else if (format == "DEFAULT") {
					options.outputFormat = vtfpp::VTF::FORMAT_DEFAULT;
				} else {
					options.outputFormat = *not_magic_enum::enum_cast<vtfpp::ImageFormat>(format);
					if (options.version == 6 && !!vtfpp::ImageFormatDetails::red(options.outputFormat) + !!vtfpp::ImageFormatDetails::green(options.outputFormat) + !!vtfpp::ImageFormatDetails::blue(options.outputFormat) + !!vtfpp::ImageFormatDetails::alpha(options.outputFormat) == 3) {
						tfout << RED << "Formats with 3 channels are not supported on DX11 and will be converted to a format with 4 channels at runtime. Consider using a compressed format such as BC7, or a format with 4 channels such as RGBA8888 or BGRX8888." << END << tfendl;
					}
				}

				// Set compression quality
				options.compressedFormatQuality = compressedFormatQuality;

				// Set filter
				options.filter = *not_magic_enum::enum_cast<vtfpp::ImageConversion::ResizeFilter>(filter);

				// Set flags
				for (const auto& flag : flags) {
					options.flags |= *not_magic_enum::enum_cast<vtfpp::VTF::Flags>(flag);
				}
				static constexpr auto addSRGBFlag = [](vtfpp::VTF::CreationOptions& opts) {
					opts.flags |= opts.version < 4 ? 0 : opts.version > 4 ? static_cast<uint32_t>(vtfpp::VTF::FLAG_V5_SRGB) : static_cast<uint32_t>(vtfpp::VTF::FLAG_V4_SRGB);
				};
				if (srgb) {
					addSRGBFlag(options);
				}
				if (clamp_s) {
					options.flags |= vtfpp::VTF::FLAG_V0_CLAMP_S;
				}
				if (clamp_t) {
					options.flags |= vtfpp::VTF::FLAG_V0_CLAMP_T;
				}
				if (clamp_u && options.version >= 2) {
					options.flags |= vtfpp::VTF::FLAG_V2_CLAMP_U;
				}
				if (point_sample) {
					options.flags |= vtfpp::VTF::FLAG_V0_POINT_SAMPLE;
				}
				if (trilinear) {
					options.flags |= vtfpp::VTF::FLAG_V0_TRILINEAR;
				}
				if (anisotropic) {
					options.flags |= vtfpp::VTF::FLAG_V0_ANISOTROPIC;
				}
				if (normal) {
					options.flags |= vtfpp::VTF::FLAG_V0_NORMAL;
				}
				if (ssbump && options.version >= 3) {
					options.flags |= vtfpp::VTF::FLAG_V3_SSBUMP;
				}

				// Set default flags or animation state based on input filename
				int frameNumberStart = 0;
				int frameNumberCount = 0;
				if (auto inputStem = std::filesystem::path{currentInputPath}.stem().string(); inputStem.ends_with("_color") || inputStem.ends_with("-color") || inputStem.ends_with("_colour") || inputStem.ends_with("-colour") || inputStem.ends_with("_albedo") || inputStem.ends_with("-albedo") || inputStem.ends_with("_diffuse") || inputStem.ends_with("-diffuse")) {
					addSRGBFlag(options);
				} else if (inputStem.ends_with("_normal") || inputStem.ends_with("-normal") || inputStem.ends_with("_norm") || inputStem.ends_with("-norm")) {
					options.flags |= vtfpp::VTF::FLAG_V0_NORMAL;
				} else if (inputStem.ends_with("_ssbump") || inputStem.ends_with("-ssbump")) {
					if (options.version >= 3) {
						options.flags |= vtfpp::VTF::FLAG_V3_SSBUMP;
					}
				} else if (animatedFrames && !hdri && inputStem.size() >= 2 && sourcepp::string::matches(inputStem.substr(inputStem.size() - 2, inputStem.size()), "%d%d")) {
					// At least 2 digits to avoid false positives
					frameNumberCount = 2;
					inputStem.pop_back();
					inputStem.pop_back();
					while (std::isdigit(inputStem.back())) {
						frameNumberCount++;
						inputStem.pop_back();
					}
					{
						// Set start number, might not always be zero
						auto temp = std::filesystem::path{currentInputPath}.stem().string();
						temp = temp.substr(temp.size() - frameNumberCount, temp.size());
						sourcepp::string::toInt(temp, frameNumberStart);
					}
					options.initialFrameCount = 1;
					while (std::filesystem::exists(std::filesystem::path{currentInputPath}.parent_path() / (inputStem + sourcepp::string::padNumber(options.initialFrameCount, frameNumberCount) + std::filesystem::path{currentInputPath}.extension().string()))) {
						options.initialFrameCount++;
					}
				}

				// Set default transparency flags
				options.computeTransparencyFlags = !noTransparencyFlags;

				// Set mipmap generation
				options.computeMips = !noMips;

				// Set thumbnail generation
				options.computeThumbnail = !noThumbnail;

				// Set platform
				options.platform = *not_magic_enum::enum_cast<vtfpp::VTF::Platform>(platform);

				// Set compression method
				options.compressionMethod = *not_magic_enum::enum_cast<vtfpp::CompressionMethod>(compressionMethod);

				// Set compression level
				if (compressionLevel < -1) {
					options.compressionLevel = -1;
					tfout << "Compression level range is between " << CYAN << "-1" << END << " and " << CYAN << '9' << END << '/' << CYAN << "22" << END << " (depending on the compression method). Setting compression level to " << CYAN << "-1" << END << "..." << tfendl;
				} else if ((options.compressionMethod == vtfpp::CompressionMethod::DEFLATE || options.compressionMethod == vtfpp::CompressionMethod::CONSOLE_LZMA) && compressionLevel > 9) {
					options.compressionLevel = 9;
					tfout << "Compression level range is between " << CYAN << "-1" << END << " and " << CYAN << '9' << END << " for Deflate and LZMA. Setting compression level to " << CYAN << '9' << END << "..." << tfendl;
				} else if (options.compressionMethod == vtfpp::CompressionMethod::ZSTD && compressionLevel > 22) {
					options.compressionLevel = 22;
					tfout << "Compression level range is between " << CYAN << "-1" << END << " and " << CYAN << "22" << END << " for Zstd. Setting compression level to " << CYAN << "22" << END << "..." << tfendl;
				} else {
					options.compressionLevel = static_cast<int16_t>(compressionLevel);
				}

				// Set start frame
				options.startFrame = static_cast<uint16_t>(startFrame);

				// Set bumpmap scale
				options.bumpMapScale = bumpMapScale;

				// Set resize methods
				options.widthResizeMethod = *not_magic_enum::enum_cast<vtfpp::ImageConversion::ResizeMethod>(cli.is_used("--width-resize-method") ? widthResizeMethod : resizeMethod);
				options.heightResizeMethod = *not_magic_enum::enum_cast<vtfpp::ImageConversion::ResizeMethod>(cli.is_used("--height-resize-method") ? heightResizeMethod : resizeMethod);

				// Set gamma correction
				if (gammaCorrection) {
					options.gammaCorrection = gammaCorrectionAmount;
				}

				// Set inversion of green channel
				options.invertGreenChannel = invertGreenChannel || invertGreenChannelAlt;

				// Set console mip scale
				options.consoleMipScale = consoleMipScale;

				// Start stopwatch
				::ElapsedTime stopwatch;

				// Function to bake the VTF
				const auto bake = [noPrettyFormatting, &END, &CYAN, &BOLD, &outputPath, &currentInputPath, &stopwatch](const vtfpp::VTF& vtf, std::string_view image) {
					const auto vtfData = vtf.bake();
					if (vtfData.empty()) {
						tferr << "Failed to TF input " << image << " at " << BOLD << currentInputPath << END << "." << tfendl;
						return EXIT_FAILURE;
					}
					const auto elapsed = stopwatch.get().count();
					if (!sourcepp::fs::writeFileBuffer(outputPath, vtfData)) {
						tferr << "Failed to write to " << BOLD << outputPath << END << "." << tfendl;
						return EXIT_FAILURE;
					}
					tfout << BOLD << currentInputPath << END << " was TF'ed in " << CYAN << elapsed << "ms" << END << (noPrettyFormatting ? "" : " 💖") << tfendl;
					return EXIT_SUCCESS;
				};

				// Special case for HDRI -> cubemap conversion
				if (hdri) {
					options.isCubeMap = true;

					// Compute mips if desired after the cubemap is constructed
					options.computeMips = false;

					// Another time-saver
					vtfpp::ImageFormat outputFormatBackup = options.outputFormat;
					options.outputFormat = vtfpp::VTF::FORMAT_UNCHANGED;

					// Load image
					vtfpp::ImageFormat hdriFormat;
					int hdriWidth, hdriHeight, hdriFrameCount;
					std::vector<std::byte> hdriData = vtfpp::ImageConversion::convertFileToImageData(sourcepp::fs::readFileBuffer(currentInputPath), hdriFormat, hdriWidth, hdriHeight, hdriFrameCount);
					if (hdriData.empty() || !hdriWidth || !hdriHeight || !hdriFrameCount) {
						tferr << "Failed to TF input HDRI at " << BOLD << currentInputPath << END << ". Is it a supported format?" << tfendl;
						return EXIT_FAILURE;
					}

					// Split HDRI
					std::array<std::vector<std::byte>, 6> cubemapFaces = vtfpp::ImageConversion::convertHDRIToCubeMap(hdriData, hdriFormat, hdriWidth, hdriHeight, 0, !hdriNoFilter);
					if (cubemapFaces[0].empty() || cubemapFaces[1].empty() || cubemapFaces[2].empty() || cubemapFaces[3].empty() || cubemapFaces[4].empty() || cubemapFaces[5].empty()) {
						tferr << "Failed to TF input HDRI at " << BOLD << currentInputPath << END << ". Couldn't split up the HDRI!" << tfendl;
						return EXIT_FAILURE;
					}

					// Create VTF
					vtfpp::VTF vtf = vtfpp::VTF::create(hdriFormat, hdriHeight, hdriHeight, options);
					vtf.setFaceCount(true);

					// Set faces
					for (int face = 0; face < 6; face++) {
						if (!vtf.setImage(cubemapFaces[face], hdriFormat, hdriHeight, hdriHeight, options.filter, 0, 0, face)) {
							tferr << "Failed to TF input HDRI at " << BOLD << currentInputPath << END << ". Face " << CYAN << face << END << " could not be set!" << tfendl;
							return EXIT_FAILURE;
						}
					}

					// Now compute mips after faces exist
					if (!noMips) {
						vtf.computeMips(options.filter);
					}

					// And now convert to output format
					if (outputFormatBackup == vtfpp::VTF::FORMAT_DEFAULT) {
						vtf.setFormat(vtfpp::VTF::getDefaultCompressedFormat(vtf.getFormat(), vtf.getVersion(), vtf.getFaceCount() > 1), vtfpp::ImageConversion::ResizeFilter::DEFAULT, compressedFormatQuality);
					} else if (outputFormatBackup != vtfpp::VTF::FORMAT_UNCHANGED) {
						vtf.setFormat(outputFormatBackup, vtfpp::ImageConversion::ResizeFilter::DEFAULT, compressedFormatQuality);
					}

					// Set resources
					handleSettingResourcesForVTF(vtf, false);

					// Bake VTF
					return bake(vtf, "HDRI");
				}

				// Special case for animated VTFs
				if (options.initialFrameCount > 1) {
					// Compute mips later
					options.computeMips = false;

					// Another time-saver
					vtfpp::ImageFormat outputFormatBackup = options.outputFormat;
					options.outputFormat = vtfpp::VTF::FORMAT_UNCHANGED;

					// Create initial VTF
					auto vtf = vtfpp::VTF::create(currentInputPath, options);
					if (!vtf) {
						tferr << "Failed to TF input animation at " << BOLD << currentInputPath << END << ". Is it a supported format?" << tfendl;
						return EXIT_FAILURE;
					}

					// Set frames
					auto currentInputPathBasePath = std::filesystem::path{currentInputPath}.stem();
					auto currentInputPathBase = currentInputPathBasePath.string();
					currentInputPathBase = currentInputPathBase.substr(0, currentInputPathBase.size() - currentInputPathBasePath.extension().string().size() - frameNumberCount);
					std::unique_ptr<indicators::ProgressBar> bar;
					if (!noPrettyFormatting && !quiet) {
						bar = std::make_unique<indicators::ProgressBar>(
							indicators::option::PostfixText{"Adding frames..."},
							indicators::option::ShowPercentage{true},
							indicators::option::MaxProgress{options.initialFrameCount}
						);
					}
					for (int frame = frameNumberStart; frame < options.initialFrameCount; frame++) {
						if (!vtf.setImage((std::filesystem::path{currentInputPath}.parent_path() / (currentInputPathBase + sourcepp::string::padNumber(frame, frameNumberCount) + std::filesystem::path{currentInputPath}.extension().string())).string(), options.filter, 0, frame)) {
							tferr << "Failed to TF input frame at " << BOLD << currentInputPath << END << ". Frame " << CYAN << frame << END << " could not be set!" << tfendl;
							return EXIT_FAILURE;
						}
						if (bar) {
							bar->tick();
						}
					}
					if (bar) {
						bar->mark_as_completed();
					}

					// Now compute mips after frames exist
					if (!noMips) {
						vtf.computeMips(options.filter);
					}

					// And now convert to output format
					if (outputFormatBackup == vtfpp::VTF::FORMAT_DEFAULT) {
						vtf.setFormat(vtfpp::VTF::getDefaultCompressedFormat(vtf.getFormat(), vtf.getVersion(), vtf.getFaceCount() > 1), vtfpp::ImageConversion::ResizeFilter::DEFAULT, compressedFormatQuality);
					} else if (outputFormatBackup != vtfpp::VTF::FORMAT_UNCHANGED) {
						vtf.setFormat(outputFormatBackup, vtfpp::ImageConversion::ResizeFilter::DEFAULT, compressedFormatQuality);
					}

					// Set resources
					handleSettingResourcesForVTF(vtf, false);

					// Bake VTF
					return bake(vtf, "animation");
				}

				// Create VTF
				auto vtf = vtfpp::VTF::create(currentInputPath, options);
				if (!vtf) {
					tferr << "Failed to TF input image at " << BOLD << currentInputPath << END << ". Is it a supported format?" << tfendl;
					return EXIT_FAILURE;
				}

				// Set resources
				handleSettingResourcesForVTF(vtf, false);

				// Bake VTF
				return bake(vtf, "image");
			};

			// Check input path
			if (inputPath.empty() || !std::filesystem::exists(inputPath)) {
				throw std::invalid_argument{"Input path does not exist!"};
			}

			// Watch mode overrides this
			if (watch) {
				noOverwrite = true;
			}

			int out = EXIT_SUCCESS;
			if (std::filesystem::is_regular_file(inputPath)) {
				tfout << BOLD << randomDeviantArtTFTrope() << "..." << END << tfendl;
				out = create(inputPath);
			} else if (std::filesystem::is_directory(inputPath)) {
				if (noRecurse) {
					for (const auto& dirEntry : std::filesystem::directory_iterator{inputPath, DIR_OPTIONS}) {
						if (::fileIsASupportedImageFileFormat(dirEntry.path().extension().string())) {
							outputPath = "";
							out = out || create(dirEntry.path().string());
						}
					}
				} else {
					for (const auto& dirEntry : std::filesystem::recursive_directory_iterator{inputPath, DIR_OPTIONS}) {
						if (::fileIsASupportedImageFileFormat(dirEntry.path().extension().string())) {
							outputPath = "";
							out = out || create(dirEntry.path().string());
						}
					}
				}
			} else {
				throw std::invalid_argument{"Input path is not a file or directory!"};
			}

			if (out != EXIT_SUCCESS) {
				return out;
			}

			if (watch) {
				overwrite = true;
				noOverwrite = false;

				const bool watchingSingleFile = std::filesystem::is_regular_file(inputPath);

				efsw::FileWatcher fileWatcher;
				std::unordered_map<std::string, std::pair<::ElapsedTime<>, efsw::Action>> fileActions;
				std::mutex fileActionsMutex;
				::MareTFFileWatchListener fileUpdateListener{
					[&](efsw::WatchID, const std::string& dir, const std::string& filename, efsw::Action action, const std::string& oldFilename) {
						const auto path = dir + filename;
						if (watchingSingleFile && std::filesystem::absolute(path) != std::filesystem::absolute(inputPath)) {
							return;
						}

						std::lock_guard fileActionsLock{fileActionsMutex};

						// Collapse Moved events so we don't need to check them in the switches below
						if (action == efsw::Actions::Moved) {
							if (!oldFilename.empty()) {
								if (const auto oldPath = dir + oldFilename; fileActions.contains(oldPath) && fileActions[oldPath].second == efsw::Action::Add) {
									fileActions.erase(oldPath);
								} else {
									fileActions[oldPath] = {{}, efsw::Actions::Delete};
								}
							}
							fileActions[path] = {{}, efsw::Actions::Add};
							return;
						}

						if (!fileActions.contains(path)) {
							fileActions[path] = {{}, action};
							return;
						}
						switch (fileActions[path].second) {
							case efsw::Actions::Add:
								switch (action) {
									case efsw::Actions::Add:
									case efsw::Actions::Modified:
										fileActions[path].first = {};
										break;
									case efsw::Actions::Delete:
										fileActions.erase(path);
										break;
									case efsw::Actions::Moved:
										break;
								}
								break;
							case efsw::Actions::Delete:
								switch (action) {
									case efsw::Actions::Add:
									case efsw::Actions::Modified:
										fileActions[path] = {{}, efsw::Actions::Add};
										break;
									case efsw::Actions::Delete:
									case efsw::Actions::Moved:
										break;
								}
								break;
							case efsw::Actions::Modified:
								switch (action) {
									case efsw::Actions::Add:
									case efsw::Actions::Modified:
										fileActions[path].first = {};
										break;
								case efsw::Actions::Delete:
										fileActions[path] = {{}, efsw::Actions::Delete};
								case efsw::Actions::Moved:
										break;
								}
								break;
							case efsw::Actions::Moved:
								break;
						}
					}
				};
				fileWatcher.addWatch(watchingSingleFile ? std::filesystem::path{inputPath}.parent_path().string() : inputPath, &fileUpdateListener, !noRecurse);

				tfout << "Watching " << BOLD << inputPath << END << " for any changes..." << tfendl;
#ifdef _WIN32
				SetConsoleCtrlHandler(static_cast<PHANDLER_ROUTINE>(+[](unsigned long type) -> int {
					if (type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT) {
						tfout << tfendl << "Closing..." << tfendl;
					}
					return false;
				}), true);
#else
				struct sigaction sigIntHandler{};
				sigIntHandler.sa_handler = +[](int) {
					tfout << tfendl << "Closing..." << tfendl;
					std::exit(0);
				};
				sigemptyset(&sigIntHandler.sa_mask);
				sigIntHandler.sa_flags = 0;
				sigaction(SIGINT, &sigIntHandler, nullptr);
#endif

				fileWatcher.watch();
				for (;; std::this_thread::sleep_for(250ms)) {
					{
						std::lock_guard fileActionsLock{fileActionsMutex};

						const auto pathsView = fileActions | std::views::keys;
						for (std::vector<std::string> paths{pathsView.begin(), pathsView.end()}; const auto& path : paths) {
							if (fileActions[path].first.get() < 750ms || !::fileIsASupportedImageFileFormat(std::filesystem::path{path}.extension().string())) {
								continue;
							}
							switch (fileActions[path].second) {
								case efsw::Actions::Add:
								case efsw::Actions::Modified:
									if (!watchingSingleFile) {
										outputPath = "";
									}
									create(path);
									break;
								case efsw::Actions::Delete: {
									const auto vtfPath = ::getOutputPathForInput(path, *not_magic_enum::enum_cast<vtfpp::VTF::Platform>(platform));
									if (std::error_code ec; std::filesystem::exists(vtfPath, ec)) {
										ec.clear();
										std::filesystem::remove(vtfPath, ec);
										tfout << "Deleted " << BOLD << vtfPath << END << "." << tfendl;
									}
									break;
								}
								case efsw::Actions::Moved:
									break;
							}
							fileActions.erase(path);
						}
					}
				}
			}

			return EXIT_SUCCESS;
		}
		if (mode == "edit") {
			const auto edit = [&](const std::string& currentInputPath) {
				// Check output path
				if (outputPath.empty()) {
					outputPath = currentInputPath;
					if (cli.is_used("--set-platform")) {
						outputPath = ::getOutputPathForInput(currentInputPath, *not_magic_enum::enum_cast<vtfpp::VTF::Platform>(setPlatform));
					}
				}
				{
					bool checkFileShouldRet;
					const int out = checkFileDoesntExist(outputPath, checkFileShouldRet);
					if (checkFileShouldRet) {
						return out;
					}
				}

				// Start to set up VTF for editing
				const ::ElapsedTime loadStopwatch;
				vtfpp::VTF vtf{currentInputPath};
				if (!vtf) {
					tferr << "Unable to load input file at " << BOLD << currentInputPath << END << " as a VTF!" << tfendl;
					return EXIT_FAILURE;
				}
				tfout << "Loaded input VTF at " << BOLD << currentInputPath << END << " in " << CYAN << loadStopwatch.get().count() << "ms" << END << (noPrettyFormatting ? "" : " 🐎") << tfendl;
				const ::ElapsedTime editStopwatch;

				// Get edit filter
				const auto editFilterActual = *not_magic_enum::enum_cast<vtfpp::ImageConversion::ResizeFilter>(editFilter);

				// Get output format
				vtfpp::ImageFormat setFormatActual = vtf.getFormat();
				if (cli.is_used("--set-format")) {
					setFormatActual = *not_magic_enum::enum_cast<vtfpp::ImageFormat>(setFormat);
					if (setFormatActual == vtfpp::VTF::FORMAT_UNCHANGED) {
						setFormatActual = vtf.getFormat();
					} else if (setFormatActual == vtfpp::VTF::FORMAT_DEFAULT) {
						setFormatActual = vtfpp::VTF::getDefaultCompressedFormat(vtf.getFormat(), vtf.getVersion(), vtf.getFaceCount() > 1);
					}
				}

				// Set platform
				if (cli.is_used("--set-platform")) {
					vtf.setPlatform(*not_magic_enum::enum_cast<vtfpp::VTF::Platform>(setPlatform));
				}

				// Set version
				if (cli.is_used("--set-version")) {
					uint32_t setMajorVersion, setMinorVersion;
					sourcepp::string::toInt(std::string_view{&setVersion[0], 1}, setMajorVersion);
					if (setMajorVersion != 7) {
						throw std::runtime_error{"Invalid version!"};
					}
					sourcepp::string::toInt(std::string_view{&setVersion[2], 1}, setMinorVersion);
					vtf.setVersion(setMinorVersion);
				}

				// Add/remove flags
				for (const auto& flag : addFlags) {
					vtf.addFlags(*not_magic_enum::enum_cast<vtfpp::VTF::Flags>(flag));
				}
				for (const auto& flag : removeFlags) {
					vtf.removeFlags(*not_magic_enum::enum_cast<vtfpp::VTF::Flags>(flag));
				}

				// Set size
				vtf.setImageWidthResizeMethod(vtfpp::ImageConversion::ResizeMethod::NONE);
				vtf.setImageHeightResizeMethod(vtfpp::ImageConversion::ResizeMethod::NONE);
				if (cli.is_used("--set-width") && cli.is_used("--set-height")) {
					vtf.setSize(setWidth, setHeight, editFilterActual);
				} else if (cli.is_used("--set-width")) {
					vtf.setSize(setWidth, vtf.getHeight(), editFilterActual);
				} else if (cli.is_used("--set-height")) {
					vtf.setSize(vtf.getWidth(), setHeight, editFilterActual);
				}

				// Set start frame
				if (cli.is_used("--set-start-frame")) {
					vtf.setStartFrame(static_cast<uint16_t>(setStartFrame));
				}

				// Set bumpmap scale
				if (cli.is_used("--set-bumpmap-scale")) {
					vtf.setBumpMapScale(setBumpMapScale);
				}

				// Set console mip scale
				if (cli.is_used("--set-console-mip-scale")) {
					vtf.setConsoleMipScale(setConsoleMipScale);
				}

				// Recompute/remove mips
				if (recomputeMips) {
					vtf.computeMips(editFilterActual);
				} else if (removeMips) {
					vtf.setMipCount(1);
				}

				// Recompute/remove thumbnail
				if (recomputeThumbnail) {
					vtf.computeThumbnail(editFilterActual);
				} else if (removeThumbnail) {
					vtf.removeThumbnail();
				}

				// Recompute reflectivity
				if (recomputeReflectivity) {
					vtf.computeReflectivity();
				}

				// Set format
				if (cli.is_used("--set-format")) {
					vtf.setFormat(*not_magic_enum::enum_cast<vtfpp::ImageFormat>(setFormat), editFilterActual);
				}

				// Recompute transparency flags
				if (recomputeTransparencyFlags) {
					vtf.computeTransparencyFlags();
				}

				// Set compression method
				if (cli.is_used("--set-compression-method")) {
					vtf.setCompressionMethod(*not_magic_enum::enum_cast<vtfpp::CompressionMethod>(setCompressionMethod));
				}

				// Set compression level
				if (cli.is_used("--set-compression-level")) {
					if (setCompressionLevel < -1) {
						setCompressionLevel = -1;
						tfout << "Compression level range is between -1 and 9/22 (depending on the compression method). Setting compression level to -1..." << tfendl;
					} else if ((vtf.getCompressionMethod() == vtfpp::CompressionMethod::DEFLATE || vtf.getCompressionMethod() == vtfpp::CompressionMethod::CONSOLE_LZMA) && setCompressionLevel > 9) {
						setCompressionLevel = 9;
						tfout << "Compression level range is between -1 and 9 for Deflate and LZMA. Setting compression level to 9..." << tfendl;
					} else if (vtf.getCompressionMethod() == vtfpp::CompressionMethod::ZSTD && setCompressionLevel > 22) {
						setCompressionLevel = 22;
						tfout << "Compression level range is between -1 and 22 for Zstd. Setting compression level to 22..." << tfendl;
					}
					vtf.setCompressionLevel(static_cast<int16_t>(setCompressionLevel));
				}

				// Set resources
				handleSettingResourcesForVTF(vtf, true);

				// Bake VTF
				const auto vtfData = vtf.bake();
				if (vtfData.empty()) {
					tferr << "Failed to bake edited VTF." << tfendl;
					return EXIT_FAILURE;
				}
				const auto time = editStopwatch.get().count();
				if (!sourcepp::fs::writeFileBuffer(outputPath, vtfData)) {
					tferr << "Failed to save edited VTF at " << BOLD << outputPath << END << "." << tfendl;
					return EXIT_FAILURE;
				}
				tfout << "Saved edited VTF to " << BOLD << outputPath << END << " in " << CYAN << time << "ms" << END << (noPrettyFormatting ? "" : " 💖") << tfendl;
				return EXIT_SUCCESS;
			};

			// Check input path
			if (inputPath.empty() || !std::filesystem::exists(inputPath)) {
				throw std::invalid_argument{"Input path does not exist!"};
			}

			if (std::filesystem::is_regular_file(inputPath)) {
				if (!inputPath.ends_with(".vtf") && !inputPath.ends_with(".xtf")) {
					throw std::invalid_argument{"Input file must be a VTF!"};
				}
				return edit(inputPath);
			}
			if (std::filesystem::is_directory(inputPath)) {
				int out = EXIT_SUCCESS;
				if (noRecurse) {
					for (const auto& dirEntry : std::filesystem::directory_iterator{inputPath, DIR_OPTIONS}) {
						if (sourcepp::string::toLower(dirEntry.path().extension().string()) == ".vtf" || sourcepp::string::toLower(dirEntry.path().extension().string()) == ".xtf") {
							outputPath = "";
							out = out || edit(dirEntry.path().string());
						}
					}
				} else {
					for (const auto& dirEntry : std::filesystem::recursive_directory_iterator{inputPath, DIR_OPTIONS}) {
						if (sourcepp::string::toLower(dirEntry.path().extension().string()) == ".vtf" || sourcepp::string::toLower(dirEntry.path().extension().string()) == ".xtf") {
							outputPath = "";
							out = out || edit(dirEntry.path().string());
						}
					}
				}
				return out;
			}
			throw std::invalid_argument{"Input path is not a file or directory!"};
		}
		if (mode == "extract") {
			const auto extract = [&](const std::string& currentInputPath) {
				// Start to set up VTF for extracting
				::ElapsedTime loadStopwatch;
				vtfpp::VTF vtf{currentInputPath};
				if (!vtf) {
					tferr << "Unable to load input file at " << BOLD << currentInputPath << END << " as a VTF!" << tfendl;
					return EXIT_FAILURE;
				}
				tfout << "Loaded input VTF at " << BOLD << currentInputPath << END << " in " << CYAN << loadStopwatch.get().count() << "ms" << END << (noPrettyFormatting ? "" : " 🐎") << tfendl;

				// Get output format
				vtfpp::ImageConversion::FileFormat fileFormat = *not_magic_enum::enum_cast<vtfpp::ImageConversion::FileFormat>(extractFormat);
				if (fileFormat == vtfpp::ImageConversion::FileFormat::DEFAULT) {
					fileFormat = vtfpp::ImageConversion::getDefaultFileFormatForImageFormat(vtf.getFormat());
				}

				// Check output path
				if (outputPath.empty()) {
					const std::filesystem::path inputPathPath{currentInputPath};
					outputPath = (inputPathPath.parent_path() / inputPathPath.stem()).string();
					outputPath.append(supportedImageFileFormatExtension(fileFormat));
				}

				// Extract all VTF image data
				::ElapsedTime extractStopwatch;
				std::vector<bool> extractionSuccessful;
				if (extractAll) {
					extractAllMips = extractAllFrames = extractAllFaces = extractAllSlices = true;
				}
				if (extractAllMips || extractAllFrames || extractAllFaces || extractAllSlices) {
					for (int frame = extractAllFrames ? 0 : extractFrame; frame < (extractAllFrames ? vtf.getFrameCount() : extractFrame + 1); frame++) {
						std::filesystem::path outputPathFixupFrame{outputPath};
						if (extractAllFrames && vtf.getFrameCount() > 1) {
							outputPathFixupFrame = outputPathFixupFrame.parent_path() / (outputPathFixupFrame.stem().string() + "_frame" + sourcepp::string::padNumber(frame, 3) + outputPathFixupFrame.extension().string());
						}
						for (int face = extractAllFaces ? 0 : extractFace; face < (extractAllFaces ? vtf.getFaceCount() : extractFace + 1); face++) {
							std::filesystem::path outputPathFixupFace = outputPathFixupFrame;
							if (extractAllFaces && vtf.getFaceCount() > 1) {
								outputPathFixupFace = outputPathFixupFace.parent_path() / (outputPathFixupFace.stem().string() + "_face" + sourcepp::string::padNumber(face, 1) + outputPathFixupFace.extension().string());
							}
							for (int slice = extractAllSlices ? 0 : extractSlice; slice < (extractAllSlices ? vtf.getDepth() : extractSlice + 1); slice++) {
								std::filesystem::path outputPathFixupSlice = outputPathFixupFace;
								if (extractAllSlices && vtf.getDepth() > 1) {
									outputPathFixupSlice = outputPathFixupSlice.parent_path() / (outputPathFixupSlice.stem().string() + "_slice" + sourcepp::string::padNumber(slice, 2) + outputPathFixupSlice.extension().string());
								}
								for (int mip = extractAllMips ? 0 : extractMip; mip < (extractAllMips ? vtf.getMipCount() : extractMip + 1); mip++) {
									std::filesystem::path outputPathFixupMip = outputPathFixupSlice;
									if (extractAllMips && vtf.getMipCount() > 1) {
										outputPathFixupMip = outputPathFixupMip.parent_path() / (outputPathFixupMip.stem().string() + "_mip" + sourcepp::string::padNumber(mip, 2) + outputPathFixupMip.extension().string());
									}
									bool notSuccess;
									checkFileDoesntExist(outputPathFixupMip.string(), notSuccess);
									extractionSuccessful.push_back(!notSuccess && vtf.saveImageToFile(outputPathFixupMip.string(), mip, frame, face, slice, fileFormat));
								}
							}
						}
					}
				} else {
					bool notSuccess;
					checkFileDoesntExist(outputPath, notSuccess);
					extractionSuccessful.push_back(!notSuccess && vtf.saveImageToFile(outputPath, extractMip, extractFrame, extractFace, extractSlice, fileFormat));
				}

				// Extract VTF
				if (int successCount = std::accumulate(extractionSuccessful.begin(), extractionSuccessful.end(), 0); successCount < extractionSuccessful.size()) {
					tferr << "Failed to save " << CYAN << (extractionSuccessful.size() - successCount) << END << " of " << CYAN << extractionSuccessful.size() << END << " files." << tfendl;
					return EXIT_FAILURE;
				}
				tfout << "Saved VTF image data in " << CYAN << extractStopwatch.get().count() << "ms" << END << (noPrettyFormatting ? "" : " 💖") << tfendl;
				return EXIT_SUCCESS;
			};

			// Check input path
			if (inputPath.empty() || !std::filesystem::exists(inputPath)) {
				throw std::invalid_argument{"Input path does not exist!"};
			}

			if (std::filesystem::is_regular_file(inputPath)) {
				if (!inputPath.ends_with(".vtf") && !inputPath.ends_with(".xtf")) {
					throw std::invalid_argument{"Input file must be a VTF!"};
				}
				return extract(inputPath);
			}
			if (std::filesystem::is_directory(inputPath)) {
				int out = EXIT_SUCCESS;
				if (noRecurse) {
					for (const auto& dirEntry : std::filesystem::directory_iterator{inputPath, DIR_OPTIONS}) {
						if (sourcepp::string::toLower(dirEntry.path().extension().string()) == ".vtf" || sourcepp::string::toLower(dirEntry.path().extension().string()) == ".xtf") {
							outputPath = "";
							out = out || extract(dirEntry.path().string());
						}
					}
				} else {
					for (const auto& dirEntry : std::filesystem::recursive_directory_iterator{inputPath, DIR_OPTIONS}) {
						if (sourcepp::string::toLower(dirEntry.path().extension().string()) == ".vtf" || sourcepp::string::toLower(dirEntry.path().extension().string()) == ".xtf") {
							outputPath = "";
							out = out || extract(dirEntry.path().string());
						}
					}
				}
				return out;
			}
			throw std::invalid_argument{"Input path is not a file or directory!"};
		}
		if (mode == "info") {
			const auto info = [&](const std::string& currentInputPath) {
				// Load VTF
				vtfpp::VTF vtf{currentInputPath};
				if (!vtf) {
					throw std::invalid_argument{"Unable to load input file as a VTF!"};
				}

				if (infoOutputMode == "human") {
					tfout << tfendl << RED << BOLD << currentInputPath << END << '\n' << tfendl;

					tfout << GREEN << BOLD << " ――― FORMAT ―――" << END << tfendl;

					tfout << BOLD << "Platform: " << CYAN << not_magic_enum::enum_name(vtf.getPlatform()) << END << tfendl;

					if (vtf.getPlatform() == vtfpp::VTF::PLATFORM_PC) {
						tfout << BOLD << "Version:  " << CYAN << 7 << '.' << vtf.getVersion() << END << tfendl;
					}

					tfout << '\n' << GREEN << BOLD << " ――― IMAGE ―――" << END << tfendl;

					tfout << BOLD << "Format:        " << CYAN << not_magic_enum::enum_name(vtf.getFormat()) << END << tfendl;
					if (vtf.getDepth() > 1) {
						tfout << BOLD << "Dimensions:    " << CYAN << vtf.getWidth() << END << " x " << CYAN << vtf.getHeight() << END << " x " << CYAN << vtf.getDepth() << END << tfendl;
					} else {
						tfout << BOLD << "Dimensions:    " << CYAN << vtf.getWidth() << END << " x " << CYAN << vtf.getHeight() << END << tfendl;
					}

					tfout << BOLD << "Flags:         " << END << CYAN << "0x" << std::hex << vtf.getFlags() << std::dec << END;
					if (vtf.getFlags()) {
						tfout << " (";
						bool first = true;
						const auto prettyFlagNames = ::getPrettyFlagNamesFor(vtf.getVersion(), vtf.getPlatform());
						for (int i = 0; i < prettyFlagNames.size(); i++) {
							if (vtf.getFlags() & 1 << i) {
								if (!first) {
									tfout << " | ";
								}
								first = false;
								tfout << CYAN << prettyFlagNames[i] << END;
							}
						}
						tfout << ')';
					}
					tfout << tfendl;

					tfout << BOLD << "Mips:          " << CYAN << static_cast<int>(vtf.getMipCount()) << END << tfendl;
					tfout << BOLD << "Frames:        " << CYAN << vtf.getFrameCount() << END << tfendl;
					tfout << BOLD << "Faces:         " << CYAN << static_cast<int>(vtf.getFaceCount()) << END << tfendl;

					tfout << BOLD << "Reflectivity:  " << END << '[' << CYAN << vtf.getReflectivity()[0] << 'f' << END << ", " << CYAN << vtf.getReflectivity()[1] << 'f' << END << ", " << CYAN << vtf.getReflectivity()[2] << 'f' << END << ']' << tfendl;

					tfout << BOLD << "Start Frame:   " << END << CYAN << vtf.getStartFrame() << END << tfendl;
					tfout << BOLD << "Bumpmap Scale: " << END << CYAN << vtf.getBumpMapScale() << 'f' << END << tfendl;

					if (vtf.getPlatform() != vtfpp::VTF::PLATFORM_PC) {
						tfout << BOLD << "Mip Scale:     " << END << CYAN << vtf.getConsoleMipScale() << END << tfendl;
					}

					tfout << BOLD << "Compression:   " << END;
					if (vtf.getCompressionLevel() == 0) {
						if (vtf.getPlatform() != vtfpp::VTF::PLATFORM_PC && vtf.getCompressionMethod() == vtfpp::CompressionMethod::CONSOLE_LZMA) {
							tfout << GREEN << not_magic_enum::enum_name(vtf.getCompressionMethod()) << END << tfendl;
						} else {
							tfout << RED << "Uncompressed" << END << tfendl;
						}
					} else {
						tfout << GREEN << not_magic_enum::enum_name(vtf.getCompressionMethod()) << END << " (level " << CYAN << vtf.getCompressionLevel() << END << ')' << tfendl;
					}

					tfout << '\n' << GREEN << BOLD << " ――― RESOURCES ―――" << END << tfendl;

					tfout << BOLD << "Thumbnail:      ";
					if (vtf.hasThumbnailData()) {
						tfout << GREEN << "Exists";
					} else {
						tfout << RED << "Doesn't exist";
					}
					tfout << END << tfendl;

					tfout << BOLD << "Palette:        ";
					if (vtf.getResource(vtfpp::Resource::TYPE_PALETTE_DATA)) {
						tfout << GREEN << "Exists";
					} else {
						tfout << RED << "Doesn't exist";
					}
					tfout << END << tfendl;

					tfout << BOLD << "Fallback:       ";
					if (vtf.hasFallbackData()) {
						tfout << GREEN << "Exists" << END << " — " << BOLD << "Dimensions: " << CYAN << static_cast<int>(vtf.getFallbackWidth()) << END << " x " << CYAN << static_cast<int>(vtf.getFallbackHeight()) << END << " — " << BOLD << "Mips: " << CYAN << static_cast<int>(vtf.getFallbackMipCount());
					} else {
						tfout << RED << "Doesn't exist";
					}
					tfout << END << tfendl;

					tfout << BOLD << "Particle Sheet: ";
					const auto* particleSheetResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA);
					if (particleSheetResourcePtr) {
						if (const auto sheet = particleSheetResourcePtr->getDataAsParticleSheet()) {
							tfout << GREEN << "Exists" << END << " — " << BOLD << "Version: " << CYAN << sheet.getVersion() << END << " — " << BOLD << "Sequences: " << CYAN << sheet.getSequences().size();
						} else {
							tfout << RED << "Exists, but failed to parse";
						}
					} else {
						tfout << RED << "Doesn't exist";
					}
					tfout << END << tfendl;

					tfout << BOLD << "Hotspot Data:   ";
					const auto* hotspotDataResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_HOTSPOT_DATA);
					if (hotspotDataResourcePtr) {
						if (const auto hotspots = hotspotDataResourcePtr->getDataAsHotspotData()) {
							tfout << GREEN << "Exists" << END << " — " << BOLD << "Version: " << CYAN << static_cast<int>(hotspots.getVersion()) << END << " — " << BOLD << "Rects: " << CYAN << hotspots.getRects().size();
						} else {
							tfout << RED << "Exists, but failed to parse";
						}
					} else {
						tfout << RED << "Doesn't exist";
					}
					tfout << END << tfendl;

					tfout << BOLD << "Image:          ";
					if (vtf.hasImageData()) {
						tfout << GREEN << "Exists";
					} else {
						tfout << RED << "Doesn't exist (HUH?)";
					}
					tfout << END << tfendl;

					tfout << BOLD << "Extended Flags: ";
					if (const auto* ts0ResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_EXTENDED_FLAGS)) {
						tfout << GREEN << "Exists" << END << " — " << CYAN << "0x" << std::hex << ts0ResourcePtr->getDataAsExtendedFlags() << std::dec;
					} else {
						tfout << RED << "Doesn't exist";
					}
					tfout << END << tfendl;

					tfout << BOLD << "CRC:            ";
					if (const auto* crcResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_CRC)) {
						tfout << GREEN << "Exists" << END << " — " << CYAN << "0x" << std::hex << crcResourcePtr->getDataAsCRC() << std::dec;
					} else {
						tfout << RED << "Doesn't exist";
					}
					tfout << END << tfendl;

					tfout << BOLD << "LOD:            ";
					if (const auto* lodResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_LOD_CONTROL_INFO)) {
						const auto lod = lodResourcePtr->getDataAsLODControlInfo();
						tfout << GREEN << "Exists" << END << " — " << BOLD << "U: " << END << CYAN << static_cast<int>(std::get<0>(lod)) << END << " — " << BOLD << "V: " << END << CYAN << static_cast<int>(std::get<1>(lod));
						if (vtf.getPlatform() != vtfpp::VTF::PLATFORM_PC) {
							tfout << END << BOLD << "U (Console): " << END << CYAN << static_cast<int>(std::get<2>(lod)) << END << " — " << BOLD << "V (Console): " << END << CYAN << static_cast<int>(std::get<3>(lod));
						}
					} else {
						tfout << RED << "Doesn't exist";
					}
					tfout << END << tfendl;

					tfout << BOLD << "KeyValues Data: ";
					const auto* kvdResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_KEYVALUES_DATA);
					if (kvdResourcePtr) {
						const auto keyvalues = kvdResourcePtr->getDataAsKeyValuesData();
						tfout << GREEN << "Exists" << END << " — " << CYAN << keyvalues.size() << " chars";
					} else {
						tfout << RED << "Doesn't exist";
					}
					tfout << END << tfendl;

					// Bail here if requested
					if (infoSkipResources) {
						return EXIT_SUCCESS;
					}

					if (particleSheetResourcePtr) {
						if (const auto sheet = particleSheetResourcePtr->getDataAsParticleSheet()) {
							tfout << '\n' << GREEN << BOLD << " ――― PARTICLE SHEET RESOURCE ―――" << END << tfendl;
							tfout << BOLD << "Version: " << END << CYAN << sheet.getVersion() << END << tfendl;
							for (const auto& sequence : sheet.getSequences()) {
								tfout << BOLD << "Sequence " << END << CYAN << sequence.id << END << BOLD << ':' << END << tfendl;
								tfout << '\t' << BOLD << "Total Duration: " << END << CYAN << sequence.durationTotal << 'f' << END << tfendl;
								tfout << '\t' << BOLD << "Loop:           " << END;
								if (sequence.loop) {
									tfout << GREEN << "Yes";
								} else {
									tfout << RED << "No";
								}
								tfout << END << tfendl;
								for (int i = 0; i < sequence.frames.size(); i++) {
									const auto& frame = sequence.frames.at(i);
									tfout << '\t' << BOLD << "Frame " << END << CYAN << i << END << BOLD << ':' << END << tfendl;
									tfout << "\t\t" << BOLD << "Duration: " << END << CYAN << frame.duration << 'f' << END << tfendl;
									tfout << "\t\t" << BOLD << "Bounds:   ";
									if (sheet.getVersion() < 1) {
										tfout << '(' << CYAN << frame.bounds.at(0).x1 << 'f' << END << ", " << CYAN << frame.bounds.at(0).y1 << 'f' << END << "), (" << CYAN << frame.bounds.at(0).x2 << 'f' << END << ", " << CYAN << frame.bounds.at(0).y2 << 'f' << END << ')' << tfendl;
									} else {
										tfout << END << tfendl;
										for (const auto& bound : frame.bounds) {
											tfout << "\t\t\t" << '(' << CYAN << bound.x1 << 'f' << END << ", " << CYAN << bound.y1 << 'f' << END << "), (" << CYAN << bound.x2 << 'f' << END << ", " << CYAN << bound.y2 << 'f' << END << ')' << tfendl;
										}
									}
								}
							}
						}
					}

					if (hotspotDataResourcePtr) {
						if (const auto hotspots = hotspotDataResourcePtr->getDataAsHotspotData()) {
							tfout << '\n' << GREEN << BOLD << " ――― HOTSPOT DATA RESOURCE ―――" << END << tfendl;
							tfout << BOLD << "Version: " << END << CYAN << static_cast<int>(hotspots.getVersion()) << END << tfendl;
							tfout << BOLD << "Flags:   " << END << CYAN << "0x" << std::hex << static_cast<int>(hotspots.getFlags()) << std::dec << END;
							if (hotspots.getFlags()) {
								tfout << " (";
								bool first = true;
								for (const auto& [rectFlag, rectFlagName] : not_magic_enum::enum_entries<vtfpp::HOT::Rect::Flags>(true)) {
									if (hotspots.getFlags() & rectFlag) {
										if (!first) {
											tfout << " | ";
										}
										first = false;
										tfout << CYAN << rectFlagName << END;
									}
								}
								tfout << ')';
							}
							tfout << tfendl;
							for (int i = 0; i < hotspots.getRects().size(); i++) {
								const auto& rect = hotspots.getRects().at(i);
								tfout << BOLD << "Rect " << END << CYAN << (i + 1) << END << BOLD << ':' << END << tfendl;
								tfout << '\t' << BOLD << "Flags:  " << END << CYAN << "0x" << std::hex << static_cast<int>(rect.flags) << std::dec << END;
								if (rect.flags) {
									tfout << " (";
									bool first = true;
									for (auto [hotspotFlag, hotspotName] : not_magic_enum::enum_entries<vtfpp::HOT::Rect::Flags>()) {
										if (rect.flags & hotspotFlag) {
											if (!first) {
												tfout << " | ";
											}
											first = false;
											tfout << CYAN << hotspotName << END;
										}
									}
									tfout << ')';
								}
								tfout << tfendl;
								tfout << '\t' << BOLD << "Bounds: " << END << '(' << CYAN << rect.x1 << END << ", " << CYAN << rect.y1 << END << "), (" << CYAN << rect.x2 << END << ", " << CYAN << rect.y2 << END << ')' << tfendl;
							}
						}
					}

					if (kvdResourcePtr) {
						tfout << '\n' << GREEN << BOLD << " ――― KEYVALUES DATA RESOURCE ―――" << END << tfendl;
						tfout << kvdResourcePtr->getDataAsKeyValuesData() << END << tfendl;
					}
				} else if (infoOutputMode == "kv1") {
					kvpp::KV1Writer kv;

					// File format
					kv["format"]["platform"] = not_magic_enum::enum_name(vtf.getPlatform());
					kv["format"]["version_major"] = 7;
					kv["format"]["version_minor"] = static_cast<int>(vtf.getVersion());

					// Image
					kv["image"]["format"] = not_magic_enum::enum_name(vtf.getFormat());
					kv["image"]["dimensions"]["width"] = static_cast<int>(vtf.getWidth());
					kv["image"]["dimensions"]["height"] = static_cast<int>(vtf.getHeight());
					kv["image"]["dimensions"]["depth"] = static_cast<int>(vtf.getDepth());
					kv["image"]["dimensions"]["mips"] = static_cast<int>(vtf.getMipCount());
					kv["image"]["dimensions"]["frames"] = static_cast<int>(vtf.getFrameCount());
					kv["image"]["dimensions"]["faces"] = static_cast<int>(vtf.getFaceCount());
					kv["image"]["flags"] = static_cast<int>(vtf.getFlags());
					kv["image"]["reflectivity"]["r"] = vtf.getReflectivity()[0];
					kv["image"]["reflectivity"]["g"] = vtf.getReflectivity()[1];
					kv["image"]["reflectivity"]["b"] = vtf.getReflectivity()[2];
					kv["image"]["start_frame"] = static_cast<int>(vtf.getStartFrame());
					kv["image"]["bumpmap_scale"] = vtf.getBumpMapScale();
					kv["image"]["mip_scale"] = vtf.getPlatform() != vtfpp::VTF::PLATFORM_PC ? static_cast<int>(vtf.getConsoleMipScale()) : 0;
					if (vtf.getCompressionLevel() == 0) {
						if (vtf.getPlatform() != vtfpp::VTF::PLATFORM_PC && vtf.getCompressionMethod() == vtfpp::CompressionMethod::CONSOLE_LZMA) {
							kv["image"]["compression"]["method"] = not_magic_enum::enum_name(vtf.getCompressionMethod());
						} else {
							kv["image"]["compression"]["method"] = "NONE";
						}
					} else {
						kv["image"]["compression"]["method"] = not_magic_enum::enum_name(vtf.getCompressionMethod());
					}
					kv["image"]["compression"]["level"] = static_cast<int>(vtf.getCompressionLevel());

					// Thumbnail
					kv["resources"]["thumbnail"]["present"] = vtf.hasThumbnailData();
					kv["resources"]["thumbnail"]["format"] = not_magic_enum::enum_name(vtf.getThumbnailFormat());
					kv["resources"]["thumbnail"]["width"] = static_cast<int>(vtf.getThumbnailWidth());
					kv["resources"]["thumbnail"]["height"] = static_cast<int>(vtf.getThumbnailHeight());

					// Palette
					kv["resources"]["palette"]["present"] = static_cast<bool>(vtf.getResource(vtfpp::Resource::TYPE_PALETTE_DATA));

					// Fallback
					kv["resources"]["fallback"]["present"] = vtf.hasFallbackData();
					kv["resources"]["fallback"]["width"] = static_cast<int>(vtf.getFallbackWidth());
					kv["resources"]["fallback"]["height"] = static_cast<int>(vtf.getFallbackHeight());
					kv["resources"]["fallback"]["mips"] = static_cast<int>(vtf.getFallbackMipCount());

					// Bail here if requested
					if (infoSkipResources) {
						tfout << kv.bake();
						return EXIT_SUCCESS;
					}

					// Resources
					if (const auto* particleSheetResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA)) {
						if (const auto sheet = particleSheetResourcePtr->getDataAsParticleSheet()) {
							kv["resources"]["particle_sheet"]["malformed"] = false;
							kv["resources"]["particle_sheet"]["version"] = static_cast<int>(sheet.getVersion());
							for (const auto& sequence : sheet.getSequences()) {
								const auto idStr = std::format("{}", sequence.id);
								kv["resources"]["particle_sheet"]["sequences"][idStr]["duration_total"] = sequence.durationTotal;
								kv["resources"]["particle_sheet"]["sequences"][idStr]["loop"] = sequence.loop;
								for (int i = 0; i < sequence.frames.size(); i++) {
									const auto iStr = std::format("{}", i);
									const auto& frame = sequence.frames.at(i);
									kv["resources"]["particle_sheet"]["sequences"][idStr]["frames"][iStr]["duration"] = frame.duration;
									for (int b = 0; b < sheet.getFrameBoundsCount(); b++) {
										const auto bStr = std::format("{}", b);
										kv["resources"]["particle_sheet"]["sequences"][idStr]["frames"][iStr]["bounds"][bStr]["x1"] = frame.bounds.at(b).x1;
										kv["resources"]["particle_sheet"]["sequences"][idStr]["frames"][iStr]["bounds"][bStr]["y1"] = frame.bounds.at(b).y1;
										kv["resources"]["particle_sheet"]["sequences"][idStr]["frames"][iStr]["bounds"][bStr]["x2"] = frame.bounds.at(b).x2;
										kv["resources"]["particle_sheet"]["sequences"][idStr]["frames"][iStr]["bounds"][bStr]["y2"] = frame.bounds.at(b).y2;
									}
								}
							}
						} else {
							kv["resources"]["particle_sheet"]["malformed"] = true;
						}
					}
					if (const auto* hotspotResource = vtf.getResource(vtfpp::Resource::TYPE_HOTSPOT_DATA)) {
						if (const auto hotspots = hotspotResource->getDataAsHotspotData()) {
							kv["resources"]["hotspot_data"]["malformed"] = false;
							kv["resources"]["hotspot_data"]["version"] = static_cast<int>(hotspots.getVersion());
							kv["resources"]["hotspot_data"]["flags"] = static_cast<int>(hotspots.getFlags());
							for (int i = 0; i < hotspots.getRects().size(); i++) {
								const auto iStr = std::format("{}", i);
								const auto& rect = hotspots.getRects().at(i);
								kv["resources"]["hotspot_data"]["rects"][iStr]["flags"] = static_cast<int>(rect.flags);
								kv["resources"]["hotspot_data"]["rects"][iStr]["x1"] = static_cast<int>(rect.x1);
								kv["resources"]["hotspot_data"]["rects"][iStr]["y1"] = static_cast<int>(rect.y1);
								kv["resources"]["hotspot_data"]["rects"][iStr]["x2"] = static_cast<int>(rect.x2);
								kv["resources"]["hotspot_data"]["rects"][iStr]["y2"] = static_cast<int>(rect.y2);
							}
						} else {
							kv["resources"]["hotspot_data"]["malformed"] = true;
						}
					}
					if (const auto* ts0ResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_EXTENDED_FLAGS)) {
						kv["resources"]["ts0"] = static_cast<int>(ts0ResourcePtr->getDataAsExtendedFlags());
					}
					if (const auto* crcResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_CRC)) {
						kv["resources"]["crc"] = static_cast<int>(crcResourcePtr->getDataAsCRC());
					}
					if (const auto* lodResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_LOD_CONTROL_INFO)) {
						const auto lod = lodResourcePtr->getDataAsLODControlInfo();
						kv["resources"]["lod"]["u"] = static_cast<int>(std::get<0>(lod));
						kv["resources"]["lod"]["v"] = static_cast<int>(std::get<1>(lod));
						kv["resources"]["lod"]["u360"] = static_cast<int>(std::get<2>(lod));
						kv["resources"]["lod"]["v360"] = static_cast<int>(std::get<3>(lod));
					}
					if (const auto* kvdResourcePtr = vtf.getResource(vtfpp::Resource::TYPE_KEYVALUES_DATA)) {
						kv["resources"]["kvd"] = kvdResourcePtr->getDataAsKeyValuesData();
					}

					// ...and print it all out
					tfout << kv.bake();
				} else {
					throw std::runtime_error{"Invalid info output mode specified!"};
				}
				return EXIT_SUCCESS;
			};

			// Check input path
			if (inputPath.empty() || !std::filesystem::exists(inputPath)) {
				// Hack: if the input path is "version", show program version and exit
				// This is for Stefan and other tool devs
				if (inputPath == "version") {
					std::cout << PROJECT_VERSION << std::endl;
					return EXIT_SUCCESS;
				}

				throw std::invalid_argument{"Input path does not exist!"};
			}

			if (std::filesystem::is_regular_file(inputPath)) {
				if (!inputPath.ends_with(".vtf") && !inputPath.ends_with(".xtf")) {
					throw std::invalid_argument{"Input file must be a VTF!"};
				}
				const auto out = info(inputPath);
				tfout << tfendl;
				return out;
			}
			if (std::filesystem::is_directory(inputPath)) {
				int out = EXIT_SUCCESS;
				if (noRecurse) {
					for (const auto& dirEntry : std::filesystem::directory_iterator{inputPath, DIR_OPTIONS}) {
						if (sourcepp::string::toLower(dirEntry.path().extension().string()) == ".vtf" || sourcepp::string::toLower(dirEntry.path().extension().string()) == ".xtf") {
							outputPath = "";
							out = out || info(dirEntry.path().string());
						}
					}
				} else {
					for (const auto& dirEntry : std::filesystem::recursive_directory_iterator{inputPath, DIR_OPTIONS}) {
						if (sourcepp::string::toLower(dirEntry.path().extension().string()) == ".vtf" || sourcepp::string::toLower(dirEntry.path().extension().string()) == ".xtf") {
							outputPath = "";
							if (infoOutputMode == "kv1") {
								tfout << '\"' << dirEntry.path().string() << "\"\n{" << tfendl;
							}
							out = out || info(dirEntry.path().string());
							if (infoOutputMode == "human") {
								tfout << tfendl;
							} else if (infoOutputMode == "kv1") {
								tfout << '}' << tfendl;
							}
						}
					}
				}
				return out;
			}
			throw std::invalid_argument{"Input path is not a file or directory!"};
		}
	} catch (const std::exception& e) {
		if (argc > 1) {
			std::cerr << e.what() << '\n' << std::endl;
			std::cerr << "Run " << argv[0] << " with no arguments for usage information." << '\n' << std::endl;
		} else {
			std::cout << cli << std::endl;
		}
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
