// ReSharper disable CppUseFamiliarTemplateSyntaxForGenericLambdas

#include "QMareCreateTextureDialog.h"

#include <format>
#include <memory>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStandardItemModel>

#include "QMareFlagsWidget.h"
#include "../cli/MareTF.h"
#include "../common/EnumMappings.h"
#include "src/common/Common.h"

QMareCreateTextureDialog::QMareCreateTextureDialog(bool createFromDir, QWidget* parent) : QDialog{parent} {
	this->setWindowTitle(createFromDir ? tr("Create Textures") : tr("Create Texture"));

	auto* layout = new QGridLayout{this};

	/* ----------------------------- VERSION BEGIN ------------------------------ */

	auto* versionGroup = new QGroupBox{tr("Version"), this};
	auto* versionLayout = new QFormLayout{versionGroup};
	versionLayout->setFormAlignment(Qt::AlignHCenter);

	auto* platformCombo = new QComboBox{versionGroup};
	for (const auto& [platform, platformName] : not_magic_enum::enum_entries<vtfpp::VTF::Platform>(true)) {
		platformCombo->addItem(platformName.data(), static_cast<int>(platform));
	}
	versionLayout->addRow(tr("Platform"), platformCombo);

	auto* versionCombo = new QComboBox{versionGroup};
	for (int i = 0; i <= 6; i++) {
		versionCombo->addItem(QString{"7.%1"}.arg(i), i);
	}
	versionCombo->setCurrentIndex(4); // 7.4
	versionLayout->addRow(tr("Version"), versionCombo);

	/* ------------------------------ VERSION END ------------------------------- */

	layout->addWidget(versionGroup, 0, 0);

	/* ----------------------------- TEXTURES BEGIN ----------------------------- */

	auto* textureGroup = new QGroupBox{tr("Texture"), this};
	auto* textureLayout = new QFormLayout{textureGroup};
	textureLayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureOpaqueFormatCombo = new QComboBox{textureGroup};
	//auto* textureTransparentFormatCombo = new QComboBox{textureGroup};
	for (const auto& [format, formatName] : not_magic_enum::enum_entries<vtfpp::ImageFormat>(true)) {
		textureOpaqueFormatCombo->addItem(formatName.data(), static_cast<int>(format));
		//textureTransparentFormatCombo->addItem(formatName.data(), static_cast<int>(format));
	}
	textureOpaqueFormatCombo->setCurrentIndex(1); // DEFAULT
	//textureTransparentFormatCombo->setCurrentIndex(1); // DEFAULT
	textureLayout->addRow(tr(/*"Opaque "*/ "Format"), textureOpaqueFormatCombo);
	//textureLayout->addRow(tr("Transparent Format"), textureTransparentFormatCombo);

	auto* textureCompressionQualitySpin = new QSpinBox{textureGroup};
	textureCompressionQualitySpin->setRange(0, 100);
	textureCompressionQualitySpin->setSingleStep(10);
	textureCompressionQualitySpin->setSuffix("%");
	textureCompressionQualitySpin->setValue(100);
	textureLayout->addRow(tr("Compression Quality"), textureCompressionQualitySpin);

	/* --------------------------------- Resize --------------------------------- */

	auto* textureResizeGroup = new QGroupBox{textureGroup};
	auto* textureResizeLayout = new QFormLayout{textureResizeGroup};
	textureResizeLayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureResizeMethodWidthCombo = new QComboBox{textureResizeGroup};
	auto* textureResizeMethodHeightCombo = new QComboBox{textureResizeGroup};
	for (const auto& [method, methodName] : not_magic_enum::enum_entries<vtfpp::ImageConversion::ResizeMethod>(true)) {
		textureResizeMethodWidthCombo->addItem(methodName.data(), static_cast<int>(method));
		textureResizeMethodHeightCombo->addItem(methodName.data(), static_cast<int>(method));
	}
	textureResizeMethodWidthCombo->setCurrentIndex(1); // Power of 2 (Bigger)
	textureResizeMethodHeightCombo->setCurrentIndex(1); // Power of 2 (Bigger)
	textureResizeLayout->addRow(tr("Width"), textureResizeMethodWidthCombo);
	textureResizeLayout->addRow(tr("Height"), textureResizeMethodHeightCombo);

	textureLayout->addRow(tr("Resize"), textureResizeGroup);

	/* -------------------------------- Mipmaps --------------------------------- */

	auto* textureMipmapsGroup = new QGroupBox{textureGroup};
	auto* textureMipmapsLayout = new QFormLayout{textureMipmapsGroup};
	textureMipmapsLayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureMipmapsGenerateCheck = new QCheckBox{textureMipmapsGroup};
	textureMipmapsGenerateCheck->setCheckState(Qt::Checked);
	textureMipmapsLayout->addRow(tr("Compute"), textureMipmapsGenerateCheck);

	auto* textureMipmapsFilterCombo = new QComboBox{textureMipmapsGroup};
	for (const auto& [filter, filterName] : not_magic_enum::enum_entries<vtfpp::ImageConversion::ResizeFilter>(true)) {
		textureMipmapsFilterCombo->addItem(filterName.data(), static_cast<int>(filter));
	}
	textureMipmapsFilterCombo->setCurrentIndex(7); // Kaiser
	textureMipmapsLayout->addRow(tr("Filter"), textureMipmapsFilterCombo);

	auto* textureMipmapsScaleSpin = new QSpinBox{textureMipmapsGroup};
	textureMipmapsScaleSpin->setMinimum(0);
	textureMipmapsLayout->addRow(tr("Scale (Console)"), textureMipmapsScaleSpin);

	textureLayout->addRow(tr("Mipmaps"), textureMipmapsGroup);

	/* ---------------------------------- HDRI ---------------------------------- */

	auto* textureHDRIGroup = new QGroupBox{textureGroup};
	auto* textureHDRILayout = new QFormLayout{textureHDRIGroup};
	textureHDRILayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureHDRIConversionMethodCombo = new QComboBox{textureHDRIGroup};
	textureHDRIConversionMethodCombo->addItem(tr("Flat Texture"));
	textureHDRIConversionMethodCombo->addItem(tr("Cubemap"));
	//textureHDRIConversionMethodCombo->addItem(tr("Skybox"));
	textureHDRILayout->addRow(tr("Convert to"), textureHDRIConversionMethodCombo);

	auto* textureHDRIFilterCheck = new QCheckBox{textureHDRIGroup};
	textureHDRIFilterCheck->setCheckState(Qt::Checked);
	textureHDRILayout->addRow(tr("Bilinear Filter"), textureHDRIFilterCheck);

	textureLayout->addRow(tr("HDRI"), textureHDRIGroup);

	/* ---------------------------- Gamme Correction ---------------------------- */

	auto* textureGammaCorrectionGroup = new QGroupBox{textureGroup};
	auto* textureGammaCorrectionLayout = new QFormLayout{textureGammaCorrectionGroup};
	textureGammaCorrectionLayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureGammaCorrectionEnableCheck = new QCheckBox{textureGammaCorrectionGroup};
	textureGammaCorrectionLayout->addRow(tr("Enabled"), textureGammaCorrectionEnableCheck);

	auto* textureGammaCorrectionAmountSpin = new QDoubleSpinBox{textureGammaCorrectionGroup};
	textureGammaCorrectionAmountSpin->setMinimum(0.0);
	textureGammaCorrectionAmountSpin->setValue(1.0);
	textureGammaCorrectionAmountSpin->setSingleStep(0.05);
	textureGammaCorrectionLayout->addRow(tr("Amount"), textureGammaCorrectionAmountSpin);

	textureLayout->addRow(tr("Gamma Correction"), textureGammaCorrectionGroup);

	/* ----------------------- Invert Green/Bumpmap Scale ----------------------- */

	auto* textureInvertGreenCheck = new QCheckBox{textureGroup};
	textureLayout->addRow(tr("Invert Green"), textureInvertGreenCheck);

	auto* textureBumpmapScaleSpin = new QDoubleSpinBox{textureGroup};
	textureBumpmapScaleSpin->setValue(1.0);
	textureBumpmapScaleSpin->setSingleStep(0.05);
	textureLayout->addRow(tr("Bumpmap Scale"), textureBumpmapScaleSpin);

	/* ---------------------------- CPU Compression ----------------------------- */

	auto* textureCompressionGroup = new QGroupBox{textureGroup};
	auto* textureCompressionLayout = new QFormLayout{textureCompressionGroup};
	textureCompressionLayout->setFormAlignment(Qt::AlignHCenter);

	auto* textureCompressionMethodCombo = new QComboBox{textureCompressionGroup};
	textureCompressionMethodCombo->addItem(tr("None"), 0);
	for (const auto& [compressionMethod, compressionMethodName] : not_magic_enum::enum_entries<vtfpp::CompressionMethod>(true)) {
		textureCompressionMethodCombo->addItem(compressionMethodName.data(), static_cast<int>(compressionMethod));
	}
	textureCompressionMethodCombo->setCurrentIndex(2); // Zstd
	textureCompressionLayout->addRow(tr("Method"), textureCompressionMethodCombo);

	auto* textureCompressionLevelSpin = new QSpinBox{textureCompressionGroup};
	textureCompressionLevelSpin->setRange(1, 22);
	textureCompressionLevelSpin->setValue(22);
	textureCompressionLayout->addRow(tr("Level"), textureCompressionLevelSpin);

	textureLayout->addRow(tr("CPU Compression"), textureCompressionGroup);

	/* ------------------------------ TEXTURES END ------------------------------ */

	layout->addWidget(textureGroup, 1, 0, 7, 1);

	/* ------------------------------ FLAGS BEGIN ------------------------------- */

	auto* flagsGroup = new QGroupBox{tr("Flags"), this};
	auto* flagsLayout = new QVBoxLayout{flagsGroup};

	auto* flagsChecks = new QMareFlagsWidget{flagsGroup};
	flagsLayout->addWidget(flagsChecks);


	/* ------------------------------- FLAGS END -------------------------------- */

	layout->addWidget(flagsGroup, 0, 1, 8, 1);

	/* ---------------------------- RESOURCES BEGIN ----------------------------- */

	auto* resourcesGroup = new QGroupBox{tr("Resources"), this};
	auto* resourcesLayout = new QFormLayout{resourcesGroup};
	resourcesLayout->setFormAlignment(Qt::AlignHCenter);

	/* ------------------------------- Thumbnail -------------------------------- */

	auto* resourcesGenerateThumbnailCheck = new QCheckBox{resourcesGroup};
	resourcesGenerateThumbnailCheck->setChecked(Qt::Checked);
	resourcesLayout->addRow(tr("Compute Thumbnail"), resourcesGenerateThumbnailCheck);

	/* ---------------------------------- SHT ----------------------------------- */

	auto* resourcesSHTGroup = new QGroupBox{resourcesGroup};
	auto* resourcesSHTLayout = new QFormLayout{resourcesSHTGroup};
	resourcesSHTLayout->setFormAlignment(Qt::AlignHCenter);

	auto* resourcesSHTEnableCheck = new QCheckBox{resourcesSHTGroup};
	resourcesSHTLayout->addRow(tr("Add Resource"), resourcesSHTEnableCheck);

	auto* resourcesSHTPathParent = new QWidget{resourcesSHTGroup};
	auto* resourcesSHTPathLayout = new QHBoxLayout{resourcesSHTPathParent};
	resourcesSHTPathLayout->setSpacing(4);
	resourcesSHTPathLayout->setContentsMargins(0, 0, 0, 0);

	auto* resourcesSHTPath = new QLineEdit{resourcesSHTGroup};
	resourcesSHTPath->setMinimumWidth(200);
	resourcesSHTPathLayout->addWidget(resourcesSHTPath);

	auto* resourcesSHTPathSearch = new QPushButton{resourcesSHTGroup};
	resourcesSHTPathSearch->setIcon(this->style()->standardIcon(QStyle::SP_DirOpenIcon));
	resourcesSHTPathLayout->addWidget(resourcesSHTPathSearch);

	resourcesSHTLayout->addRow(tr("Path"), resourcesSHTPathParent);

	resourcesLayout->addRow(tr("Particle Sheet"), resourcesSHTGroup);

	/* ---------------------------------- CRC ----------------------------------- */

	auto* resourcesCRCGroup = new QGroupBox{resourcesGroup};
	auto* resourcesCRCLayout = new QFormLayout{resourcesCRCGroup};
	resourcesCRCLayout->setFormAlignment(Qt::AlignHCenter);

	auto* resourcesCRCEnableCheck = new QCheckBox{resourcesCRCGroup};
	resourcesCRCLayout->addRow(tr("Add Resource"), resourcesCRCEnableCheck);

	auto* resourcesCRCValue = new QLineEdit{resourcesCRCGroup};
	resourcesCRCValue->setInputMask("HHHHHHHH");
	resourcesCRCLayout->addRow(tr("Value"), resourcesCRCValue);

	resourcesLayout->addRow(tr("CRC32"), resourcesCRCGroup);

	/* ---------------------------------- LOD ----------------------------------- */

	auto* resourcesLODGroup = new QGroupBox{resourcesGroup};
	auto* resourcesLODLayout = new QFormLayout{resourcesLODGroup};
	resourcesLODLayout->setFormAlignment(Qt::AlignHCenter);

	auto* resourcesLODEnableCheck = new QCheckBox{resourcesLODGroup};
	resourcesLODLayout->addRow(tr("Add Resource"), resourcesLODEnableCheck);

	auto* resourcesLODUSpin = new QSpinBox{resourcesLODGroup};
	resourcesLODUSpin->setRange(0, 31);
	resourcesLODLayout->addRow(tr("U"), resourcesLODUSpin);

	auto* resourcesLODVSpin = new QSpinBox{resourcesLODGroup};
	resourcesLODVSpin->setRange(0, 31);
	resourcesLODLayout->addRow(tr("V"), resourcesLODVSpin);

	auto* resourcesLODUConsoleSpin = new QSpinBox{resourcesLODGroup};
	resourcesLODUConsoleSpin->setRange(0, 31);
	resourcesLODLayout->addRow(tr("U (Console)"), resourcesLODUConsoleSpin);

	auto* resourcesLODVConsoleSpin = new QSpinBox{resourcesLODGroup};
	resourcesLODVConsoleSpin->setRange(0, 31);
	resourcesLODLayout->addRow(tr("V (Console)"), resourcesLODVConsoleSpin);

	resourcesLayout->addRow(tr("LOD Control Info"), resourcesLODGroup);

	/* ----------------------------- RESOURCES END ------------------------------ */

	layout->addWidget(resourcesGroup, 0, 2, 6, 1);

	/* ---------------------------- FILESYSTEM BEGIN ---------------------------- */

	auto* filesystemGroup = new QGroupBox{tr("Filesystem"), this};
	auto* filesystemLayout = new QFormLayout{filesystemGroup};
	filesystemLayout->setFormAlignment(Qt::AlignHCenter);

	/* ------------------------------  Output Path ------------------------------ */

	auto* filesystemOutputPathParent = new QWidget{filesystemGroup};
	auto* filesystemOutputPathLayout = new QHBoxLayout{filesystemOutputPathParent};
	filesystemOutputPathLayout->setSpacing(4);
	filesystemOutputPathLayout->setContentsMargins(0, 0, 0, 0);

	auto* filesystemOutputPath = new QLineEdit{filesystemGroup};
	filesystemOutputPath->setMinimumWidth(200);
	filesystemOutputPathLayout->addWidget(filesystemOutputPath);

	auto* filesystemOutputPathSearch = new QPushButton{filesystemGroup};
	filesystemOutputPathSearch->setIcon(this->style()->standardIcon(QStyle::SP_DirOpenIcon));
	filesystemOutputPathLayout->addWidget(filesystemOutputPathSearch);

	filesystemLayout->addRow(tr("Output Path"), filesystemOutputPathParent);

	/* ------------------------------- Overwrite -------------------------------- */

	auto* overwriteGroup = new QGroupBox{filesystemGroup};
	auto* overwriteLayout = new QHBoxLayout{overwriteGroup};
	overwriteLayout->setSpacing(24);
	overwriteLayout->setAlignment(Qt::AlignHCenter);

	auto* overwriteRadioGroup = new QButtonGroup{overwriteGroup};
	overwriteRadioGroup->setExclusive(true);

	auto* overwriteRadioYes = new QRadioButton{tr("Yes"), overwriteGroup};
	overwriteRadioGroup->addButton(overwriteRadioYes);
	overwriteLayout->addWidget(overwriteRadioYes);

	auto* overwriteRadioAsk = new QRadioButton{tr("Ask"), overwriteGroup};
	overwriteRadioGroup->addButton(overwriteRadioAsk);
	overwriteLayout->addWidget(overwriteRadioAsk);

	auto* overwriteRadioNo = new QRadioButton{tr("No"), overwriteGroup};
	overwriteRadioGroup->addButton(overwriteRadioNo);
	overwriteLayout->addWidget(overwriteRadioNo);

	overwriteRadioAsk->setChecked(Qt::Checked);

	filesystemLayout->addRow(tr("Overwrite"), overwriteGroup);

	/* ----------------------------- Recurse/Watch ------------------------------ */

	auto* recurseIntoSubdirsCheck = new QCheckBox{filesystemGroup};
	recurseIntoSubdirsCheck->setChecked(createFromDir);
	recurseIntoSubdirsCheck->setDisabled(!createFromDir);
	filesystemLayout->addRow(tr("Enter Subfolders"), recurseIntoSubdirsCheck);

	auto* watchFilesCheck = new QCheckBox{filesystemGroup};
	filesystemLayout->addRow(tr("Watch For Changes"), watchFilesCheck);

	/* ----------------------------- FILESYSTEM END ----------------------------- */

	layout->addWidget(filesystemGroup, 6, 2);

	/* ----------------------------- BUTTONS BEGIN ------------------------------ */

	auto* dialogButtons = new QDialogButtonBox{this};
	dialogButtons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	/* ------------------------------ BUTTONS END ------------------------------- */

	layout->addWidget(dialogButtons, 7, 2, Qt::AlignBottom | Qt::AlignRight);

	/* ------------------------------ LOGIC BEGIN ------------------------------- */

	// Disable "Compressed Quality" when format is uncompressed

	connect(textureOpaqueFormatCombo, &QComboBox::currentIndexChanged, this, [textureOpaqueFormatCombo, textureCompressionQualitySpin](int index) {
		const auto format = static_cast<vtfpp::ImageFormat>(textureOpaqueFormatCombo->itemData(index).toInt());
		textureCompressionQualitySpin->setEnabled(format == vtfpp::VTF::FORMAT_UNCHANGED || format == vtfpp::VTF::FORMAT_DEFAULT || vtfpp::ImageFormatDetails::compressed(format));
	});

	// Disable "Filter" and "Scale (Console)" when mipmaps aren't computed, or "Scale (Console)" when platform is not PC

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(textureMipmapsGenerateCheck, &QCheckBox::checkStateChanged, this, [textureMipmapsFilterCombo, textureMipmapsScaleSpin, platformCombo](Qt::CheckState state) {
		textureMipmapsFilterCombo->setDisabled(state != Qt::Checked);
		textureMipmapsScaleSpin->setDisabled(state != Qt::Checked || static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt()) == vtfpp::VTF::PLATFORM_PC);
	});
	textureMipmapsGenerateCheck->checkStateChanged(textureMipmapsGenerateCheck->checkState());
