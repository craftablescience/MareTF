#include "QMareCreateTextureDialog.h"

#include <format>

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QTimer>

#include "Common.h"
#include "EnumMappings.h"

#include "utility/QMareCLIWrapper.h"
#include "utility/QMareOptions.h"
#include "widgets/QMareComboBox.h"
#include "widgets/QMareFilesystemBox.h"
#include "widgets/QMareFlagsWidget.h"
#include "widgets/QMareSpinBox.h"

QMareCreateTextureDialog::QMareCreateTextureDialog(const QStringList& inputPaths, bool createFromDir, QWidget* parent) : QDialog{parent} {
	this->setWindowTitle(createFromDir || inputPaths.size() > 1 ? tr("Create Textures") : tr("Create Texture"));
	this->setMinimumWidth(500);

	auto* layout = new QVBoxLayout{this};

	auto* tabs = new QTabWidget{this};
	layout->addWidget(tabs);

	// General texture settings tab
	auto* textureTabScroll = new QScrollArea{tabs};
	auto* textureTab = new QWidget{textureTabScroll};
	textureTabScroll->setWidgetResizable(true);
	textureTabScroll->setWidget(textureTab);
	auto* textureTabLayout = new QFormLayout{textureTab};
	textureTabLayout->setFormAlignment(Qt::AlignHCenter);
	tabs->addTab(textureTabScroll, tr("General"));

	// Platform
	auto* platformCombo = new QMareComboBox{textureTab};
	for (const auto& [platform, platformName] : not_magic_enum::enum_entries<vtfpp::VTF::Platform>(true)) {
		platformCombo->addItem(platformName.data(), static_cast<int>(platform));
	}
	textureTabLayout->addRow(tr("Platform"), platformCombo);

	// Version
	auto* versionCombo = new QMareComboBox{textureTab};
	for (int i = 0; i <= 6; i++) {
		versionCombo->addItem(QString{"7.%1"}.arg(i), i);
	}
#ifdef MARETF_BUILD_FOR_STRATA_SOURCE
	versionCombo->setCurrentIndex(6); // 7.6
#else
	versionCombo->setCurrentIndex(4); // 7.4
#endif
	textureTabLayout->addRow(tr("Version"), versionCombo);

	// Format
	auto* textureOpaqueFormatCombo = new QMareComboBox{textureTab};
	//auto* textureTransparentFormatCombo = new QMareComboBox{textureTab};
	for (const auto& [format, formatName] : not_magic_enum::enum_entries<vtfpp::ImageFormat>(true)) {
		textureOpaqueFormatCombo->addItem(formatName.data(), static_cast<int>(format));
		//textureTransparentFormatCombo->addItem(formatName.data(), static_cast<int>(format));
	}
	textureOpaqueFormatCombo->setCurrentIndex(1); // DEFAULT
	//textureTransparentFormatCombo->setCurrentIndex(1); // DEFAULT
	textureTabLayout->addRow(tr(/*"Opaque "*/ "Format"), textureOpaqueFormatCombo);
	//generalTabLayout->addRow(tr("Transparent Format"), textureTransparentFormatCombo);

	// Compression quality
	auto* textureCompressionQualityEnableCheck = new QCheckBox{tr("Compression Quality"), textureTab};
	textureCompressionQualityEnableCheck->setLayoutDirection(Qt::RightToLeft);

	auto* textureCompressionQualitySpin = new QMareSpinBox{textureTab};
	textureCompressionQualitySpin->setRange(0, 100);
	textureCompressionQualitySpin->setSingleStep(10);
	textureCompressionQualitySpin->setSuffix("%");
	textureCompressionQualitySpin->setValue(100);
	textureCompressionQualitySpin->setDisabled(true);

	textureTabLayout->addRow(textureCompressionQualityEnableCheck, textureCompressionQualitySpin);

	// Width
	auto* textureWidthGroup = new QGroupBox{textureTab};
	auto* textureWidthLayout = new QFormLayout{textureWidthGroup};
	textureWidthLayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureWidthResizeMethodCombo = new QMareComboBox{textureWidthGroup};
	for (const auto& [method, methodName] : not_magic_enum::enum_entries<vtfpp::ImageConversion::ResizeMethod>(true)) {
		textureWidthResizeMethodCombo->addItem(methodName.data(), static_cast<int>(method));
	}
	textureWidthResizeMethodCombo->setCurrentIndex(3); // Power of 2 (Nearest)
	textureWidthLayout->addRow(tr("Resize"), textureWidthResizeMethodCombo);

	auto* textureWidthClampModeCombo = new QMareComboBox{textureWidthGroup};
	textureWidthClampModeCombo->addItems({
		tr("None"),
		tr("Exact"),
		tr("Bounds"),
	});
	textureWidthLayout->addRow(tr("Clamp"), textureWidthClampModeCombo);

	auto* textureWidthExactSizeSpin = new QMareSpinBox{textureWidthGroup};
	textureWidthExactSizeSpin->setRange(1, std::numeric_limits<uint16_t>::max());
	textureWidthExactSizeSpin->setValue(1024);
	textureWidthExactSizeSpin->setSuffix("px");
	textureWidthLayout->addRow(tr("Exact"), textureWidthExactSizeSpin);
	textureWidthLayout->setRowVisible(2, false);

	auto* textureWidthMinimumSizeSpin = new QMareSpinBox{textureWidthGroup};
	textureWidthMinimumSizeSpin->setRange(1, std::numeric_limits<uint16_t>::max());
	textureWidthMinimumSizeSpin->setValue(512);
	textureWidthMinimumSizeSpin->setSuffix("px");
	textureWidthMinimumSizeSpin->setVisible(false);
	textureWidthLayout->addRow(tr("Minimum"), textureWidthMinimumSizeSpin);
	textureWidthLayout->setRowVisible(3, false);

	auto* textureWidthMaximumSizeSpin = new QMareSpinBox{textureWidthGroup};
	textureWidthMaximumSizeSpin->setRange(1, std::numeric_limits<uint16_t>::max());
	textureWidthMaximumSizeSpin->setValue(2048);
	textureWidthMaximumSizeSpin->setSuffix("px");
	textureWidthMinimumSizeSpin->setVisible(false);
	textureWidthLayout->addRow(tr("Maximum"), textureWidthMaximumSizeSpin);
	textureWidthLayout->setRowVisible(4, false);

	textureTabLayout->addRow(tr("Width"), textureWidthGroup);

	// Height
	auto* textureHeightGroup = new QGroupBox{textureTab};
	auto* textureHeightLayout = new QFormLayout{textureHeightGroup};
	textureHeightLayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureHeightResizeMethodCombo = new QMareComboBox{textureHeightGroup};
	for (const auto& [method, methodName] : not_magic_enum::enum_entries<vtfpp::ImageConversion::ResizeMethod>(true)) {
		textureHeightResizeMethodCombo->addItem(methodName.data(), static_cast<int>(method));
	}
	textureHeightResizeMethodCombo->setCurrentIndex(3); // Power of 2 (Nearest)
	textureHeightLayout->addRow(tr("Resize"), textureHeightResizeMethodCombo);

	auto* textureHeightClampModeCombo = new QMareComboBox{textureHeightGroup};
	textureHeightClampModeCombo->addItems({
		tr("None"),
		tr("Exact"),
		tr("Bounds"),
	});
	textureHeightLayout->addRow(tr("Clamp"), textureHeightClampModeCombo);

	auto* textureHeightExactSizeSpin = new QMareSpinBox{textureHeightGroup};
	textureHeightExactSizeSpin->setRange(1, std::numeric_limits<uint16_t>::max());
	textureHeightExactSizeSpin->setValue(1024);
	textureHeightExactSizeSpin->setSuffix("px");
	textureHeightLayout->addRow(tr("Exact"), textureHeightExactSizeSpin);
	textureHeightLayout->setRowVisible(2, false);

	auto* textureHeightMinimumSizeSpin = new QMareSpinBox{textureHeightGroup};
	textureHeightMinimumSizeSpin->setRange(1, std::numeric_limits<uint16_t>::max());
	textureHeightMinimumSizeSpin->setValue(512);
	textureHeightMinimumSizeSpin->setSuffix("px");
	textureHeightLayout->addRow(tr("Minimum"), textureHeightMinimumSizeSpin);
	textureHeightLayout->setRowVisible(3, false);

	auto* textureHeightMaximumSizeSpin = new QMareSpinBox{textureHeightGroup};
	textureHeightMaximumSizeSpin->setRange(1, std::numeric_limits<uint16_t>::max());
	textureHeightMaximumSizeSpin->setValue(2048);
	textureHeightMaximumSizeSpin->setSuffix("px");
	textureHeightExactSizeSpin->setVisible(false);
	textureHeightLayout->addRow(tr("Maximum"), textureHeightMaximumSizeSpin);
	textureHeightLayout->setRowVisible(4, false);

	textureTabLayout->addRow(tr("Height"), textureHeightGroup);

	// Mipmaps
	auto* textureMipmapsGenerateCheck = new QCheckBox{tr("Mipmaps"), textureTab};
	textureMipmapsGenerateCheck->setLayoutDirection(Qt::RightToLeft);
	textureMipmapsGenerateCheck->setCheckState(Qt::Checked);

	auto* textureMipmapsGroup = new QGroupBox{textureTab};
	auto* textureMipmapsLayout = new QFormLayout{textureMipmapsGroup};
	textureMipmapsLayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureMipmapsFilterCombo = new QMareComboBox{textureMipmapsGroup};
	for (const auto& [filter, filterName] : not_magic_enum::enum_entries<vtfpp::ImageConversion::ResizeFilter>(true)) {
		textureMipmapsFilterCombo->addItem(filterName.data(), static_cast<int>(filter));
	}
	textureMipmapsFilterCombo->setCurrentIndex(8); // NICE
	textureMipmapsLayout->addRow(tr("Filter"), textureMipmapsFilterCombo);

	auto* textureMipmapsScaleSpin = new QMareSpinBox{textureMipmapsGroup};
	textureMipmapsScaleSpin->setMinimum(0);
	textureMipmapsLayout->addRow(tr("Scale (Console)"), textureMipmapsScaleSpin);

	textureTabLayout->addRow(textureMipmapsGenerateCheck, textureMipmapsGroup);

	// Animated frames
	auto* textureAnimatedFramesCheck = new QCheckBox{textureTab};
	textureTabLayout->addRow(tr("Animate Numbered Sequences"), textureAnimatedFramesCheck);

	// HDRI
	auto* textureHDRIGroup = new QGroupBox{textureTab};
	auto* textureHDRILayout = new QFormLayout{textureHDRIGroup};
	textureHDRILayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureHDRIConversionMethodCombo = new QMareComboBox{textureHDRIGroup};
	for (const auto& [hdriMode, hdriModeName] : not_magic_enum::enum_entries<maretf::HDRIMode>(true)) {
		textureHDRIConversionMethodCombo->addItem(hdriModeName.data(), static_cast<int>(hdriMode));
	}
	textureHDRILayout->addRow(tr("Convert to"), textureHDRIConversionMethodCombo);

	auto* textureHDRIFilterCheck = new QCheckBox{textureHDRIGroup};
	textureHDRIFilterCheck->setCheckState(Qt::Checked);
	textureHDRILayout->addRow(tr("Bilinear Filter"), textureHDRIFilterCheck);

	textureTabLayout->addRow(tr("HDRI"), textureHDRIGroup);

	// Gamma correction
	auto* textureGammaCorrectionEnableCheck = new QCheckBox{tr("Gamma Correction"), textureTab};
	textureGammaCorrectionEnableCheck->setLayoutDirection(Qt::RightToLeft);

	auto* textureGammaCorrectionAmountSpin = new QMareDoubleSpinBox{textureTab};
	textureGammaCorrectionAmountSpin->setMinimum(0.0);
	textureGammaCorrectionAmountSpin->setValue(1.0);
	textureGammaCorrectionAmountSpin->setSingleStep(0.05);

	textureTabLayout->addRow(textureGammaCorrectionEnableCheck, textureGammaCorrectionAmountSpin);

	// Distance mapping
	auto* textureUseDistanceMapping = new QCheckBox{textureTab};
	textureTabLayout->addRow(tr("Distance Mapping"), textureUseDistanceMapping);

	// Premultiplied alpha
	auto* texturePremultipliedAlpha = new QCheckBox{textureTab};
	textureTabLayout->addRow(tr("Treat Alpha As Mask (Premultiplied)"), texturePremultipliedAlpha);

	// Invert green channel
	auto* textureInvertGreenCheck = new QCheckBox{textureTab};
	textureTabLayout->addRow(tr("Invert Green Channel"), textureInvertGreenCheck);

	// Bumpmap scale
	auto* textureBumpmapScaleSpin = new QMareDoubleSpinBox{textureTab};
	textureBumpmapScaleSpin->setValue(1.0);
	textureBumpmapScaleSpin->setSingleStep(0.05);
	textureTabLayout->addRow(tr("Bumpmap Scale"), textureBumpmapScaleSpin);

	// CPU compression
	auto* textureCompressionGroup = new QGroupBox{textureTab};
	auto* textureCompressionLayout = new QFormLayout{textureCompressionGroup};
	textureCompressionLayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureCompressionMethodCombo = new QMareComboBox{textureCompressionGroup};
	textureCompressionMethodCombo->addItem(tr("None"), 0);
	for (const auto& [compressionMethod, compressionMethodName] : not_magic_enum::enum_entries<vtfpp::CompressionMethod>(true)) {
		textureCompressionMethodCombo->addItem(compressionMethodName.data(), static_cast<int>(compressionMethod));
	}
	textureCompressionMethodCombo->setCurrentIndex(2); // Zstd
	textureCompressionLayout->addRow(tr("Method"), textureCompressionMethodCombo);

	auto* textureCompressionLevelEnableCheck = new QCheckBox{tr("Level"), textureCompressionGroup};
	textureCompressionLevelEnableCheck->setLayoutDirection(Qt::RightToLeft);

	auto* textureCompressionLevelSpin = new QMareSpinBox{textureCompressionGroup};
	textureCompressionLevelSpin->setRange(0, 100);
	textureCompressionLevelSpin->setSingleStep(4);
	textureCompressionLevelSpin->setSuffix("%");
	textureCompressionLevelSpin->setValue(100);
	textureCompressionLevelSpin->setDisabled(true);

	textureCompressionLayout->addRow(textureCompressionLevelEnableCheck, textureCompressionLevelSpin);

	textureTabLayout->addRow(tr("CPU Compression"), textureCompressionGroup);

	// Distance mapping tab
	auto* distanceTabScroll = new QScrollArea{tabs};
	auto* distanceTab = new QWidget{distanceTabScroll};
	distanceTabScroll->setWidgetResizable(true);
	distanceTabScroll->setWidget(distanceTab);
	auto* distanceTabLayout = new QFormLayout{distanceTab};
	distanceTabLayout->setFormAlignment(Qt::AlignHCenter);
	tabs->addTab(distanceTabScroll, tr("Distance Mapping"));

	auto* distanceReduceGroup = new QGroupBox{distanceTab};
	auto* distanceReduceLayout = new QFormLayout{distanceReduceGroup};
	distanceReduceLayout->setFormAlignment(Qt::AlignHCenter);

	auto* distanceReduceAxesDifferCheck = new QCheckBox{distanceReduceGroup};
	distanceReduceLayout->addRow(tr("Axes Differ"), distanceReduceAxesDifferCheck);

	auto* distanceReduceFactorSpin = new QMareSpinBox{distanceReduceGroup};
	distanceReduceFactorSpin->setValue(4);
	distanceReduceLayout->addRow(tr("Factor"), distanceReduceFactorSpin);

	auto* distanceReduceFactorXSpin = new QMareSpinBox{distanceReduceGroup};
	distanceReduceFactorXSpin->setValue(4);
	distanceReduceLayout->addRow(tr("Factor (X)"), distanceReduceFactorXSpin);

	auto* distanceReduceFactorYSpin = new QMareSpinBox{distanceReduceGroup};
	distanceReduceFactorYSpin->setValue(4);
	distanceReduceLayout->addRow(tr("Factor (Y)"), distanceReduceFactorYSpin);

	auto* distanceReduceEdgeCombo = new QMareComboBox{distanceReduceGroup};
	for (const auto& [edge, edgeName] : not_magic_enum::enum_entries<vtfpp::ImageConversion::ResizeEdge>(true)) {
		distanceReduceEdgeCombo->addItem(edgeName.data(), static_cast<int>(edge));
	}
	distanceReduceEdgeCombo->setCurrentIndex(0); // CLAMP
	distanceReduceLayout->addRow(tr("Edge"), distanceReduceEdgeCombo);

	distanceTabLayout->addRow(tr("Reduce"), distanceReduceGroup);

	auto* distanceNoValveQuirksCheck = new QCheckBox{distanceTab};
	distanceTabLayout->addRow(tr("No Valve Quirks"), distanceNoValveQuirksCheck);

	auto* distanceGradientDitherCheck = new QCheckBox{distanceTab};
	distanceTabLayout->addRow(tr("(Experimental) Gradient-Aligned Dither Filter"), distanceGradientDitherCheck);

	auto* distanceSpreadSpin = new QMareDoubleSpinBox{distanceTab};
	distanceSpreadSpin->setValue(1.0);
	distanceSpreadSpin->setSingleStep(0.1);
	distanceTabLayout->addRow(tr("Search Radius"), distanceSpreadSpin);

	auto* distanceAlphaThresholdSpin = new QMareDoubleSpinBox{distanceTab};
	distanceAlphaThresholdSpin->setRange(0.0, 100.0);
	distanceAlphaThresholdSpin->setSuffix("%");
	distanceAlphaThresholdSpin->setValue(4.0);
	distanceTabLayout->addRow(tr("Alpha Threshold"), distanceAlphaThresholdSpin);

	auto* distanceAntialiasingCheck = new QCheckBox{distanceTab};
	distanceTabLayout->addRow(tr("Antialiased Alpha"), distanceAntialiasingCheck);

	auto* distanceEuclideanCheck = new QCheckBox{distanceTab};
	distanceTabLayout->addRow(tr("Euclidean"), distanceEuclideanCheck);

	auto* distanceSampleCenteredCheck = new QCheckBox{distanceTab};
	distanceTabLayout->addRow(tr("Sample Centered"), distanceSampleCenteredCheck);

	// Flags tab
	auto* flagsChecks = new QMareFlagsWidget{tabs};
	tabs->addTab(flagsChecks, tr("Flags"));

	// Resources tab
	auto* resourcesTabScroll = new QScrollArea{tabs};
	auto* resourcesTab = new QWidget{resourcesTabScroll};
	resourcesTabScroll->setWidgetResizable(true);
	resourcesTabScroll->setWidget(resourcesTab);
	auto* resourcesTabLayout = new QFormLayout{resourcesTab};
	resourcesTabLayout->setFormAlignment(Qt::AlignHCenter);
	tabs->addTab(resourcesTabScroll, tr("Resources"));

	// Thumbnail resource
	auto* resourcesGenerateThumbnailCheck = new QCheckBox{resourcesTab};
	resourcesGenerateThumbnailCheck->setChecked(true);
	resourcesGenerateThumbnailCheck->setToolTip(tr("Used to recolor impact particle SFX."));
	resourcesTabLayout->addRow(tr("Compute Thumbnail"), resourcesGenerateThumbnailCheck);

	// Author resource
	auto* resourcesATHInfo = new QLineEdit{resourcesTab};
	resourcesATHInfo->setMinimumWidth(200);
	resourcesATHInfo->setToolTip("Nonstandard resource used to specify the author of a given texture.");
	resourcesTabLayout->addRow(tr("Author"), resourcesATHInfo);

	// LOD control info resource
	auto* resourcesLODEnableCheck = new QCheckBox{tr("LOD Control Info"), resourcesTab};
	resourcesLODEnableCheck->setLayoutDirection(Qt::RightToLeft);

	auto* resourcesLODGroup = new QGroupBox{resourcesTab};
	auto* resourcesLODLayout = new QFormLayout{resourcesLODGroup};
	resourcesLODLayout->setFormAlignment(Qt::AlignHCenter);

	auto* resourcesLODUSpin = new QMareSpinBox{resourcesLODGroup};
	resourcesLODUSpin->setRange(0, 31);
	resourcesLODLayout->addRow(tr("U"), resourcesLODUSpin);

	auto* resourcesLODVSpin = new QMareSpinBox{resourcesLODGroup};
	resourcesLODVSpin->setRange(0, 31);
	resourcesLODLayout->addRow(tr("V"), resourcesLODVSpin);

	auto* resourcesLODUConsoleSpin = new QMareSpinBox{resourcesLODGroup};
	resourcesLODUConsoleSpin->setRange(0, 31);
	resourcesLODLayout->addRow(tr("U (Console)"), resourcesLODUConsoleSpin);

	auto* resourcesLODVConsoleSpin = new QMareSpinBox{resourcesLODGroup};
	resourcesLODVConsoleSpin->setRange(0, 31);
	resourcesLODLayout->addRow(tr("V (Console)"), resourcesLODVConsoleSpin);

	resourcesTabLayout->addRow(resourcesLODEnableCheck, resourcesLODGroup);

	// CRC resource
	auto* resourcesCRCValue = new QLineEdit{resourcesTab};
	resourcesCRCValue->setInputMask("HHHHHHHH");
	resourcesTabLayout->addRow(tr("CRC32"), resourcesCRCValue);

	// SHT resource
	auto* resourcesSHTPathParent = new QWidget{resourcesTab};
	auto* resourcesSHTPathLayout = new QHBoxLayout{resourcesSHTPathParent};
	resourcesSHTPathLayout->setSpacing(4);
	resourcesSHTPathLayout->setContentsMargins(0, 0, 0, 0);

	auto* resourcesSHTPath = new QLineEdit{resourcesSHTPathParent};
	resourcesSHTPath->setMinimumWidth(200);
	resourcesSHTPathLayout->addWidget(resourcesSHTPath);

	auto* resourcesSHTPathSearch = new QPushButton{resourcesSHTPathParent};
	resourcesSHTPathSearch->setIcon(this->style()->standardIcon(QStyle::SP_DirOpenIcon));
	resourcesSHTPathLayout->addWidget(resourcesSHTPathSearch);

	resourcesTabLayout->addRow(tr("Particle Sheet Path"), resourcesSHTPathParent);

	// Extended flags resource
	auto* resourcesTS0Value = new QLineEdit{resourcesTab};
	resourcesTS0Value->setInputMask("HHHHHHHH");
	resourcesTabLayout->addRow(tr("Extended Flags"), resourcesTS0Value);

	// KeyValues Data resource
	auto* resourcesKVDPathParent = new QWidget{resourcesTab};
	auto* resourcesKVDPathLayout = new QHBoxLayout{resourcesKVDPathParent};
	resourcesKVDPathLayout->setSpacing(4);
	resourcesKVDPathLayout->setContentsMargins(0, 0, 0, 0);

	auto* resourcesKVDPath = new QLineEdit{resourcesKVDPathParent};
	resourcesKVDPath->setMinimumWidth(200);
	resourcesKVDPathLayout->addWidget(resourcesKVDPath);

	auto* resourcesKVDPathSearch = new QPushButton{resourcesKVDPathParent};
	resourcesKVDPathSearch->setIcon(this->style()->standardIcon(QStyle::SP_DirOpenIcon));
	resourcesKVDPathLayout->addWidget(resourcesKVDPathSearch);

	resourcesTabLayout->addRow(tr("KeyValues Data Path"), resourcesKVDPathParent);

	/* ---------------------------- FILESYSTEM BEGIN ---------------------------- */

	auto* filesystemGroup = new QMareFilesystemBox{inputPaths, createFromDir, true, this};

	/* ----------------------------- FILESYSTEM END ----------------------------- */

	layout->addWidget(filesystemGroup);

	/* ----------------------------- BUTTONS BEGIN ------------------------------ */

	auto* dialogButtons = new QDialogButtonBox{this};

	auto* dialogButtonsCopyCommand = dialogButtons->addButton(tr("Copy Command"), QDialogButtonBox::HelpRole);
	dialogButtonsCopyCommand->setIcon(this->style()->standardIcon(QStyle::SP_DialogSaveButton)); // needs to match change on click

	dialogButtons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	/* ------------------------------ BUTTONS END ------------------------------- */

	layout->addWidget(dialogButtons, Qt::AlignBottom | Qt::AlignRight);

	/* ------------------------------ LOGIC BEGIN ------------------------------- */

	// Disable "Compressed Quality" when format is uncompressed

	connect(textureOpaqueFormatCombo, &QMareComboBox::currentIndexChanged, this, [=](int index) {
		const auto format = static_cast<vtfpp::ImageFormat>(textureOpaqueFormatCombo->itemData(index).toInt());
		textureCompressionQualitySpin->setEnabled(format == vtfpp::VTF::FORMAT_UNCHANGED || format == vtfpp::VTF::FORMAT_DEFAULT || vtfpp::ImageFormatDetails::compressed(format));
	});

	// Disable "Amount" spin when Compression Quality is disabled

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(textureCompressionQualityEnableCheck, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state) {
		textureCompressionQualitySpin->setDisabled(state != Qt::Checked);
	});
	textureCompressionQualityEnableCheck->checkStateChanged(textureCompressionQualityEnableCheck->checkState());
