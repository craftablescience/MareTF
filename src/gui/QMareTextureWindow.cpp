#include "QMareTextureWindow.h"

#include <limits>

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QScreen>
#include <QScrollArea>
#include <QSpinBox>
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

	// File menu ------------------------------------------

	auto* fileMenu = this->menuBar()->addMenu(tr("&File"));

	fileMenu->addAction(QIcon{":/button_new.png"}, tr("&New Texture"), [this] {
		// todo: create new texture from image
	})->setDisabled(true);

	fileMenu->addAction(QIcon{":/button_new_multi.png"}, tr("N&ew Textures"), [this] {
		// todo: create new textures from folder
	})->setDisabled(true);

	fileMenu->addAction(QIcon{":/button_load.png"}, tr("&Load Textures"), [this] {
		for (const auto& file : QFileDialog::getOpenFileNames(this, tr("Load Textures"), {}, QString{"Valve Texture Format (*.vtf *.xtf);;"} + tr("All Files %1").arg("(*)"))) {
			this->loadTexture(file);
		}
	});

	fileMenu->addSeparator();

	fileMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("Donate On &Ko-fi..."), [] {
		QDesktopServices::openUrl({"https://ko-fi.com/craftablescience"});
	});

	fileMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogCancelButton), tr("&Quit"), [this] {
		this->close();
	});

	// Edit menu ------------------------------------------

#ifdef DEBUG // todo: edit textures
	auto* editMenu = this->menuBar()->addMenu(tr("&Edit"));
