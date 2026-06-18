#include "QMareTextureWindow.h"

#include <format>
#include <limits>

#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QStyleFactory>
#include <QTabWidget>
#include <QTimer>

#include "Common.h"
#include "Config.h"
#include "EnumMappings.h"

#include "dialogs/QMareCreateTextureDialog.h"
#include "dialogs/QMareCreditsDialog.h"
#include "dialogs/QMareExtractFromTextureDialog.h"
#include "utility/QMareDiscordPresence.h"
#include "utility/QMareOptions.h"
#include "widgets/QMareComboBox.h"
#include "widgets/QMareFlagsWidget.h"
#include "widgets/QMareMiddleClickTabWidget.h"
#include "widgets/QMareSpinBox.h"
#include "widgets/QMareTextureWidget.h"

QMareTextureWindow* g_ManeWindow = nullptr;

QMareTextureWindow::QMareTextureWindow() {
	// Window setup ---------------------------------------

	this->setWindowIcon(QIcon(":/logo.png"));
#if !defined(Q_OS_WASM) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	this->resize(this->screen()->availableGeometry().size() * 0.7);
#endif

	// Settings helper ------------------------------------

	const auto showRestartWarning = [this] {
		static bool shownRestartMessage = false;
		if (!shownRestartMessage) {
			QMessageBox::warning(this, tr("Restart Required"), tr("The application must be restarted for these settings to take effect."));
			shownRestartMessage = true;
		}
	};

	// File menu ------------------------------------------

	auto* fileMenu = this->menuBar()->addMenu(tr("&File"));

	fileMenu->addAction(QIcon{":/button_new.png"}, tr("&Create"), Qt::CTRL | Qt::Key_N, [this] {
		if (auto* createTextureDialog = QMareCreateTextureDialog::fromImages(this)) {
			connect(createTextureDialog, &QMareCreateTextureDialog::createdTextures, this, [this](const QStringList& paths) {
				for (const auto& path : paths) {
					this->loadTexture(path);
				}
			});
			createTextureDialog->setAttribute(Qt::WA_DeleteOnClose);
			createTextureDialog->open();
		}
	});

	fileMenu->addAction(QIcon{":/button_new_multi.png"}, tr("C&reate from Folder"), Qt::CTRL | Qt::SHIFT | Qt::Key_N, [this] {
		if (auto* createTextureDialog = QMareCreateTextureDialog::fromDir(this)) {
			createTextureDialog->setAttribute(Qt::WA_DeleteOnClose);
			createTextureDialog->open();
		}
	});

	fileMenu->addSeparator();

	fileMenu->addAction(QIcon{":/button_load.png"}, tr("&Open"), Qt::CTRL | Qt::Key_O, [this] {
		for (const auto& file : QFileDialog::getOpenFileNames(this, tr("Open Textures"), {}, QString{"Valve Texture Format (*.vtf *.xtf);;"} + tr("All Files") + " (*)")) {
			this->loadTexture(file);
		}
	});

	fileMenu->addSeparator();

	fileMenu->addAction(QIcon{QMareOptions::get<bool>(QMareOptions::BOOL_ENABLE_TRYPANOPHOBIA_MODE) ? ":/button_extract_alt.png" : ":/button_extract.png"}, tr("&Extract"), Qt::CTRL | Qt::Key_E, [this] {
		if (auto* extractFromTextureDialog = QMareExtractFromTextureDialog::fromTextures(this)) {
			extractFromTextureDialog->setAttribute(Qt::WA_DeleteOnClose);
			extractFromTextureDialog->open();
		}
	});

	fileMenu->addAction(QIcon{QMareOptions::get<bool>(QMareOptions::BOOL_ENABLE_TRYPANOPHOBIA_MODE) ? ":/button_extract_multi_alt.png" : ":/button_extract_multi.png"}, tr("E&xtract from Folder"), Qt::CTRL | Qt::SHIFT | Qt::Key_E, [this] {
		if (auto* extractFromTextureDialog = QMareExtractFromTextureDialog::fromDir(this)) {
			extractFromTextureDialog->setAttribute(Qt::WA_DeleteOnClose);
			extractFromTextureDialog->open();
		}
	});

	fileMenu->addSeparator();

	fileMenu->addAction(QIcon{":/button_kofi.png"}, tr("&Donate"), [] {
		QDesktopServices::openUrl({"https://ko-fi.com/craftablescience"});
	});

#if !defined(Q_OS_WASM) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	fileMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogCancelButton), tr("&Exit"), Qt::ALT | Qt::Key_F4, [this] {
		this->close();
	});
#endif

	// View menu ------------------------------------------

	auto* viewMenu = this->menuBar()->addMenu(tr("&View"));

	// Options menu ---------------------------------------

	auto* optionsMenu = this->menuBar()->addMenu(tr("&Options"));

#if !defined(Q_OS_WASM) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	auto* generalMenu = optionsMenu->addMenu(QIcon{":/logo.png"}, tr("&General"));

	auto* optionRaiseToTopOpeningFileAction = generalMenu->addAction(tr("Raise When Opening New File"), std::bind_front(&QMareOptions::invert, QMareOptions::BOOL_RAISE_TO_TOP_OPENING_FILE));
	optionRaiseToTopOpeningFileAction->setCheckable(true);
	optionRaiseToTopOpeningFileAction->setChecked(QMareOptions::get<bool>(QMareOptions::BOOL_RAISE_TO_TOP_OPENING_FILE));

	auto* optionAllowMultipleAppInstancesAction = generalMenu->addAction(tr("Allow Multiple Application Instances"), [showRestartWarning] {
		showRestartWarning();
		QMareOptions::invert(QMareOptions::BOOL_ALLOW_MULTIPLE_APP_INSTANCES);
	});
	optionAllowMultipleAppInstancesAction->setCheckable(true);
	optionAllowMultipleAppInstancesAction->setChecked(QMareOptions::get<bool>(QMareOptions::BOOL_ALLOW_MULTIPLE_APP_INSTANCES));
