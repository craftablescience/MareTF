#include "MareTF.h"

#include <cstdlib>
#include <utility>

#include <discord_rpc.h>
#include <QApplication>
#include <QDockWidget>
#include <QFileInfo>
#include <QListWidget>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPainter>
#include <QTabWidget>

#include "../common/Config.h"
#include "../common/EnumMappings.h"

#ifdef _WIN32
#define MSVC_SEH_IGNORE_BEGIN() __try {
#define MSVC_SEH_IGNORE_END() } __except (1 /*EXCEPTION_EXECUTE_HANDLER*/) {}
#else
#define MSVC_SEH_IGNORE_BEGIN() {
#define MSVC_SEH_IGNORE_END() }
#endif

QMareEmptyWindow::QMareEmptyWindow() : QMainWindow(nullptr) {
	this->setWindowTitle(PROJECT_TITLE);
	this->setWindowIcon(QIcon(":/logo.png"));
	this->resize(this->screen()->availableGeometry().size() * 0.6);

	auto* central = new QWidget{this};
	this->setCentralWidget(central);
}

void QMareEmptyWindow::paintEvent(QPaintEvent*) {
	QPainter painter{this};

	static const QPixmap backgroundGradientImage{":/background_gradient.png"};
	painter.drawPixmap(0, 0, this->width(), this->height(), backgroundGradientImage);

	static const QPixmap envSunImage{":/env_sun.png"};
	static constexpr auto envSunScaleFactor = 0.1;
	const auto envSunScale = envSunScaleFactor * this->width() > envSunScaleFactor * this->height() ? envSunScaleFactor * this->width() : envSunScaleFactor * this->height();
	painter.drawPixmap(0.75 * this->width() - envSunScaleFactor / 2, 0.1 * this->height() - envSunScaleFactor / 2, envSunScale, envSunScale, envSunImage);

	static const QPixmap backgroundLandscapeImage{":/background_landscape.png"};
	painter.drawPixmap(0, 0, this->width(), this->height(), backgroundLandscapeImage);

	static const QPixmap oliveShadeSleepImage{":/olive_shade_sleep.png"};
	static constexpr auto oliveShadeScaleFactor = 0.25;
	const auto oliveShadeScale = oliveShadeScaleFactor * this->width() > oliveShadeScaleFactor * this->height() ? oliveShadeScaleFactor * this->width() : oliveShadeScaleFactor * this->height();
	painter.drawPixmap(0.55 * this->width() - oliveShadeScale / 2, 0.8 * this->height() - oliveShadeScale, oliveShadeScale, oliveShadeScale, oliveShadeSleepImage);
}

QMareTextureWidget::QMareTextureWidget(QWidget* parent) : QWidget(parent) {}

void QMareTextureWidget::loadTexture(const QString& path_) {
	if (auto loadVTF = vtfpp::VTF{path_.toUtf8().constData()}) {
		this->path = path_;
		this->vtf = std::move(loadVTF);
		this->textureCurrentData = this->vtf.getImageDataAsRGBA8888();
		this->textureCurrent = QImage{reinterpret_cast<const uchar*>(this->textureCurrentData.data()), this->vtf.getWidth(), this->vtf.getHeight(), QImage::Format_RGBA8888};
	}
}

QIcon QMareTextureWidget::getIcon() const {
	if (this->textureCurrent.isNull()) {
		return {};
	}
	return {QPixmap::fromImage(this->textureCurrent).scaled(64, 64, Qt::KeepAspectRatio)};
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

	this->textureOffset.setX(std::clamp<double>(this->textureOffset.x() + diff.x(), this->width() / -2, this->width() / 2));
	this->textureOffset.setY(std::clamp<double>(this->textureOffset.y() + diff.y(), this->height() / -2, this->height() / 2));

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
		static_cast<int>(this->textureOffset.x() * this->textureZoom + this->width() / 2 - actualWidth * this->textureZoom / 2),
		static_cast<int>(this->textureOffset.y() * this->textureZoom + this->height() / 2 - actualHeight * this->textureZoom / 2),
		static_cast<int>(actualWidth * this->textureZoom),
		static_cast<int>(actualHeight * this->textureZoom),
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
	this->textureZoom += e->angleDelta().y() / 360.f;
	this->textureZoom = qMax(1.f, this->textureZoom);

	this->update();
	e->accept();
}

