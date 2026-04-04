#include <argparse/argparse.hpp>

#include "Config.h"

#include "CommonThumbnailer.h"

int createThumbnail(const std::string& in, const std::string& out, int targetWidth, int targetHeight) {
	const auto [data, format] = ::createThumbnail(in, targetWidth, targetHeight);
	if (data.empty() || format == vtfpp::ImageFormat::EMPTY) {
		return 2;
	}
	if (out.ends_with(".jpg") || out.ends_with(".jpeg")) {
		return !sourcepp::fs::writeFileBuffer(out, vtfpp::ImageConversion::convertImageDataToFile(data, format, targetWidth, targetHeight, vtfpp::ImageConversion::FileFormat::JPG));
	}
	if (out.ends_with(".png")) {
		return !sourcepp::fs::writeFileBuffer(out, vtfpp::ImageConversion::convertImageDataToFile(data, format, targetWidth, targetHeight, vtfpp::ImageConversion::FileFormat::PNG));
	}
	if (out.ends_with(".tga")) {
		return !sourcepp::fs::writeFileBuffer(out, vtfpp::ImageConversion::convertImageDataToFile(data, format, targetWidth, targetHeight, vtfpp::ImageConversion::FileFormat::TGA));
	}
	return 3;
}

int main(int argc, const char* argv[]) {
	argparse::ArgumentParser cli{PROJECT_NAME_PRETTY " Thumbnailer", PROJECT_VERSION_PRETTY, argparse::default_arguments::help};
	cli.add_argument("-i").help("Input file");
	cli.add_argument("-o").help("Output file");
	cli.add_argument("-s").help("Output size");
	cli.parse_args(argc, argv);

	if (!cli.is_used("-i") || !cli.is_used("-o")) {
		return 4;
	}
	const auto size = cli.is_used("-s") ? std::stoi(cli.get("-s")) : -1;
	return ::createThumbnail(cli.get("-i"), cli.get("-o"), size, size);
}