#endif

	// View menu ------------------------------------------

	auto* viewMenu = this->menuBar()->addMenu(tr("&View"));

	// Help menu ------------------------------------------

	auto* helpMenu = this->menuBar()->addMenu(tr("&Help"));

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("&About"), [this] {
		auto* box = new QMessageBox{this};
		box->setWindowTitle(tr("About"));
		box->setIconPixmap(QPixmap{":/logo.png"}.scaledToWidth(64));
		box->setTextFormat(Qt::MarkdownText);
		box->setText(tr("## %1\nCreated by %2\n\nThis project lives [on GitHub here](%3).").arg(PROJECT_TITLE).arg(PROJECT_ORGANIZATION_NAME).arg(PROJECT_HOMEPAGE_URL));
		box->setStandardButtons(QMessageBox::Ok);
		box->exec();
	});

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_TitleBarMenuButton), tr("About &Qt"), [this] {
		QMessageBox::aboutQt(this);
	});

	helpMenu->addSeparator();

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("Report an &Issue"), [this] {
		QDesktopServices::openUrl({PROJECT_HOMEPAGE_URL "/issues/new"});
	});

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("Request a &Feature"), [this] {
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

	// Preview dock ------------------------------------------

	this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

	auto* previewDock = new QDockWidget(tr("&Preview"), this);
	previewDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	// todo: widget
	previewDock->setWidget(new QWidget);
	this->addDockWidget(Qt::LeftDockWidgetArea, previewDock);
	viewMenu->addAction(previewDock->toggleViewAction());

	// Details dock ------------------------------------------

	auto* detailsDock = new QDockWidget{tr("&Details"), this};
	detailsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	auto* detailsWidget = new QWidget{detailsDock};
	auto* detailsWidgetLayout = new QVBoxLayout{detailsWidget};

	this->detailsFileTypeGroup = new QGroupBox(tr("File Type"), detailsWidget);
	auto* detailsFileTypeLayout = new QFormLayout{this->detailsFileTypeGroup};
	detailsFileTypeLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsPlatform = new QComboBox{this->detailsFileTypeGroup};
	this->detailsPlatform->addItem(tr("PC"));
	this->detailsPlatform->setItemData(0, vtfpp::VTF::PLATFORM_PC);
	this->detailsPlatform->addItem(tr("Xbox 360"));
	this->detailsPlatform->setItemData(1, vtfpp::VTF::PLATFORM_X360);
	this->detailsPlatform->addItem(tr("PS3 (Orange Box)"));
	this->detailsPlatform->setItemData(2, vtfpp::VTF::PLATFORM_PS3_ORANGEBOX);
	this->detailsPlatform->addItem(tr("PS3 (P2, CS:GO)"));
	this->detailsPlatform->setItemData(3, vtfpp::VTF::PLATFORM_PS3_PORTAL2);
	detailsFileTypeLayout->addRow(tr("Platform"), this->detailsPlatform);

	this->detailsVersion = new QComboBox{this->detailsFileTypeGroup};
	for (int i = 0; i <= 6; i++) {
		this->detailsVersion->addItem(QString{"7.%1"}.arg(i));
		this->detailsVersion->setItemData(i, i);
	}
	detailsFileTypeLayout->addRow(tr("Version"), this->detailsVersion);

	// Hide the Version combo when Platform != PLATFORM_PC
	connect(this->detailsPlatform, &QComboBox::currentIndexChanged, this->detailsVersion, [this, detailsFileTypeLayout](int index) {
		detailsFileTypeLayout->setRowVisible(1, this->detailsPlatform->itemData(index).toInt() == vtfpp::VTF::PLATFORM_PC);
	});

	this->detailsFormat = new QComboBox{this->detailsFileTypeGroup};
	for (const auto& [value, name] : not_magic_enum::enum_entries<vtfpp::ImageFormat>()) {
		if (static_cast<int>(value) < 0) {
			continue;
		}
		this->detailsFormat->addItem(name.starts_with("CONSOLE_") ? name.substr(8).data() : name.data());
		this->detailsFormat->setItemData(this->detailsFormat->count() - 1, static_cast<int>(value));
	}
	detailsFileTypeLayout->addRow(tr("Format"), this->detailsFormat);

	detailsWidgetLayout->addWidget(this->detailsFileTypeGroup);

	this->detailsDimsGroup = new QGroupBox(tr("Dimensions"), detailsWidget);
	auto* detailsDimsLayout = new QFormLayout{this->detailsDimsGroup};
	detailsDimsLayout->setFormAlignment(Qt::AlignHCenter);
	detailsDimsLayout->setLabelAlignment(Qt::AlignLeft);

	this->detailsWidth = new QSpinBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Width"), this->detailsWidth);
	this->detailsHeight = new QSpinBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Height"), this->detailsHeight);
	this->detailsDepth = new QSpinBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Depth"), this->detailsDepth);
	this->detailsFrames = new QSpinBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Frames"), this->detailsFrames);
	this->detailsStartFrame = new QSpinBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Start Frame"), this->detailsStartFrame);
	for (auto* spinBox : {this->detailsWidth, this->detailsHeight, this->detailsDepth, this->detailsFrames, this->detailsStartFrame}) {
		spinBox->setMinimum(0);
		spinBox->setMaximum(std::numeric_limits<uint16_t>::max());
	}
	this->detailsCubemap = new QCheckBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Cubemap"), this->detailsCubemap);
	this->detailsMipmaps = new QCheckBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Mipmaps"), this->detailsMipmaps);

	detailsWidgetLayout->addWidget(this->detailsDimsGroup);

	this->detailsMiscellaneousGroup = new QGroupBox(tr("Miscellaneous"), detailsWidget);
	auto* detailsMiscellaneousLayout = new QFormLayout{this->detailsMiscellaneousGroup};
	detailsMiscellaneousLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsBumpmapScale = new QDoubleSpinBox{this->detailsMiscellaneousGroup};
	this->detailsBumpmapScale->setSingleStep(0.1);
	this->detailsBumpmapScale->setDecimals(3);
	detailsMiscellaneousLayout->addRow(tr("Bumpmap Scale"), this->detailsBumpmapScale);

	auto* detailsReflectivityGroup = new QGroupBox(this->detailsMiscellaneousGroup);
	auto* detailsReflectivityLayout = new QFormLayout{detailsReflectivityGroup};
	detailsReflectivityLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsReflectivityR = new QDoubleSpinBox{detailsReflectivityGroup};
	detailsReflectivityLayout->addRow(tr("R"), this->detailsReflectivityR);
	this->detailsReflectivityG = new QDoubleSpinBox{detailsReflectivityGroup};
	detailsReflectivityLayout->addRow(tr("G"), this->detailsReflectivityG);
	this->detailsReflectivityB = new QDoubleSpinBox{detailsReflectivityGroup};
	detailsReflectivityLayout->addRow(tr("B"), this->detailsReflectivityB);
	for (auto* spinBox : {this->detailsReflectivityR, this->detailsReflectivityG, this->detailsReflectivityB}) {
		spinBox->setDecimals(6);
		spinBox->setDisabled(true);
	}

	detailsMiscellaneousLayout->addRow(tr("Reflectivity"), detailsReflectivityGroup);

	detailsWidgetLayout->addWidget(this->detailsMiscellaneousGroup);

	this->detailsCompressionGroup = new QGroupBox(tr("Compression"), detailsWidget);
	auto* detailsCompressionLayout = new QFormLayout{this->detailsCompressionGroup};
	detailsCompressionLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsCompressionMethod = new QComboBox{this->detailsCompressionGroup};
	this->detailsCompressionMethod->addItem("None");
	this->detailsCompressionMethod->setItemData(0, 0);
	for (const auto& [value, name] : not_magic_enum::enum_entries<vtfpp::CompressionMethod>()) {
		this->detailsCompressionMethod->addItem(name.data());
		this->detailsCompressionMethod->setItemData(this->detailsCompressionMethod->count() - 1, static_cast<int>(value));
	}
	detailsCompressionLayout->addRow(tr("Method"), this->detailsCompressionMethod);

	this->detailsCompressionLevel = new QSpinBox{this->detailsCompressionGroup};
	detailsCompressionLayout->addRow(tr("Level"), this->detailsCompressionLevel);

	// Limit strength to 0-0 for None, 1-9 for Deflate/LZMA, 1-22 for Zstd
	connect(this->detailsCompressionMethod, &QComboBox::currentIndexChanged, this->detailsCompressionLevel, [this](int index) {
		switch (this->detailsCompressionMethod->itemData(index).toInt()) {
			case static_cast<int>(vtfpp::CompressionMethod::DEFLATE):
			case static_cast<int>(vtfpp::CompressionMethod::CONSOLE_LZMA):
				this->detailsCompressionLevel->setRange(1, 9);
				break;
			case static_cast<int>(vtfpp::CompressionMethod::ZSTD):
				this->detailsCompressionLevel->setRange(1, 22);
				break;
			default:
			case 0:
				this->detailsCompressionLevel->setRange(0, 0);
				break;
		}
	});

	detailsWidgetLayout->addWidget(this->detailsCompressionGroup);

	// Compression group is unused for PC 7.0-7.5
	connect(this->detailsPlatform, &QComboBox::currentIndexChanged, this->detailsCompressionGroup, [this](int index) {
		if (this->detailsPlatform->itemData(index).toInt() == vtfpp::VTF::PLATFORM_PC) {
			this->detailsCompressionGroup->setEnabled(this->detailsVersion->currentIndex() == 6);
		} else {
			this->detailsCompressionGroup->setVisible(true);
		}
	});
	connect(this->detailsVersion, &QComboBox::currentIndexChanged, this->detailsCompressionGroup, [this](int index) {
		this->detailsCompressionGroup->setEnabled(index == 6);
	});

	detailsWidgetLayout->addStretch(1);

	auto* detailsScroll = new QScrollArea{detailsDock};
	detailsScroll->setMinimumWidth(detailsWidget->sizeHint().width());
	detailsScroll->setWidgetResizable(true);
	detailsScroll->setWidget(detailsWidget);

	detailsDock->setWidget(detailsScroll);
	this->addDockWidget(Qt::LeftDockWidgetArea, detailsDock);
	viewMenu->addAction(detailsDock->toggleViewAction());

	// Resources dock ------------------------------------------

	auto* resDock = new QDockWidget{tr("&Resources"), this};
	resDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	// todo: widget here
	resDock->setWidget(new QWidget);
	this->addDockWidget(Qt::RightDockWidgetArea, resDock);
	viewMenu->addAction(resDock->toggleViewAction());

	// Flags dock ------------------------------------------

	auto* flagsDock = new QDockWidget{tr("&Flags"), this};
	flagsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	this->flagsChecks = new QListWidget{flagsDock};
	flagsDock->setWidget(this->flagsChecks);
	this->addDockWidget(Qt::RightDockWidgetArea, flagsDock);
	this->tabifyDockWidget(flagsDock, resDock);
	flagsDock->raise();
	viewMenu->addAction(flagsDock->toggleViewAction());

	// Final setup ------------------------------------------

	this->regenerateDetails();
}