#else
	connect(textureCompressionQualityEnableCheck, &QCheckBox::stateChanged, this, [=](bool state) {
		textureCompressionQualitySpin->setDisabled(!state);
	});
	textureCompressionQualityEnableCheck->stateChanged(textureCompressionQualityEnableCheck->isChecked());
#endif

	// Change visibility of items in "Width" depending on mode

	connect(textureWidthClampModeCombo, &QMareComboBox::currentIndexChanged, this, [=](int index) {
		switch (index) {
			case 0:
				textureWidthLayout->setRowVisible(2, false);
				textureWidthLayout->setRowVisible(3, false);
				textureWidthLayout->setRowVisible(4, false);
				break;
			case 1:
				textureWidthLayout->setRowVisible(2, true);
				textureWidthLayout->setRowVisible(3, false);
				textureWidthLayout->setRowVisible(4, false);
				break;
			case 2:
				textureWidthLayout->setRowVisible(2, false);
				textureWidthLayout->setRowVisible(3, true);
				textureWidthLayout->setRowVisible(4, true);
				break;
			default:
				break;
		}
	});

	// Change visibility of items in "Height" depending on mode

	connect(textureHeightClampModeCombo, &QMareComboBox::currentIndexChanged, this, [=](int index) {
		switch (index) {
			case 0:
				textureHeightLayout->setRowVisible(2, false);
				textureHeightLayout->setRowVisible(3, false);
				textureHeightLayout->setRowVisible(4, false);
				break;
			case 1:
				textureHeightLayout->setRowVisible(2, true);
				textureHeightLayout->setRowVisible(3, false);
				textureHeightLayout->setRowVisible(4, false);
				break;
			case 2:
				textureHeightLayout->setRowVisible(2, false);
				textureHeightLayout->setRowVisible(3, true);
				textureHeightLayout->setRowVisible(4, true);
				break;
			default:
				break;
		}
	});

	// Disable "Filter" and "Scale (Console)" when mipmaps aren't computed, or "Scale (Console)" when platform is not PC

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(textureMipmapsGenerateCheck, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state) {
		textureMipmapsFilterCombo->setDisabled(state != Qt::Checked);
		textureMipmapsLayout->setRowVisible(textureMipmapsScaleSpin, state == Qt::Checked && static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt()) != vtfpp::VTF::PLATFORM_PC);
	});
	textureMipmapsGenerateCheck->checkStateChanged(textureMipmapsGenerateCheck->checkState());
