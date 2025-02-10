#include <cstdlib>
#include <chrono>
#include <filesystem>
#include <exception>
#include <random>

#include <argparse/argparse.hpp>
#include <sourcepp/String.h>
#include <vtfpp/vtfpp.h>

#include "../common/Config.h"
#include "../common/EnumMappings.h"

#ifdef _WIN32
#define CP_UTF8 65001
extern "C" __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int);
#endif

namespace {

int randomInt(int from, int to) {
	static std::random_device device;
	static std::mt19937 generator(device());
	std::uniform_int_distribution<int> dist{from, to};
	return dist(generator);
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

	std::string mode;
	cli
		.add_argument("mode")
		.metavar("MODE")
		.help("The mode to run the program in.")
		.choices("create")
		.required()
		.store_into(mode);

	std::string inputPath;
	cli
		.add_argument("path")
		.metavar("PATH")
		.help("The path to the input file.")
		.required()
		.store_into(inputPath);

	bool overwrite;
	cli
		.add_argument("-y")
		.help("Automatically say yes to any prompts.")
		.flag()
		.store_into(overwrite);

	auto& createCLI = cli.add_group(R"("create" mode)");

	std::string outputPath;
	createCLI
		.add_argument("-o", "--output")
		.metavar("PATH")
		.help("The path to the output VTF.")
		.default_value<std::string>("")
		.store_into(outputPath);

	std::string version;
	createCLI
		.add_argument("-v", "--version")
		.metavar("X.Y")
		.help("Major and minor version, split by a period.")
		.choices("7.0", "7.1", "7.2", "7.3", "7.4", "7.5", "7.6")
		.default_value<std::string>("7.4")
		.store_into(version);

	std::string format;
	auto& formatArg = createCLI
		.add_argument("-f", "--format")
		.metavar("IMAGE_FORMAT")
		.help("Output format.");
	for (auto name : not_magic_enum::enum_names<vtfpp::ImageFormat>()) {
		formatArg.add_choice(name);
	}
	formatArg.default_value<std::string>("DEFAULT");
	formatArg.store_into(format);

	std::string filter;
	auto& filterArg = createCLI
		.add_argument("-r", "--filter")
		.metavar("RESIZE_FILTER")
		.help("The resize filter used to generate mipmaps and when resizing the base texture to match a power of 2 (if necessary).");
	for (auto name : not_magic_enum::enum_names<vtfpp::ImageConversion::ResizeFilter>()) {
		filterArg.add_choice(name);
	}
	filterArg.default_value(std::string{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeFilter::KAISER)});
	filterArg.store_into(filter);

	std::vector<std::string> flags;
	auto& flagsArg = createCLI
		.add_argument("--flag")
		.metavar("FLAG")
		.help("Extra flags to apply to the VTF. ENVMAP, ONE_BIT_ALPHA, MULTI_BIT_ALPHA, NO_MIP, and NO_LOD flags are applied automatically based on the VTF properties.")
		.append();
	for (auto [flag, name] : not_magic_enum::enum_entries<vtfpp::VTF::Flags>()) {
		if (!(flag & vtfpp::VTF::FLAG_MASK_GENERATED)) {
			flagsArg.add_choice(name);
		}
	}
	flagsArg.store_into(flags);

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

	std::string platform;
	auto& platformArg = createCLI
		.add_argument("-p", "--platform")
		.metavar("PLATFORM")
		.help("Set the platform (PC/console) the VTF will be built for.");
	for (auto name : not_magic_enum::enum_names<vtfpp::VTF::Platform>()) {
		platformArg.add_choice(name);
	}
	platformArg.default_value(std::string{not_magic_enum::enum_name(vtfpp::VTF::Platform::PLATFORM_PC)});
	platformArg.store_into(platform);

	std::string compressionMethod;
	auto& compressionMethodArg = createCLI
		.add_argument("-m", "--compression-method")
		.metavar("COMPRESSION_METHOD")
		.help("Set the compression method. Deflate is supported on all Strata Source games for VTF v7.6. Zstd is supported on all Strata Source games for VTF v7.6 besides Portal: Revolution. LZMA is supported for console VTFs.");
	for (auto name : not_magic_enum::enum_names<vtfpp::CompressionMethod>()) {
		compressionMethodArg.add_choice(name);
	}
	compressionMethodArg.default_value(std::string{not_magic_enum::enum_name(vtfpp::CompressionMethod::ZSTD)});
	compressionMethodArg.store_into(compressionMethod);

	int compressionLevel;
	createCLI
		.add_argument("-c", "--compression-level")
		.metavar("LEVEL")
		.help("Set the compression level. -1 to 9 for Deflate and LZMA, -1 to 22 for Zstd.")
		.default_value<int>(6)
		.store_into(compressionLevel);

	int startFrame;
	createCLI
		.add_argument("--start-frame")
		.metavar("FRAME_INDEX")
		.help("The start frame used in animations, counting from zero. Ignored when creating console VTFs.")
		.default_value<int>(0)
		.store_into(startFrame);