void QMareTextureWindow::loadTexture(const QString& path) {
	for (int i = 0; i < this->textureTabs->count(); i++) {
		if (const auto* textureWidget = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(i)); textureWidget && QDir{textureWidget->getPath()}.absolutePath() == QDir{path}.absolutePath()) {
			this->textureTabs->setCurrentIndex(i);
			return;
		}
	}

	auto* widget = new QMareTextureWidget{this->textureTabs};
	widget->loadTexture(path);
	if (*widget) {
		const int index = this->textureTabs->addTab(widget, QFileInfo{path}.fileName());
		this->textureTabs->setTabIcon(index, widget->getIcon());
		this->textureTabs->setCurrentIndex(index);
	} else {
		widget->deleteLater();
		QMessageBox::critical(this, tr("Error"), tr("Failed to load texture at location: %1").arg("\n\n" + path));
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

		this->detailsFileTypeGroup->setDisabled(true);
		this->detailsPlatform->setCurrentIndex(0);
		this->detailsVersion->setCurrentIndex(0);
		this->detailsFormat->setCurrentIndex(0);

		this->detailsDimsGroup->setDisabled(true);
		this->detailsWidth->setValue(0);
		this->detailsHeight->setValue(0);
		this->detailsDepth->setValue(0);
		this->detailsFrames->setValue(0);
		this->detailsStartFrame->setValue(0);
		this->detailsCubemap->setChecked(false);
		this->detailsMipmaps->setChecked(false);

		this->detailsMiscellaneousGroup->setDisabled(true);
		this->detailsBumpmapScale->setValue(0.0);
		this->detailsReflectivityR->setValue(0.0);
		this->detailsReflectivityG->setValue(0.0);
		this->detailsReflectivityB->setValue(0.0);

		// Disable/enable is automatic
		//this->detailsCompressionGroup->setDisabled(true);
		this->detailsCompressionMethod->setCurrentIndex(0);
		this->detailsCompressionLevel->setValue(0);

		this->flagsChecks->clear();

		return;
	}

	const vtfpp::VTF& vtf = activeTexture->getVTF();
	this->setWindowTitle(QString{PROJECT_TITLE} + " | " + this->textureTabs->tabText(activeIndex) + "[*]");

	static constexpr auto searchAndSetCombo = [](QComboBox* combo, int value, bool condition = true) {
		combo->setCurrentIndex(0);
		if (condition) {
			for (int i = 0; i < combo->count(); i++) {
				if (combo->itemData(i).toInt() == value) {
					combo->setCurrentIndex(i);
					break;
				}
			}
		}
	};

	this->detailsFileTypeGroup->setEnabled(true);
	searchAndSetCombo(this->detailsPlatform, vtf.getPlatform());
	this->detailsVersion->setCurrentIndex(vtf.getVersion());
	searchAndSetCombo(this->detailsFormat, static_cast<int>(vtf.getFormat()));

	this->detailsDimsGroup->setEnabled(true);
	this->detailsWidth->setValue(vtf.getWidth());
	this->detailsHeight->setValue(vtf.getHeight());
	this->detailsDepth->setValue(vtf.getSliceCount());
	this->detailsFrames->setValue(vtf.getFrameCount());
	this->detailsStartFrame->setValue(vtf.getStartFrame());
	this->detailsCubemap->setChecked(vtf.getFaceCount() > 1);
	this->detailsMipmaps->setChecked(vtf.getMipCount() > 1);

	this->detailsMiscellaneousGroup->setEnabled(true);
	this->detailsBumpmapScale->setValue(vtf.getBumpMapScale());
	this->detailsReflectivityR->setValue(vtf.getReflectivity()[0]);
	this->detailsReflectivityG->setValue(vtf.getReflectivity()[1]);
	this->detailsReflectivityB->setValue(vtf.getReflectivity()[2]);

	// Disable/enable is automatic
	//this->detailsCompressionGroup->setEnabled(true);
	searchAndSetCombo(this->detailsCompressionMethod, static_cast<int>(vtf.getCompressionMethod()), vtf.getCompressionLevel() > 0 || vtf.getCompressionMethod() == vtfpp::CompressionMethod::CONSOLE_LZMA);
	this->detailsCompressionLevel->setValue(vtf.getCompressionLevel());

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
