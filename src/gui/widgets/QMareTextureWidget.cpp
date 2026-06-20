#include "QMareTextureWidget.h"

#include <cmath>
#include <stdexcept>

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QStyle>
#include <QWheelEvent>
#include <vtfpp/ImagePixel.h>

#include "Common.h"

#include "utility/QMareOptions.h"

namespace {

[[nodiscard]] std::vector<std::byte> drawCubemapNet(uint16_t faceWidth, uint16_t faceHeight, const QImage& xp, const QImage& xn, const QImage& yp, const QImage& yn, const QImage& zp, const QImage& zn, const QImage& sm) {
	QImage net{faceWidth * 4, faceHeight * 3, QImage::Format_ARGB32};
	net.fill(Qt::transparent);

	QPainter painter{&net};
	painter.drawImage(faceWidth * 1, faceHeight * 0, yp);
	painter.drawImage(faceWidth * 0, faceHeight * 1, xn);
	painter.drawImage(faceWidth * 1, faceHeight * 1, zp);
	painter.drawImage(faceWidth * 2, faceHeight * 1, xp);
	painter.drawImage(faceWidth * 3, faceHeight * 1, zn);
	painter.drawImage(faceWidth * 1, faceHeight * 2, yn);
	if (!sm.isNull()) {
		painter.drawImage(faceWidth * 2, faceHeight * 2, sm);
	}
	painter.end();

	return {reinterpret_cast<const std::byte*>(net.constBits()), reinterpret_cast<const std::byte*>(net.constBits()) + net.sizeInBytes()};
}

} // namespace

QMareTextureWidget::QMareTextureWidget(QWidget* parent) : QWidget{parent} {
	this->setContextMenuPolicy(Qt::CustomContextMenu);

	auto* contextMenu = new QMenu{this};

	contextMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogSaveButton), tr("&Copy Image"), [this] {
		QApplication::clipboard()->setImage(this->textureCurrent, QClipboard::Clipboard);
	});
	contextMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogSaveButton), tr("&Save Image As..."), [this] {
		this->saveCurrentTexture();
	});

	contextMenu->addSeparator();

	contextMenu->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom &In"), [this] {
		this->applyIncrementalZoom(1.f);
		this->update();
	});

	contextMenu->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom &Out"), [this] {
		this->applyIncrementalZoom(-1.f);
		this->update();
	});

	contextMenu->addAction(QIcon::fromTheme("zoom-original"), tr("Reset &Zoom"), [this] {
		this->textureZoom = 1.f;
		this->update();
	});

	contextMenu->addAction(QIcon::fromTheme("view-restore"), tr("Reset &Pan"), [this] {
		this->textureOffset = {};
		this->update();
	});

	QObject::connect(this, &QMareTextureWidget::customContextMenuRequested, this, [this, contextMenu](const QPoint& pos) {
		if (!this->textureCurrent.isNull()) {
			(void) contextMenu->exec(this->mapToGlobal(pos));
		}
	});
}

void QMareTextureWidget::loadTexture(const QString& path_) {
	try {
#if defined(Q_OS_WASM) || defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
		vtfpp::VTF tmpVTF;
		{
			QFile vtfFile{path_};
			if (!vtfFile.open(QIODevice::ReadOnly)) {
				return;
			}
			const auto data = vtfFile.readAll();
			vtfFile.close();
			tmpVTF = vtfpp::VTF{
				std::span{reinterpret_cast<const std::byte*>(data.data()), static_cast<std::span<const std::byte>::size_type>(data.size())},
				[](std::string path__) {
					sourcepp::string::toLower(path__);
					return path__.ends_with(".hdr.vtf") || path__.ends_with(".hdr.360.vtf") || path__.ends_with(".hdr.ps3.vtf");
				}(std::filesystem::path{path_.toUtf8().constData()}.filename().string()),
			};
		}
#else
		auto tmpVTF = vtfpp::VTF{path_.toUtf8().constData()};
		if (!tmpVTF) {
			return;
		}
#endif
		this->path = path_;
		this->vtf = std::move(tmpVTF);
		if (this->vtf.getFaceCount() >= 6) {
			this->a = false;
		}
		this->reloadCurrentTexture();
	} catch (const std::overflow_error&) {}
}

