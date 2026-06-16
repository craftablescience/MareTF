#include "QMareEmptyWindow.h"

#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QScreen>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>

#include "Common.h"
#include "Config.h"

#include "dialogs/QMareCreateTextureDialog.h"
#include "dialogs/QMareCreditsDialog.h"
#include "dialogs/QMareExtractFromTextureDialog.h"
#include "utility/QMareOptions.h"
#include "QMareTextureWindow.h"

QMareEmptyWindow::QMareEmptyWindow() : QMainWindow{nullptr} {
	this->setWindowTitle(PROJECT_TITLE);
	this->setWindowIcon(QIcon{":/logo.png"});

	this->toolbar = new QToolBar{this};
	this->toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	this->toolbar->setMovable(false);
	this->toolbar->setAutoFillBackground(true);

	auto* toolbarExpanderBegin = new QWidget{this};
	toolbarExpanderBegin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->toolbar->addWidget(toolbarExpanderBegin);

	auto* toolbarCreateAction = new QAction{QIcon{":/button_new.png"}, tr("&Create"), this};
	toolbarCreateAction->setShortcut(Qt::CTRL | Qt::Key_N);
	connect(toolbarCreateAction, &QAction::triggered, this, [this] {
		if (auto* createTextureDialog = QMareCreateTextureDialog::fromImages(this)) {
			connect(createTextureDialog, &QMareCreateTextureDialog::createdTextures, this, [this](const QStringList& paths) {
				if (!g_ManeWindow) {
					g_ManeWindow = new QMareTextureWindow;
				}
				for (const auto& path : paths) {
					g_ManeWindow->loadTexture(path);
				}
				g_ManeWindow->show();
				this->close();
			});
			createTextureDialog->setAttribute(Qt::WA_DeleteOnClose);
			createTextureDialog->open();
		}
	});

	auto* toolbarCreateFromFolderAction = new QAction{QIcon{":/button_new_multi.png"}, tr("C&reate from Folder"), this};
	toolbarCreateFromFolderAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);
	connect(toolbarCreateFromFolderAction, &QAction::triggered, this, [this] {
		if (auto* createTextureDialog = QMareCreateTextureDialog::fromDir(this)) {
			createTextureDialog->setAttribute(Qt::WA_DeleteOnClose);
			createTextureDialog->open();
		}
	});

	auto* toolbarCreateButton = new QToolButton{this};
	toolbarCreateButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	toolbarCreateButton->setDefaultAction(toolbarCreateAction);

	auto* toolbarCreateMenu = new QMenu{toolbarCreateButton};
	toolbarCreateMenu->addActions({toolbarCreateAction, toolbarCreateFromFolderAction});
	toolbarCreateButton->setMenu(toolbarCreateMenu);
	toolbarCreateButton->setPopupMode(QToolButton::MenuButtonPopup);

	this->toolbar->addWidget(toolbarCreateButton);

	this->toolbar->addAction(QIcon{":/button_load.png"}, tr("&Open"), Qt::CTRL | Qt::Key_O, [this] {
		if (const auto files = QFileDialog::getOpenFileNames(this, tr("Open Textures"), {}, QString{"Valve Texture Format (*.vtf *.xtf);;"} + tr("All Files") + " (*)"); !files.empty()) {
			if (!g_ManeWindow) {
				g_ManeWindow = new QMareTextureWindow;
			}
			for (const auto& file : files) {
				g_ManeWindow->loadTexture(file);
			}
			g_ManeWindow->show();
			this->close();
		}
	});

	auto* toolbarExtractAction = new QAction{QIcon{QMareOptions::get<bool>(QMareOptions::BOOL_ENABLE_TRYPANOPHOBIA_MODE) ? ":/button_extract_alt.png" : ":/button_extract.png"}, tr("&Extract"), this};
	toolbarExtractAction->setShortcut(Qt::CTRL | Qt::Key_E);
	connect(toolbarExtractAction, &QAction::triggered, this, [this] {
		if (auto* extractFromTextureDialog = QMareExtractFromTextureDialog::fromTextures(this)) {
			extractFromTextureDialog->setAttribute(Qt::WA_DeleteOnClose);
			extractFromTextureDialog->open();
		}
	});

	auto* toolbarExtractFromFolderAction = new QAction{QIcon{QMareOptions::get<bool>(QMareOptions::BOOL_ENABLE_TRYPANOPHOBIA_MODE) ? ":/button_extract_multi_alt.png" : ":/button_extract_multi.png"}, tr("E&xtract from Folder"), this};
	toolbarExtractFromFolderAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_E);
	connect(toolbarExtractFromFolderAction, &QAction::triggered, this, [this] {
		if (auto* extractFromTextureDialog = QMareExtractFromTextureDialog::fromDir(this)) {
			extractFromTextureDialog->setAttribute(Qt::WA_DeleteOnClose);
			extractFromTextureDialog->open();
		}
	});

	auto* toolbarExtractButton = new QToolButton{this};
	toolbarExtractButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	toolbarExtractButton->setDefaultAction(toolbarExtractAction);

	auto* toolbarExtractMenu = new QMenu{toolbarExtractButton};
	toolbarExtractMenu->addActions({toolbarExtractAction, toolbarExtractFromFolderAction});
	toolbarExtractButton->setMenu(toolbarExtractMenu);
	toolbarExtractButton->setPopupMode(QToolButton::MenuButtonPopup);

	this->toolbar->addWidget(toolbarExtractButton);

	this->toolbar->addSeparator();

	this->toolbar->addAction(QIcon{":/button_kofi.png"}, tr("&Donate"), [] {
		QDesktopServices::openUrl({"https://ko-fi.com/craftablescience"});
	});

	this->toolbar->addAction(this->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("Credi&ts"), Qt::Key_F1, [this] {
		QMareCreditsDialog::showCredits(this);
	});

	auto* toolbarExpanderEnd = new QWidget{this};
	toolbarExpanderEnd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->toolbar->addWidget(toolbarExpanderEnd);

	this->addToolBar(Qt::ToolBarArea::TopToolBarArea, this->toolbar);

#if !defined(Q_OS_WASM) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	const auto scaledScreenSize = this->screen()->availableGeometry().size() * 0.5f;
	const auto scaledScreenSizeMinDim = qMax(qMin(scaledScreenSize.width(), scaledScreenSize.height()), 300);
	this->setFixedSize(scaledScreenSizeMinDim, scaledScreenSizeMinDim + this->toolbar->height());
#endif

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
		if (!(
			url.isLocalFile() &&
			(url.fileName().endsWith(".vtf") || url.fileName().endsWith(".xtf")) ||
			::fileIsASupportedImageFileFormat(std::filesystem::path{url.fileName().toUtf8().constData()}.extension().string())
		)) {
			return;
		}
	}
	event->acceptProposedAction();
}

void QMareEmptyWindow::dropEvent(QDropEvent* event) {
	g_ManeWindow = new QMareTextureWindow;
	g_ManeWindow->show();
	g_ManeWindow->dropEvent(event);
	this->close();
}
