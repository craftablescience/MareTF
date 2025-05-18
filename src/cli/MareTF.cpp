#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <exception>
#include <random>

#include <argparse/argparse.hpp>
#include <kvpp/KV1.h>
#include <sourcepp/FS.h>
#include <sourcepp/String.h>
#include <vtfpp/vtfpp.h>

#include "../common/Config.h"
#include "../common/EnumMappings.h"

#ifdef _WIN32
#define CP_UTF8 65001
extern "C" __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int);
#endif

namespace {

std::string_view randomDeviantArtTFTrope() {
	static constexpr std::array<std::string_view, 10> DEVIANTART_TF_TROPES{
		"Splicing DNA",
		"Drinking a potion stolen from a wizard neighbor",
		"Tampering with a sacred equine relic",
		"Downloading a mysterious app",
		"Irritating a farmhand who's secretly a witch",
		"Getting isekai'd into Ponyville",
		"Falling into the contraption",
		"Entering the light of the full moon on Nightmare Night",
		"Wearing a saddle on a dare",
		"Drawing a pony with a magic marker",
	};
	static std::random_device device;
	static std::mt19937 generator(device());
	std::uniform_int_distribution<int> dist{0, DEVIANTART_TF_TROPES.size()};
	return DEVIANTART_TF_TROPES[dist(generator)];
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
		.help(R"(The mode to run the program in. This determines what arguments are processed. Valid options: "create", "edit", and "info".)")
		.choices("create", "edit", "info")
		.required()
		.store_into(mode);

	std::string inputPath;
	cli
		.add_argument("path")
		.metavar("PATH")
		.help("The path to the input file.")
		.required()
		.store_into(inputPath);

	std::string outputPath;
	cli
		.add_argument("-o", "--output")
		.metavar("PATH")
		.help("The path to the output file, if the current mode outputs a file.")
		.store_into(outputPath);

	bool overwrite;
	cli
		.add_argument("-y")
		.help("Automatically say yes to any prompts.")
		.flag()
		.store_into(overwrite);

	bool noPrettyFormatting;
	cli
		.add_argument("--no-pretty-formatting")
		.help("Disables printing ANSI color codes and emojis.")
		.flag()
		.store_into(noPrettyFormatting);

	//endregion

	//region Create Mode Arguments

	auto& createCLI = cli.add_group(R"("create" mode)");

	std::string version{"7.4"};
	createCLI
		.add_argument("-v", "--version")
		.metavar("X.Y")
		.help("Major and minor version, split by a period. Ignored if platform is specified as anything other than PC.")
		.choices("7.0", "7.1", "7.2", "7.3", "7.4", "7.5", "7.6")
		.default_value(version).store_into(version);

	std::string format{not_magic_enum::enum_name(vtfpp::VTF::FORMAT_DEFAULT)};
	auto& formatArg = createCLI
		.add_argument("-f", "--format")
		.metavar("IMAGE_FORMAT")
		.help("Output format.");
	for (auto name : not_magic_enum::enum_names<vtfpp::ImageFormat>()) {
		formatArg.add_choice(name);
	}
	formatArg.default_value(format).store_into(format);

	std::string filter{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeFilter::KAISER)};
	auto& filterArg = createCLI
		.add_argument("-r", "--filter")
		.metavar("RESIZE_FILTER")
		.help("The resize filter used to generate mipmaps and when resizing the base texture to match a power of 2"
		      " (if necessary).");
	for (auto name : not_magic_enum::enum_names<vtfpp::ImageConversion::ResizeFilter>()) {
		filterArg.add_choice(name);
	}
	filterArg.default_value(filter).store_into(filter);

	std::vector<std::string> flags;
	auto& flagsArg = createCLI
		.add_argument("--flag")
		.metavar("FLAG")
		.help("Extra flags to add. ENVMAP, ONE_BIT_ALPHA, MULTI_BIT_ALPHA, NO_MIP, and NO_LOD flags are applied"
		      " automatically based on the VTF properties.")
		.append();
	for (auto [flag, name] : not_magic_enum::enum_entries<vtfpp::VTF::Flags>()) {
		if (!(flag & vtfpp::VTF::FLAG_MASK_INTERNAL)) {
			flagsArg.add_choice(name);
		}
	}
	flagsArg.store_into(flags);

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

	bool noThumbnail;
	createCLI
		.add_argument("--no-thumbnail")
		.help("Disable thumbnail generation.")
		.flag()
		.store_into(noThumbnail);

	std::string platform{not_magic_enum::enum_name(vtfpp::VTF::Platform::PLATFORM_PC)};
	auto& platformArg = createCLI
		.add_argument("-p", "--platform")
		.metavar("PLATFORM")
		.help("Set the platform (PC/console) to build for.");
	for (auto name : not_magic_enum::enum_names<vtfpp::VTF::Platform>()) {
		platformArg.add_choice(name);
	}
	platformArg.default_value(platform).store_into(platform);