void QMareTextureWidget::reloadCurrentTexture() {
	this->textureCurrent = QImage{};
	this->textureCurrentData.clear();

	if (this->vtf) {
		vtfpp::ImageFormat format;
		QImage::Format formatQt;
		if (this->a) {
			if (!this->r && !this->g && !this->b && !this->aMask) {
				format = vtfpp::ImageFormat::A8;
				formatQt = QImage::Format_Grayscale8;
			} else {
				format = vtfpp::ImageFormat::BGRA8888;
				formatQt = vtfpp::ImageFormatDetails::decompressedAlpha(this->vtf.getFormat()) ? QImage::Format_ARGB32 : QImage::Format_ARGB32_Premultiplied;
			}
		} else {
			format = vtfpp::ImageFormat::RGB888;
			formatQt = QImage::Format_RGB888;
		}

		const auto width = this->vtf.getWidth(this->currentMip);
		const auto height = this->vtf.getHeight(this->currentMip);
		if (this->vtf.getFaceCount() >= 6 && this->cubemapMode == 0) {
			const auto xpData = this->vtf.getImageDataAs(format, this->currentMip, this->currentFrame, 0, this->currentDepth);
			const auto xnData = this->vtf.getImageDataAs(format, this->currentMip, this->currentFrame, 1, this->currentDepth);
			const auto ypData = this->vtf.getImageDataAs(format, this->currentMip, this->currentFrame, 2, this->currentDepth);
			const auto ynData = this->vtf.getImageDataAs(format, this->currentMip, this->currentFrame, 3, this->currentDepth);
			const auto zpData = this->vtf.getImageDataAs(format, this->currentMip, this->currentFrame, 4, this->currentDepth);
			const auto znData = this->vtf.getImageDataAs(format, this->currentMip, this->currentFrame, 5, this->currentDepth);
			const auto smData = this->vtf.getFaceCount() > 6 ? this->vtf.getImageDataAs(format, this->currentMip, this->currentFrame, 6, this->currentDepth) : std::vector<std::byte>{};

			const QImage xp{reinterpret_cast<const uchar*>(xpData.data()), width, height, formatQt};
			const QImage xn{reinterpret_cast<const uchar*>(xnData.data()), width, height, formatQt};
			const QImage yp{reinterpret_cast<const uchar*>(ypData.data()), width, height, formatQt};
			const QImage yn{reinterpret_cast<const uchar*>(ynData.data()), width, height, formatQt};
			const QImage zp{reinterpret_cast<const uchar*>(zpData.data()), width, height, formatQt};
			const QImage zn{reinterpret_cast<const uchar*>(znData.data()), width, height, formatQt};
			const auto sm = !smData.empty() ? QImage{reinterpret_cast<const uchar*>(smData.data()), width, height, formatQt} : QImage{};

			this->textureCurrentData = ::drawCubemapNet(this->vtf.getWidth(this->currentMip), this->vtf.getHeight(this->currentMip), xp, xn, yp, yn, zp, zn, sm);
			this->textureCurrent = {reinterpret_cast<const uchar*>(this->textureCurrentData.data()), width * 4, height * 3, formatQt = QImage::Format_ARGB32};
		} else {
			this->textureCurrentData = this->vtf.getImageDataAs(format, this->currentMip, this->currentFrame, this->currentFace, this->currentDepth);
			this->textureCurrent = {reinterpret_cast<const uchar*>(this->textureCurrentData.data()), width, height, formatQt};
		}
		if ((!this->r && vtfpp::ImageFormatDetails::red(format) > 0) || (!this->g && vtfpp::ImageFormatDetails::green(format) > 0) || (!this->b && vtfpp::ImageFormatDetails::blue(format) > 0)) {
			for (int y = 0; y < this->textureCurrent.height(); y++) {
				const auto processScanLine = [this, scanLineRaw = this->textureCurrent.scanLine(y)]<vtfpp::ImagePixel::PixelType P> {
					auto* scanLine = reinterpret_cast<P*>(scanLineRaw);
					for (int x = 0; x < this->textureCurrent.width(); x++) {
						if (!this->r) {
							scanLine[x].setR(0);
						}
						if (!this->g) {
							scanLine[x].setG(0);
						}
						if (!this->b) {
							scanLine[x].setB(0);
						}
					}
				};
				if (formatQt == QImage::Format_ARGB32_Premultiplied || formatQt == QImage::Format_ARGB32) {
					processScanLine.operator()<vtfpp::ImagePixel::BGRA8888>();
				} else /*if (formatQt == QImage::Format_RGB888)*/ {
					processScanLine.operator()<vtfpp::ImagePixel::RGB888>();
				}
			}
		}
		this->update();
	}
}