	float bumpMapScale;
	createCLI
		.add_argument("--bumpmap-scale")
		.metavar("SCALE")
		.help("The bumpmap scale. It can have a decimal point.")
		.default_value<float>(1.f)
		.store_into(bumpMapScale);

	std::string widthResizeMethod;
	auto& widthResizeMethodArg = createCLI
			.add_argument("--width-resize-method")
			.metavar("RESIZE_METHOD")
			.help("How to resize the texture's width to match a power of 2.");
	for (auto name : not_magic_enum::enum_names<vtfpp::ImageConversion::ResizeMethod>()) {
		widthResizeMethodArg.add_choice(name);
	}
	widthResizeMethodArg.default_value(std::string{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_BIGGER)});
	widthResizeMethodArg.store_into(widthResizeMethod);

	std::string heightResizeMethod;
	auto& heightResizeMethodArg = createCLI
			.add_argument("--height-resize-method")
			.metavar("RESIZE_METHOD")
			.help("How to resize the texture's height to match a power of 2.");
	for (auto name : not_magic_enum::enum_names<vtfpp::ImageConversion::ResizeMethod>()) {
		heightResizeMethodArg.add_choice(name);
	}
	heightResizeMethodArg.default_value(std::string{not_magic_enum::enum_name(vtfpp::ImageConversion::ResizeMethod::POWER_OF_TWO_BIGGER)});
	heightResizeMethodArg.store_into(heightResizeMethod);

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
		PROJECT_NAME_PRETTY " — version v" PROJECT_VERSION " — created by craftablescience — licensed under MIT\n\n"
		"Sample usage:\n"
		" •\033[32m" " maretf create input.png --version 7.4 --format UNCHANGED --filter KAISER " "\033[0m"
	};

	cli.add_epilog(enumInfo + std::string{PROGRAM_DETAILS});

	try {
		cli.parse_args(argc, argv);

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
					std::cout << "Output file already exists. Overwrite? (y/N) ";
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
			sourcepp::string::toInt(std::string_view{&version[0], 1}, options.majorVersion);
			sourcepp::string::toInt(std::string_view{&version[2], 1}, options.minorVersion);

			// Set format
			if (format == "UNCHANGED") {
				options.outputFormat = vtfpp::VTF::FORMAT_UNCHANGED;
			} else if (format == "DEFAULT") {
				options.outputFormat = vtfpp::VTF::FORMAT_DEFAULT;
			} else {
				options.outputFormat = *not_magic_enum::enum_cast<vtfpp::ImageFormat>(format);
			}

			// Set filter
			options.filter = *not_magic_enum::enum_cast<vtfpp::ImageConversion::ResizeFilter>(filter);

			// Set flags
			for (const auto& flag : flags) {
				options.flags |= *not_magic_enum::enum_cast<vtfpp::VTF::Flags>(flag);
			}

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
				std::cout << "Compression level range is between -1 and 9/22 (depending on the compression method). Setting compression level to -1..." << std::endl;
			} else if ((options.compressionMethod == vtfpp::CompressionMethod::DEFLATE || options.compressionMethod == vtfpp::CompressionMethod::CONSOLE_LZMA) && compressionLevel > 9) {
				options.compressionLevel = 9;
				std::cout << "Compression level range is between -1 and 9 for Deflate and LZMA. Setting compression level to 9..." << std::endl;
			} else if (options.compressionMethod == vtfpp::CompressionMethod::ZSTD && compressionLevel > 22) {
				options.compressionLevel = 22;
				std::cout << "Compression level range is between -1 and 22 for Zstd. Setting compression level to 22..." << std::endl;
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

			// Bake VTF
			static constexpr std::array<std::string_view, 10> DEVIANTART_MLP_TF_PLOT_SUMMARIES{
				"Splicing DNA",
				"Drinking a potion stolen from a wizard neighbor",
				"Tampering with a sacred equine relic",
				"Downloading a mysterious app",
				"Irritating a farmhand who's secretly a witch",
				"Getting isekai'd into Ponyville",
				"Falling into the contraption",
				"Entering the light of the full moon on Nightmare Night",
				"Wearing a saddle on a dare",
				"Doodling a pony with a magic marker",
			};
			std::cout << DEVIANTART_MLP_TF_PLOT_SUMMARIES[::randomInt(0, DEVIANTART_MLP_TF_PLOT_SUMMARIES.size())] << "..." << std::endl;
			::ElapsedTime stopwatch;
			if (!vtfpp::VTF::create(inputPath, outputPath, options)) {
				std::cerr << "Failed to TF input image. Is it a supported format?" << std::endl;
				return EXIT_FAILURE;
			}
			const auto elapsed = stopwatch.get().count();
			std::cout << "Input image was TF'ed in " << elapsed << "ms 💖" << std::endl;
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