#else
	connect(textureMipmapsGenerateCheck, &QCheckBox::stateChanged, this, [textureMipmapsFilterCombo, textureMipmapsLayout, textureMipmapsScaleSpin, platformCombo](bool state) {
		textureMipmapsFilterCombo->setDisabled(!state);
		textureMipmapsLayout->setRowVisible(textureMipmapsScaleSpin, state == Qt::Checked && static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt()) != vtfpp::VTF::PLATFORM_PC);
	});
	textureMipmapsGenerateCheck->stateChanged(textureMipmapsGenerateCheck->isChecked());
#endif

	// Disable "Bilinear Filter" check when HDRI mode is flat image

	connect(textureHDRIConversionMethodCombo, &QMareComboBox::currentIndexChanged, this, [=](int index) {
		textureHDRILayout->setRowVisible(textureHDRIFilterCheck, index != 0);
	});
	textureHDRIConversionMethodCombo->currentIndexChanged(textureHDRIConversionMethodCombo->currentIndex());

	// Disable "Amount" spin when Gamma Correction is disabled

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(textureGammaCorrectionEnableCheck, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state) {
		textureGammaCorrectionAmountSpin->setDisabled(state != Qt::Checked);
	});
	textureGammaCorrectionEnableCheck->checkStateChanged(textureGammaCorrectionEnableCheck->checkState());