#endif

	auto* optionTrypanophobiaModeAction = generalMenu->addAction(tr("Trypanophobia Mode"), [showRestartWarning] {
		showRestartWarning();
		QMareOptions::invert(QMareOptions::BOOL_ENABLE_TRYPANOPHOBIA_MODE);
	});
	optionTrypanophobiaModeAction->setCheckable(true);
	optionTrypanophobiaModeAction->setChecked(QMareOptions::get<bool>(QMareOptions::BOOL_ENABLE_TRYPANOPHOBIA_MODE));

	auto* workspaceMenu = optionsMenu->addMenu(this->style()->standardIcon(QStyle::SP_FileIcon), tr("&Workspace"));

	auto* optionShowTabBarForSingleFileAction = workspaceMenu->addAction(tr("Show Tab Bar for Single File"), [this] {
		QMareOptions::invert(QMareOptions::BOOL_SHOW_TAB_BAR_FOR_SINGLE_FILE);
		this->textureTabs->setTabBarAutoHide(!QMareOptions::get<bool>(QMareOptions::BOOL_SHOW_TAB_BAR_FOR_SINGLE_FILE));
	});
	optionShowTabBarForSingleFileAction->setCheckable(true);
	optionShowTabBarForSingleFileAction->setChecked(QMareOptions::get<bool>(QMareOptions::BOOL_SHOW_TAB_BAR_FOR_SINGLE_FILE));

	auto* optionHighQualityThumbnailsAction = workspaceMenu->addAction(tr("High Quality File Thumbnails"), std::bind_front(&QMareOptions::invert, QMareOptions::BOOL_HIGH_QUALITY_THUMBNAILS));
	optionHighQualityThumbnailsAction->setCheckable(true);
	optionHighQualityThumbnailsAction->setChecked(QMareOptions::get<bool>(QMareOptions::BOOL_HIGH_QUALITY_THUMBNAILS));

	auto* optionHighQualityMinimapAction = workspaceMenu->addAction(tr("High Quality Minimap"), [this] {
		QMareOptions::invert(QMareOptions::BOOL_HIGH_QUALITY_MINIMAP);
		if (auto* tab = this->textureTabs->currentWidget()) {
			tab->update();
		}
	});
	optionHighQualityMinimapAction->setCheckable(true);
	optionHighQualityMinimapAction->setChecked(QMareOptions::get<bool>(QMareOptions::BOOL_HIGH_QUALITY_MINIMAP));

	auto* minimapScaleMenu = workspaceMenu->addMenu(tr("&Minimap Scale"));
	auto* minimapScaleMenuGroup = new QActionGroup{minimapScaleMenu};
	minimapScaleMenuGroup->setExclusive(true);
	const QVector<QPair<QString, int>> minimapScaleMapping{
		{tr("Disabled"),  0},
		{tr("Small"),    -1},
		{tr("Medium"),   -2},
		{tr("Large"),    -3},
	};
	for (const auto& [minimapScaleName, minimapScaleValue] : minimapScaleMapping) {
		auto* action = minimapScaleMenu->addAction(minimapScaleName, [this, minimapScaleValue] {
			QMareOptions::set(QMareOptions::INT_MINIMAP_SCALE, minimapScaleValue);
			if (auto* tab = this->textureTabs->currentWidget()) {
				tab->update();
			}
		});
		action->setCheckable(true);
		action->setChecked(minimapScaleValue == QMareOptions::get<int>(QMareOptions::INT_MINIMAP_SCALE));
		minimapScaleMenuGroup->addAction(action);
	}

	auto* languageMenu = optionsMenu->addMenu(this->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("&Language"));
	auto* languageMenuGroup = new QActionGroup{languageMenu};
	languageMenuGroup->setExclusive(true);
	const QVector<QPair<QString, QString>> languageToLocaleMapping{
		{tr("System Language"), ""},
		{"", ""}, // Separator
		{u8"English", "en"},
		{u8"日本語",   "ja"},
		{u8"Русский", "ru_RU"},
		{u8"Español", "es"},
	};
	for (const auto& [language, locale] : languageToLocaleMapping) {
		if (language.isEmpty() && locale.isEmpty()) {
			languageMenu->addSeparator();
			continue;
		}
		auto* action = languageMenu->addAction(language, [showRestartWarning, locale_=locale] {
			showRestartWarning();
			QMareOptions::set(QMareOptions::STR_LANGUAGE_OVERRIDE, locale_);
		});
		action->setCheckable(true);
		action->setChecked(locale == QMareOptions::get<QString>(QMareOptions::STR_LANGUAGE_OVERRIDE));
		languageMenuGroup->addAction(action);
	}

	auto* themeMenu = optionsMenu->addMenu(this->style()->standardIcon(QStyle::SP_DesktopIcon), tr("&Theme"));
	auto* themeMenuGroup = new QActionGroup{themeMenu};
	themeMenuGroup->setExclusive(true);
	for (const auto& themeName : QStyleFactory::keys()) {
		auto* action = themeMenu->addAction(themeName, [this, themeName] {
			QApplication::setStyle(themeName);
			QMareOptions::set(QMareOptions::STR_STYLE, themeName);
			emit this->themeUpdated();
		});
		action->setCheckable(true);
		if (themeName.toLower() == QMareOptions::get<QString>(QMareOptions::STR_STYLE).toLower()) {
			action->setChecked(true);
		}
		themeMenuGroup->addAction(action);
	}

