#include "CommonThumbnailer.h"

#include <QPainter>

namespace {

[[nodiscard]] std::vector<std::byte> drawCubemapNetCentered(uint16_t& faceWidth, uint16_t& faceHeight, const QImage& xp, const QImage& xn, const QImage& yp, const QImage& yn, const QImage& zp, const QImage& zn, const QImage& sm) {
	QImage net{faceWidth * 4, faceHeight * 4, QImage::Format_ARGB32_Premultiplied};
	net.fill(Qt::transparent);

	QPainter painter{&net};
	painter.drawImage(faceWidth * 1, faceHeight * 0 + faceHeight / 2, yp);
	painter.drawImage(faceWidth * 0, faceHeight * 1 + faceHeight / 2, xn);
	painter.drawImage(faceWidth * 1, faceHeight * 1 + faceHeight / 2, zp);
	painter.drawImage(faceWidth * 2, faceHeight * 1 + faceHeight / 2, xp);
	painter.drawImage(faceWidth * 3, faceHeight * 1 + faceHeight / 2, zn);
	painter.drawImage(faceWidth * 1, faceHeight * 2 + faceHeight / 2, yn);
	if (!sm.isNull()) {
		painter.drawImage(faceWidth * 2, faceHeight * 2 + faceHeight / 2, sm);
	}
	painter.end();

	faceWidth *= 4;
	faceHeight *= 4;

	return {reinterpret_cast<const std::byte*>(net.constBits()), reinterpret_cast<const std::byte*>(net.constBits()) + net.sizeInBytes()};
}

} // namespace

std::pair<std::vector<std::byte>, vtfpp::ImageFormat> createThumbnail(const vtfpp::VTF& vtf, int& targetWidth, int& targetHeight) {
	try {
		if (!vtf) {
			return {};
		}

		auto width = vtf.getWidth();
		auto height = vtf.getHeight();
		vtfpp::ImageFormat format;
		std::vector<std::byte> data;

		if (vtf.getFaceCount() >= 6) {
			// Cubemap
			// Ignore alpha because it's unused uninitialized memory
			const auto xpData = vtf.getImageDataAs(vtfpp::ImageFormat::RGB888, 0, 0, 0);
			const auto xnData = vtf.getImageDataAs(vtfpp::ImageFormat::RGB888, 0, 0, 1);
			const auto ypData = vtf.getImageDataAs(vtfpp::ImageFormat::RGB888, 0, 0, 2);
			const auto ynData = vtf.getImageDataAs(vtfpp::ImageFormat::RGB888, 0, 0, 3);
			const auto zpData = vtf.getImageDataAs(vtfpp::ImageFormat::RGB888, 0, 0, 4);
			const auto znData = vtf.getImageDataAs(vtfpp::ImageFormat::RGB888, 0, 0, 5);
			const auto smData = vtf.getFaceCount() > 6 ? vtf.getImageDataAs(vtfpp::ImageFormat::RGB888, 0, 0, 6) : std::vector<std::byte>{};

			const QImage xp{reinterpret_cast<const uchar*>(xpData.data()), width, height, QImage::Format_RGB888};
			const QImage xn{reinterpret_cast<const uchar*>(xnData.data()), width, height, QImage::Format_RGB888};
			const QImage yp{reinterpret_cast<const uchar*>(ypData.data()), width, height, QImage::Format_RGB888};
			const QImage yn{reinterpret_cast<const uchar*>(ynData.data()), width, height, QImage::Format_RGB888};
			const QImage zp{reinterpret_cast<const uchar*>(zpData.data()), width, height, QImage::Format_RGB888};
			const QImage zn{reinterpret_cast<const uchar*>(znData.data()), width, height, QImage::Format_RGB888};
			const auto sm = !smData.empty() ? QImage{reinterpret_cast<const uchar*>(smData.data()), width, height, QImage::Format_RGB888} : QImage{};

			data = ::drawCubemapNetCentered(width, height, xp, xn, yp, yn, zp, zn, sm);
			format = vtfpp::ImageFormat::BGRA8888;
		} else {
			// Regular texture
			data = vtf.getImageDataAsRGBA8888();
			format = vtfpp::ImageFormat::RGBA8888;
		}

		if ((targetWidth > 0 && width != targetWidth) || (targetHeight > 0 && height != targetHeight)) {
			if (targetWidth <= 0) {
				targetWidth = width;
			}
			if (targetHeight <= 0) {
				targetHeight = height;
			}
			return {vtfpp::ImageConversion::resizeImageData(data, format, width, targetWidth, height, targetHeight, vtf.isSRGB(), vtfpp::ImageConversion::ResizeFilter::BILINEAR), format};
		}
		targetWidth = width;
		targetHeight = height;
		return {data, format};
	} catch (const std::overflow_error&) {
		return {};
	} catch (const std::runtime_error&) {
		return {};
	}
}

std::pair<std::vector<std::byte>, vtfpp::ImageFormat> createThumbnail(const std::string& in, int& targetWidth, int& targetHeight) {
	try {
		const vtfpp::VTF vtf{in};
		return ::createThumbnail(vtf, targetWidth, targetHeight);
	} catch (const std::overflow_error&) {
		return {};
	} catch (const std::runtime_error&) {
		return {};
	}
}
