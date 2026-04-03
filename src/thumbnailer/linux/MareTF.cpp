#include <argparse/argparse.hpp>
#include <sourcepp/FS.h>
#include <vtfpp/VTF.h>

#include "Config.h"

namespace {

std::vector<std::byte> createThumbnail(const std::string& in, int& targetWidth, int& targetHeight) {
	try {
		const vtfpp::VTF vtf{in};
		if (!vtf) {
			return {};
		}
		auto data = vtf.getImageDataAsRGBA8888();
		if ((targetWidth > 0 && vtf.getWidth() != targetWidth) || (targetHeight > 0 && vtf.getHeight() != targetHeight)) {
			if (targetWidth <= 0) {
				targetWidth = vtf.getWidth();
			}
			if (targetHeight <= 0) {
				targetHeight = vtf.getHeight();
			}
			return vtfpp::ImageConversion::resizeImageData(data, vtfpp::ImageFormat::RGBA8888, vtf.getWidth(), targetWidth, vtf.getHeight(), targetHeight, vtf.isSRGB(), vtfpp::ImageConversion::ResizeFilter::BILINEAR);
		}
		targetWidth = vtf.getWidth();
		targetHeight = vtf.getHeight();
		return data;
	} catch (const std::overflow_error&) {
		return {};
	} catch (const std::runtime_error&) {
		return {};
	}
}

int createThumbnail(const std::string& in, const std::string& out, int targetWidth, int targetHeight) {
	auto data = ::createThumbnail(in, targetWidth, targetHeight);
	if (data.empty()) {
		return 2;
	}
	if (out.ends_with(".jpg") || out.ends_with(".jpeg")) {
		return !sourcepp::fs::writeFileBuffer(out, vtfpp::ImageConversion::convertImageDataToFile(data, vtfpp::ImageFormat::RGBA8888, targetWidth, targetHeight, vtfpp::ImageConversion::FileFormat::JPG));
	}
	if (out.ends_with(".png")) {
		return !sourcepp::fs::writeFileBuffer(out, vtfpp::ImageConversion::convertImageDataToFile(data, vtfpp::ImageFormat::RGBA8888, targetWidth, targetHeight, vtfpp::ImageConversion::FileFormat::PNG));
	}
	if (out.ends_with(".tga")) {
		return !sourcepp::fs::writeFileBuffer(out, vtfpp::ImageConversion::convertImageDataToFile(data, vtfpp::ImageFormat::RGBA8888, targetWidth, targetHeight, vtfpp::ImageConversion::FileFormat::TGA));
	}
	return 3;
}

} // namespace

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
