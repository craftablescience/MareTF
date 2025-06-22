#include "QMareEmptyWindow.h"

#include <array>

#include <QPainter>
#include <QPushButton>
#include <QScreen>

#include "../common/Config.h"
#include "QMareTextureWindow.h"

QMareEmptyWindow::QMareEmptyWindow() : QMainWindow(nullptr) {
	this->setWindowTitle(PROJECT_TITLE);
	this->setWindowIcon(QIcon(":/logo.png"));
	this->resize(this->screen()->availableGeometry().size() * 0.6);
	this->setMinimumSize(400, 250);

	this->newTexture = new QPushButton{QIcon{":/logo.png"}, "", this};
	this->newTextures = new QPushButton{QIcon{":/logo.png"}, "", this};
	this->loadTexture = new QPushButton{QIcon{":/logo.png"}, "", this};

	for (auto* button : std::array{this->newTexture, this->newTextures, this->loadTexture}) {
		static constexpr auto BUTTON_SIZE = 80;
		button->setFlat(true);
		button->setIconSize(QSize{BUTTON_SIZE, BUTTON_SIZE});
		button->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
	}

	connect(this->newTexture, &QPushButton::clicked, this, [this] {
		// todo: create new texture from image
	});
	connect(this->newTextures, &QPushButton::clicked, this, [this] {
		// todo: create new textures from folder
	});
	connect(this->loadTexture, &QPushButton::clicked, this, [this] {
		// todo: open file dialog with multi select, then open QMareTextureWindow and load all textures
	});
}

void QMareEmptyWindow::paintEvent(QPaintEvent*) {
	QPainter painter{this};

	static const QPixmap backgroundGradientImage{":/background_gradient.png"};
	painter.drawPixmap(0, 0, this->width(), this->height(), backgroundGradientImage);

	static const QPixmap envSunImage{":/env_sun.png"};
	static constexpr auto envSunScaleFactor = 0.1;
	const auto envSunScale = envSunScaleFactor * this->width() > envSunScaleFactor * this->height() ? envSunScaleFactor * this->width() : envSunScaleFactor * this->height();
	painter.drawPixmap(static_cast<int>(0.75 * this->width() - envSunScaleFactor / 2), static_cast<int>(0.1 * this->height() - envSunScaleFactor / 2), static_cast<int>(envSunScale), static_cast<int>(envSunScale), envSunImage);

	static const QPixmap backgroundLandscapeImage{":/background_landscape.png"};
	painter.drawPixmap(0, 0, this->width(), this->height(), backgroundLandscapeImage);

	static const QPixmap oliveShadeSleepImage{":/olive_shade_sleep.png"};
	static constexpr auto oliveShadeScaleFactor = 0.25;
	const auto oliveShadeScale = oliveShadeScaleFactor * this->width() > oliveShadeScaleFactor * this->height() ? oliveShadeScaleFactor * this->width() : oliveShadeScaleFactor * this->height();
	painter.drawPixmap(static_cast<int>(0.55 * this->width() - oliveShadeScale / 2), static_cast<int>(0.8 * this->height() - oliveShadeScale), static_cast<int>(oliveShadeScale), static_cast<int>(oliveShadeScale), oliveShadeSleepImage);
}

void QMareEmptyWindow::resizeEvent(QResizeEvent*) {
	this->newTexture->setGeometry(QRect{static_cast<int>(0.14 * this->width()), static_cast<int>(0.4 * this->height()), this->newTexture->width(), this->newTexture->height()});
	this->newTextures->setGeometry(QRect{static_cast<int>(0.24 * this->width()), static_cast<int>(0.55 * this->height()), this->newTextures->width(), this->newTextures->height()});
	this->loadTexture->setGeometry(QRect{static_cast<int>(0.12 * this->width()), static_cast<int>(0.65 * this->height()), this->loadTexture->width(), this->loadTexture->height()});
}
