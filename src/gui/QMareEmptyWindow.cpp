#include "QMareEmptyWindow.h"

#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QScreen>
#include <QStyle>
#include <QToolBar>

#include "../common/Config.h"
#include "QMareCredits.h"
#include "QMareTextureWindow.h"

QMareEmptyWindow::QMareEmptyWindow() : QMainWindow(nullptr) {
	this->setWindowTitle(PROJECT_TITLE);
	this->setWindowIcon(QIcon{":/logo.png"});

	this->toolbar = new QToolBar{this};
	this->toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	this->toolbar->setMovable(false);
	this->toolbar->setAutoFillBackground(true);

	auto* toolbarExpanderBegin = new QWidget{this};
	toolbarExpanderBegin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->toolbar->addWidget(toolbarExpanderBegin);

	this->toolbar->addAction(QIcon{":/button_new.png"}, tr("&Create"), Qt::CTRL | Qt::Key_N, [] {
		// todo: create new texture from image
	})->setDisabled(true);

	this->toolbar->addAction(QIcon{":/button_new_multi.png"}, tr("Create en &Masse"), Qt::CTRL | Qt::SHIFT | Qt::Key_N, [] {
		// todo: create new textures from folder
	})->setDisabled(true);

	this->toolbar->addSeparator();

	this->toolbar->addAction(QIcon{":/button_load.png"}, tr("&Load"), Qt::CTRL | Qt::Key_O, [this] {
		if (const auto files = QFileDialog::getOpenFileNames(this, tr("Load Textures"), {}, QString{"Valve Texture Format (*.vtf *.xtf);;"} + tr("All Files") + " (*)"); !files.empty()) {
			auto* window = new QMareTextureWindow;
			for (const auto& file : files) {
				window->loadTexture(file);
			}
			window->show();
			this->close();
		}
	});

	this->toolbar->addSeparator();

	this->toolbar->addAction(QIcon{":/button_kofi.png"}, tr("&Donate"), [] {
		QDesktopServices::openUrl({"https://ko-fi.com/craftablescience"});
	});

	this->toolbar->addAction(this->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("&Credits"), Qt::Key_F1, [this] {
		QMareCredits::show(this);
	});

	this->toolbar->addAction(this->style()->standardIcon(QStyle::SP_TitleBarMenuButton), tr("&About Qt"), [this] {
		QMessageBox::aboutQt(this);
	});

	auto* toolbarExpanderEnd = new QWidget{this};
	toolbarExpanderEnd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->toolbar->addWidget(toolbarExpanderEnd);

	this->addToolBar(Qt::ToolBarArea::TopToolBarArea, this->toolbar);

	const auto scaledScreenSize = this->screen()->availableGeometry().size() * 0.5f;
	const auto scaledScreenSizeMinDim = qMax(qMin(scaledScreenSize.width(), scaledScreenSize.height()), 300);
	this->setFixedSize(scaledScreenSizeMinDim, scaledScreenSizeMinDim + this->toolbar->height());

	this->setContextMenuPolicy(Qt::NoContextMenu);
	this->toolbar->setContextMenuPolicy(Qt::NoContextMenu);

	this->setAcceptDrops(true);
}

void QMareEmptyWindow::paintEvent(QPaintEvent*) {
	QPainter painter{this};

	static const QPixmap SPLASH_IMAGE{":/splash.png"};
	const QPixmap splashImageScaled = SPLASH_IMAGE.scaled(this->width(), this->height() - this->toolbar->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	painter.drawPixmap(0, this->toolbar->height(), splashImageScaled.width(), splashImageScaled.height(), splashImageScaled);
}

void QMareEmptyWindow::dragEnterEvent(QDragEnterEvent* event) {
	if (!event->mimeData()->hasUrls()) {
		return;
	}
	for (const auto& url : event->mimeData()->urls()) {
		if (!url.isLocalFile() || (!url.fileName().endsWith(".vtf") && !url.fileName().endsWith(".xtf"))) {
			return;
		}
	}
	event->acceptProposedAction();
}

void QMareEmptyWindow::dropEvent(QDropEvent* event) {
	auto* window = new QMareTextureWindow;
	for (const auto& url : event->mimeData()->urls()) {
		window->loadTexture(url.toLocalFile());
	}
	window->show();
	event->acceptProposedAction();
	this->close();
}
