#include "QMareTextureWindow.h"

#include <format>
#include <limits>

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QScreen>
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>

#include "../common/Common.h"
#include "../common/Config.h"
#include "../common/EnumMappings.h"
#include "QMareCredits.h"
#include "QMareTextureWidget.h"

QMareTextureWindow::QMareTextureWindow() : QMainWindow(nullptr) {
	// Window setup ------------------------------------------

	this->setWindowIcon(QIcon(":/logo.png"));
	this->resize(this->screen()->availableGeometry().size() * 0.7);

	// File menu ------------------------------------------

	auto* fileMenu = this->menuBar()->addMenu(tr("&File"));

	fileMenu->addAction(QIcon{":/button_new.png"}, tr("&Create"), Qt::CTRL | Qt::Key_N, [] {
		// todo: create new texture from image
	})->setDisabled(true);

	fileMenu->addAction(QIcon{":/button_new_multi.png"}, tr("Create en &Masse"), Qt::CTRL | Qt::SHIFT | Qt::Key_N, [] {
		// todo: create new textures from folder
	})->setDisabled(true);

	fileMenu->addSeparator();

	fileMenu->addAction(QIcon{":/button_load.png"}, tr("&Load"), Qt::CTRL | Qt::Key_O, [this] {
		for (const auto& file : QFileDialog::getOpenFileNames(this, tr("Load Textures"), {}, QString{"Valve Texture Format (*.vtf *.xtf);;"} + tr("All Files") + " (*)")) {
			this->loadTexture(file);
		}
	});

	fileMenu->addSeparator();

	fileMenu->addAction(QIcon{":/button_kofi.png"}, tr("&Donate"), [] {
		QDesktopServices::openUrl({"https://ko-fi.com/craftablescience"});
	});

	fileMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogCancelButton), tr("&Exit"), Qt::ALT | Qt::Key_F4, [this] {
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

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("&Credits"), Qt::Key_F1, [this] {
		QMareCredits::show(this);
	});

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_TitleBarMenuButton), tr("&About Qt"), [this] {
		QMessageBox::aboutQt(this);
	});

	helpMenu->addSeparator();

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("Report an &Issue"), [] {
		QDesktopServices::openUrl({PROJECT_HOMEPAGE_URL "/issues/new"});
	});

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("Request a &Feature"), [] {
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

	auto* previewWidget = new QWidget{previewDock};
	auto* previewWidgetLayout = new QVBoxLayout{previewWidget};

	this->previewGeneralGroup = new QGroupBox{tr("General"), previewWidget};
	auto* previewGeneralLayout = new QFormLayout{this->previewGeneralGroup};
	previewGeneralLayout->setFormAlignment(Qt::AlignHCenter);

	this->previewCurrentMip = new QSpinBox{previewWidget};
	previewGeneralLayout->addRow(tr("Current Mip"), this->previewCurrentMip);

	// Change the current mip
	connect(this->previewCurrentMip, &QSpinBox::valueChanged, this, [this](int value) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setCurrentMip(value);
		}
	});

	this->previewAlpha = new QCheckBox{previewWidget};
	this->previewAlpha->setChecked(true);
	previewGeneralLayout->addRow(tr("Alpha"), this->previewAlpha);

	// Change alpha
	QMareTextureWidget::useAlpha() = true;
	connect(this->previewAlpha, &QCheckBox::toggled, this, [this](bool checked) {
		QMareTextureWidget::useAlpha() = checked;
		for (int i = 0; i < this->textureTabs->count(); i++) {
			if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(i))) {
				activeTexture->reloadCurrentTexture();
			}
		}
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->update();
		}
	});

	this->previewBackground = new QCheckBox{previewWidget};
	this->previewBackground->setChecked(true);
	previewGeneralLayout->addRow(tr("Background"), this->previewBackground);

	// Change background
	QMareTextureWidget::useBackground() = true;
	connect(this->previewBackground, &QCheckBox::toggled, this, [this](bool checked) {
		QMareTextureWidget::useBackground() = checked;
		if (auto* activeTextureWidget = this->textureTabs->widget(this->textureTabs->currentIndex())) {
			activeTextureWidget->update();
		}
	});

	previewWidgetLayout->addWidget(this->previewGeneralGroup);

	this->previewAnimationGroup = new QGroupBox{tr("Animation"), previewWidget};
	auto* previewAnimationLayout = new QFormLayout{this->previewAnimationGroup};
	previewAnimationLayout->setFormAlignment(Qt::AlignHCenter);

	this->previewCurrentFrame = new QSpinBox{previewWidget};
	previewAnimationLayout->addRow(tr("Current Frame"), this->previewCurrentFrame);

	// Change the current frame
	connect(this->previewCurrentFrame, &QSpinBox::valueChanged, this, [this](int value) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setCurrentFrame(value);
		}
	});

	this->previewAnimationSpeed = new QDoubleSpinBox{previewWidget};
	this->previewAnimationSpeed->setRange(0.0, 120.0);
	this->previewAnimationSpeed->setValue(24.0);
	previewAnimationLayout->addRow(tr("Animation FPS"), this->previewAnimationSpeed);

	this->previewAnimate = new QCheckBox{previewWidget};
	previewAnimationLayout->addRow(tr("Animate"), this->previewAnimate);

	this->previewAnimateTimer = new QTimer{previewWidget};
	this->previewAnimateTimer->setTimerType(Qt::PreciseTimer);
	this->previewAnimateTimer->start(static_cast<int>(1000.0 / this->previewAnimationSpeed->value()));

	// Set the timer duration
	connect(this->previewAnimationSpeed, &QDoubleSpinBox::valueChanged, this, [this](double value) {
		this->previewAnimateTimer->stop();
		this->previewAnimateTimer->start(static_cast<int>(1000.0 / value));
	});

	// Increment the current frame if animate is enabled
	connect(this->previewAnimateTimer, &QTimer::timeout, this, [this] {
		if (!this->previewAnimate->isChecked()) {
			return;
		}
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			if (this->previewCurrentFrame->value() + 1 == activeTexture->getVTF().getFrameCount()) {
				this->previewCurrentFrame->setValue(0);
			} else {
				this->previewCurrentFrame->setValue(this->previewCurrentFrame->value() + 1);
			}
		}
	});

	previewWidgetLayout->addWidget(this->previewAnimationGroup);

	this->previewCubemapGroup = new QGroupBox{tr("Cubemap"), previewWidget};
	auto* previewCubemapLayout = new QFormLayout{this->previewCubemapGroup};
	previewCubemapLayout->setFormAlignment(Qt::AlignHCenter);

	this->previewCurrentFace = new QSpinBox{previewWidget};
	previewCubemapLayout->addRow(tr("Current Face"), this->previewCurrentFace);

	// Change the current face
	connect(this->previewCurrentFace, &QSpinBox::valueChanged, this, [this](int value) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setCurrentFace(value);
		}
	});

	previewWidgetLayout->addWidget(this->previewCubemapGroup);

	this->previewDepthGroup = new QGroupBox{tr("Depth"), previewWidget};
	auto* previewDepthLayout = new QFormLayout{this->previewDepthGroup};
	previewDepthLayout->setFormAlignment(Qt::AlignHCenter);

	this->previewCurrentDepth = new QSpinBox{previewWidget};
	previewDepthLayout->addRow(tr("Current Depth"), this->previewCurrentDepth);

	// Change the current depth
	connect(this->previewCurrentDepth, &QSpinBox::valueChanged, this, [this](int value) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setCurrentDepth(value);
		}
	});

	previewWidgetLayout->addWidget(this->previewDepthGroup);

	previewWidgetLayout->addStretch(1);

	previewDock->setWidget(previewWidget);
	this->addDockWidget(Qt::LeftDockWidgetArea, previewDock);
	viewMenu->addAction(previewDock->toggleViewAction());

	// Details dock ------------------------------------------

	auto* detailsDock = new QDockWidget{tr("&Details"), this};
	detailsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	auto* detailsScroll = new QScrollArea{detailsDock};

	auto* detailsWidget = new QWidget{detailsScroll};
	auto* detailsWidgetLayout = new QVBoxLayout{detailsWidget};

	this->detailsFileTypeGroup = new QGroupBox{tr("File Type"), detailsWidget};
	auto* detailsFileTypeLayout = new QFormLayout{this->detailsFileTypeGroup};
	detailsFileTypeLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsPlatform = new QComboBox{this->detailsFileTypeGroup};
	this->detailsPlatform->addItem(tr("PC"),               vtfpp::VTF::PLATFORM_PC);
	this->detailsPlatform->addItem(tr("Xbox"),             vtfpp::VTF::PLATFORM_XBOX);
	this->detailsPlatform->addItem(tr("Xbox 360"),         vtfpp::VTF::PLATFORM_X360);
	this->detailsPlatform->addItem(tr("PS3 (Orange Box)"), vtfpp::VTF::PLATFORM_PS3_ORANGEBOX);
	this->detailsPlatform->addItem(tr("PS3 (P2, CS:GO)"),  vtfpp::VTF::PLATFORM_PS3_PORTAL2);
	detailsFileTypeLayout->addRow(tr("Platform"), this->detailsPlatform);

	this->detailsVersion = new QComboBox{this->detailsFileTypeGroup};
	for (int i = 0; i <= 6; i++) {
		this->detailsVersion->addItem(QString{"7.%1"}.arg(i), i);
	}
	detailsFileTypeLayout->addRow(tr("Version"), this->detailsVersion);

	// Hide the Version combo when Platform != PLATFORM_PC
	connect(this->detailsPlatform, &QComboBox::currentIndexChanged, this, [this, detailsFileTypeLayout](int index) {
		detailsFileTypeLayout->setRowVisible(1, this->detailsPlatform->itemData(index).toInt() == vtfpp::VTF::PLATFORM_PC);
	});

	this->detailsFormat = new QComboBox{this->detailsFileTypeGroup};
	for (const auto& [value, name] : not_magic_enum::enum_entries<vtfpp::ImageFormat>()) {
		if (static_cast<int>(value) < 0) {
			continue;
		}
		this->detailsFormat->addItem(name.starts_with("CONSOLE_") ? name.substr(8).data() : name.data(), static_cast<int>(value));
	}
	detailsFileTypeLayout->addRow(tr("Format"), this->detailsFormat);

	detailsWidgetLayout->addWidget(this->detailsFileTypeGroup);

	this->detailsDimsGroup = new QGroupBox{tr("Dimensions"), detailsWidget};
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
	this->detailsConsoleMipScale = new QSpinBox{this->detailsDimsGroup};
	this->detailsConsoleMipScale->setRange(0, 31);
	detailsDimsLayout->addRow(tr("Mip Scale"), this->detailsConsoleMipScale);

	detailsWidgetLayout->addWidget(this->detailsDimsGroup);

	this->detailsMiscellaneousGroup = new QGroupBox{tr("Miscellaneous"), detailsWidget};
	auto* detailsMiscellaneousLayout = new QFormLayout{this->detailsMiscellaneousGroup};
	detailsMiscellaneousLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsBumpmapScale = new QDoubleSpinBox{this->detailsMiscellaneousGroup};
	this->detailsBumpmapScale->setRange(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
	this->detailsBumpmapScale->setSingleStep(0.1);
	this->detailsBumpmapScale->setDecimals(3);
	detailsMiscellaneousLayout->addRow(tr("Bumpmap Scale"), this->detailsBumpmapScale);

	this->detailsReflectivityGroup = new QGroupBox{this->detailsMiscellaneousGroup};
	auto* detailsReflectivityLayout = new QFormLayout{this->detailsReflectivityGroup};
	detailsReflectivityLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsReflectivityR = new QLabel{this->detailsReflectivityGroup};
	detailsReflectivityLayout->addRow(tr("R:"), this->detailsReflectivityR);
	this->detailsReflectivityG = new QLabel{this->detailsReflectivityGroup};
	detailsReflectivityLayout->addRow(tr("G:"), this->detailsReflectivityG);
	this->detailsReflectivityB = new QLabel{this->detailsReflectivityGroup};
	detailsReflectivityLayout->addRow(tr("B:"), this->detailsReflectivityB);

	detailsMiscellaneousLayout->addRow(tr("Reflectivity"), this->detailsReflectivityGroup);

	detailsWidgetLayout->addWidget(this->detailsMiscellaneousGroup);

	this->detailsCompressionGroup = new QGroupBox{tr("Compression"), detailsWidget};
	auto* detailsCompressionLayout = new QFormLayout{this->detailsCompressionGroup};
	detailsCompressionLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsCompressionMethod = new QComboBox{this->detailsCompressionGroup};
	this->detailsCompressionMethod->addItem("None", 0);
	for (const auto& [value, name] : not_magic_enum::enum_entries<vtfpp::CompressionMethod>(true)) {
		this->detailsCompressionMethod->addItem(name.data(), static_cast<int>(value));
	}
	detailsCompressionLayout->addRow(tr("Method"), this->detailsCompressionMethod);

	this->detailsCompressionLevel = new QSpinBox{this->detailsCompressionGroup};
	detailsCompressionLayout->addRow(tr("Level"), this->detailsCompressionLevel);

	// Limit strength to 0-0 for None, 1-9 for Deflate/LZMA, 1-22 for Zstd
	connect(this->detailsCompressionMethod, &QComboBox::currentIndexChanged, this, [this](int index) {
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
	connect(this->detailsPlatform, &QComboBox::currentIndexChanged, this, [this](int index) {
		this->detailsCompressionGroup->setVisible(this->detailsPlatform->itemData(index).toInt() != vtfpp::VTF::PLATFORM_PC || this->detailsVersion->currentIndex() == 6);
	});
	connect(this->detailsVersion, &QComboBox::currentIndexChanged, this, [this](int index) {
		this->detailsCompressionGroup->setVisible(index == 6);
	});

	detailsWidgetLayout->addStretch(1);

	detailsScroll->setMinimumWidth(detailsWidget->sizeHint().width() + this->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
	detailsScroll->setWidgetResizable(true);
	detailsScroll->setWidget(detailsWidget);

	detailsDock->setWidget(detailsScroll);
	this->addDockWidget(Qt::LeftDockWidgetArea, detailsDock);
	viewMenu->addAction(detailsDock->toggleViewAction());

	// Resources dock ------------------------------------------

	auto* resDock = new QDockWidget{tr("&Resources"), this};
	resDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	auto* resScroll = new QScrollArea{resDock};

	auto* resWidget = new QWidget{resScroll};
	auto* resWidgetLayout = new QVBoxLayout{resWidget};

	this->resThumbnailGroup = new QGroupBox{tr("Thumbnail"), resWidget};
	auto* resThumbnailLayout = new QFormLayout{this->resThumbnailGroup};
	resThumbnailLayout->setFormAlignment(Qt::AlignHCenter);

	this->resThumbnailPreview = new QLabel{resWidget};
	resThumbnailLayout->addRow(tr("Preview"), this->resThumbnailPreview);

	this->resThumbnailWidth = new QSpinBox{resWidget};
	this->resThumbnailWidth->setDisabled(true);
	resThumbnailLayout->addRow(tr("Width"), this->resThumbnailWidth);

	this->resThumbnailHeight = new QSpinBox{resWidget};
	this->resThumbnailHeight->setDisabled(true);
	resThumbnailLayout->addRow(tr("Height"), this->resThumbnailHeight);

	resWidgetLayout->addWidget(this->resThumbnailGroup);

	this->resPaletteGroup = new QGroupBox{tr("Palette"), resWidget};
	auto* resPaletteLayout = new QFormLayout{this->resPaletteGroup};
	resPaletteLayout->setFormAlignment(Qt::AlignHCenter);

	this->resPalettePreview = new QLabel{resWidget};
	resPaletteLayout->addRow(tr("Preview"), this->resPalettePreview);

	this->resPaletteFrame = new QSpinBox{resWidget};
	this->resPaletteFrame->setDisabled(true);
	resPaletteLayout->addRow(tr("Frame"), this->resPaletteFrame);

	resWidgetLayout->addWidget(this->resPaletteGroup);

	this->resFallbackGroup = new QGroupBox{tr("Fallback"), resWidget};
	auto* resFallbackLayout = new QFormLayout{this->resFallbackGroup};
	resFallbackLayout->setFormAlignment(Qt::AlignHCenter);

	this->resFallbackWidth = new QSpinBox{resWidget};
	this->resFallbackWidth->setDisabled(true);
	resFallbackLayout->addRow(tr("Width"), this->resFallbackWidth);

	this->resFallbackHeight = new QSpinBox{resWidget};
	this->resFallbackHeight->setDisabled(true);
	resFallbackLayout->addRow(tr("Height"), this->resFallbackHeight);

	this->resFallbackMips = new QSpinBox{resWidget};
	this->resFallbackMips->setDisabled(true);
	resFallbackLayout->addRow(tr("Mips"), this->resFallbackMips);

	resWidgetLayout->addWidget(this->resFallbackGroup);

	this->resCRCGroup = new QGroupBox{tr("CRC"), resWidget};
	auto* resCRCLayout = new QFormLayout{this->resCRCGroup};
	resCRCLayout->setFormAlignment(Qt::AlignHCenter);

	this->resCRCValue = new QLineEdit{resWidget};
	this->resCRCValue->setInputMask("HHHHHHHH");
	resCRCLayout->addRow(tr("Value"), this->resCRCValue);

	resWidgetLayout->addWidget(this->resCRCGroup);

	this->resTS0Group = new QGroupBox{tr("TS0"), resWidget};
	auto* resTS0Layout = new QFormLayout{this->resTS0Group};
	resTS0Layout->setFormAlignment(Qt::AlignHCenter);

	this->resTS0Value = new QLineEdit{resWidget};
	this->resTS0Value->setInputMask("HHHHHHHH");
	resTS0Layout->addRow(tr("Value"), this->resTS0Value);

	resWidgetLayout->addWidget(this->resTS0Group);

	this->resLODGroup = new QGroupBox{tr("LOD Control"), resWidget};
	auto* resLODLayout = new QFormLayout{this->resLODGroup};
	resLODLayout->setFormAlignment(Qt::AlignHCenter);

	this->resLODValueU = new QSpinBox{resWidget};
	this->resLODValueU->setRange(0, std::numeric_limits<uint8_t>::max());
	resLODLayout->addRow(tr("U"), this->resLODValueU);

	this->resLODValueV = new QSpinBox{resWidget};
	this->resLODValueV->setRange(0, std::numeric_limits<uint8_t>::max());
	resLODLayout->addRow(tr("V"), this->resLODValueV);

	this->resLODValueU360 = new QSpinBox{resWidget};
	this->resLODValueU360->setRange(0, std::numeric_limits<uint8_t>::max());
	resLODLayout->addRow(tr("U360"), this->resLODValueU360);

	this->resLODValueV360 = new QSpinBox{resWidget};
	this->resLODValueV360->setRange(0, std::numeric_limits<uint8_t>::max());
	resLODLayout->addRow(tr("V360"), this->resLODValueV360);

	resWidgetLayout->addWidget(this->resLODGroup);

	this->resKeyValuesGroup = new QGroupBox{tr("KeyValues"), resWidget};
	auto* resKeyValuesLayout = new QFormLayout{this->resKeyValuesGroup};
	resKeyValuesLayout->setFormAlignment(Qt::AlignHCenter);

	this->resKeyValuesData = new QPlainTextEdit{resWidget};
	resKeyValuesLayout->addRow(tr("Data"), this->resKeyValuesData);

	resWidgetLayout->addWidget(this->resKeyValuesGroup);

	// todo: particle sheet

	// todo: hotspot

	resWidgetLayout->addStretch(1);

	resScroll->setMinimumWidth(resWidget->sizeHint().width() + this->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
	resScroll->setWidgetResizable(true);
	resScroll->setWidget(resWidget);

	resDock->setWidget(resScroll);
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

		this->previewGeneralGroup->setVisible(false);
		this->previewCurrentMip->setValue(0);

		this->previewAnimationGroup->setVisible(false);
		this->previewCurrentFrame->setValue(0);

		this->previewCubemapGroup->setVisible(false);
		this->previewCurrentFace->setValue(0);

		this->previewDepthGroup->setVisible(false);
		this->previewCurrentDepth->setValue(0);

		this->detailsFileTypeGroup->setVisible(false);
		this->detailsPlatform->setCurrentIndex(0);
		this->detailsVersion->setCurrentIndex(0);
		this->detailsFormat->setCurrentIndex(0);

		this->detailsDimsGroup->setVisible(false);
		this->detailsWidth->setValue(0);
		this->detailsHeight->setValue(0);
		this->detailsDepth->setValue(0);
		this->detailsFrames->setValue(0);
		this->detailsStartFrame->setValue(0);
		this->detailsCubemap->setChecked(false);
		this->detailsMipmaps->setChecked(false);
		this->detailsConsoleMipScale->setValue(0);

		this->detailsMiscellaneousGroup->setVisible(false);
		this->detailsBumpmapScale->setValue(0.0);
		this->detailsReflectivityGroup->setStyleSheet("");
		this->detailsReflectivityR->setText("0.0f");
		this->detailsReflectivityG->setText("0.0f");
		this->detailsReflectivityB->setText("0.0f");

		this->detailsCompressionGroup->setVisible(false);
		this->detailsCompressionMethod->setCurrentIndex(0);
		this->detailsCompressionLevel->setValue(0);

		this->flagsChecks->clear();

		this->resThumbnailGroup->setVisible(false);
		this->resThumbnailPreview->clear();
		this->resThumbnailWidth->setValue(0);
		this->resThumbnailHeight->setValue(0);

		this->resPaletteGroup->setVisible(false);
		this->resPalettePreview->clear();
		(void) this->resPaletteFrame->disconnect();
		this->resPaletteFrame->setValue(0);
		this->resPaletteFrame->setRange(0, 0);
		this->resPaletteFrame->setDisabled(true);

		this->resFallbackGroup->setVisible(false);
		this->resFallbackWidth->setValue(0);
		this->resFallbackHeight->setValue(0);
		this->resFallbackMips->setValue(0);

		this->resCRCGroup->setVisible(false);
		this->resCRCValue->setText("00000000");

		this->resTS0Group->setVisible(false);
		this->resTS0Value->setText("00000000");

		this->resLODGroup->setVisible(false);
		this->resLODValueU->setValue(0);
		this->resLODValueV->setValue(0);
		this->resLODValueU360->setValue(0);
		this->resLODValueV360->setValue(0);

		this->resKeyValuesGroup->setVisible(false);
		this->resKeyValuesData->clear();

		return;
	}

	const vtfpp::VTF& vtf = activeTexture->getVTF();
	this->setWindowTitle(QString{"%1 | %2 | %3x%4%5[*]"}.arg(PROJECT_TITLE).arg(this->textureTabs->tabText(activeIndex)).arg(vtf.getWidth()).arg(vtf.getHeight()).arg(vtf.getDepth() > 1 ? QString{"x%1"}.arg(vtf.getDepth()) : QString{}));

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

	this->previewGeneralGroup->setVisible(true);
	this->previewCurrentMip->setValue(activeTexture->getCurrentMip());
	this->previewCurrentMip->setRange(0, vtf.getMipCount() - 1);
	this->previewCurrentMip->setEnabled(vtf.getMipCount() > 1);

	this->previewAnimationGroup->setVisible(vtf.getFrameCount() > 1);
	this->previewCurrentFrame->setValue(activeTexture->getCurrentFrame());
	this->previewCurrentFrame->setRange(0, vtf.getFrameCount() - 1);

	this->previewCubemapGroup->setVisible(vtf.getFaceCount() > 1);
	this->previewCurrentFace->setValue(activeTexture->getCurrentFace());
	this->previewCurrentFace->setRange(0, vtf.getFaceCount() - 1);

	this->previewDepthGroup->setVisible(vtf.getDepth() > 1);
	this->previewCurrentDepth->setValue(activeTexture->getCurrentDepth());
	this->previewCurrentDepth->setRange(0, vtf.getDepth() - 1);

	this->detailsFileTypeGroup->setVisible(true);
	searchAndSetCombo(this->detailsPlatform, vtf.getPlatform());
	this->detailsVersion->setCurrentIndex(static_cast<int>(vtf.getVersion()));
	searchAndSetCombo(this->detailsFormat, static_cast<int>(vtf.getFormat()));

	this->detailsDimsGroup->setVisible(true);
	this->detailsWidth->setValue(vtf.getWidth());
	this->detailsHeight->setValue(vtf.getHeight());
	this->detailsDepth->setValue(vtf.getDepth());
	this->detailsFrames->setValue(vtf.getFrameCount());
	this->detailsStartFrame->setValue(vtf.getStartFrame());
	this->detailsCubemap->setChecked(vtf.getFaceCount() > 1);
	this->detailsMipmaps->setChecked(vtf.getMipCount() > 1);
	this->detailsConsoleMipScale->setValue(vtf.getConsoleMipScale());

	this->detailsMiscellaneousGroup->setVisible(true);
	this->detailsBumpmapScale->setValue(vtf.getBumpMapScale());
	if (
		vtf.getReflectivity()[0] >= 0.f && vtf.getReflectivity()[1] >= 0.f && vtf.getReflectivity()[2] >= 0.f &&
		vtf.getReflectivity()[0] <= 1.f && vtf.getReflectivity()[1] <= 1.f && vtf.getReflectivity()[2] <= 1.f
	) {
		this->detailsReflectivityGroup->setStyleSheet(std::format(
			"QGroupBox {{ border: 2px solid #{:02X}{:02X}{:02X}; border-radius: 4px; }}",
			std::clamp<int>(static_cast<int>(std::pow(vtf.getReflectivity()[0], 1 / 2.2f) * 255), 0, 255),
			std::clamp<int>(static_cast<int>(std::pow(vtf.getReflectivity()[1], 1 / 2.2f) * 255), 0, 255),
			std::clamp<int>(static_cast<int>(std::pow(vtf.getReflectivity()[2], 1 / 2.2f) * 255), 0, 255)
		).c_str());
	}
	this->detailsReflectivityR->setText(QString{"%1f"}.arg(vtf.getReflectivity()[0]));
	this->detailsReflectivityG->setText(QString{"%1f"}.arg(vtf.getReflectivity()[1]));
	this->detailsReflectivityB->setText(QString{"%1f"}.arg(vtf.getReflectivity()[2]));

	// Visibility is controlled elsewhere
	//this->detailsCompressionGroup->setVisible(true);
	searchAndSetCombo(this->detailsCompressionMethod, static_cast<int>(vtf.getCompressionMethod()), vtf.getCompressionLevel() > 0 || vtf.getCompressionMethod() == vtfpp::CompressionMethod::CONSOLE_LZMA);
	this->detailsCompressionLevel->setValue(vtf.getCompressionLevel());

	this->flagsChecks->clear();
	const auto prettyFlagNames = ::getPrettyFlagNamesFor(vtf.getVersion(), vtf.getPlatform());
	for (int i = 0; i < 32; i++) {
		auto* flagItem = new QListWidgetItem{prettyFlagNames[i].data(), this->flagsChecks};
		flagItem->setCheckState(activeTexture->getVTF().getFlags() & (vtf.getFlags() & 1 << i) ? Qt::Checked : Qt::Unchecked);
		// basically just populate this differently per VTF version/platform - repopulate on every tab change
		if (1 << i & vtfpp::VTF::FLAG_MASK_INTERNAL) {
			flagItem->setFlags(flagItem->flags() & ~Qt::ItemIsEnabled);
		}
		if (flagItem->text().startsWith("Unused") || flagItem->text().contains("(vtex)")) {
			flagItem->setForeground(Qt::gray);
		}
		this->flagsChecks->addItem(flagItem);
	}

	if (vtf.hasThumbnailData()) {
		this->resThumbnailGroup->setVisible(true);
		const auto thumbnailPreviewImageData = vtf.getThumbnailDataAs(vtfpp::ImageFormat::BGR888);
		this->resThumbnailPreview->setPixmap(QPixmap::fromImage({reinterpret_cast<const uchar*>(thumbnailPreviewImageData.data()), vtf.getThumbnailWidth(), vtf.getThumbnailHeight(), QImage::Format_BGR888}).scaledToWidth(64));
		this->resThumbnailWidth->setValue(vtf.getThumbnailWidth());
		this->resThumbnailHeight->setValue(vtf.getThumbnailHeight());
	} else {
		this->resThumbnailGroup->setVisible(false);
		this->resThumbnailPreview->clear();
		this->resThumbnailWidth->setValue(0);
		this->resThumbnailHeight->setValue(0);
	}

	if (const auto resource = vtf.getResource(vtfpp::Resource::TYPE_PALETTE_DATA)) {
		this->resPaletteGroup->setVisible(true);

		std::vector<std::byte> paletteFramesPreviewImageData;
		for (int i = 0; i < vtf.getFrameCount(); i++) {
			const auto data = vtfpp::ImageConversion::convertImageDataToFormat(resource->getDataAsPalette(i), vtfpp::ImageFormat::BGRA8888, vtfpp::ImageFormat::BGR888, 16, 16);
			paletteFramesPreviewImageData.insert(paletteFramesPreviewImageData.begin(), data.begin(), data.end());
		}
		connect(this->resPaletteFrame, &QSpinBox::valueChanged, this, [this, paletteFramesPreviewImageData_ = std::move(paletteFramesPreviewImageData)](int value) {
			this->resPalettePreview->setPixmap(QPixmap::fromImage({reinterpret_cast<const uchar*>(paletteFramesPreviewImageData_.data() + value * 256 * sizeof(vtfpp::ImagePixel::BGRA8888)), 16, 16, QImage::Format_BGR888}).scaledToWidth(64));
		});

		if (this->resPaletteFrame->value() == 0) {
			this->resPaletteFrame->valueChanged(0);
		} else {
			this->resPaletteFrame->setValue(0);
		}
		this->resPaletteFrame->setRange(0, vtf.getFrameCount() - 1);
		this->resPaletteFrame->setDisabled(vtf.getFrameCount() <= 1);
	} else {
		this->resPaletteGroup->setVisible(false);
		this->resPalettePreview->clear();
		(void) this->resPaletteFrame->disconnect();
		this->resPaletteFrame->setValue(0);
		this->resPaletteFrame->setRange(0, 0);
		this->resPaletteFrame->setDisabled(true);
	}

	if (vtf.hasFallbackData()) {
		this->resFallbackGroup->setVisible(true);
		this->resFallbackWidth->setValue(vtf.getFallbackWidth());
		this->resFallbackHeight->setValue(vtf.getFallbackHeight());
		this->resFallbackMips->setValue(vtf.getFallbackMipCount());
	} else {
		this->resFallbackGroup->setVisible(false);
		this->resFallbackWidth->setValue(0);
		this->resFallbackHeight->setValue(0);
		this->resFallbackMips->setValue(0);
	}

	if (const auto resource = vtf.getResource(vtfpp::Resource::TYPE_CRC)) {
		this->resCRCGroup->setVisible(true);
		this->resCRCValue->setText(QString{"%1"}.arg(resource->getDataAsCRC(), 8, 16, QChar{u'0'}));
	} else {
		this->resCRCGroup->setVisible(false);
		this->resCRCValue->setText("00000000");
	}

	if (const auto resource = vtf.getResource(vtfpp::Resource::TYPE_EXTENDED_FLAGS)) {
		this->resTS0Group->setVisible(true);
		this->resTS0Value->setText(QString{"%1"}.arg(resource->getDataAsExtendedFlags(), 8, 16, QChar{u'0'}));
	} else {
		this->resTS0Group->setVisible(false);
		this->resTS0Value->setText("00000000");
	}

	if (const auto resource = vtf.getResource(vtfpp::Resource::TYPE_LOD_CONTROL_INFO)) {
		const auto lodData = resource->getDataAsLODControlInfo();
		this->resLODGroup->setVisible(true);
		this->resLODValueU->setValue(std::get<0>(lodData));
		this->resLODValueV->setValue(std::get<1>(lodData));
		this->resLODValueU360->setValue(std::get<2>(lodData));
		this->resLODValueV360->setValue(std::get<3>(lodData));
	} else {
		this->resLODGroup->setVisible(false);
		this->resLODValueU->setValue(0);
		this->resLODValueV->setValue(0);
		this->resLODValueU360->setValue(0);
		this->resLODValueV360->setValue(0);
	}

	if (const auto resource = vtf.getResource(vtfpp::Resource::TYPE_KEYVALUES_DATA)) {
		this->resKeyValuesGroup->setVisible(true);
		this->resKeyValuesData->setPlainText(resource->getDataAsKeyValuesData().c_str());
	} else {
		this->resKeyValuesGroup->setVisible(false);
		this->resKeyValuesData->clear();
	}
}