#else
	connect(textureGammaCorrectionEnableCheck, &QCheckBox::stateChanged, this, [=](bool state) {
		textureGammaCorrectionAmountSpin->setDisabled(!state);
	});
	textureGammaCorrectionEnableCheck->stateChanged(textureGammaCorrectionEnableCheck->isChecked());
#endif

	// Enable "Distance Mapping" tab when Distance Mapping is checked

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(textureUseDistanceMapping, &QCheckBox::checkStateChanged, this, [tabs](Qt::CheckState state) {
		tabs->setTabVisible(1, state == Qt::Checked);
	});
	textureUseDistanceMapping->checkStateChanged(textureUseDistanceMapping->checkState());
#else
	connect(textureUseDistanceMapping, &QCheckBox::stateChanged, this, [tabs](bool state) {
		tabs->setTabVisible(1, state);
	});
	textureUseDistanceMapping->stateChanged(textureUseDistanceMapping->isChecked());
#endif

	// Disable "Version" combo when platform is not PC, and set it to the appropriate version so the flags list is correct
	// "CPU Compression" group has many restrictions from "Version" group:
	// - Disable group entirely if platform is PC and version is not 7.6, or if platform is XBOX, or if platform is PS3_ORANGEBOX
	// - Disable Deflate and Zstd options in "Method" combo when platform is PS3_PORTAL2 or X360, and disable LZMA (Console) option in "Method" combo when platform is PC
	// Disable "Scale (Console)" in "Mipmaps" group if mipmaps are not enabled or platform is not PC
	// Disable "U (Console)" and "V (Console)" in "LOD" group if platform is PC
	// Disable "Particle Sheet", "CRC32", "LOD", "Extended Flags", "KeyValues Data" groups if platform is PC and version is less than 3, or platform is XBOX
	// Repopulate "Flags" group checklist

	connect(platformCombo, &QMareComboBox::currentIndexChanged, this, [=](int index) {
		const auto currentPlatform = static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(index).toInt());

		versionCombo->setDisabled(currentPlatform != vtfpp::VTF::PLATFORM_PC);
		switch (currentPlatform) {
			case vtfpp::VTF::PLATFORM_UNKNOWN:
			case vtfpp::VTF::PLATFORM_PC:
				break;
			case vtfpp::VTF::PLATFORM_XBOX:
				versionCombo->setCurrentIndex(2);
				break;
			case vtfpp::VTF::PLATFORM_X360:
			case vtfpp::VTF::PLATFORM_PS3_ORANGEBOX:
				versionCombo->setCurrentIndex(4);
				break;
			case vtfpp::VTF::PLATFORM_PS3_PORTAL2:
				versionCombo->setCurrentIndex(5);
				break;
		}

		textureTabLayout->setRowVisible(textureCompressionGroup, (currentPlatform == vtfpp::VTF::PLATFORM_PC && versionCombo->currentIndex() == 6) || currentPlatform == vtfpp::VTF::PLATFORM_X360 || currentPlatform == vtfpp::VTF::PLATFORM_PS3_PORTAL2);

		const auto* textureCompressionMethodModel = qobject_cast<QStandardItemModel*>(textureCompressionMethodCombo->model());
		if (currentPlatform == vtfpp::VTF::PLATFORM_X360 || currentPlatform == vtfpp::VTF::PLATFORM_PS3_PORTAL2) {
			textureCompressionMethodModel->item(1)->setFlags(textureCompressionMethodModel->item(1)->flags() & ~Qt::ItemIsEnabled); // Deflate
			textureCompressionMethodModel->item(2)->setFlags(textureCompressionMethodModel->item(2)->flags() & ~Qt::ItemIsEnabled); // Zstd
			textureCompressionMethodModel->item(3)->setFlags(textureCompressionMethodModel->item(3)->flags() |  Qt::ItemIsEnabled); // LZMA (Console)
			if (textureCompressionMethodCombo->currentIndex() != 0) { // None
				textureCompressionMethodCombo->setCurrentIndex(3); // LZMA (Console)
			}
		} else {
			textureCompressionMethodModel->item(1)->setFlags(textureCompressionMethodModel->item(1)->flags() |  Qt::ItemIsEnabled); // Deflate
			textureCompressionMethodModel->item(2)->setFlags(textureCompressionMethodModel->item(2)->flags() |  Qt::ItemIsEnabled); // Zstd
			textureCompressionMethodModel->item(3)->setFlags(textureCompressionMethodModel->item(3)->flags() & ~Qt::ItemIsEnabled); // LZMA (Console)
			if (textureCompressionMethodCombo->currentIndex() == 3) { // LZMA (Console)
				textureCompressionMethodCombo->setCurrentIndex(2); // Zstd
			}
		}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
		textureMipmapsLayout->setRowVisible(textureMipmapsScaleSpin, textureMipmapsGenerateCheck->checkState() == Qt::Checked && currentPlatform != vtfpp::VTF::PLATFORM_PC);
#else
		textureMipmapsLayout->setRowVisible(textureMipmapsScaleSpin, textureMipmapsGenerateCheck->isChecked() && currentPlatform != vtfpp::VTF::PLATFORM_PC);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
		resourcesLODUConsoleSpin->setDisabled(resourcesLODEnableCheck->checkState() != Qt::Checked || currentPlatform == vtfpp::VTF::PLATFORM_PC);
		resourcesLODVConsoleSpin->setDisabled(resourcesLODEnableCheck->checkState() != Qt::Checked || currentPlatform == vtfpp::VTF::PLATFORM_PC);
#else
		resourcesLODUConsoleSpin->setDisabled(!resourcesLODEnableCheck->isChecked() || currentPlatform == vtfpp::VTF::PLATFORM_PC);
		resourcesLODVConsoleSpin->setDisabled(!resourcesLODEnableCheck->isChecked() || currentPlatform == vtfpp::VTF::PLATFORM_PC);
#endif

		const bool disableResources = (currentPlatform == vtfpp::VTF::PLATFORM_PC && versionCombo->currentIndex() < 3) || currentPlatform == vtfpp::VTF::PLATFORM_XBOX;
		resourcesATHInfo->setDisabled(disableResources);
		resourcesLODGroup->setDisabled(disableResources);
		resourcesCRCValue->setDisabled(disableResources);
		resourcesSHTPathParent->setDisabled(disableResources);
		resourcesTS0Value->setDisabled(disableResources);
		resourcesKVDPathParent->setDisabled(disableResources);

		flagsChecks->repopulateFlagList(0, currentPlatform, versionCombo->currentIndex());
	});
	platformCombo->currentIndexChanged(platformCombo->currentIndex());

	connect(versionCombo, &QMareComboBox::currentIndexChanged, this, [=](int index) {
		const auto currentPlatform = static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt());

		textureTabLayout->setRowVisible(textureCompressionGroup, index == 6);

		const bool disableResources = (currentPlatform == vtfpp::VTF::PLATFORM_PC && index < 3) || currentPlatform == vtfpp::VTF::PLATFORM_XBOX;
		resourcesATHInfo->setDisabled(disableResources);
		resourcesLODGroup->setDisabled(disableResources);
		resourcesCRCValue->setDisabled(disableResources);
		resourcesSHTPathParent->setDisabled(disableResources);
		resourcesTS0Value->setDisabled(disableResources);
		resourcesKVDPathParent->setDisabled(disableResources);

		flagsChecks->repopulateFlagList(0, currentPlatform, index);
	});
	versionCombo->currentIndexChanged(versionCombo->currentIndex());

	// Disable "Level" spin when Compression Level is disabled

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(textureCompressionLevelEnableCheck, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state) {
		textureCompressionLevelSpin->setDisabled(state != Qt::Checked);
	});
	textureCompressionLevelEnableCheck->checkStateChanged(textureCompressionLevelEnableCheck->checkState());
