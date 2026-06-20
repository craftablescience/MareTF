#include "QMareExtractFromTextureDialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollArea>
#include <QTimer>

#include "Common.h"

#include "utility/QMareCLIWrapper.h"
#include "utility/QMareOptions.h"
#include "widgets/QMareFilesystemBox.h"

QMareExtractFromTextureDialog::QMareExtractFromTextureDialog(const QStringList& inputPaths, bool createFromDir, QWidget* parent) : QDialog{parent} {
	this->setWindowTitle(createFromDir || inputPaths.size() > 1 ? tr("Extract From Textures") : tr("Extract From Texture"));
	this->setMinimumWidth(500);

	auto* layout = new QVBoxLayout{this};

	auto* tabs = new QTabWidget{this};
	layout->addWidget(tabs);

	auto* extractTabScroll = new QScrollArea{tabs};
	auto* extractTab = new QWidget{extractTabScroll};
	extractTabScroll->setWidgetResizable(true);
	extractTabScroll->setWidget(extractTab);
	auto* extractTabLayout = new QFormLayout{extractTab};
	extractTabLayout->setFormAlignment(Qt::AlignHCenter);
	tabs->addTab(extractTabScroll, tr("Extract"));

	/* ---------------------------- IMAGE DATA BEGIN ---------------------------- */

	auto* extractImageDataCheck = new QCheckBox{tr("Image Data"), extractTab};
	extractImageDataCheck->setLayoutDirection(Qt::RightToLeft);
	extractImageDataCheck->setChecked(true);

	auto* extractImageDataGroup = new QGroupBox{extractTab};
	auto* extractImageDataLayout = new QFormLayout{extractImageDataGroup};
	extractImageDataLayout->setFormAlignment(Qt::AlignHCenter);

	// File format
	auto* extractFileFormatCombo = new QMareComboBox{extractImageDataGroup};
	for (const auto& [format, formatName] : not_magic_enum::enum_entries<vtfpp::ImageConversion::FileFormat>(true)) {
		if (formatName == "JPG") {
			continue; // duplicate of JPEG
		}
		extractFileFormatCombo->addItem(formatName.data(), static_cast<int>(format));
	}
	extractFileFormatCombo->setCurrentIndex(0); // DEFAULT
	extractImageDataLayout->addRow(tr("File Format"), extractFileFormatCombo);

	// Image format
	auto* extractImageFormatCombo = new QMareComboBox{extractImageDataGroup};
	for (const auto& [format, formatName] : not_magic_enum::enum_entries<vtfpp::ImageFormat>(true)) {
		if (format == vtfpp::VTF::FORMAT_DEFAULT) {
			continue; // ignored
		}
		extractImageFormatCombo->addItem(formatName.data(), static_cast<int>(format));
	}
	extractFileFormatCombo->setCurrentIndex(0); // UNCHANGED
	extractImageDataLayout->addRow(tr("Pre-extraction Conversion"), extractImageFormatCombo);

	// Alpha
	auto* extractAlphaCheck = new QCheckBox{extractImageDataGroup};
	extractImageDataLayout->addRow(tr("Alpha to Separate Image"), extractAlphaCheck);

	// Images
	auto* extractImageParticularGroup = new QGroupBox{extractImageDataGroup};
	auto* extractImageParticularLayout = new QFormLayout{extractImageParticularGroup};
	extractImageParticularLayout->setFormAlignment(Qt::AlignHCenter);

	auto* extractAllImagesCheck = new QCheckBox{extractImageParticularGroup};
	extractImageParticularLayout->addRow(tr("Every Image"), extractAllImagesCheck);

	auto* extractAllMipsCheck = new QCheckBox{extractImageParticularGroup};
	extractImageParticularLayout->addRow(tr("Every Mip"), extractAllMipsCheck);

	auto* extractAllFramesCheck = new QCheckBox{extractImageParticularGroup};
	extractImageParticularLayout->addRow(tr("Every Frame"), extractAllFramesCheck);

	auto* extractAllFacesCheck = new QCheckBox{extractImageParticularGroup};
	extractImageParticularLayout->addRow(tr("Every Face"), extractAllFacesCheck);

	auto* extractAllSlicesCheck = new QCheckBox{extractImageParticularGroup};
	extractImageParticularLayout->addRow(tr("Every Slice"), extractAllSlicesCheck);

	extractImageDataLayout->addRow("Images", extractImageParticularGroup);

	/* ----------------------------- IMAGE DATA END ----------------------------- */

	extractTabLayout->addRow(extractImageDataCheck, extractImageDataGroup);

	/* --------------------------- RESOURCE DATA BEGIN -------------------------- */

	auto* extractResourcesCheck = new QCheckBox{tr("Resources"), extractTab};
	extractResourcesCheck->setLayoutDirection(Qt::RightToLeft);

	auto* extractResourcesGroup = new QGroupBox{extractTab};
	auto* extractResourcesLayout = new QFormLayout{extractResourcesGroup};
	extractResourcesLayout->setFormAlignment(Qt::AlignHCenter);

	// All
	auto* extractAllResourcesCheck = new QCheckBox{extractResourcesGroup};
	extractResourcesLayout->addRow(tr("Every Resource"), extractAllResourcesCheck);

	// Thumbnail
	auto* extractThumbnailCheck = new QCheckBox{extractResourcesGroup};
	extractResourcesLayout->addRow(tr("Thumbnail"), extractThumbnailCheck);

	// Particle sheet
	auto* extractParticleSheetCheck = new QCheckBox{extractResourcesGroup};
	extractResourcesLayout->addRow(tr("Particle Sheet"), extractParticleSheetCheck);

	// KeyValues data
	auto* extractKVDCheck = new QCheckBox{extractResourcesGroup};
	extractResourcesLayout->addRow(tr("KeyValues Data"), extractKVDCheck);

	// Author info
	auto* extractATHCheck = new QCheckBox{extractResourcesGroup};
	extractResourcesLayout->addRow(tr("Author Info"), extractATHCheck);

	// Hotspot data
	auto* extractHotspotDataCheck = new QCheckBox{extractResourcesGroup};
	extractResourcesLayout->addRow(tr("Hotspot Data"), extractHotspotDataCheck);

	/* ---------------------------- RESOURCE DATA END --------------------------- */

	extractTabLayout->addRow(extractResourcesCheck, extractResourcesGroup);

	/* ---------------------------- FILESYSTEM BEGIN ---------------------------- */

	auto* filesystemGroup = new QMareFilesystemBox{inputPaths, createFromDir, false, this};

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

	// Disable/enable Image Data group when toggled

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(extractImageDataCheck, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state) {
		extractImageDataGroup->setDisabled(state != Qt::Checked);
	});
	extractImageDataCheck->checkStateChanged(extractImageDataCheck->checkState());
