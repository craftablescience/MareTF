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
	extractImageDataLayout->addRow(tr("Pre-extraction Image Format Conversion"), extractImageFormatCombo);

	// Alpha
	auto* extractAlphaCheck = new QCheckBox{extractImageDataGroup};
	extractImageDataLayout->addRow(tr("Alpha to Separate Image"), extractAlphaCheck);

	/* ----------------------------- IMAGE DATA END ----------------------------- */

	extractTabLayout->addRow(extractImageDataCheck, extractImageDataGroup);

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

			// todo: bind these up
			// --extract-mip
			// --extract-all-mips
			// --extract-frame
			// --extract-all-frames
			// --extract-face
			// --extract-all-faces
			// --extract-slice
			// --extract-all-slices
			// --extract-all-images
			// --extract-thumbnail
			// --extract-particle-sheet-resource
			// --extract-kvd-resource
			// --extract-ath-resource
			// --extract-hotspot-data-resource
			// --extract-all-resources

		} else {
			cli->addArg("--extract-skip-image");
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