QMareTextureWindow::QMareTextureWindow() : QMainWindow(nullptr) {
	// Window setup ------------------------------------------

	this->regenerateTitle();
	this->setWindowIcon(QIcon(":/logo.png"));
	this->resize(this->screen()->availableGeometry().size() * 0.7);

	// Menus ------------------------------------------

	auto* fileMenu = this->menuBar()->addMenu(tr("File"));
	auto* editMenu = this->menuBar()->addMenu(tr("Edit"));
	auto* viewMenu = this->menuBar()->addMenu(tr("View"));
	auto* helpMenu = this->menuBar()->addMenu(tr("Help"));

	// Texture tabs ------------------------------------------

	this->textureTabs = new QTabWidget{this};
	this->textureTabs->setDocumentMode(true);
	this->textureTabs->setIconSize({64, 64});
	this->textureTabs->setMovable(true);
	this->textureTabs->setTabsClosable(true);
	this->setCentralWidget(this->textureTabs);

	connect(this->textureTabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
		// todo: save confirmation
		this->textureTabs->removeTab(index);
	});

	// Docks ------------------------------------------

	this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

	auto* infoDock = new QDockWidget{tr("Info"), this};
	// widget here
	infoDock->setWidget(new QWidget);
	this->addDockWidget(Qt::RightDockWidgetArea, infoDock);
	viewMenu->addAction(infoDock->toggleViewAction());

	auto* resDock = new QDockWidget{tr("Resources"), this};
	// widget here
	resDock->setWidget(new QWidget);
	this->addDockWidget(Qt::RightDockWidgetArea, resDock);
	this->tabifyDockWidget(infoDock, resDock);
	infoDock->raise();
	viewMenu->addAction(resDock->toggleViewAction());

	auto* flagsDock = new QDockWidget{tr("Flags"), this};
	flagsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	this->flagsChecks = new QListWidget{flagsDock};
	for (const auto& flagName : not_magic_enum::enum_names<vtfpp::VTF::Flags>()) {
		auto* flagItem = new QListWidgetItem{QString{flagName.data()}.replace('_', ' '), this->flagsChecks};
		flagItem->setCheckState(Qt::Unchecked);
		// todo: this sucks, and XBOX flags are duplicates - also use pre-7.4 names based on version
		// basically just populate this differently per VTF version/platform - repopulate on every tab change
		if (flagItem->text() == "NO MIP" || flagItem->text() == "ENVMAP") {
			flagItem->setFlags(flagItem->flags() & ~Qt::ItemIsEnabled);
		}
		this->flagsChecks->addItem(flagItem);
	}
	flagsDock->setWidget(this->flagsChecks);
	this->addDockWidget(Qt::RightDockWidgetArea, flagsDock);
	viewMenu->addAction(flagsDock->toggleViewAction());
}

void QMareTextureWindow::loadTexture(const QString& path) const {
	auto* widget = new QMareTextureWidget{this->textureTabs};
	widget->loadTexture(path);
	if (*widget) {
		const int index = this->textureTabs->addTab(widget, QFileInfo{path}.fileName());
		this->textureTabs->setTabIcon(index, widget->getIcon());
	} else {
		widget->deleteLater();
	}
}

void QMareTextureWindow::regenerateTitle() {
	this->setWindowTitle(PROJECT_TITLE);
	// todo
	//if (this->path.isEmpty()) {
	//	this->setWindowTitle(PROJECT_TITLE);
	//} else {
	//	this->setWindowTitle(QString{PROJECT_TITLE} + " | " + this->path + "[*]");
	//}
}

int main(int argc, char* argv[]) {
	QApplication app{argc, argv};

	QCoreApplication::setOrganizationName(PROJECT_ORGANIZATION_NAME);
	QCoreApplication::setApplicationName(PROJECT_NAME);
	QCoreApplication::setApplicationVersion(PROJECT_VERSION);

#if !defined(__APPLE__) && !defined(_WIN32)
	QGuiApplication::setDesktopFileName(PROJECT_NAME);
#endif

	// Discord integration
	// todo: make optional
	MSVC_SEH_IGNORE_BEGIN()
		DiscordEventHandlers handlers{};
		Discord_Initialize("1384260711202422845", &handlers, true, nullptr);

		static constexpr DiscordButton buttons[2] {{"View on GitHub", PROJECT_HOMEPAGE_URL}, {}};
		static constexpr DiscordRichPresence discordPresence{
			.state = "v" PROJECT_VERSION,
			.largeImageKey = "icon",
			.largeImageText = PROJECT_TITLE,
			.buttons = buttons,
		};
		Discord_UpdatePresence(&discordPresence);

		std::atexit(&Discord_Shutdown);
	MSVC_SEH_IGNORE_END()

// random bullshit go

	auto* emptyWindow = new QMareEmptyWindow;
	emptyWindow->show();

	auto* window = new QMareTextureWindow;
	window->loadTexture("/home/lxlewis/Downloads/test.vtf");
	window->loadTexture("/home/lxlewis/Downloads/test2.vtf");
	window->show();

// random bullshit stop

	return QApplication::exec();
}