#if !defined(Q_OS_WASM) && !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	// Not translating this menu name, the translation is the same everywhere
	auto* discordMenu = optionsMenu->addMenu(QIcon{":/button_discord.png"}, "&Discord");
	const auto setupDiscordRichPresence = [] {
		QMareDiscordPresence::init("1384260711202422845");
		QMareDiscordPresence::setState("v" PROJECT_VERSION_PRETTY);
		QMareDiscordPresence::setLargeImage("icon");
		QMareDiscordPresence::setLargeImageText(PROJECT_TITLE);
		QMareDiscordPresence::setTopButton({"View on GitHub", PROJECT_GITHUB_URL});
	};
	auto* discordEnableAction = discordMenu->addAction(tr("Enable Rich Presence"), [setupDiscordRichPresence] {
		QMareOptions::invert(QMareOptions::BOOL_ENABLE_DISCORD_RICH_PRESENCE);
		if (QMareOptions::get<bool>(QMareOptions::BOOL_ENABLE_DISCORD_RICH_PRESENCE)) {
			setupDiscordRichPresence();
		} else {
			QMareDiscordPresence::shutdown();
		}
	});
	discordEnableAction->setCheckable(true);
	discordEnableAction->setChecked(QMareOptions::get<bool>(QMareOptions::BOOL_ENABLE_DISCORD_RICH_PRESENCE));

	if (QMareOptions::get<bool>(QMareOptions::BOOL_ENABLE_DISCORD_RICH_PRESENCE)) {
		setupDiscordRichPresence();
	}
	auto* discordUpdateTimer = new QTimer{this};
	QObject::connect(discordUpdateTimer, &QTimer::timeout, this, &QMareDiscordPresence::update);
	discordUpdateTimer->start(20);