#else
	connect(textureMipmapsGenerateCheck, &QCheckBox::stateChanged, this, [textureMipmapsFilterCombo, textureMipmapsScaleSpin, platformCombo](bool state) {
		textureMipmapsFilterCombo->setDisabled(!state);
		textureMipmapsScaleSpin->setDisabled(!state || static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt()) == vtfpp::VTF::PLATFORM_PC);
	});
	textureMipmapsGenerateCheck->stateChanged(textureMipmapsGenerateCheck->isChecked());
#endif

	// Disable "Bilinear Filter" check when HDRI mode is flat image

	connect(textureHDRIConversionMethodCombo, &QComboBox::currentIndexChanged, this, [textureHDRIFilterCheck](int index) {
		textureHDRIFilterCheck->setDisabled(index == 0);
	});
	textureHDRIConversionMethodCombo->currentIndexChanged(textureHDRIConversionMethodCombo->currentIndex());

	// Disable "Amount" spin when Gamma Correction is disabled

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(textureGammaCorrectionEnableCheck, &QCheckBox::checkStateChanged, this, [textureGammaCorrectionAmountSpin](Qt::CheckState state) {
		textureGammaCorrectionAmountSpin->setDisabled(state != Qt::Checked);
	});
	textureGammaCorrectionEnableCheck->checkStateChanged(textureGammaCorrectionEnableCheck->checkState());