#else
	connect(extractImageDataCheck, &QCheckBox::stateChanged, this, [=](bool state) {
		extractImageDataGroup->setDisabled(!state);
	});
	extractImageDataCheck->stateChanged(extractImageDataCheck->isChecked());
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(extractAllImagesCheck, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state) {
		extractImageParticularLayout->setRowVisible(extractAllMipsCheck,   state != Qt::Checked);
		extractImageParticularLayout->setRowVisible(extractAllFramesCheck, state != Qt::Checked);
		extractImageParticularLayout->setRowVisible(extractAllFacesCheck,  state != Qt::Checked);
		extractImageParticularLayout->setRowVisible(extractAllSlicesCheck, state != Qt::Checked);
	});
	extractAllImagesCheck->checkStateChanged(extractAllImagesCheck->checkState());
#else
	connect(extractAllImagesCheck, &QCheckBox::stateChanged, this, [=](bool state) {
		extractImageParticularLayout->setRowVisible(extractAllMipsCheck,   !state);
		extractImageParticularLayout->setRowVisible(extractAllFramesCheck, !state);
		extractImageParticularLayout->setRowVisible(extractAllFacesCheck,  !state);
		extractImageParticularLayout->setRowVisible(extractAllSlicesCheck, !state);
	});
	extractAllImagesCheck->stateChanged(extractAllImagesCheck->isChecked());
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(extractResourcesCheck, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state) {
		extractResourcesGroup->setDisabled(state != Qt::Checked);
	});
	extractResourcesCheck->checkStateChanged(extractResourcesCheck->checkState());
#else
	connect(extractResourcesCheck, &QCheckBox::stateChanged, this, [=](bool state) {
		extractResourcesGroup->setDisabled(!state);
	});
	extractResourcesCheck->stateChanged(extractResourcesCheck->isChecked());
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(extractAllResourcesCheck, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState state) {
		extractResourcesLayout->setRowVisible(extractThumbnailCheck,     state != Qt::Checked);
		extractResourcesLayout->setRowVisible(extractParticleSheetCheck, state != Qt::Checked);
		extractResourcesLayout->setRowVisible(extractKVDCheck,           state != Qt::Checked);
		extractResourcesLayout->setRowVisible(extractATHCheck,           state != Qt::Checked);
		extractResourcesLayout->setRowVisible(extractATHCheck,           state != Qt::Checked);
		extractResourcesLayout->setRowVisible(extractHotspotDataCheck,   state != Qt::Checked);
	});
#else
	connect(extractAllResourcesCheck, &QCheckBox::stateChanged, this, [=](bool state) {
		extractResourcesLayout->setRowVisible(extractThumbnailCheck,     !state);
		extractResourcesLayout->setRowVisible(extractParticleSheetCheck, !state);
		extractResourcesLayout->setRowVisible(extractKVDCheck,           !state);
		extractResourcesLayout->setRowVisible(extractATHCheck,           !state);
		extractResourcesLayout->setRowVisible(extractATHCheck,           !state);
		extractResourcesLayout->setRowVisible(extractHotspotDataCheck,   !state);
	});