#endif

	// Help menu ------------------------------------------

	auto* helpMenu = this->menuBar()->addMenu(tr("&Help"));

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("Credi&ts"), Qt::Key_F1, [this] {
		QMareCreditsDialog::showCredits(this);
	});

	helpMenu->addSeparator();

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("Report an &Issue"), [] {
		QDesktopServices::openUrl({PROJECT_GITHUB_URL "/issues/new"});
	});

	helpMenu->addAction(this->style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("Request a &Feature"), [] {
		QDesktopServices::openUrl({PROJECT_GITHUB_URL "/issues/new"});
	});

	// Texture tabs ---------------------------------------

	this->textureTabs = new QMareMiddleClickTabWidget{this};
	this->textureTabs->setDocumentMode(true);
	this->textureTabs->setIconSize({64, 64});
	this->textureTabs->setMovable(true);
	this->textureTabs->setTabsClosable(true);
	this->textureTabs->setTabBarAutoHide(!QMareOptions::get<bool>(QMareOptions::BOOL_SHOW_TAB_BAR_FOR_SINGLE_FILE));
	this->setCentralWidget(this->textureTabs);

	connect(this->textureTabs, &QMareMiddleClickTabWidget::currentChanged, this, &QMareTextureWindow::regenerateDetails);
	connect(this->textureTabs, &QMareMiddleClickTabWidget::tabCloseRequested, this, [this](int index) {
		// todo: save confirmation
		this->textureTabs->removeTab(index);
	});

	// Preview dock ---------------------------------------

	this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

	this->previewDock = new QDockWidget{tr("Preview"), this};
	this->previewDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	auto* previewWidget = new QWidget{this->previewDock};
	auto* previewWidgetLayout = new QVBoxLayout{previewWidget};

	this->previewGeneralGroup = new QGroupBox{tr("General"), previewWidget};
	auto* previewGeneralLayout = new QFormLayout{this->previewGeneralGroup};
	previewGeneralLayout->setFormAlignment(Qt::AlignHCenter);

	auto* rgbaParent = new QWidget{previewWidget};
	auto* rgbaLayout = new QHBoxLayout{rgbaParent};
	rgbaLayout->setContentsMargins(0, 0, 0, 0);

	this->previewR = new QCheckBox{tr("R"), rgbaParent};
	rgbaLayout->addWidget(this->previewR);

	// Change red
	connect(this->previewR, &QCheckBox::toggled, this, [this](bool checked) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setR(checked);
		}
	});

	this->previewG = new QCheckBox{tr("G"), rgbaParent};
	rgbaLayout->addWidget(this->previewG);

	// Change green
	connect(this->previewG, &QCheckBox::toggled, this, [this](bool checked) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setG(checked);
		}
	});

	this->previewB = new QCheckBox{tr("B"), rgbaParent};
	rgbaLayout->addWidget(this->previewB);

	// Change blue
	connect(this->previewB, &QCheckBox::toggled, this, [this](bool checked) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setB(checked);
		}
	});

	this->previewA = new QCheckBox{tr("A"), rgbaParent};
	rgbaLayout->addWidget(this->previewA);

	// Change alpha
	connect(this->previewA, &QCheckBox::toggled, this, [this](bool checked) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setA(checked);
		}
	});

	previewGeneralLayout->addRow(tr("Channels"), rgbaParent);

	auto* previewExtrasParent = new QWidget{previewWidget};
	auto* previewExtrasLayout = new QHBoxLayout{previewExtrasParent};
	previewExtrasLayout->setContentsMargins(0, 0, 0, 0);

	this->previewBackground = new QCheckBox{tr("Background"), previewExtrasParent};
	previewExtrasLayout->addWidget(this->previewBackground);

	// Change background
	connect(this->previewBackground, &QCheckBox::toggled, this, [this](bool checked) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setBackground(checked);
		}
	});

	this->previewTiled = new QCheckBox{tr("Tiled"), previewExtrasParent};
	previewExtrasLayout->addWidget(this->previewTiled);

	// Change tiled
	connect(this->previewTiled, &QCheckBox::toggled, this, [this](bool checked) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setTiled(checked);
		}
	});

	previewGeneralLayout->addWidget(previewExtrasParent);

	this->previewCurrentMip = new QMareSpinBox{previewWidget};
	previewGeneralLayout->addRow(tr("Current Mip"), this->previewCurrentMip);

	// Change the current mip
	connect(this->previewCurrentMip, &QMareSpinBox::valueChanged, this, [this](int value) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setCurrentMip(value);
		}
	});

	previewWidgetLayout->addWidget(this->previewGeneralGroup);

	this->previewAnimationGroup = new QGroupBox{tr("Animation"), previewWidget};
	auto* previewAnimationLayout = new QFormLayout{this->previewAnimationGroup};
	previewAnimationLayout->setFormAlignment(Qt::AlignHCenter);

	this->previewCurrentFrame = new QMareSpinBox{previewWidget};
	previewAnimationLayout->addRow(tr("Current Frame"), this->previewCurrentFrame);

	// Change the current frame
	connect(this->previewCurrentFrame, &QMareSpinBox::valueChanged, this, [this](int value) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setCurrentFrame(value);
		}
	});

	this->previewAnimationSpeed = new QMareDoubleSpinBox{previewWidget};
	this->previewAnimationSpeed->setRange(0.0, 120.0);
	this->previewAnimationSpeed->setValue(24.0);
	previewAnimationLayout->addRow(tr("Animation FPS"), this->previewAnimationSpeed);

	this->previewAnimate = new QCheckBox{previewWidget};
	previewAnimationLayout->addRow(tr("Animate"), this->previewAnimate);

	this->previewAnimateTimer = new QTimer{previewWidget};
	this->previewAnimateTimer->setTimerType(Qt::PreciseTimer);
	this->previewAnimateTimer->start(static_cast<int>(1000.0 / this->previewAnimationSpeed->value()));

	// Set the timer duration
	connect(this->previewAnimationSpeed, &QMareDoubleSpinBox::valueChanged, this, [this](double value) {
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

	this->previewCubemapMode = new QMareComboBox{previewWidget};
	this->previewCubemapMode->addItems({tr("Net"), tr("Single Face")});
	previewCubemapLayout->addRow(tr("Preview Mode"), this->previewCubemapMode);

	// Change cubemap preview mode
	connect(this->previewCubemapMode, &QMareComboBox::currentIndexChanged, this, [this](int index) {
		this->previewCurrentFace->setDisabled(index != 1);
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setCurrentCubemapMode(index);
		}
	});

	this->previewCurrentFace = new QMareSpinBox{previewWidget};
	this->previewCurrentFace->setDisabled(true);
	previewCubemapLayout->addRow(tr("Current Face"), this->previewCurrentFace);

	// Change the current face
	connect(this->previewCurrentFace, &QMareSpinBox::valueChanged, this, [this](int value) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setCurrentFace(value);
		}
	});

	previewWidgetLayout->addWidget(this->previewCubemapGroup);

	this->previewDepthGroup = new QGroupBox{tr("Depth"), previewWidget};
	auto* previewDepthLayout = new QFormLayout{this->previewDepthGroup};
	previewDepthLayout->setFormAlignment(Qt::AlignHCenter);

	this->previewCurrentDepth = new QMareSpinBox{previewWidget};
	previewDepthLayout->addRow(tr("Current Depth"), this->previewCurrentDepth);

	// Change the current depth
	connect(this->previewCurrentDepth, &QMareSpinBox::valueChanged, this, [this](int value) {
		if (auto* activeTexture = dynamic_cast<QMareTextureWidget*>(this->textureTabs->widget(this->textureTabs->currentIndex()))) {
			activeTexture->setCurrentDepth(value);
		}
	});

	previewWidgetLayout->addWidget(this->previewDepthGroup);

	previewWidgetLayout->addStretch(1);

	this->previewDock->setWidget(previewWidget);
	this->addDockWidget(Qt::LeftDockWidgetArea, this->previewDock);
	viewMenu->addAction(this->previewDock->toggleViewAction());

	// Details dock ------------------------------------------

	this->detailsDock = new QDockWidget{tr("Details"), this};
	this->detailsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	auto* detailsScroll = new QScrollArea{this->detailsDock};

	auto* detailsWidget = new QWidget{detailsScroll};
	auto* detailsWidgetLayout = new QVBoxLayout{detailsWidget};

	this->detailsFileTypeGroup = new QGroupBox{tr("File Type"), detailsWidget};
	auto* detailsFileTypeLayout = new QFormLayout{this->detailsFileTypeGroup};
	detailsFileTypeLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsPlatform = new QMareComboBox{this->detailsFileTypeGroup};
	this->detailsPlatform->addItem(tr("PC"),               vtfpp::VTF::PLATFORM_PC);
	this->detailsPlatform->addItem(tr("Xbox"),             vtfpp::VTF::PLATFORM_XBOX);
	this->detailsPlatform->addItem(tr("Xbox 360"),         vtfpp::VTF::PLATFORM_X360);
	this->detailsPlatform->addItem(tr("PS3 (Orange Box)"), vtfpp::VTF::PLATFORM_PS3_ORANGEBOX);
	this->detailsPlatform->addItem(tr("PS3 (P2, CS:GO)"),  vtfpp::VTF::PLATFORM_PS3_PORTAL2);
	detailsFileTypeLayout->addRow(tr("Platform"), this->detailsPlatform);

	this->detailsVersion = new QMareComboBox{this->detailsFileTypeGroup};
	for (int i = 0; i <= 6; i++) {
		this->detailsVersion->addItem(QString{"7.%1"}.arg(i), i);
	}
	detailsFileTypeLayout->addRow(tr("Version"), this->detailsVersion);

	// Hide the Version combo when Platform != PLATFORM_PC
	connect(this->detailsPlatform, &QMareComboBox::currentIndexChanged, this, [this, detailsFileTypeLayout](int index) {
		detailsFileTypeLayout->setRowVisible(1, this->detailsPlatform->itemData(index).toInt() == vtfpp::VTF::PLATFORM_PC);
	});

	this->detailsFormat = new QMareComboBox{this->detailsFileTypeGroup};
	for (const auto& [value, name] : not_magic_enum::enum_entries<vtfpp::ImageFormat>()) {
		if (static_cast<int>(value) < 0) {
			continue;
		}
		this->detailsFormat->addItem(name.data(), static_cast<int>(value));
	}
	detailsFileTypeLayout->addRow(tr("Format"), this->detailsFormat);

	this->detailsFileSizeLabel = new QLabel{this->detailsFileTypeGroup};
	this->detailsFileSize = new QMareDoubleSpinBox{this->detailsFileTypeGroup};
	this->detailsFileSize->setDecimals(3);
	this->detailsFileSize->setDisabled(true);
	detailsFileTypeLayout->addRow(this->detailsFileSizeLabel, this->detailsFileSize);

	detailsWidgetLayout->addWidget(this->detailsFileTypeGroup);

	this->detailsDimsGroup = new QGroupBox{tr("Dimensions"), detailsWidget};
	auto* detailsDimsLayout = new QFormLayout{this->detailsDimsGroup};
	detailsDimsLayout->setFormAlignment(Qt::AlignHCenter);
	detailsDimsLayout->setLabelAlignment(Qt::AlignLeft);

	this->detailsWidth = new QMareSpinBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Width"), this->detailsWidth);
	this->detailsHeight = new QMareSpinBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Height"), this->detailsHeight);
	this->detailsDepth = new QMareSpinBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Depth"), this->detailsDepth);
	this->detailsFrames = new QMareSpinBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Frames"), this->detailsFrames);
	this->detailsStartFrame = new QMareSpinBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Start Frame"), this->detailsStartFrame);
	for (auto* spinBox : {this->detailsWidth, this->detailsHeight, this->detailsDepth, this->detailsFrames, this->detailsStartFrame}) {
		spinBox->setRange(0, std::numeric_limits<uint16_t>::max());
	}
	for (auto* spinBox : {this->detailsWidth, this->detailsHeight, this->detailsDepth}) {
		spinBox->setSuffix("px");
	}
	this->detailsCubemap = new QCheckBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Cubemap"), this->detailsCubemap);
	this->detailsMipmaps = new QCheckBox{this->detailsDimsGroup};
	detailsDimsLayout->addRow(tr("Mipmaps"), this->detailsMipmaps);
	this->detailsConsoleMipScale = new QMareSpinBox{this->detailsDimsGroup};
	this->detailsConsoleMipScale->setRange(0, 31);
	detailsDimsLayout->addRow(tr("Mip Scale (Console)"), this->detailsConsoleMipScale);

	detailsWidgetLayout->addWidget(this->detailsDimsGroup);

	this->detailsMiscellaneousGroup = new QGroupBox{tr("Miscellaneous"), detailsWidget};
	auto* detailsMiscellaneousLayout = new QFormLayout{this->detailsMiscellaneousGroup};
	detailsMiscellaneousLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsBumpmapScale = new QMareDoubleSpinBox{this->detailsMiscellaneousGroup};
	this->detailsBumpmapScale->setRange(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
	this->detailsBumpmapScale->setSingleStep(0.1);
	this->detailsBumpmapScale->setDecimals(3);
	detailsMiscellaneousLayout->addRow(tr("Bumpmap Scale"), this->detailsBumpmapScale);

	this->detailsReflectivityGroup = new QGroupBox{this->detailsMiscellaneousGroup};
	auto* detailsReflectivityLayout = new QFormLayout{this->detailsReflectivityGroup};
	detailsReflectivityLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsReflectivityR = new QLabel{this->detailsReflectivityGroup};
	detailsReflectivityLayout->addRow(QString{"%1  "}.arg(tr("R")), this->detailsReflectivityR);
	this->detailsReflectivityG = new QLabel{this->detailsReflectivityGroup};
	detailsReflectivityLayout->addRow(QString{"%1  "}.arg(tr("G")), this->detailsReflectivityG);
	this->detailsReflectivityB = new QLabel{this->detailsReflectivityGroup};
	detailsReflectivityLayout->addRow(QString{"%1  "}.arg(tr("B")), this->detailsReflectivityB);

	detailsMiscellaneousLayout->addRow(tr("Reflectivity"), this->detailsReflectivityGroup);

	detailsWidgetLayout->addWidget(this->detailsMiscellaneousGroup);

	this->detailsCompressionGroup = new QGroupBox{tr("CPU Compression"), detailsWidget};
	auto* detailsCompressionLayout = new QFormLayout{this->detailsCompressionGroup};
	detailsCompressionLayout->setFormAlignment(Qt::AlignHCenter);

	this->detailsCompressionMethod = new QMareComboBox{this->detailsCompressionGroup};
	this->detailsCompressionMethod->addItem("None", 0);
	for (const auto& [value, name] : not_magic_enum::enum_entries<vtfpp::CompressionMethod>(true)) {
		this->detailsCompressionMethod->addItem(name.data(), static_cast<int>(value));
	}
	detailsCompressionLayout->addRow(tr("Method"), this->detailsCompressionMethod);

	this->detailsCompressionLevel = new QMareSpinBox{this->detailsCompressionGroup};
	detailsCompressionLayout->addRow(tr("Level"), this->detailsCompressionLevel);

	// Limit strength to 0-0 for None, 1-9 for Deflate/LZMA, 1-22 for Zstd
	connect(this->detailsCompressionMethod, &QMareComboBox::currentIndexChanged, this, [this](int index) {
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
	connect(this->detailsPlatform, &QMareComboBox::currentIndexChanged, this, [this](int index) {
		this->detailsCompressionGroup->setVisible(this->detailsPlatform->itemData(index).toInt() != vtfpp::VTF::PLATFORM_PC || this->detailsVersion->currentIndex() == 6);
	});
	connect(this->detailsVersion, &QMareComboBox::currentIndexChanged, this, [this](int index) {
		this->detailsCompressionGroup->setVisible(index == 6);
	});

	detailsWidgetLayout->addStretch(1);

	detailsScroll->setMinimumWidth(detailsWidget->sizeHint().width() + this->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
	detailsScroll->setWidgetResizable(true);
	detailsScroll->setWidget(detailsWidget);

	this->detailsDock->setWidget(detailsScroll);
	this->addDockWidget(Qt::LeftDockWidgetArea, this->detailsDock);
	viewMenu->addAction(this->detailsDock->toggleViewAction());

	// Resources dock -------------------------------------

	this->resDock = new QDockWidget{tr("Resources"), this};
	this->resDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	auto* resScroll = new QScrollArea{this->resDock};

	auto* resWidget = new QWidget{resScroll};
	auto* resWidgetLayout = new QVBoxLayout{resWidget};

	this->resThumbnailGroup = new QGroupBox{tr("Thumbnail"), resWidget};
	auto* resThumbnailLayout = new QFormLayout{this->resThumbnailGroup};
	resThumbnailLayout->setFormAlignment(Qt::AlignHCenter);

	this->resThumbnailPreview = new QLabel{resWidget};
	resThumbnailLayout->addRow(tr("Preview"), this->resThumbnailPreview);

	this->resThumbnailWidth = new QMareSpinBox{resWidget};
	this->resThumbnailWidth->setSuffix("px");
	this->resThumbnailWidth->setDisabled(true);
	resThumbnailLayout->addRow(tr("Width"), this->resThumbnailWidth);

	this->resThumbnailHeight = new QMareSpinBox{resWidget};
	this->resThumbnailHeight->setSuffix("px");
	this->resThumbnailHeight->setDisabled(true);
	resThumbnailLayout->addRow(tr("Height"), this->resThumbnailHeight);

	resWidgetLayout->addWidget(this->resThumbnailGroup);

	this->resPaletteGroup = new QGroupBox{tr("Palette"), resWidget};
	auto* resPaletteLayout = new QFormLayout{this->resPaletteGroup};
	resPaletteLayout->setFormAlignment(Qt::AlignHCenter);

	this->resPalettePreview = new QLabel{resWidget};
	resPaletteLayout->addRow(tr("Preview"), this->resPalettePreview);

	this->resPaletteFrame = new QMareSpinBox{resWidget};
	this->resPaletteFrame->setDisabled(true);
	resPaletteLayout->addRow(tr("Frame"), this->resPaletteFrame);

	resWidgetLayout->addWidget(this->resPaletteGroup);

	this->resFallbackGroup = new QGroupBox{tr("Fallback"), resWidget};
	auto* resFallbackLayout = new QFormLayout{this->resFallbackGroup};
	resFallbackLayout->setFormAlignment(Qt::AlignHCenter);

	this->resFallbackWidth = new QMareSpinBox{resWidget};
	this->resFallbackWidth->setSuffix("px");
	this->resFallbackWidth->setDisabled(true);
	resFallbackLayout->addRow(tr("Width"), this->resFallbackWidth);

	this->resFallbackHeight = new QMareSpinBox{resWidget};
	this->resFallbackHeight->setSuffix("px");
	this->resFallbackHeight->setDisabled(true);
	resFallbackLayout->addRow(tr("Height"), this->resFallbackHeight);

	this->resFallbackMips = new QMareSpinBox{resWidget};
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

	this->resLODValueU = new QMareSpinBox{resWidget};
	this->resLODValueU->setRange(0, std::numeric_limits<uint8_t>::max());
	resLODLayout->addRow(tr("U"), this->resLODValueU);

	this->resLODValueV = new QMareSpinBox{resWidget};
	this->resLODValueV->setRange(0, std::numeric_limits<uint8_t>::max());
	resLODLayout->addRow(tr("V"), this->resLODValueV);

	this->resLODValueU360 = new QMareSpinBox{resWidget};
	this->resLODValueU360->setRange(0, std::numeric_limits<uint8_t>::max());
	resLODLayout->addRow(tr("U (Console)"), this->resLODValueU360);

	this->resLODValueV360 = new QMareSpinBox{resWidget};
	this->resLODValueV360->setRange(0, std::numeric_limits<uint8_t>::max());
	resLODLayout->addRow(tr("V (Console)"), this->resLODValueV360);

	resWidgetLayout->addWidget(this->resLODGroup);

	this->resKeyValuesGroup = new QGroupBox{tr("KeyValues"), resWidget};
	auto* resKeyValuesLayout = new QVBoxLayout{this->resKeyValuesGroup};
	resKeyValuesLayout->setAlignment(Qt::AlignHCenter);

	this->resKeyValuesData = new QPlainTextEdit{resWidget};
	resKeyValuesLayout->addWidget(this->resKeyValuesData);

	resWidgetLayout->addWidget(this->resKeyValuesGroup);

	this->resAuthorInfoGroup = new QGroupBox{tr("Author Info"), resWidget};
	auto* resAuthorInfoLayout = new QVBoxLayout{this->resAuthorInfoGroup};
	resAuthorInfoLayout->setAlignment(Qt::AlignHCenter);

	this->resAuthorInfoData = new QLineEdit{resWidget};
	resAuthorInfoLayout->addWidget(this->resAuthorInfoData);

	resWidgetLayout->addWidget(this->resAuthorInfoGroup);

	this->resSourcePPFlagsGroup = new QGroupBox{tr("MareTF Flags"), resWidget};
	auto* resSourcePPFlagsLayout = new QVBoxLayout{this->resSourcePPFlagsGroup};
	resSourcePPFlagsLayout->setAlignment(Qt::AlignHCenter);

	this->resSourcePPFlagsList = new QMareFlagsExtraWidget{this->resSourcePPFlagsGroup};
	resSourcePPFlagsLayout->addWidget(this->resSourcePPFlagsList);

	resWidgetLayout->addWidget(this->resSourcePPFlagsGroup);

	// todo: particle sheet

	// todo: hotspot

	resWidgetLayout->addStretch(1);

	resScroll->setMinimumWidth(resWidget->sizeHint().width() + this->style()->pixelMetric(QStyle::PM_ScrollBarExtent));
	resScroll->setWidgetResizable(true);
	resScroll->setWidget(resWidget);

	this->resDock->setWidget(resScroll);
	this->addDockWidget(Qt::RightDockWidgetArea, this->resDock);
	viewMenu->addAction(this->resDock->toggleViewAction());

	// Flags dock -----------------------------------------

	this->flagsDock = new QDockWidget{tr("Flags"), this};
	this->flagsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	this->flagsChecks = new QMareFlagsWidget{this->flagsDock};
	this->flagsDock->setWidget(this->flagsChecks);
	this->addDockWidget(Qt::RightDockWidgetArea, this->flagsDock);
	this->tabifyDockWidget(this->flagsDock, this->resDock);
	this->flagsDock->raise();
	viewMenu->addAction(this->flagsDock->toggleViewAction());

	// Final setup ----------------------------------------

	this->regenerateDetails();

	this->setAcceptDrops(true);
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
		this->previewR->setChecked(true);
		this->previewG->setChecked(true);
		this->previewB->setChecked(true);
		this->previewA->setChecked(true);
		this->previewBackground->setChecked(true);
		this->previewTiled->setChecked(false);
		this->previewCurrentMip->setValue(0);

		this->previewAnimationGroup->setVisible(false);
		this->previewCurrentFrame->setValue(0);

		this->previewCubemapGroup->setVisible(false);
		this->previewCubemapMode->setCurrentIndex(0);
		this->previewCurrentFace->setValue(0);

		this->previewDepthGroup->setVisible(false);
		this->previewCurrentDepth->setValue(0);

		this->detailsFileTypeGroup->setVisible(false);
		this->detailsPlatform->setCurrentIndex(0);
		this->detailsVersion->setCurrentIndex(0);
		this->detailsFormat->setCurrentIndex(0);
		this->detailsFileSizeLabel->setText(tr("File Size"));
		this->detailsFileSize->setSuffix(tr("b"));
		this->detailsFileSize->setValue(0.0);

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
		this->resPaletteFrame->setRange(0, 0);
		this->resPaletteFrame->setValue(0);
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

		this->resAuthorInfoGroup->setVisible(false);
		this->resAuthorInfoData->clear();

		this->resSourcePPFlagsGroup->setVisible(false);
		this->resSourcePPFlagsList->clear();

		return;
	}

	const vtfpp::VTF& vtf = activeTexture->getVTF();
	this->setWindowTitle(QString{"%1 | %2 | %3x%4%5[*]"}.arg(PROJECT_TITLE).arg(this->textureTabs->tabText(activeIndex)).arg(vtf.getWidth()).arg(vtf.getHeight()).arg(vtf.getDepth() > 1 ? QString{"x%1"}.arg(vtf.getDepth()) : QString{}));

	static constexpr auto searchAndSetCombo = [](QMareComboBox* combo, int value, bool condition = true) {
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
	this->previewR->setChecked(activeTexture->useR());
	this->previewG->setChecked(activeTexture->useG());
	this->previewB->setChecked(activeTexture->useB());
	this->previewA->setChecked(activeTexture->useA());
	this->previewBackground->setChecked(activeTexture->useBackground());
	this->previewTiled->setChecked(activeTexture->useTiled());
	this->previewCurrentMip->setRange(0, vtf.getMipCount() - 1);
	this->previewCurrentMip->setValue(activeTexture->getCurrentMip());
	this->previewCurrentMip->setEnabled(vtf.getMipCount() > 1);

	this->previewAnimationGroup->setVisible(vtf.getFrameCount() > 1);
	this->previewCurrentFrame->setRange(0, vtf.getFrameCount() - 1);
	this->previewCurrentFrame->setValue(activeTexture->getCurrentFrame());

	this->previewCubemapGroup->setVisible(vtf.getFaceCount() > 1);
	this->previewCubemapMode->setCurrentIndex(activeTexture->getCurrentCubemapMode());
	this->previewCurrentFace->setRange(0, vtf.getFaceCount() - 1);
	this->previewCurrentFace->setValue(activeTexture->getCurrentFace());

	this->previewDepthGroup->setVisible(vtf.getDepth() > 1);
	this->previewCurrentDepth->setRange(0, vtf.getDepth() - 1);
	this->previewCurrentDepth->setValue(activeTexture->getCurrentDepth());

	this->detailsFileTypeGroup->setVisible(true);
	searchAndSetCombo(this->detailsPlatform, vtf.getPlatform());
	this->detailsVersion->setCurrentIndex(static_cast<int>(vtf.getVersion()));
	searchAndSetCombo(this->detailsFormat, static_cast<int>(vtf.getFormat()));
	{
		bool exactSize = true;
		auto fileSize = static_cast<double>(vtf.estimateBakeSize(exactSize));
		if (exactSize) {
			this->detailsFileSizeLabel->setText(tr("File Size"));
		} else {
			this->detailsFileSizeLabel->setText(tr("Est. File Size"));
		}
		this->detailsFileSize->setSuffix(tr("b"));
		if (fileSize >= 1024) {
			fileSize /= 1024;
			this->detailsFileSize->setSuffix(tr("kb"));
		}
		if (fileSize > 1024) {
			fileSize /= 1024;
			this->detailsFileSize->setSuffix(tr("mb"));
		}
		this->detailsFileSize->setValue(fileSize);
	}

	this->detailsDimsGroup->setVisible(true);
	this->detailsWidth->setRange(0, std::numeric_limits<uint16_t>::max());
	this->detailsWidth->setValue(vtf.getWidth());
	this->detailsHeight->setRange(0, std::numeric_limits<uint16_t>::max());
	this->detailsHeight->setValue(vtf.getHeight());
	this->detailsDepth->setRange(0, std::numeric_limits<uint16_t>::max());
	this->detailsDepth->setValue(vtf.getDepth());
	this->detailsFrames->setRange(0, std::numeric_limits<uint16_t>::max());
	this->detailsFrames->setValue(vtf.getFrameCount());
	this->detailsStartFrame->setRange(0, std::numeric_limits<uint16_t>::max());
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

	this->flagsChecks->repopulateFlagList(vtf.getFlags(), vtf.getPlatform(), vtf.getVersion());

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
		connect(this->resPaletteFrame, &QMareSpinBox::valueChanged, this, [this, paletteFramesPreviewImageData_ = std::move(paletteFramesPreviewImageData)](int value) {
			this->resPalettePreview->setPixmap(QPixmap::fromImage({reinterpret_cast<const uchar*>(paletteFramesPreviewImageData_.data() + value * 256 * sizeof(vtfpp::ImagePixel::BGRA8888)), 16, 16, QImage::Format_BGR888}).scaledToWidth(64));
		});

		this->resPaletteFrame->setRange(0, vtf.getFrameCount() - 1);
		if (this->resPaletteFrame->value() == 0) {
			this->resPaletteFrame->valueChanged(0);
		} else {
			this->resPaletteFrame->setValue(0);
		}
		this->resPaletteFrame->setDisabled(vtf.getFrameCount() <= 1);
	} else {
		this->resPaletteGroup->setVisible(false);
		this->resPalettePreview->clear();
		(void) this->resPaletteFrame->disconnect();
		this->resPaletteFrame->setRange(0, 0);
		this->resPaletteFrame->setValue(0);
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
		this->resTS0Value->setText(QString{"%1"}.arg(resource->getDataAsFlags(), 8, 16, QChar{u'0'}));
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

	if (const auto resource = vtf.getResource(vtfpp::Resource::TYPE_AUTHOR_INFO)) {
		this->resAuthorInfoGroup->setVisible(true);
		this->resAuthorInfoData->setText(resource->getDataAsAuthorInfo().c_str());
	} else {
		this->resAuthorInfoGroup->setVisible(false);
		this->resAuthorInfoData->clear();
	}

	if (const auto sppFlags = vtf.getFlagsExtra()) {
		this->resSourcePPFlagsGroup->setVisible(true);
		this->resSourcePPFlagsList->repopulateFlagList(sppFlags);
	} else {
		this->resSourcePPFlagsGroup->setVisible(false);
		this->resSourcePPFlagsList->clear();
	}

	// Delay a tick to allow everything to be laid out
	QTimer::singleShot(0, this, [this] {
		this->resizeDocks({
			this->previewDock,
		}, {
			this->previewDock->minimumSizeHint().height(),
		}, Qt::Vertical);
	});
}

void QMareTextureWindow::dragEnterEvent(QDragEnterEvent* event) {
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

void QMareTextureWindow::dropEvent(QDropEvent* event) {
	QStringList texturesToCreate;
	for (const auto& url : event->mimeData()->urls()) {
		if (::fileIsASupportedImageFileFormat(std::filesystem::path{url.fileName().toUtf8().constData()}.extension().string())) {
			texturesToCreate.append(url.toLocalFile());
		} else {
			this->loadTexture(url.toLocalFile());
		}
	}
	event->acceptProposedAction();

	if (!texturesToCreate.isEmpty()) {
		if (auto* createTextureDialog = QMareCreateTextureDialog::fromImages(this, texturesToCreate)) {
			connect(createTextureDialog, &QMareCreateTextureDialog::createdTextures, this, [this](const QStringList& paths) {
				for (const auto& path : paths) {
					this->loadTexture(path);
				}
			});
			createTextureDialog->setAttribute(Qt::WA_DeleteOnClose);
			createTextureDialog->open();
		}
	}
}