#else
	connect(textureCompressionLevelEnableCheck, &QCheckBox::stateChanged, this, [=](bool state) {
		textureCompressionLevelSpin->setDisabled(!state);
	});
	textureCompressionLevelEnableCheck->stateChanged(textureCompressionLevelEnableCheck->isChecked());
#endif

	// Enable individual x, y factor spins when Axes Differ is checked

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(distanceReduceAxesDifferCheck, &QCheckBox::checkStateChanged, this, [distanceReduceLayout, distanceReduceFactorSpin, distanceReduceFactorXSpin, distanceReduceFactorYSpin](Qt::CheckState state) {
		distanceReduceLayout->setRowVisible(distanceReduceFactorSpin, state != Qt::Checked);
		distanceReduceLayout->setRowVisible(distanceReduceFactorXSpin, state == Qt::Checked);
		distanceReduceLayout->setRowVisible(distanceReduceFactorYSpin, state == Qt::Checked);
	});
	distanceReduceAxesDifferCheck->checkStateChanged(distanceReduceAxesDifferCheck->checkState());
#else
	connect(distanceReduceAxesDifferCheck, &QCheckBox::stateChanged, this, [distanceReduceLayout, distanceReduceFactorSpin, distanceReduceFactorXSpin, distanceReduceFactorYSpin](bool state) {
		distanceReduceLayout->setRowVisible(distanceReduceFactorSpin, !state);
		distanceReduceLayout->setRowVisible(distanceReduceFactorXSpin, state);
		distanceReduceLayout->setRowVisible(distanceReduceFactorYSpin, state);
	});
	distanceReduceAxesDifferCheck->stateChanged(distanceReduceAxesDifferCheck->isChecked());
