#include "QMareEmptyWindow.h"

#include <QFileDialog>
#include <QPainter>
#include <QPushButton>
#include <QScreen>

#include "../common/Config.h"
#include "QMareTextureWindow.h"

QMareEmptyWindow::QMareEmptyWindow() : QMainWindow(nullptr) {
	this->setWindowTitle(PROJECT_TITLE);
	this->setWindowIcon(QIcon{":/logo.png"});
	this->resize(this->screen()->availableGeometry().size() * 0.6);
	this->setMinimumSize(400, 250);

	this->newTexture = new QPushButton{QIcon{":/button_new.png"}, "", this};
	this->newTexture->setToolTip(tr("New Texture"));
	this->newTextures = new QPushButton{QIcon{":/button_new_multi.png"}, "", this};
	this->newTextures->setToolTip(tr("New Textures"));
	this->loadTextures = new QPushButton{QIcon{":/button_load.png"}, "", this};
	this->loadTextures->setToolTip(tr("Load Textures"));

	for (auto* button : {this->newTexture, this->newTextures, this->loadTextures}) {
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
	connect(this->loadTextures, &QPushButton::clicked, this, [this] {
		if (const auto files = QFileDialog::getOpenFileNames(this, tr("Load Textures"), {}, QString{"Valve Texture Format (*.vtf *.xtf);;"} + tr("All Files %1").arg("(*)")); !files.empty()) {
			auto* window = new QMareTextureWindow;
			for (const auto& file : files) {
				window->loadTexture(file);
			}
			window->show();
			this->close();
		}
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
	this->newTexture->setGeometry(QRect{static_cast<int>(0.14 * this->width() - static_cast<float>(this->newTexture->width()) / 2), static_cast<int>(0.4 * this->height() - static_cast<float>(this->newTexture->height())), this->newTexture->width(), this->newTexture->height()});
	this->newTextures->setGeometry(QRect{static_cast<int>(0.24 * this->width() - static_cast<float>(this->newTextures->width()) / 2), static_cast<int>(0.55 * this->height() - static_cast<float>(this->newTextures->height())), this->newTextures->width(), this->newTextures->height()});
	this->loadTextures->setGeometry(QRect{static_cast<int>(0.12 * this->width() - static_cast<float>(this->loadTextures->width()) / 2), static_cast<int>(0.65 * this->height() - static_cast<float>(this->loadTextures->height())), this->loadTextures->width(), this->loadTextures->height()});
}
