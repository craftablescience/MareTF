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
			this->textureCurrentData = this->vtf.getImageDataAsRGBA8888();
			this->textureCurrent = QImage{reinterpret_cast<const uchar*>(this->textureCurrentData.data()), this->vtf.getWidth(), this->vtf.getHeight(), QImage::Format_RGBA8888};
		}
	} catch (const std::overflow_error&) {}
}

QIcon QMareTextureWidget::getIcon() const {
	if (this->textureCurrent.isNull()) {
		return {};
	}
	return {QPixmap::fromImage(this->textureCurrent).scaled(64, 64, Qt::KeepAspectRatio)};
}

const vtfpp::VTF& QMareTextureWidget::getVTF() const {
	return this->vtf;
}

vtfpp::VTF& QMareTextureWidget::getVTF() {
	return this->vtf;
}

QMareTextureWidget::operator bool() const {
	return !this->path.isEmpty() && !this->textureCurrent.isNull();
}

void QMareTextureWidget::mouseMoveEvent(QMouseEvent* e) {
	if (e->buttons() & Qt::MiddleButton) {
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
	if (e->buttons() & Qt::MiddleButton) {
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
	painter.fillRect(0, 0, this->width(), this->height(), QColor{32, 32, 32});
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
	this->textureZoom = qMax(1.f, this->textureZoom);

	this->update();
	e->accept();
}