#endif

	// Set path in "Particle Sheet" group when "Search" clicked

	connect(resourcesSHTPathSearch, &QPushButton::pressed, this, [=, this] {
		if (const auto path = QFileDialog::getOpenFileName(this, tr("Open Particle Sheet"), QString{}, "Particle Sheet (*.sht)"); !path.isEmpty()) {
			resourcesSHTPath->setText(path);
		}
	});

	// Disable "U", "V", "U (Console)", "V (Console)" in "LOD" group if disabled
	// - Also disable "U (Console)" and "V (Console)" if platform is PC

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(resourcesLODEnableCheck, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state) {
		const auto currentPlatform = static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt());

		resourcesLODUSpin->setDisabled(state != Qt::Checked);
		resourcesLODVSpin->setDisabled(state != Qt::Checked);
		resourcesLODUConsoleSpin->setDisabled(state != Qt::Checked || currentPlatform == vtfpp::VTF::PLATFORM_PC);
		resourcesLODVConsoleSpin->setDisabled(state != Qt::Checked || currentPlatform == vtfpp::VTF::PLATFORM_PC);
	});
	resourcesLODEnableCheck->checkStateChanged(resourcesLODEnableCheck->checkState());
#else
	connect(resourcesLODEnableCheck, &QCheckBox::stateChanged, this, [=](bool state) {
		const auto currentPlatform = static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt());

		resourcesLODUSpin->setDisabled(!state);
		resourcesLODVSpin->setDisabled(!state);
		resourcesLODUConsoleSpin->setDisabled(!state || currentPlatform == vtfpp::VTF::PLATFORM_PC);
		resourcesLODVConsoleSpin->setDisabled(!state || currentPlatform == vtfpp::VTF::PLATFORM_PC);
	});
	resourcesLODEnableCheck->stateChanged(resourcesLODEnableCheck->isChecked());