	std::string compressionMethod{not_magic_enum::enum_name(vtfpp::CompressionMethod::ZSTD)};
	auto& compressionMethodArg = createCLI
		.add_argument("-m", "--compression-method")
		.metavar("COMPRESSION_METHOD")
		.help("Set the compression method. Deflate is supported on all Strata Source games for VTF v7.6."
		      " Zstd is supported on all Strata Source games for VTF v7.6 besides Portal: Revolution."
		      " LZMA is supported for console VTFs.");
	for (auto name : not_magic_enum::enum_names<vtfpp::CompressionMethod>()) {
		compressionMethodArg.add_choice(name);
	}
	compressionMethodArg.default_value(compressionMethod).store_into(compressionMethod);

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

	std::string widthResizeMethod{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_BIGGER)};
	auto& widthResizeMethodArg = createCLI
			.add_argument("--width-resize-method")
			.metavar("RESIZE_METHOD")
			.help("How to resize the texture's width to match a power of 2.");
	for (auto name : not_magic_enum::enum_names<vtfpp::ImageConversion::ResizeMethod>()) {
		widthResizeMethodArg.add_choice(name);
	}
	widthResizeMethodArg.default_value(widthResizeMethod).store_into(widthResizeMethod);

	std::string heightResizeMethod{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_BIGGER)};
	auto& heightResizeMethodArg = createCLI
			.add_argument("--height-resize-method")
			.metavar("RESIZE_METHOD")
			.help("How to resize the texture's height to match a power of 2.");
	for (auto name : not_magic_enum::enum_names<vtfpp::ImageConversion::ResizeMethod>()) {
		heightResizeMethodArg.add_choice(name);
	}
	heightResizeMethodArg.default_value(heightResizeMethod).store_into(heightResizeMethod);

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
	auto& setFormatArg = editCLI
		.add_argument("--set-format")
		.metavar("IMAGE_FORMAT")
		.help("Set the image format. Keep in mind converting to a lossy format like DXTn means irreversibly losing"
		      " information. Recommended to pair this with the recompute transparency flags argument.");
	for (auto name : not_magic_enum::enum_names<vtfpp::ImageFormat>()) {
		setFormatArg.add_choice(name);
	}
	setFormatArg.store_into(setFormat);

	int setWidth = 0;
	editCLI
		.add_argument("--set-width")
		.metavar("WIDTH")
		.help("Set the lowest mip's width. Ignores power of two resize rule.")
		.scan<'d', int>()
		.store_into(setWidth);

	int setHeight = 0;
	editCLI
		.add_argument("--set-height")
		.metavar("HEIGHT")
		.help("Set the lowest mip's height. Ignores power of two resize rule.")
		.scan<'d', int>()
		.store_into(setHeight);

	std::string editFilter{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeFilter::KAISER)};
	auto& editFilterArg = editCLI
		.add_argument("--edit-filter")
		.metavar("RESIZE_FILTER")
		.help("Use this resize filter for all resizing operations that accept a filter parameter,"
		      " including mipmap generation.");
	for (auto name : not_magic_enum::enum_names<vtfpp::ImageConversion::ResizeFilter>()) {
		editFilterArg.add_choice(name);
	}
	editFilterArg.default_value(editFilter).store_into(editFilter);

	std::vector<std::string> addFlags;
	auto& addFlagsArg = editCLI
		.add_argument("--add-flag")
		.metavar("FLAG")
		.help("Flags to add. ENVMAP and NO_MIP flags are ignored.")
		.append();
	for (auto [flag, name] : not_magic_enum::enum_entries<vtfpp::VTF::Flags>()) {
		if (!(flag & vtfpp::VTF::FLAG_MASK_INTERNAL)) {
			addFlagsArg.add_choice(name);
		}
	}
	addFlagsArg.store_into(addFlags);

	std::vector<std::string> removeFlags;
	auto& removeFlagsArg = editCLI
		.add_argument("--remove-flag")
		.metavar("FLAG")
		.help("Flags to remove. ENVMAP and NO_MIP flags are ignored.")
		.append();
	for (auto [flag, name] : not_magic_enum::enum_entries<vtfpp::VTF::Flags>()) {
		if (!(flag & vtfpp::VTF::FLAG_MASK_INTERNAL)) {
			removeFlagsArg.add_choice(name);
		}
	}
	removeFlagsArg.store_into(removeFlags);

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

	// todo: add/remove/set for frame, face, slice

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
	auto& setPlatformArg = editCLI
		.add_argument("--set-platform")
		.metavar("PLATFORM")
		.help("Set the VTF platform.");
	for (auto name : not_magic_enum::enum_names<vtfpp::VTF::Platform>()) {
		setPlatformArg.add_choice(name);
	}
	setPlatformArg.store_into(setPlatform);

	std::string setCompressionMethod;
	auto& setCompressionMethodArg = editCLI
		.add_argument("--set-compression-method")
		.metavar("COMPRESSION_METHOD")
		.help("Set the compression method. Deflate is supported on all Strata Source games for VTF v7.6."
		      " Zstd is supported on all Strata Source games for VTF v7.6 besides Portal: Revolution."
		      " LZMA is supported for console VTFs.");
	for (auto name : not_magic_enum::enum_names<vtfpp::CompressionMethod>()) {
		setCompressionMethodArg.add_choice(name);
	}
	setCompressionMethodArg.store_into(compressionMethod);

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

	int setTSOResource;
	editCLI
		.add_argument("--set-tso-resource")
		.metavar("COMBINED_FLAGS")
		.help("Set the TSO (extended flags) resource. You'll have to do the math to combine the flags"
		      " into one integer yourself.")
		.scan<'d', int>()
		.store_into(setTSOResource);

	bool removeTSOResource;
	editCLI
		.add_argument("--remove-tso-resource")
		.help("Remove the TSO (extended flags) resource. If set TSO resource is specified, this argument is ignored.")
		.flag()
		.store_into(removeTSOResource);

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

	//endregion

	//region Info Mode Arguments

	auto& infoCLI = cli.add_group(R"("info" mode)");

	std::string infoOutputMode{"human"};
	createCLI
		.add_argument("--info-output-mode")
		.help(R"(The mode to output information in. Can be "human" or "kv1".)")
		.choices("human", "kv1")
		.default_value(infoOutputMode).store_into(infoOutputMode);

	//endregion

	//todo: extraction mode to pull out images/resources

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
	addEnumInfo.template operator()<vtfpp::ImageFormat>("IMAGE_FORMAT");
	addEnumInfo.template operator()<vtfpp::ImageConversion::ResizeFilter>("RESIZE_FILTER");
	addEnumInfo.template operator()<vtfpp::VTF::Flags>("FLAG");
	addEnumInfo.template operator()<vtfpp::VTF::Platform>("PLATFORM");
	addEnumInfo.template operator()<vtfpp::CompressionMethod>("COMPRESSION_METHOD");
	addEnumInfo.template operator()<vtfpp::ImageConversion::ResizeMethod>("RESIZE_METHOD");

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
		}

		if (mode == "create") {
			// Check input path
			if (inputPath.empty() || !std::filesystem::exists(inputPath) || !std::filesystem::is_regular_file(inputPath)) {
				throw std::invalid_argument{"Input path must be a valid file!"};
			}

			// Check output path
			if (outputPath.empty()) {
				const std::filesystem::path inputPathPath{inputPath};
				outputPath = (inputPathPath.parent_path() / inputPathPath.stem()).string() + ".vtf";
			}
			if (const bool exists = std::filesystem::exists(outputPath); exists && !std::filesystem::is_regular_file(outputPath)) {
				throw std::invalid_argument{"Output path must not be a directory!"};
			} else if (exists && !overwrite) {
				std::string in;
				while (in.empty() || (!in.starts_with('y') && !in.starts_with('Y') && !in.starts_with('n') && !in.starts_with('N'))) {
					std::cout << "Output file already exists. Overwrite? (" << RED << 'y' << END << '/' << GREEN << 'N' << END << ") ";
					std::cin >> in;
				}
				if (in.empty() || in.starts_with('n') || in.starts_with('N')) {
					std::cout << "Output file already exists. Aborting." << std::endl;
					return EXIT_SUCCESS;
				}
			} else if (exists) {
				std::cout << "Output file already exists, overwriting..." << std::endl;
			}

			// Start to set up options
			vtfpp::VTF::CreationOptions options;

			// Set version
			if (version.size() != 3) {
				throw std::runtime_error{"Invalid version!"};
			}
			sourcepp::string::toInt(std::string_view{&version[0], 1}, options.majorVersion);
			sourcepp::string::toInt(std::string_view{&version[2], 1}, options.minorVersion);

			// Set format
			if (format == "UNCHANGED") {
				options.outputFormat = vtfpp::VTF::FORMAT_UNCHANGED;
			} else if (format == "DEFAULT") {
				options.outputFormat = vtfpp::VTF::FORMAT_DEFAULT;
			} else {
				options.outputFormat = *not_magic_enum::enum_cast<vtfpp::ImageFormat>(format);
				if (options.majorVersion == 7 && options.minorVersion == 6 && !!vtfpp::ImageFormatDetails::red(options.outputFormat) + !!vtfpp::ImageFormatDetails::green(options.outputFormat) + !!vtfpp::ImageFormatDetails::blue(options.outputFormat) + !!vtfpp::ImageFormatDetails::alpha(options.outputFormat) == 3) {
					std::cerr << "Formats with 3 channels are not supported on DX11 and will be converted to a format with 4 channels at runtime. Consider using a compressed format such as BC7, or a format with 4 channels such as RGBA8888 or BGRX8888." << std::endl;
				}
			}

			// Set filter
			options.filter = *not_magic_enum::enum_cast<vtfpp::ImageConversion::ResizeFilter>(filter);

			// Set flags
			for (const auto& flag : flags) {
				options.flags |= *not_magic_enum::enum_cast<vtfpp::VTF::Flags>(flag);
			}
			static constexpr auto addSRGBFlag = [](vtfpp::VTF::CreationOptions& opts) {
				opts.flags |= opts.minorVersion > 3 ? vtfpp::VTF::FLAG_SRGB : vtfpp::VTF::FLAG_PWL_CORRECTED;
			};
			if (srgb) {
				addSRGBFlag(options);
			}
			if (clamp_s) {
				options.flags |= vtfpp::VTF::FLAG_CLAMP_S;
			}
			if (clamp_t) {
				options.flags |= vtfpp::VTF::FLAG_CLAMP_T;
			}
			if (clamp_u) {
				options.flags |= vtfpp::VTF::FLAG_CLAMP_U;
			}
			if (point_sample) {
				options.flags |= vtfpp::VTF::FLAG_POINT_SAMPLE;
			}
			if (trilinear) {
				options.flags |= vtfpp::VTF::FLAG_TRILINEAR;
			}
			if (anisotropic) {
				options.flags |= vtfpp::VTF::FLAG_ANISOTROPIC;
			}

			// Set default flags based on input filename
			if (const auto inputStem = std::filesystem::path{inputPath}.stem().string(); inputStem.ends_with("_color") || inputStem.ends_with("-color") || inputStem.ends_with("_colour") || inputStem.ends_with("-colour")) {
				addSRGBFlag(options);
			} else if (inputStem.ends_with("_normal") || inputStem.ends_with("-normal")) {
				options.flags |= vtfpp::VTF::FLAG_NORMAL;
			} else if (inputStem.ends_with("_ssbump") || inputStem.ends_with("-ssbump")) {
				options.flags |= vtfpp::VTF::FLAG_SSBUMP;
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
				std::cout << "Compression level range is between " << CYAN << "-1" << END << " and " << CYAN << '9' << END << '/' << CYAN << "22" << END << " (depending on the compression method). Setting compression level to " << CYAN << "-1" << END << "..." << std::endl;
			} else if ((options.compressionMethod == vtfpp::CompressionMethod::DEFLATE || options.compressionMethod == vtfpp::CompressionMethod::CONSOLE_LZMA) && compressionLevel > 9) {
				options.compressionLevel = 9;
				std::cout << "Compression level range is between " << CYAN << "-1" << END << " and " << CYAN << '9' << END << " for Deflate and LZMA. Setting compression level to " << CYAN << '9' << END << "..." << std::endl;
			} else if (options.compressionMethod == vtfpp::CompressionMethod::ZSTD && compressionLevel > 22) {
				options.compressionLevel = 22;
				std::cout << "Compression level range is between " << CYAN << "-1" << END << " and " << CYAN << "22" << END << " for Zstd. Setting compression level to " << CYAN << "22" << END << "..." << std::endl;
			} else {
				options.compressionLevel = static_cast<int16_t>(compressionLevel);
			}

			// Set start frame
			options.startFrame = static_cast<uint16_t>(startFrame);

			// Set bumpmap scale
			options.bumpMapScale = bumpMapScale;

			// Set resize methods
			options.widthResizeMethod = *not_magic_enum::enum_cast<vtfpp::ImageConversion::ResizeMethod>(widthResizeMethod);
			options.heightResizeMethod = *not_magic_enum::enum_cast<vtfpp::ImageConversion::ResizeMethod>(heightResizeMethod);

			// Set inversion of green channel
			options.invertGreenChannel = invertGreenChannel || invertGreenChannelAlt;

			// Bake VTF
			std::cout << randomDeviantArtTFTrope() << "..." << std::endl;
			::ElapsedTime stopwatch;
			if (!vtfpp::VTF::create(inputPath, outputPath, options)) {
				std::cerr << "Failed to TF input image. Is it a supported format?" << std::endl;
				return EXIT_FAILURE;
			}
			const auto elapsed = stopwatch.get().count();
			std::cout << "Input image was TF'ed in " << CYAN << elapsed << "ms" << END << (noPrettyFormatting ? "" : " 💖") << std::endl;
		} else if (mode == "edit") {
			// Check input path
			if (inputPath.empty() || !std::filesystem::exists(inputPath) || !std::filesystem::is_regular_file(inputPath)) {
				throw std::invalid_argument{"Input path must be a valid file!"};
			} else if (!inputPath.ends_with(".vtf")) {
				throw std::invalid_argument{"Input file must be a VTF!"};
			}

			// Check output path
			if (outputPath.empty()) {
				outputPath = inputPath;
			}
			if (const bool exists = std::filesystem::exists(outputPath); exists && !std::filesystem::is_regular_file(outputPath)) {
				throw std::invalid_argument{"Output path must not be a directory!"};
			} else if (exists && !overwrite) {
				std::string in;
				while (in.empty() || (!in.starts_with('y') && !in.starts_with('Y') && !in.starts_with('n') && !in.starts_with('N'))) {
					std::cout << "Output file already exists. Overwrite? (" << RED << 'y' << END << '/' << GREEN << 'N' << END << ") ";
					std::cin >> in;
				}
				if (in.empty() || in.starts_with('n') || in.starts_with('N')) {
					std::cout << "Output file already exists. Aborting." << std::endl;
					return EXIT_SUCCESS;
				}
			} else if (exists) {
				std::cout << "Output file already exists, overwriting..." << std::endl;
			}

			// Start to set up VTF for editing
			::ElapsedTime loadStopwatch;
			vtfpp::VTF vtf{inputPath};
			if (!vtf) {
				throw std::invalid_argument{"Unable to load input file as a VTF!"};
			}
			std::cout << "Loaded input VTF in " << CYAN << loadStopwatch.get().count() << "ms" << END << (noPrettyFormatting ? "" : " 🐎") << std::endl;
			::ElapsedTime editStopwatch;

			// Get edit filter
			const auto editFilterActual = *not_magic_enum::enum_cast<vtfpp::ImageConversion::ResizeFilter>(editFilter);

			// Get output format
			vtfpp::ImageFormat setFormatActual = vtf.getFormat();
			if (cli.is_used("--set-format")) {
				setFormatActual = *not_magic_enum::enum_cast<vtfpp::ImageFormat>(setFormat);
				if (setFormatActual == vtfpp::VTF::FORMAT_UNCHANGED) {
					setFormatActual = vtf.getFormat();
				} else if (setFormatActual == vtfpp::VTF::FORMAT_DEFAULT) {
					setFormatActual = vtfpp::VTF::getDefaultCompressedFormat(vtf.getFormat(), vtf.getMajorVersion(), vtf.getMinorVersion());
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
				sourcepp::string::toInt(std::string_view{&setVersion[2], 1}, setMinorVersion);
				vtf.setVersion(setMajorVersion, setMinorVersion);
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

			// Recompute/remove mips
			if (recomputeMips) {
				vtf.setMipCount(vtfpp::ImageDimensions::getRecommendedMipCountForDims(setFormatActual, vtf.getWidth(), vtf.getHeight()));
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
					std::cout << "Compression level range is between -1 and 9/22 (depending on the compression method). Setting compression level to -1..." << std::endl;
				} else if ((vtf.getCompressionMethod() == vtfpp::CompressionMethod::DEFLATE || vtf.getCompressionMethod() == vtfpp::CompressionMethod::CONSOLE_LZMA) && setCompressionLevel > 9) {
					setCompressionLevel = 9;
					std::cout << "Compression level range is between -1 and 9 for Deflate and LZMA. Setting compression level to 9..." << std::endl;
				} else if (vtf.getCompressionMethod() == vtfpp::CompressionMethod::ZSTD && setCompressionLevel > 22) {
					setCompressionLevel = 22;
					std::cout << "Compression level range is between -1 and 22 for Zstd. Setting compression level to 22..." << std::endl;
				}
				vtf.setCompressionLevel(static_cast<int16_t>(setCompressionLevel));
			}

			// Modify particle sheet resource
			if (cli.is_used("--set-particle-sheet-resource")) {
				try {
					vtfpp::SHT sht{setParticleSheetResource};
					if (!sht) {
						throw std::overflow_error{""};
					}
					vtf.setParticleSheetResource(sht);
				} catch (const std::overflow_error&) {
					std::cerr << "Failed to parse specified file for particle sheet resource! Check the file exists and has a .sht extension." << std::endl;
				}
			} else if (removeParticleSheetResource) {
				vtf.removeParticleSheetResource();
			}

			// Modify CRC resource
			if (cli.is_used("--set-crc-resource")) {
				vtf.setCRCResource(static_cast<uint32_t>(setCRCResource));
			} else if (removeCRCResource) {
				vtf.removeCRCResource();
			}

			// Modify LOD resource
			if (cli.is_used("--set-lod-resource")) {
				uint8_t setU, setV;
				const auto uv = sourcepp::string::split(setVersion, '.');
				sourcepp::string::toInt(uv[0], setU);
				sourcepp::string::toInt(uv[1], setV);
				vtf.setLODResource(setU, setV);
			} else if (removeLODResource) {
				vtf.removeLODResource();
			}

			// Modify TSO resource
			if (cli.is_used("--set-tso-resource")) {
				vtf.setExtendedFlagsResource(static_cast<uint32_t>(setTSOResource));
			} else if (removeTSOResource) {
				vtf.removeExtendedFlagsResource();
			}

			// Modify KVD resource
			if (cli.is_used("--set-kvd-resource")) {
				if (const auto txt = sourcepp::fs::readFileText(setKVDResource); txt.empty()) {
					std::cerr << "Failed to read contents of specified file for KVD (KeyValues Data) resource! Check the file exists and is not empty." << std::endl;
				} else {
					vtf.setKeyValuesDataResource(txt);
				}
			} else if (removeKVDResource) {
				vtf.removeKeyValuesDataResource();
			}

			// Bake VTF
			if (!vtf.bake(outputPath)) {
				std::cerr << "Failed to edit input VTF." << std::endl;
				return EXIT_FAILURE;
			}
			std::cout << "Edited input VTF in " << CYAN << editStopwatch.get().count() << "ms" << END << (noPrettyFormatting ? "" : " 💖") << std::endl;
		} else if (mode == "info") {
			// Check input path
			if (inputPath.empty() || !std::filesystem::exists(inputPath) || !std::filesystem::is_regular_file(inputPath)) {
				throw std::invalid_argument{"Input path must be a valid file!"};
			} else if (!inputPath.ends_with(".vtf")) {
				throw std::invalid_argument{"Input file must be a VTF!"};
			}

			// Load VTF
			vtfpp::VTF vtf{inputPath};
			if (!vtf) {
				throw std::invalid_argument{"Unable to load input file as a VTF!"};
			}

			if (infoOutputMode == "human") {
				std::cout << GREEN << BOLD << " ――― FORMAT ―――" << END << std::endl;

				std::cout << BOLD << "Platform: " << CYAN << not_magic_enum::enum_name(vtf.getPlatform()) << END << std::endl;

				if (vtf.getPlatform() == vtfpp::VTF::PLATFORM_PC) {
					std::cout << BOLD << "Version:  " << CYAN << vtf.getMajorVersion() << '.' << vtf.getMinorVersion() << END << std::endl;
				}

				std::cout << '\n' << GREEN << BOLD << " ――― IMAGE ―――" << END << std::endl;

				std::cout << BOLD << "Format:        " << CYAN << not_magic_enum::enum_name(vtf.getFormat()) << END << std::endl;
				if (vtf.getSliceCount() > 1) {
					std::cout << BOLD << "Dimensions:    " << CYAN << vtf.getWidth() << END << " x " << CYAN << vtf.getHeight() << END << " x " << CYAN << vtf.getSliceCount() << END << std::endl;
				} else {
					std::cout << BOLD << "Dimensions:    " << CYAN << vtf.getWidth() << END << " x " << CYAN << vtf.getHeight() << END << std::endl;
				}

				std::cout << BOLD << "Flags:         " << END;
				bool first = true;
				for (auto [flag, name] : not_magic_enum::enum_entries<vtfpp::VTF::Flags>()) {
					if (vtf.getFlags() & flag) {
						if (!first) {
							std::cout << " | ";
						}
						first = false;
						std::cout << CYAN << name << END;
					}
				}
				std::cout << std::endl;

				std::cout << BOLD << "Mips:          " << CYAN << static_cast<int>(vtf.getMipCount()) << END << std::endl;
				std::cout << BOLD << "Frames:        " << CYAN << vtf.getFrameCount() << END << std::endl;
				std::cout << BOLD << "Faces:         " << CYAN << static_cast<int>(vtf.getFaceCount()) << END << std::endl;

				std::cout << BOLD << "Reflectivity:  " << END << '[' << CYAN << vtf.getReflectivity()[0] << 'f' << END << ", " << CYAN << vtf.getReflectivity()[1] << 'f' << END << ", " << CYAN << vtf.getReflectivity()[2] << 'f' << END << ']' << std::endl;

				std::cout << BOLD << "Start Frame:   " << END << CYAN << vtf.getStartFrame() << END << std::endl;
				std::cout << BOLD << "Bumpmap Scale: " << END << CYAN << vtf.getBumpMapScale() << 'f' << END << std::endl;

				std::cout << BOLD << "Compression:   " << END;
				if (vtf.getCompressionLevel() == 0) {
					if (vtf.getPlatform() != vtfpp::VTF::PLATFORM_PC && vtf.getCompressionMethod() == vtfpp::CompressionMethod::CONSOLE_LZMA) {
						std::cout << GREEN << not_magic_enum::enum_name(vtf.getCompressionMethod()) << END << std::endl;
					} else {
						std::cout << RED << "Uncompressed" << END << std::endl;
					}
				} else {
					std::cout << GREEN << not_magic_enum::enum_name(vtf.getCompressionMethod()) << END << " (level " << CYAN << vtf.getCompressionLevel() << END << ')' << std::endl;
				}

				std::cout << '\n' << GREEN << BOLD << " ――― RESOURCES ―――" << END << std::endl;

				std::cout << BOLD << "Thumbnail:      ";
				if (vtf.hasThumbnailData()) {
					std::cout << GREEN << "Exists";
				} else {
					std::cout << RED << "Doesn't exist";
				}
				std::cout << END << std::endl;

				std::cout << BOLD << "Particle Sheet: ";
				const auto* particleSheetResource = vtf.getResource(vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA);
				if (particleSheetResource) {
					const auto sheet = particleSheetResource->getDataAsParticleSheet();
					if (!sheet) {
						std::cout << RED << "Exists, but failed to parse";
					} else {
						std::cout << GREEN << "Exists" << END << " — " << BOLD << "Version: " << CYAN << sheet.getVersion() << END << " — " << BOLD << "Sequences: " << CYAN << sheet.getSequences().size();
					}
				} else {
					std::cout << RED << "Doesn't exist";
				}
				std::cout << END << std::endl;

				std::cout << BOLD << "Image:          ";
				if (vtf.hasImageData()) {
					std::cout << GREEN << "Exists";
				} else {
					std::cout << RED << "Doesn't exist (HUH?)";
				}
				std::cout << END << std::endl;

				std::cout << BOLD << "CRC:            ";
				if (const auto* crcResource = vtf.getResource(vtfpp::Resource::TYPE_CRC)) {
					std::cout << GREEN << "Exists" << END << " — " << CYAN << crcResource->getDataAsCRC() << " (base 10)";
				} else {
					std::cout << RED << "Doesn't exist";
				}
				std::cout << END << std::endl;

				std::cout << BOLD << "LOD:            ";
				if (const auto* lodResource = vtf.getResource(vtfpp::Resource::TYPE_LOD_CONTROL_INFO)) {
					const auto lod = lodResource->getDataAsLODControlInfo();
					std::cout << GREEN << "Exists" << END << " — " << BOLD << "U: " << END << CYAN << std::get<0>(lod) << END << " — " << BOLD << "V: " << END << CYAN << std::get<1>(lod);
					if (vtf.getPlatform() != vtfpp::VTF::PLATFORM_PC) {
						std::cout << END << BOLD << "U (360): " << END << CYAN << std::get<2>(lod) << END << " — " << BOLD << "V (360): " << END << CYAN << std::get<3>(lod);
					}
				} else {
					std::cout << RED << "Doesn't exist";
				}
				std::cout << END << std::endl;

				std::cout << BOLD << "KeyValues Data: ";
				const auto* kvdResource = vtf.getResource(vtfpp::Resource::TYPE_KEYVALUES_DATA);
				if (kvdResource) {
					const auto keyvalues = kvdResource->getDataAsKeyValuesData();
					std::cout << GREEN << "Exists" << END << " — " << CYAN << keyvalues.size() << " chars";
				} else {
					std::cout << RED << "Doesn't exist";
				}
				std::cout << END << std::endl;

				std::cout << BOLD << "Extended Flags: ";
				if (const auto* tsoResource = vtf.getResource(vtfpp::Resource::TYPE_EXTENDED_FLAGS)) {
					std::cout << GREEN << "Exists" << END << " — " << CYAN << tsoResource->getDataAsExtendedFlags() << " (base 10)";
				} else {
					std::cout << RED << "Doesn't exist";
				}
				std::cout << END << std::endl;

				if (particleSheetResource) {
					const auto sheet = particleSheetResource->getDataAsParticleSheet();
					if (sheet) {
						std::cout << '\n' << GREEN << BOLD << " ――― PARTICLE SHEET RESOURCE ―――" << END << std::endl;
						std::cout << BOLD << "Version: " << END << CYAN << sheet.getVersion() << END << std::endl;
						for (const auto& sequence : sheet.getSequences()) {
							std::cout << BOLD << "Sequence " << END << CYAN << sequence.id << END << BOLD << ':' << END << std::endl;
							std::cout << '\t' << BOLD << "Total Duration: " << END << CYAN << sequence.durationTotal << 'f' << END << std::endl;
							std::cout << '\t' << BOLD << "Loop:           " << END;
							if (sequence.loop) {
								std::cout << GREEN << "Yes";
							} else {
								std::cout << RED << "No";
							}
							std::cout << END << std::endl;
							for (int i = 0; i < sequence.frames.size(); i++) {
								const auto& frame = sequence.frames.at(i);
								std::cout << '\t' << BOLD << "Frame " << END << CYAN << i << END << BOLD << ':' << END << std::endl;
								std::cout << "\t\t" << BOLD << "Duration: " << END << CYAN << frame.duration << 'f' << END << std::endl;
								std::cout << "\t\t" << BOLD << "Bounds:   ";
								if (sheet.getVersion() < 1) {
									std::cout << '(' << CYAN << frame.bounds.at(0).x1 << 'f' << END << ", " << CYAN << frame.bounds.at(0).y1 << 'f' << END << "), (" << CYAN << frame.bounds.at(0).x2 << 'f' << END << ", " << CYAN << frame.bounds.at(0).y2 << 'f' << END << ')' << std::endl;
								} else {
									std::cout << END << std::endl;
									for (const auto& bound : frame.bounds) {
										std::cout << "\t\t\t" << '(' << CYAN << bound.x1 << 'f' << END << ", " << CYAN << bound.y1 << 'f' << END << "), (" << CYAN << bound.x2 << 'f' << END << ", " << CYAN << bound.y2 << 'f' << END << ')' << std::endl;
									}
								}
							}
						}
					}
				}

				if (kvdResource) {
					std::cout << '\n' << GREEN << BOLD << " ――― KEYVALUES DATA RESOURCE ―――" << END << std::endl;
					std::cout << kvdResource->getDataAsKeyValuesData() << END << std::endl;
				}
			} else if (infoOutputMode == "kv1") {
				kvpp::KV1Writer kv;

				// File format
				kv["format"]["platform"] = not_magic_enum::enum_name(vtf.getPlatform());
				kv["format"]["version_major"] = static_cast<int>(vtf.getMajorVersion());
				kv["format"]["version_minor"] = static_cast<int>(vtf.getMinorVersion());

				// Image
				kv["image"]["format"] = not_magic_enum::enum_name(vtf.getFormat());
				kv["image"]["dimensions"]["width"] = static_cast<int>(vtf.getWidth());
				kv["image"]["dimensions"]["height"] = static_cast<int>(vtf.getHeight());
				kv["image"]["dimensions"]["depth"] = static_cast<int>(vtf.getSliceCount());
				kv["image"]["dimensions"]["mips"] = static_cast<int>(vtf.getMipCount());
				kv["image"]["dimensions"]["frames"] = static_cast<int>(vtf.getFrameCount());
				kv["image"]["dimensions"]["faces"] = static_cast<int>(vtf.getFaceCount());
				kv["image"]["flags"] = static_cast<int>(vtf.getFlags());
				kv["image"]["reflectivity"]["r"] = vtf.getReflectivity()[0];
				kv["image"]["reflectivity"]["g"] = vtf.getReflectivity()[1];
				kv["image"]["reflectivity"]["b"] = vtf.getReflectivity()[2];
				kv["image"]["start_frame"] = static_cast<int>(vtf.getStartFrame());
				kv["image"]["bumpmap_scale"] = vtf.getBumpMapScale();
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

				// Resources
				if (const auto* particleSheetResource = vtf.getResource(vtfpp::Resource::TYPE_PARTICLE_SHEET_DATA)) {
					const auto sheet = particleSheetResource->getDataAsParticleSheet();
					if (sheet) {
						kv["resources"]["particle_sheet"]["malformed"] = false;
						kv["resources"]["particle_sheet"]["version"] = static_cast<int>(sheet.getVersion());
						for (const auto& sequence : sheet.getSequences()) {
							kv["resources"]["particle_sheet"]["sequences"][static_cast<int>(sequence.id)] = static_cast<int>(sequence.id);
							kv["resources"]["particle_sheet"]["sequences"][static_cast<int>(sequence.id)]["duration_total"] = sequence.durationTotal;
							kv["resources"]["particle_sheet"]["sequences"][static_cast<int>(sequence.id)]["loop"] = sequence.loop;
							for (int i = 0; i < sequence.frames.size(); i++) {
								const auto& frame = sequence.frames.at(i);
								kv["resources"]["particle_sheet"]["sequences"][static_cast<int>(sequence.id)]["frames"][i]["duration"] = frame.duration;
								for (int b = 0; b < (sheet.getVersion() < 1 ? 1 : frame.bounds.size()); b++) {
									kv["resources"]["particle_sheet"]["sequences"][static_cast<int>(sequence.id)]["frames"][i]["bounds"][b]["x1"] = frame.bounds.at(b).x1;
									kv["resources"]["particle_sheet"]["sequences"][static_cast<int>(sequence.id)]["frames"][i]["bounds"][b]["y1"] = frame.bounds.at(b).y1;
									kv["resources"]["particle_sheet"]["sequences"][static_cast<int>(sequence.id)]["frames"][i]["bounds"][b]["x2"] = frame.bounds.at(b).x2;
									kv["resources"]["particle_sheet"]["sequences"][static_cast<int>(sequence.id)]["frames"][i]["bounds"][b]["y2"] = frame.bounds.at(b).y2;
								}
							}
						}
					} else {
						kv["resources"]["particle_sheet"]["malformed"] = true;
					}
				}
				if (const auto* crcResource = vtf.getResource(vtfpp::Resource::TYPE_CRC)) {
					kv["resources"]["crc"] = static_cast<int>(crcResource->getDataAsCRC());
				}
				if (const auto* lodResource = vtf.getResource(vtfpp::Resource::TYPE_LOD_CONTROL_INFO)) {
					const auto lod = lodResource->getDataAsLODControlInfo();
					kv["resources"]["lod"]["u"] = static_cast<int>(std::get<0>(lod));
					kv["resources"]["lod"]["v"] = static_cast<int>(std::get<1>(lod));
					kv["resources"]["lod"]["u360"] = static_cast<int>(std::get<2>(lod));
					kv["resources"]["lod"]["v360"] = static_cast<int>(std::get<3>(lod));
				}
				if (const auto* kvdResource = vtf.getResource(vtfpp::Resource::TYPE_KEYVALUES_DATA)) {
					kv["resources"]["kvd"] = kvdResource->getDataAsKeyValuesData();
				}
				if (const auto* tsoResource = vtf.getResource(vtfpp::Resource::TYPE_EXTENDED_FLAGS)) {
					kv["resources"]["tso"] = static_cast<int>(tsoResource->getDataAsExtendedFlags());
				}

				// ...and print it all out
				std::cout << kv.bake() << std::endl;
			} else {
				throw std::runtime_error{"Invalid info output mode specified!"};
			}
		}
	} catch (const std::exception& e) {
		if (argc > 1) {
			std::cerr << e.what() << '\n' << std::endl;
			std::cerr << cli << std::endl;
		} else {
			std::cout << cli << std::endl;
		}
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