#else
	connect(textureGammaCorrectionEnableCheck, &QCheckBox::stateChanged, this, [textureGammaCorrectionAmountSpin](bool state) {
		textureGammaCorrectionAmountSpin->setDisabled(!state);
	});
	textureGammaCorrectionEnableCheck->stateChanged(textureGammaCorrectionEnableCheck->isChecked());
#endif

	// Disable "Version" combo when platform is not PC, and set it to the appropriate version so the flags list is correct
	// "CPU Compression" group has many restrictions from "Version" group:
	// - Disable group entirely if platform is PC and version is not 7.6, or if platform is XBOX, or if platform is PS3_ORANGEBOX
	// - Disable Deflate and Zstd options in "Method" combo when platform is PS3_PORTAL2 or X360, and disable LZMA (Console) option in "Method" combo when platform is PC
	// Disable "Scale (Console)" in "Mipmaps" group if mipmaps are not enabled or platform is not PC
	// Disable "U (Console)" and "V (Console)" in "LOD" group if platform is PC
	// Disable "Particle Sheet", "CRC32", and "LOD" groups if platform is PC and version is less than 3, or platform is XBOX
	// Repopulate "Flags" group checklist

	connect(platformCombo, &QComboBox::currentIndexChanged, this, [textureMipmapsGenerateCheck, textureMipmapsScaleSpin, textureCompressionGroup, textureCompressionMethodCombo, flagsChecks, platformCombo, versionCombo, resourcesSHTGroup, resourcesCRCGroup, resourcesLODGroup, resourcesLODEnableCheck, resourcesLODUConsoleSpin, resourcesLODVConsoleSpin](int index) {
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

		textureCompressionGroup->setDisabled((currentPlatform != vtfpp::VTF::PLATFORM_PC || versionCombo->currentIndex() != 6) && currentPlatform != vtfpp::VTF::PLATFORM_X360 && currentPlatform != vtfpp::VTF::PLATFORM_PS3_PORTAL2);

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
		textureMipmapsScaleSpin->setDisabled(textureMipmapsGenerateCheck->checkState() != Qt::Checked || currentPlatform == vtfpp::VTF::PLATFORM_PC);
#else
		textureMipmapsScaleSpin->setDisabled(!textureMipmapsGenerateCheck->isChecked() || currentPlatform == vtfpp::VTF::PLATFORM_PC);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
		resourcesLODUConsoleSpin->setDisabled(resourcesLODEnableCheck->checkState() != Qt::Checked || currentPlatform == vtfpp::VTF::PLATFORM_PC);
		resourcesLODVConsoleSpin->setDisabled(resourcesLODEnableCheck->checkState() != Qt::Checked || currentPlatform == vtfpp::VTF::PLATFORM_PC);
#else
		resourcesLODUConsoleSpin->setDisabled(!resourcesLODEnableCheck->isChecked() || currentPlatform == vtfpp::VTF::PLATFORM_PC);
		resourcesLODVConsoleSpin->setDisabled(!resourcesLODEnableCheck->isChecked() || currentPlatform == vtfpp::VTF::PLATFORM_PC);
#endif

		resourcesSHTGroup->setDisabled((currentPlatform == vtfpp::VTF::PLATFORM_PC && versionCombo->currentIndex() < 3) || currentPlatform == vtfpp::VTF::PLATFORM_XBOX);
		resourcesCRCGroup->setDisabled((currentPlatform == vtfpp::VTF::PLATFORM_PC && versionCombo->currentIndex() < 3) || currentPlatform == vtfpp::VTF::PLATFORM_XBOX);
		resourcesLODGroup->setDisabled((currentPlatform == vtfpp::VTF::PLATFORM_PC && versionCombo->currentIndex() < 3) || currentPlatform == vtfpp::VTF::PLATFORM_XBOX);

		flagsChecks->repopulateFlagList(0, currentPlatform, versionCombo->currentIndex());
	});
	platformCombo->currentIndexChanged(platformCombo->currentIndex());

	connect(versionCombo, &QComboBox::currentIndexChanged, this, [textureCompressionGroup, flagsChecks, platformCombo, resourcesSHTGroup, resourcesCRCGroup, resourcesLODGroup](int index) {
		const auto currentPlatform = static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt());

		textureCompressionGroup->setDisabled(index != 6);

		resourcesSHTGroup->setDisabled((currentPlatform == vtfpp::VTF::PLATFORM_PC && index < 3) || currentPlatform == vtfpp::VTF::PLATFORM_XBOX);
		resourcesCRCGroup->setDisabled((currentPlatform == vtfpp::VTF::PLATFORM_PC && index < 3) || currentPlatform == vtfpp::VTF::PLATFORM_XBOX);
		resourcesLODGroup->setDisabled((currentPlatform == vtfpp::VTF::PLATFORM_PC && index < 3) || currentPlatform == vtfpp::VTF::PLATFORM_XBOX);

		flagsChecks->repopulateFlagList(0, currentPlatform, index);
	});
	versionCombo->currentIndexChanged(versionCombo->currentIndex());

	// Disable "Path" and "Search" in "Particle Sheet" group if disabled

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(resourcesSHTEnableCheck, &QCheckBox::checkStateChanged, this, [resourcesSHTPath, resourcesSHTPathSearch](Qt::CheckState state) {
		resourcesSHTPath->setDisabled(state != Qt::Checked);
		resourcesSHTPathSearch->setDisabled(state != Qt::Checked);
	});
	resourcesSHTEnableCheck->checkStateChanged(resourcesSHTEnableCheck->checkState());