void QMareTextureWidget::saveCurrentTexture() {
	const std::filesystem::path savePath{reinterpret_cast<const char8_t*>(QFileDialog::getSaveFileName(this, tr("Save Image"), QString{}, ::supportedImageFileFormatsForSave().data()).toUtf8().constData())};
	if (savePath.empty()) {
		return;
	}
	const auto fileFormat = ::supportedImageFileFormatExtension(std::filesystem::path{savePath}.extension().string());
	if (this->vtf.getFaceCount() > 1 && this->cubemapMode == 0) {
		// ReSharper disable once CppRedundantCastExpression
		const std::span imageData{reinterpret_cast<const std::byte*>(this->textureCurrent.constBits()), static_cast<std::span<const std::byte>::size_type>(this->textureCurrent.sizeInBytes())};
		const auto fileData = vtfpp::ImageConversion::convertImageDataToFile(imageData, vtfpp::ImageFormat::BGRA8888, this->textureCurrent.width(), this->textureCurrent.height());
		if (!fileData.empty() && sourcepp::fs::writeFileBuffer(savePath, fileData)) {
			return;
		}
	} else if (this->vtf.saveImageToFile(savePath, this->currentMip, this->currentFrame, this->currentFace, this->currentDepth, fileFormat)) {
		return;
	}
	QMessageBox::warning(this, tr("Error"), tr("Failed to save image."));
}

QIcon QMareTextureWidget::getIcon() const {
	if (this->textureCurrent.isNull()) {
		return {};
	}
	return {QPixmap::fromImage(this->textureCurrent).scaled(64, 64, Qt::KeepAspectRatio, QMareOptions::get<bool>(QMareOptions::BOOL_HIGH_QUALITY_THUMBNAILS) ? Qt::SmoothTransformation : Qt::FastTransformation)};
}

QString QMareTextureWidget::getPath() const {
	return this->path;
}

const vtfpp::VTF& QMareTextureWidget::getVTF() const {
	return this->vtf;
}

vtfpp::VTF& QMareTextureWidget::getVTF() {
	return this->vtf;
}

void QMareTextureWidget::applyIncrementalZoom(float increment) {
	this->textureZoom *= std::exp(increment * 0.25f);
	this->textureZoom = qMax(0.25f, this->textureZoom);
	this->update();
}

uint8_t QMareTextureWidget::getCurrentMip() const {
	return this->currentMip;
}

void QMareTextureWidget::setCurrentMip(uint8_t mip) {
	this->currentMip = std::clamp<uint8_t>(mip, 0, this->vtf.getMipCount() - 1);
	this->reloadCurrentTexture();
}

uint16_t QMareTextureWidget::getCurrentFrame() const {
	return this->currentFrame;
}

void QMareTextureWidget::setCurrentFrame(uint16_t frame) {
	this->currentFrame = std::clamp<uint16_t>(frame, 0, this->vtf.getFrameCount() - 1);
	this->reloadCurrentTexture();
}

uint8_t QMareTextureWidget::getCurrentFace() const {
	return this->currentFace;
}

void QMareTextureWidget::setCurrentFace(uint8_t face) {
	this->currentFace = std::clamp<uint8_t>(face, 0, this->vtf.getFaceCount() - 1);
	this->reloadCurrentTexture();
}