#endif

	// Set path in "KeyValues Data" group when "Search" clicked

	connect(resourcesKVDPathSearch, &QPushButton::pressed, this, [=, this] {
		if (const auto path = QFileDialog::getOpenFileName(this, tr("Open KeyValues File"), QString{}, "KeyValues File (*.kv *.txt *.vdf)"); !path.isEmpty()) {
			resourcesKVDPath->setText(path);
		}
	});

	// Set input path in "Filesystem" group when "Input Search" clicked

	connect(filesystemGroup->filesystemInputPathSearch, &QPushButton::pressed, this, [=, this] {
		if (
			const auto paths = !createFromDir
				? QFileDialog::getOpenFileNames(this, tr("Open Images"), QString{}, ::supportedImageFileFormatsForLoad().data())
				: QStringList{QFileDialog::getExistingDirectory(this, tr("Open Folder"))};
			!paths.isEmpty() && !paths[0].isEmpty()
		) {
			filesystemGroup->filesystemInputPath->setText(QMareCLIWrapper::joinPaths(paths));
		}
	});

	// Set output path in "Filesystem" group when "Output Search" clicked

	connect(filesystemGroup->filesystemOutputPathSearch, &QPushButton::pressed, this, [this, &inputPaths, createFromDir, filesystemGroup] {
		if (
			const auto path = !createFromDir || inputPaths.size() > 1
				? QFileDialog::getSaveFileName(this, tr("Save Texture"), QFileInfo{filesystemGroup->filesystemInputPath->text()}.canonicalFilePath(), "Valve Texture Format (*.vtf *.xtf)", nullptr, QFileDialog::DontConfirmOverwrite)
				: QFileDialog::getExistingDirectory(this, tr("Save to Folder"));
			!path.isEmpty()
		) {
			filesystemGroup->filesystemOutputPath->setText(path);
		}
	});

	// Get list of arguments from the selected options

	const auto getCLI = [=, this] {
		auto* cli = new QMareCLIWrapper{"create", this};

		filesystemGroup->addArguments(*cli);

		if (static_cast<vtfpp::VTF::Platform>(platformCombo->currentData().toInt()) != vtfpp::VTF::PLATFORM_PC) {
			cli->addEnum<vtfpp::VTF::Platform>(platformCombo, "--platform");
		}

		cli->addArgPair("--version", std::format("7.{}", versionCombo->currentData().toInt()).data());

		cli->addEnum<vtfpp::ImageFormat>(textureOpaqueFormatCombo, "--format");

		if (textureCompressionQualityEnableCheck->isChecked()) {
			cli->addArgPair("--quality", std::format("{}", static_cast<float>(textureCompressionQualitySpin->value()) / 100.f).data());
		}

		if (textureWidthResizeMethodCombo->currentIndex() == textureHeightResizeMethodCombo->currentIndex()) {
			cli->addEnum<vtfpp::ImageConversion::ResizeMethod>(textureWidthResizeMethodCombo, "--resize-method");
		} else {
			cli->addEnum<vtfpp::ImageConversion::ResizeMethod>(textureWidthResizeMethodCombo, "--width-resize-method");
			cli->addEnum<vtfpp::ImageConversion::ResizeMethod>(textureHeightResizeMethodCombo, "--height-resize-method");
		}

		switch (textureWidthClampModeCombo->currentIndex()) {
			default:
			case 0:
				break;
			case 1:
				cli->addInt(textureWidthExactSizeSpin, "--width");
				break;
			case 2:
				cli->addInt(textureWidthMinimumSizeSpin, "--min-width");
				cli->addInt(textureWidthMaximumSizeSpin, "--max-width");
				break;
		}

		switch (textureHeightClampModeCombo->currentIndex()) {
			default:
			case 0:
				break;
			case 1:
				cli->addInt(textureHeightExactSizeSpin, "--height");
				break;
			case 2:
				cli->addInt(textureHeightMinimumSizeSpin, "--min-height");
				cli->addInt(textureHeightMaximumSizeSpin, "--max-height");
				break;
		}

		cli->addFlag(textureMipmapsGenerateCheck, "--no-mips", true);

		cli->addEnum<vtfpp::ImageConversion::ResizeFilter>(textureMipmapsFilterCombo, "--filter");

		if (static_cast<vtfpp::VTF::Platform>(platformCombo->currentData().toInt()) != vtfpp::VTF::PLATFORM_PC && textureMipmapsScaleSpin->value() != 0) {
			cli->addInt(textureMipmapsScaleSpin, "--console-mip-scale");
		}

		if (textureAnimatedFramesCheck->isChecked()) {
			cli->addFlag(textureAnimatedFramesCheck, "--animated-frames");
		}

		if (static_cast<maretf::HDRIMode>(textureHDRIConversionMethodCombo->currentData().toInt()) != maretf::HDRIMode::FLAT) {
			cli->addEnum<maretf::HDRIMode>(textureHDRIConversionMethodCombo, "--hdri-autodetect");
			cli->addFlag(textureHDRIFilterCheck, "--hdri-no-filter", true);
		}

		cli->addFlag(textureGammaCorrectionEnableCheck, "--gamma-correct");

		if (textureGammaCorrectionEnableCheck->isChecked()) {
			cli->addFloat(textureGammaCorrectionAmountSpin, "--gamma-correct-amount");
		}

		if (texturePremultipliedAlpha->isChecked()) {
			// Change this if we ever add more flags
			cli->addArgPair("--flags-extra-uint", std::format("{}", static_cast<uint32_t>(vtfpp::VTF::FLAG_EXTRA_USING_PREMULTIPLIED_ALPHA_RESIZE)).data());
		}

		cli->addFlag(textureUseDistanceMapping, "--alpha-to-distance");
		if (textureUseDistanceMapping->isChecked()) {
			if (distanceReduceAxesDifferCheck->isChecked()) {
				if (distanceReduceFactorXSpin->value() != 4) {
					cli->addInt(distanceReduceFactorXSpin, "--distance-reduce-x");
				}
				if (distanceReduceFactorYSpin->value() != 4) {
					cli->addInt(distanceReduceFactorYSpin, "--distance-reduce-y");
				}
			} else if (distanceReduceFactorSpin->value() != 4) {
				cli->addInt(distanceReduceFactorSpin, "--distance-reduce");
			}

			cli->addEnum<vtfpp::ImageConversion::ResizeEdge>(distanceReduceEdgeCombo, "--edge");

			cli->addFlag(distanceNoValveQuirksCheck, "--distance-no-valve-quirks");

			cli->addFlag(distanceGradientDitherCheck, "--distance-dither");

			cli->addFloat(distanceSpreadSpin, "--distance-spread");

			if (distanceAlphaThresholdSpin->value() != 4.0) {
				cli->addArgPair("--distance-alpha-threshold", std::format("{}", static_cast<float>(distanceAlphaThresholdSpin->value()) / 100.f).data());
			}

			cli->addFlag(distanceAntialiasingCheck, "--distance-aa");

			cli->addFlag(distanceEuclideanCheck, "--distance-euclidean");

			cli->addFlag(distanceSampleCenteredCheck, "--distance-sample-centered");
		}

		cli->addFlag(textureInvertGreenCheck, "--invert-green");

		if (textureBumpmapScaleSpin->value() != 1.0) {
			cli->addFloat(textureBumpmapScaleSpin, "--bumpscale");
		}

		if (textureCompressionGroup->isEnabled() && textureCompressionMethodCombo->currentIndex() != 0) {
			cli->addEnum<vtfpp::CompressionMethod>(textureCompressionMethodCombo, "--compression-method");

			if (textureCompressionLevelSpin->isEnabled()) {
				cli->addArgPair("--compression-level", std::format("{}", static_cast<float>(textureCompressionLevelSpin->value()) / 100.f).data());
			}
		}

		if (const auto flagsUInt =  flagsChecks->getFlags(); flagsUInt != 0) {
			cli->addArgPair("--flags-uint", std::format("{}", flagsUInt).data());
		}

		cli->addFlag(resourcesGenerateThumbnailCheck, "--no-thumbnail", true);

		if (resourcesATHInfo->isEnabled() && !resourcesATHInfo->text().isEmpty()) {
			cli->addArgPair("--ath-resource", resourcesATHInfo->text());
		}

		if (resourcesLODGroup->isEnabled() && resourcesLODEnableCheck->isChecked()) {
			if (resourcesLODUConsoleSpin->isEnabled() || resourcesLODVConsoleSpin->isEnabled()) {
				cli->addArgPair("--lod-resource", std::format("{}.{}.{}.{}", resourcesLODUSpin->value(), resourcesLODVSpin->value(), resourcesLODUConsoleSpin->value(), resourcesLODVConsoleSpin->value()).data());
			} else {
				cli->addArgPair("--lod-resource", std::format("{}.{}", resourcesLODUSpin->value(), resourcesLODVSpin->value()).data());
			}
		}

		if (resourcesCRCValue->isEnabled() && !resourcesCRCValue->text().isEmpty()) {
			uint32_t crc;
			sourcepp::string::toInt(resourcesCRCValue->text().toUtf8().constData(), crc, 16);
			cli->addArgPair("--crc-resource", std::format("{}", crc).data());
		}

		if (resourcesSHTPathParent->isEnabled() && !resourcesSHTPath->text().isEmpty()) {
			cli->addArgPair("--particle-sheet-resource", resourcesSHTPath->text());
		}

		if (resourcesTS0Value->isEnabled() && !resourcesTS0Value->text().isEmpty()) {
			uint32_t ts0;
			sourcepp::string::toInt(resourcesTS0Value->text().toUtf8().constData(), ts0, 16);
			cli->addArgPair("--ts0-resource", std::format("{}", ts0).data());
		}

		if (resourcesKVDPathParent->isEnabled() && !resourcesKVDPath->text().isEmpty()) {
			cli->addArgPair("--kvd-resource", resourcesKVDPath->text());
		}

		return cli;
	};

	// Copy the CLI command to clipboard

	connect(dialogButtonsCopyCommand, &QPushButton::clicked, this, [=, this] {
		dialogButtonsCopyCommand->setIcon(this->style()->standardIcon(QStyle::SP_DialogApplyButton));
		QTimer::singleShot(1000, this, [this, dialogButtonsCopyCommand] {
			dialogButtonsCopyCommand->setIcon(this->style()->standardIcon(QStyle::SP_DialogSaveButton));
		});

		auto* cli = getCLI();
		QApplication::clipboard()->setText(cli->getCommand());
		cli->deleteLater();
	});

	// On "OK", create the texture
	// On "Cancel", close the dialog

	connect(dialogButtons, &QDialogButtonBox::accepted, this, [=, this] {
		auto* cli = getCLI();
		if (const auto& [code, errMsg] = cli->exec(); code) {
			QMessageBox::warning(this, tr("Error Creating Texture"), errMsg.c_str());
			cli->deleteLater();
			return;
		}
		cli->deleteLater();

		if (!createFromDir) {
			QStringList outputPaths;
			if (!filesystemGroup->filesystemOutputPath->text().isEmpty()) {
				outputPaths.push_back(filesystemGroup->filesystemOutputPath->text());
			} else {
				for (const auto& inputPath : QMareCLIWrapper::splitPaths(filesystemGroup->filesystemInputPath->text())) {
					const auto platformEnum = static_cast<vtfpp::VTF::Platform>(platformCombo->currentData().toInt());
					if (static_cast<maretf::HDRIMode>(textureHDRIConversionMethodCombo->currentData().toInt()) == maretf::HDRIMode::SKYBOX) {
						for (const auto& skyboxOutputPath : ::getOutputSkyboxPathsForInput(inputPath.toUtf8().constData(), platformEnum)) {
							outputPaths.push_back(skyboxOutputPath.c_str());
						}
					} else {
						outputPaths.push_back(::getOutputPathForInput(inputPath.toUtf8().constData(), platformEnum).c_str());
					}
				}
			}
			emit this->createdTextures(outputPaths);
		}
		emit this->accept();
	});

	connect(dialogButtons, &QDialogButtonBox::rejected, this, &QMareCreateTextureDialog::reject);
}