#else
	connect(resourcesSHTEnableCheck, &QCheckBox::stateChanged, this, [resourcesSHTPath, resourcesSHTPathSearch](bool state) {
		resourcesSHTPath->setDisabled(!state);
		resourcesSHTPathSearch->setDisabled(!state);
	});
	resourcesSHTEnableCheck->stateChanged(resourcesSHTEnableCheck->isChecked());
#endif

	// Disable "Value" in "CRC32" group if disabled

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(resourcesCRCEnableCheck, &QCheckBox::checkStateChanged, this, [resourcesCRCValue](Qt::CheckState state) {
		resourcesCRCValue->setDisabled(state != Qt::Checked);
	});
	resourcesCRCEnableCheck->checkStateChanged(resourcesCRCEnableCheck->checkState());
#else
	connect(resourcesCRCEnableCheck, &QCheckBox::stateChanged, this, [resourcesCRCValue](bool state) {
		resourcesCRCValue->setDisabled(!state);
	});
	resourcesCRCEnableCheck->stateChanged(resourcesCRCEnableCheck->isChecked());
#endif

	// Disable "U", "V", "U (Console)", "V (Console)" in "LOD" group if disabled
	// - Also disable "U (Console)" and "V (Console)" if platform is PC

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(resourcesLODEnableCheck, &QCheckBox::checkStateChanged, this, [platformCombo, resourcesLODUSpin, resourcesLODVSpin, resourcesLODUConsoleSpin, resourcesLODVConsoleSpin](Qt::CheckState state) {
		const auto currentPlatform = static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt());

		resourcesLODUSpin->setDisabled(state != Qt::Checked);
		resourcesLODVSpin->setDisabled(state != Qt::Checked);
		resourcesLODUConsoleSpin->setDisabled(state != Qt::Checked || currentPlatform == vtfpp::VTF::PLATFORM_PC);
		resourcesLODVConsoleSpin->setDisabled(state != Qt::Checked || currentPlatform == vtfpp::VTF::PLATFORM_PC);
	});
	resourcesLODEnableCheck->checkStateChanged(resourcesLODEnableCheck->checkState());