uint16_t QMareTextureWidget::getCurrentDepth() const {
	return this->currentDepth;
}

void QMareTextureWidget::setCurrentDepth(uint16_t depth) {
	this->currentDepth = std::clamp<uint16_t>(depth, 0, this->vtf.getDepth() - 1);
	this->reloadCurrentTexture();
}

bool QMareTextureWidget::useR() const {
	return this->r;
}

bool QMareTextureWidget::useG() const {
	return this->g;
}

bool QMareTextureWidget::useB() const {
	return this->b;
}

void QMareTextureWidget::setRGB(bool newR, bool newG, bool newB) {
	this->r = newR;
	this->g = newG;
	this->b = newB;
	this->reloadCurrentTexture();
}

bool QMareTextureWidget::useA() const {
	return this->a;
}

void QMareTextureWidget::setA(bool newA) {
	this->a = newA;
	this->reloadCurrentTexture();
}

bool QMareTextureWidget::useAMask() const {
	return this->aMask;
}

void QMareTextureWidget::setAMask(bool newAMask) {
	this->aMask = newAMask;
	this->reloadCurrentTexture();
}

bool QMareTextureWidget::useTiled() const {
	return this->tiled;
}

void QMareTextureWidget::setTiled(bool newTiled) {
	this->tiled = newTiled;
	this->reloadCurrentTexture();
}

int QMareTextureWidget::getCurrentCubemapMode() const {
	return this->cubemapMode;
}

void QMareTextureWidget::setCurrentCubemapMode(int mode) {
	this->cubemapMode = mode;
	this->reloadCurrentTexture();
}

QMareTextureWidget::operator bool() const {
	return !this->path.isEmpty() && !this->textureCurrent.isNull();
}

void QMareTextureWidget::mouseMoveEvent(QMouseEvent* e) {
	if (!(e->buttons() & Qt::LeftButton || e->buttons() & Qt::MiddleButton)) {
		return;
	}

	QPointF diff{e->position() - this->mousePressPosition};
	diff /= this->textureZoom;

	this->textureOffset.setX(std::clamp<double>(this->textureOffset.x() + diff.x(), static_cast<float>(this->width()) / -2.f, static_cast<float>(this->width()) / 2.f));
	this->textureOffset.setY(std::clamp<double>(this->textureOffset.y() + diff.y(), static_cast<float>(this->height()) / -2.f, static_cast<float>(this->height()) / 2.f));

	this->mousePressPosition = e->position();

	this->update();
	e->accept();
}

void QMareTextureWidget::mousePressEvent(QMouseEvent* e) {
	if (!(e->buttons() & Qt::LeftButton || e->buttons() & Qt::MiddleButton)) {
		return;
	}

	this->mousePressPosition = e->position();
	this->setCursor({Qt::CursorShape::ClosedHandCursor});
}

void QMareTextureWidget::mouseReleaseEvent(QMouseEvent* e) {
	this->setCursor({Qt::CursorShape::ArrowCursor});
}