#endif

	// Set input path in "Filesystem" group when "Input Search" clicked

	connect(filesystemGroup->filesystemInputPathSearch, &QPushButton::pressed, this, [=, this] {
		if (
			const auto paths = !createFromDir
				? QFileDialog::getOpenFileNames(this, tr("Open Textures"), QString{}, "Valve Texture Format (*.vtf *.xtf)")
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
				? QFileDialog::getSaveFileName(this, tr("Save Image"), QFileInfo{filesystemGroup->filesystemInputPath->text()}.canonicalFilePath(), ::supportedImageFileFormatsForSave().data(), nullptr, QFileDialog::DontConfirmOverwrite)
				: QFileDialog::getExistingDirectory(this, tr("Save to Folder"));
			!path.isEmpty()
		) {
			filesystemGroup->filesystemOutputPath->setText(path);
		}
	});

	// Get list of arguments from the selected options

	const auto getCLI = [=, this] {
		auto* cli = new QMareCLIWrapper{"extract", this};

		filesystemGroup->addArguments(*cli);

		if (extractImageDataCheck->isChecked()) {
			cli->addEnum<vtfpp::ImageConversion::FileFormat>(extractFileFormatCombo, "--extract-file-format");

			cli->addEnum<vtfpp::ImageFormat>(extractImageFormatCombo, "--extract-image-format");

			cli->addFlag(extractAlphaCheck, "--extract-alpha-channel");

			if (extractAllImagesCheck->isChecked()) {
				cli->addArg("--extract-all-images");
			} else {
				cli->addFlag(extractAllMipsCheck, "--extract-all-mips");

				cli->addFlag(extractAllFramesCheck, "--extract-all-frames");

				cli->addFlag(extractAllFacesCheck, "--extract-all-faces");

				cli->addFlag(extractAllSlicesCheck, "--extract-all-slices");
			}
		} else {
			cli->addArg("--extract-skip-image");
		}

		if (extractResourcesCheck->isChecked()) {
			if (extractAllResourcesCheck->isChecked()) {
				cli->addArg("--extract-all-resources");
			} else {
				cli->addFlag(extractThumbnailCheck, "--extract-thumbnail");

				cli->addFlag(extractParticleSheetCheck, "--extract-particle-sheet-resource");

				cli->addFlag(extractKVDCheck, "--extract-kvd-resource");

				cli->addFlag(extractATHCheck, "--extract-ath-resource");

				cli->addFlag(extractHotspotDataCheck, "--extract-hotspot-data-resource");
			}
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

	// On "OK", perform extraction
	// On "Cancel", close the dialog

	connect(dialogButtons, &QDialogButtonBox::accepted, this, [=, this] {
		auto* cli = getCLI();
		if (const auto& [code, errMsg] = cli->exec(); code) {
			QMessageBox::warning(this, tr("Error Extracting From Texture"), errMsg.c_str());
			cli->deleteLater();
			return;
		}
		cli->deleteLater();
		emit this->accept();
	});

	connect(dialogButtons, &QDialogButtonBox::rejected, this, &QMareExtractFromTextureDialog::reject);
}

QMareExtractFromTextureDialog* QMareExtractFromTextureDialog::fromTextures(QWidget* parent, QStringList texturePaths) {
	if (texturePaths.isEmpty()) {
		texturePaths = QFileDialog::getOpenFileNames(parent, tr("Open Textures"), QMareOptions::get<QString>(QMareOptions::STR_DEFAULT_EXTRACT_DIALOG_DIR), "Valve Texture Format (*.vtf *.xtf)");
		if (texturePaths.isEmpty()) {
			return nullptr;
		}
	}
	QMareOptions::set(QMareOptions::STR_DEFAULT_EXTRACT_DIALOG_DIR, QFileInfo{texturePaths.last()}.canonicalPath());
	return new QMareExtractFromTextureDialog{texturePaths, false, parent};
}

QMareExtractFromTextureDialog* QMareExtractFromTextureDialog::fromDir(QWidget* parent, QString dirPath) {
	if (dirPath.isEmpty()) {
		dirPath = QFileDialog::getExistingDirectory(parent, tr("Open Folder"), QMareOptions::get<QString>(QMareOptions::STR_DEFAULT_EXTRACT_DIALOG_DIR));
		if (dirPath.isEmpty()) {
			return nullptr;
		}
	}
	QMareOptions::set(QMareOptions::STR_DEFAULT_EXTRACT_DIALOG_DIR, QFileInfo{dirPath}.canonicalPath());
	return new QMareExtractFromTextureDialog{{dirPath}, true, parent};
}