#else
	connect(resourcesLODEnableCheck, &QCheckBox::stateChanged, this, [platformCombo, resourcesLODUSpin, resourcesLODVSpin, resourcesLODUConsoleSpin, resourcesLODVConsoleSpin](bool state) {
		const auto currentPlatform = static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt());

		resourcesLODUSpin->setDisabled(!state);
		resourcesLODVSpin->setDisabled(!state);
		resourcesLODUConsoleSpin->setDisabled(!state || currentPlatform == vtfpp::VTF::PLATFORM_PC);
		resourcesLODVConsoleSpin->setDisabled(!state || currentPlatform == vtfpp::VTF::PLATFORM_PC);
	});
	resourcesLODEnableCheck->stateChanged(resourcesLODEnableCheck->isChecked());
#endif

	// On "OK", create the texture
	// On "Cancel", close the dialog

	connect(dialogButtons, &QDialogButtonBox::accepted, this, [=, this] {
		const auto inputPath = !createFromDir
			? QFileDialog::getOpenFileName(this, tr("Open Image"), QString{}, ::supportedImageFileFormatsForLoad().data())
			: QFileDialog::getExistingDirectory(this, tr("Open Folder"));
		if (inputPath.isEmpty()) {
			return;
		}

		std::vector<std::string> arguments{"maretf", "create", inputPath.toUtf8().constData()};

		const auto addArg = [&arguments](auto&& arg1, auto&& arg2) {
			arguments.emplace_back(std::forward<decltype(arg1)>(arg1));
			arguments.emplace_back(std::forward<decltype(arg2)>(arg2));
		};

		const auto addFlag = [&arguments](const QCheckBox* checkBox, auto&& arg, bool negated = false) {
			if ((!negated && checkBox->isChecked()) || (negated && !checkBox->isChecked())) {
				arguments.emplace_back(std::forward<decltype(arg)>(arg));
			}
		};

		const auto addInt = [&addArg](const QSpinBox* spinBox, auto&& arg) {
			addArg(std::forward<decltype(arg)>(arg), std::format("{}", spinBox->value()));
		};

		const auto addFloat = [&addArg](const QDoubleSpinBox* spinBox, auto&& arg) {
			addArg(std::forward<decltype(arg)>(arg), std::format("{}", spinBox->value()));
		};

		const auto applyForEnum = [&addArg]<typename E>(const QComboBox* combo, std::string_view name) {
			const auto e = static_cast<E>(combo->itemData(combo->currentIndex()).toInt());
			addArg(name, not_magic_enum::enum_name(e));
		};

		applyForEnum.operator()<vtfpp::VTF::Platform>(platformCombo, "--platform");
		addArg("--version", std::format("7.{}", versionCombo->itemData(versionCombo->currentIndex()).toInt()));
		applyForEnum.operator()<vtfpp::ImageFormat>(textureOpaqueFormatCombo, "--format");
		addArg("--quality", std::format("{}", textureCompressionQualitySpin->value()));
		applyForEnum.operator()<vtfpp::ImageConversion::ResizeMethod>(textureResizeMethodWidthCombo, "--width-resize-method");
		applyForEnum.operator()<vtfpp::ImageConversion::ResizeMethod>(textureResizeMethodHeightCombo, "--height-resize-method");
		addFlag(textureMipmapsGenerateCheck, "--no-mips", true);
		applyForEnum.operator()<vtfpp::ImageConversion::ResizeFilter>(textureMipmapsFilterCombo, "--filter");
		addInt(textureMipmapsScaleSpin, "--console-mip-scale");

		std::unique_ptr<const char*[]> cArgs{new const char*[arguments.size()]};
		for (int i = 0; i < arguments.size(); i++) {
			cArgs[i] = arguments[i].c_str();
		}
		if (maretf_cli(static_cast<int>(arguments.size()), cArgs.get())) {
			QMessageBox::warning(this, tr("Error"), tr("Failed to create texture."));
		}
		emit this->createdTexture(::getOutputPathForInput(arguments[2], static_cast<vtfpp::VTF::Platform>(platformCombo->itemData(platformCombo->currentIndex()).toInt())).c_str());
		emit this->accept();
	});

	connect(dialogButtons, &QDialogButtonBox::rejected, this, &QMareCreateTextureDialog::reject);
}
