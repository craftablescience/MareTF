#include "QMareTextureWindow.h"

#include <QDesktopServices>
#include <QDockWidget>
#include <QFileInfo>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QScreen>
#include <QTabWidget>
#include <qurl.h>

#include "../common/Common.h"
#include "../common/Config.h"
#include "../common/EnumMappings.h"
#include "QMareTextureWidget.h"

QMareTextureWindow::QMareTextureWindow() : QMainWindow(nullptr) {
	// Window setup ------------------------------------------

	this->setWindowIcon(QIcon(":/logo.png"));
	this->resize(this->screen()->availableGeometry().size() * 0.7);

	// Menus ------------------------------------------

	auto* fileMenu = this->menuBar()->addMenu(tr("&File"));
	auto* editMenu = this->menuBar()->addMenu(tr("&Edit"));
	auto* viewMenu = this->menuBar()->addMenu(tr("&View"));

	// Help menu ------------------------------------------

	auto* helpMenu = this->menuBar()->addMenu(tr("&Help"));

	helpMenu->addAction(tr("&About"), [this] {
		auto* box = new QMessageBox{this};
		box->setWindowTitle(tr("About"));
		box->setIconPixmap(QPixmap{":/logo.png"}.scaledToWidth(64));
		box->setTextFormat(Qt::MarkdownText);
		box->setText(tr("## %1\nCreated by %2\n\nThis project lives [on GitHub here](%3).").arg(PROJECT_TITLE).arg(PROJECT_ORGANIZATION_NAME).arg(PROJECT_HOMEPAGE_URL));
		box->setStandardButtons(QMessageBox::Ok);
		box->exec();
	});

	helpMenu->addAction(tr("About &Qt"), [this] {
		QMessageBox::aboutQt(this);
	});

	helpMenu->addSeparator();

	helpMenu->addAction(tr("&Make an Issue"), [this] {
		QDesktopServices::openUrl({PROJECT_HOMEPAGE_URL "/issues/new"});
	});

	// Texture tabs ------------------------------------------

	this->textureTabs = new QTabWidget{this};
	this->textureTabs->setDocumentMode(true);
	this->textureTabs->setIconSize({64, 64});
	this->textureTabs->setMovable(true);
	this->textureTabs->setTabsClosable(true);
	this->setCentralWidget(this->textureTabs);

	connect(this->textureTabs, &QTabWidget::currentChanged, this, &QMareTextureWindow::regenerateDetails);
	connect(this->textureTabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
		// todo: save confirmation
		this->textureTabs->removeTab(index);
	});

	// Docks ------------------------------------------

	this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

	auto* infoDock = new QDockWidget{tr("&Info"), this};
	infoDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	// todo: widget here
	infoDock->setWidget(new QWidget);
	this->addDockWidget(Qt::LeftDockWidgetArea, infoDock);
	viewMenu->addAction(infoDock->toggleViewAction());

	auto* resDock = new QDockWidget{tr("&Resources"), this};
	resDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	// todo: widget here
	resDock->setWidget(new QWidget);
	this->addDockWidget(Qt::LeftDockWidgetArea, resDock);
	viewMenu->addAction(resDock->toggleViewAction());

	auto* flagsDock = new QDockWidget{tr("&Flags"), this};
	flagsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	this->flagsChecks = new QListWidget{flagsDock};
	flagsDock->setWidget(this->flagsChecks);
	this->addDockWidget(Qt::RightDockWidgetArea, flagsDock);
	viewMenu->addAction(flagsDock->toggleViewAction());

	// Final setup ------------------------------------------

	this->regenerateDetails();
}

void QMareTextureWindow::loadTexture(const QString& path) {
	auto* widget = new QMareTextureWidget{this->textureTabs};
	widget->loadTexture(path);
	if (*widget) {
		const int index = this->textureTabs->addTab(widget, QFileInfo{path}.fileName());
		this->textureTabs->setTabIcon(index, widget->getIcon());
	} else {
		widget->deleteLater();
		QMessageBox::critical(this, tr("Error"), tr("Failed to load texture at location: %1%2").arg("\n\n").arg(path));
	}
}

void QMareTextureWindow::regenerateDetails() {
	const auto activeIndex = this->textureTabs->currentIndex();
	QMareTextureWidget* activeTexture = nullptr;
	if (activeIndex >= 0) {
		activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(activeIndex));
	}
	if (!activeTexture) {
		this->setWindowTitle(PROJECT_TITLE);
		return;
	}
	const vtfpp::VTF& vtf = activeTexture->getVTF();

	this->setWindowTitle(QString{PROJECT_TITLE} + " | " + this->textureTabs->tabText(activeIndex) + "[*]");

	this->flagsChecks->clear();
	const auto prettyFlagNames = ::getPrettyFlagNamesFor(vtf.getVersion(), vtf.getPlatform());
	for (int i = 0; i < 32; i++) {
		auto* flagItem = new QListWidgetItem{prettyFlagNames[i].data(), this->flagsChecks};
		flagItem->setCheckState(activeTexture->getVTF().getFlags() & (vtf.getFlags() & 1 << i) ? Qt::Checked : Qt::Unchecked);
		// basically just populate this differently per VTF version/platform - repopulate on every tab change
		if (1 << i & vtfpp::VTF::FLAGS_MASK_INTERNAL) {
			flagItem->setFlags(flagItem->flags() & ~Qt::ItemIsEnabled);
		}
		if (flagItem->text().startsWith("Unused")) {
			flagItem->setForeground(Qt::gray);
		}
		this->flagsChecks->addItem(flagItem);
	}
}