QMareCreateTextureDialog* QMareCreateTextureDialog::fromImages(QWidget* parent, QStringList imagePaths) {
	if (imagePaths.isEmpty()) {
		imagePaths = QFileDialog::getOpenFileNames(parent, tr("Open Images"), QMareOptions::get<QString>(QMareOptions::STR_DEFAULT_CREATE_DIALOG_DIR), ::supportedImageFileFormatsForLoad().data());
		if (imagePaths.isEmpty()) {
			return nullptr;
		}
	}
	QMareOptions::set(QMareOptions::STR_DEFAULT_CREATE_DIALOG_DIR, QFileInfo{imagePaths.last()}.canonicalPath());
	return new QMareCreateTextureDialog{imagePaths, false, parent};
}

QMareCreateTextureDialog* QMareCreateTextureDialog::fromDir(QWidget* parent, QString dirPath) {
	if (dirPath.isEmpty()) {
		dirPath = QFileDialog::getExistingDirectory(parent, tr("Open Folder"), QMareOptions::get<QString>(QMareOptions::STR_DEFAULT_CREATE_DIALOG_DIR));
		if (dirPath.isEmpty()) {
			return nullptr;
		}
	}
	QMareOptions::set(QMareOptions::STR_DEFAULT_CREATE_DIALOG_DIR, QFileInfo{dirPath}.canonicalPath());
	return new QMareCreateTextureDialog{{dirPath}, true, parent};
}
