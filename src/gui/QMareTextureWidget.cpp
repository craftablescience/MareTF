#include "QMareTextureWidget.h"

#include <stdexcept>

#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWheelEvent>

QMareTextureWidget::QMareTextureWidget(QWidget* parent) : QWidget(parent) {}

void QMareTextureWidget::loadTexture(const QString& path_) {
	try {
		if (auto loadVTF = vtfpp::VTF{path_.toUtf8().constData()}) {
			this->path = path_;
			this->vtf = std::move(loadVTF);
			this->reloadCurrentTexture();
		}
	} catch (const std::overflow_error&) {}
}

void QMareTextureWidget::reloadCurrentTexture() {
	if (this->vtf) {
		if (useAlpha()) {
			this->textureCurrentData = this->vtf.getImageDataAs(vtfpp::ImageFormat::BGRA8888, this->currentMip, this->currentFrame, this->currentFace, this->currentDepth);
			// I don't want to implement premultiplied alpha calculations, but it's supposed to be faster for sw rendering, so let's do it when there's no alpha lol
			this->textureCurrent = {reinterpret_cast<const uchar*>(this->textureCurrentData.data()), this->vtf.getWidth(this->currentMip), this->vtf.getHeight(this->currentMip), vtfpp::ImageFormatDetails::decompressedAlpha(this->vtf.getFormat()) ? QImage::Format_ARGB32 : QImage::Format_ARGB32_Premultiplied};
		} else {
			this->textureCurrentData = this->vtf.getImageDataAs(vtfpp::ImageFormat::BGR888, this->currentMip, this->currentFrame, this->currentFace, this->currentDepth);
			this->textureCurrent = {reinterpret_cast<const uchar*>(this->textureCurrentData.data()), this->vtf.getWidth(this->currentMip), this->vtf.getHeight(this->currentMip), QImage::Format_BGR888};
		}
		this->update();
	}
}

QIcon QMareTextureWidget::getIcon() const {
	if (this->textureCurrent.isNull()) {
		return {};
	}
	return {QPixmap::fromImage(this->textureCurrent).scaled(64, 64, Qt::KeepAspectRatio)};
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

QMareTextureWidget::operator bool() const {
	return !this->path.isEmpty() && !this->textureCurrent.isNull();
}

bool& QMareTextureWidget::useBackground() {
	static bool background = true;
	return background;
}

bool& QMareTextureWidget::useAlpha() {
	static bool alpha = true;
	return alpha;
}

void QMareTextureWidget::mouseMoveEvent(QMouseEvent* e) {
	if (e->buttons() & Qt::RightButton) {
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
	if (e->buttons() & Qt::RightButton) {
		this->textureOffset = {};
		this->textureZoom = 1.f;

		this->update();
		e->accept();
	} else {
		this->mousePressPosition = e->position();
	}
}

void QMareTextureWidget::paintEvent(QPaintEvent*) {
	const auto availableWidth = this->width() * 0.85;
	const auto availableHeight = this->height() * 0.85;
	const auto aspectRatio = static_cast<double>(this->textureCurrent.width()) / this->textureCurrent.height();

	int actualWidth, actualHeight;
	if (aspectRatio > 1.0) {
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

	QPainter painter{this};
	painter.fillRect(0, 0, this->width(), this->height(), {20, 22, 24});

	if (useBackground() && useAlpha() && vtfpp::ImageFormatDetails::decompressedAlpha(this->vtf.getFormat())) {
		static constexpr auto SQUARE_SIZE = 32;
		for (int x = targetRect.left(), i = 0; x < targetRect.left() + targetRect.width(); x += SQUARE_SIZE, i++) {
			for (int y = targetRect.top(), j = 0; y < targetRect.top() + targetRect.height(); y += SQUARE_SIZE, j++) {
				if ((i + j) % 2 == 0) {
					painter.fillRect(x, y, std::min(SQUARE_SIZE, targetRect.left() + targetRect.width() - x), std::min(SQUARE_SIZE, targetRect.top() + targetRect.height() - y), {214, 214, 214});
				} else {
					painter.fillRect(x, y, std::min(SQUARE_SIZE, targetRect.left() + targetRect.width() - x), std::min(SQUARE_SIZE, targetRect.top() + targetRect.height() - y), {242, 242, 242});
				}
			}
		}
	}

	painter.drawImage(targetRect, this->textureCurrent, QRect{0, 0, this->textureCurrent.width(), this->textureCurrent.height()});
}

void QMareTextureWidget::resizeEvent(QResizeEvent* e) {
	if (!e->oldSize().width() || !e->oldSize().height()) {
		return;
	}
	this->textureOffset.setX(this->textureOffset.x() * e->size().width() / e->oldSize().width());
	this->textureOffset.setY(this->textureOffset.y() * e->size().height() / e->oldSize().height());
}

void QMareTextureWidget::wheelEvent(QWheelEvent* e) {
	this->textureZoom += static_cast<float>(e->angleDelta().y()) / 360.f;
	this->textureZoom = qMax(0.25f, this->textureZoom);

	this->update();
	e->accept();
}