void QMareTextureWidget::paintEvent(QPaintEvent*) {
	const auto availableWidth = this->width() * 0.85;
	const auto availableHeight = this->height() * 0.85;
	const auto aspectRatio = static_cast<double>(this->textureCurrent.width()) / this->textureCurrent.height();

	int actualWidth, actualHeight;
	if (aspectRatio > 1) {
		actualWidth = static_cast<int>(availableWidth);
		actualHeight = static_cast<int>(availableWidth / aspectRatio);
		if (actualHeight > availableHeight) {
			actualHeight = static_cast<int>(availableHeight);
			actualWidth = static_cast<int>(availableHeight * aspectRatio);
		}
	} else {
		actualHeight = static_cast<int>(availableHeight);
		actualWidth = static_cast<int>(availableHeight * aspectRatio);
		if (actualWidth > availableWidth) {
			actualWidth = static_cast<int>(availableWidth);
			actualHeight = static_cast<int>(availableWidth / aspectRatio);
		}
	}

	const QRect targetRect{
		static_cast<int>(this->textureOffset.x() * this->textureZoom + static_cast<float>(this->width()) / 2.f - static_cast<float>(actualWidth) * this->textureZoom / 2),
		static_cast<int>(this->textureOffset.y() * this->textureZoom + static_cast<float>(this->height()) / 2.f - static_cast<float>(actualHeight) * this->textureZoom / 2),
		static_cast<int>(static_cast<float>(actualWidth) * this->textureZoom),
		static_cast<int>(static_cast<float>(actualHeight) * this->textureZoom),
	};

	static constexpr QColor BACKGROUND_COLOR{20, 22, 24};

	QPainter painter{this};
	painter.fillRect(0, 0, this->width(), this->height(), BACKGROUND_COLOR);

	if (this->useA() && vtfpp::ImageFormatDetails::decompressedAlpha(this->vtf.getFormat())) {
		const QRect backgroundRect = this->rect().intersected(this->tiled ? targetRect.adjusted(-targetRect.width(), -targetRect.height(), targetRect.width(), targetRect.height()) : targetRect);
		if (this->useAMask()) {
			static constexpr auto SQUARE_SIZE = 32;
			for (int x = backgroundRect.left() / SQUARE_SIZE * SQUARE_SIZE; x < backgroundRect.right(); x += SQUARE_SIZE) {
				for (int y = backgroundRect.top() / SQUARE_SIZE * SQUARE_SIZE; y < backgroundRect.bottom(); y += SQUARE_SIZE) {
					if ((x / SQUARE_SIZE + y / SQUARE_SIZE) % 2 == 0) {
						painter.fillRect(qMax(x, backgroundRect.left()), qMax(y, backgroundRect.top()), qMin(SQUARE_SIZE, backgroundRect.right() - x), qMin(SQUARE_SIZE, backgroundRect.bottom() - y), {214, 214, 214});
					} else {
						painter.fillRect(qMax(x, backgroundRect.left()), qMax(y, backgroundRect.top()), qMin(SQUARE_SIZE, backgroundRect.right() - x), qMin(SQUARE_SIZE, backgroundRect.bottom() - y), {242, 242, 242});
					}
				}
			}
		} else {
			painter.fillRect(backgroundRect, Qt::black);
		}
	}

	const QRect srcRect{0, 0, this->textureCurrent.width(), this->textureCurrent.height()};
	painter.drawImage(targetRect, this->textureCurrent, srcRect);

	// Tiles
	if (this->tiled) {
		painter.drawImage(targetRect.adjusted(-targetRect.width(), -targetRect.height(), -targetRect.width(), -targetRect.height()), this->textureCurrent, srcRect);
		painter.drawImage(targetRect.adjusted(0,                   -targetRect.height(), 0,                   -targetRect.height()), this->textureCurrent, srcRect);
		painter.drawImage(targetRect.adjusted(targetRect.width(),  -targetRect.height(), targetRect.width(),  -targetRect.height()), this->textureCurrent, srcRect);
		painter.drawImage(targetRect.adjusted(targetRect.width(),  0,                    targetRect.width(),  0                   ), this->textureCurrent, srcRect);
		painter.drawImage(targetRect.adjusted(targetRect.width(),  targetRect.height(),  targetRect.width(),  targetRect.height() ), this->textureCurrent, srcRect);
		painter.drawImage(targetRect.adjusted(0,                   targetRect.height(),  0,                   targetRect.height() ), this->textureCurrent, srcRect);
		painter.drawImage(targetRect.adjusted(-targetRect.width(), targetRect.height(),  -targetRect.width(), targetRect.height() ), this->textureCurrent, srcRect);
		painter.drawImage(targetRect.adjusted(-targetRect.width(), 0,                    -targetRect.width(), 0                   ), this->textureCurrent, srcRect);
	}

	// Minimap
	if (const auto minimapScaleChoice = QMareOptions::get<int>(QMareOptions::INT_MINIMAP_SCALE); minimapScaleChoice != 0 && !this->geometry().contains(targetRect)) {
		static constexpr auto MINIMAP_SCALE_FACTOR_SMALL = 8;
		static constexpr auto MINIMAP_SCALE_FACTOR_MEDIUM = 5.5;
		static constexpr auto MINIMAP_SCALE_FACTOR_LARGE = 4;
		static constexpr auto MINIMAP_MARGIN = 8;
		static constexpr auto MINIMAP_PADDING = 4;
		static constexpr auto MINIMAP_OVERLAY_BORDER_SIZE = 2;

		auto minimapBackgroundColor = BACKGROUND_COLOR.darker();
		minimapBackgroundColor.setAlpha(0xA0);
		auto minimapOverlayBorderColor = this->palette().color(QPalette::Highlight);
		minimapOverlayBorderColor.setAlpha(0xC0);
		auto minimapOverlayFillColor = this->palette().color(QPalette::Highlight);
		minimapOverlayFillColor.setAlpha(0x60);

		const auto referenceDim = static_cast<int>(qMin(this->rect().width(), this->rect().height()) / (
			minimapScaleChoice == -1
				? MINIMAP_SCALE_FACTOR_SMALL
				: minimapScaleChoice == -2
					? MINIMAP_SCALE_FACTOR_MEDIUM
					: minimapScaleChoice == -3
						? MINIMAP_SCALE_FACTOR_LARGE
						: minimapScaleChoice
		));

		QRect minimapTargetRect;
		if (aspectRatio > 1) {
			minimapTargetRect = QRect{this->rect().right() - referenceDim - MINIMAP_MARGIN, this->rect().top() + MINIMAP_MARGIN, referenceDim, static_cast<int>(static_cast<float>(referenceDim) / aspectRatio)};
		} else {
			const auto minimapWidth = static_cast<int>(static_cast<float>(referenceDim) * aspectRatio);
			minimapTargetRect = QRect{this->rect().right() - minimapWidth - MINIMAP_MARGIN, this->rect().top() + MINIMAP_MARGIN, minimapWidth, referenceDim};
		}
		const auto minimapViewportScaleX = static_cast<float>(minimapTargetRect.width()) / (static_cast<float>(actualWidth) * this->textureZoom);
		const auto minimapViewportScaleY = static_cast<float>(minimapTargetRect.height()) / (static_cast<float>(actualHeight) * this->textureZoom);

		painter.fillRect(minimapTargetRect.adjusted(-MINIMAP_PADDING, -MINIMAP_PADDING, MINIMAP_PADDING, MINIMAP_PADDING), minimapBackgroundColor);
		painter.drawImage(minimapTargetRect, this->textureCurrent.scaled(minimapTargetRect.width(), minimapTargetRect.height(), Qt::IgnoreAspectRatio, QMareOptions::get<bool>(QMareOptions::BOOL_HIGH_QUALITY_MINIMAP) ? Qt::SmoothTransformation : Qt::FastTransformation), {0, 0, minimapTargetRect.width(), minimapTargetRect.height()});

		painter.setPen({minimapOverlayBorderColor, MINIMAP_OVERLAY_BORDER_SIZE});
		painter.setBrush({minimapOverlayFillColor});
		painter.drawRect(QRect{
			minimapTargetRect.left() + static_cast<int>(static_cast<float>(this->rect().left() - targetRect.left()) * minimapViewportScaleX),
			minimapTargetRect.top() + static_cast<int>(static_cast<float>(this->rect().top() - targetRect.top()) * minimapViewportScaleY),
			static_cast<int>(static_cast<float>(this->rect().width()) * minimapViewportScaleX),
			static_cast<int>(static_cast<float>(this->rect().height()) * minimapViewportScaleY)
		});
	}
}

void QMareTextureWidget::resizeEvent(QResizeEvent* e) {
	if (!e->oldSize().width() || !e->oldSize().height()) {
		return;
	}
	this->textureOffset.setX(this->textureOffset.x() * e->size().width() / e->oldSize().width());
	this->textureOffset.setY(this->textureOffset.y() * e->size().height() / e->oldSize().height());
}

void QMareTextureWidget::wheelEvent(QWheelEvent* e) {
	this->applyIncrementalZoom(static_cast<float>(e->angleDelta().y()) / 360.f);
	e->accept();
}
