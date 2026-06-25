#include "QMareFilesystemBox.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QStyle>

#include "utility/QMareCLIWrapper.h"

QMareFilesystemBox::QMareFilesystemBox(const QStringList& inputPaths, bool createFromDir, bool allowsWatching, QWidget* parent) : QGroupBox{tr("Filesystem"), parent} {
	auto* layout = new QFormLayout{this};
	layout->setFormAlignment(Qt::AlignHCenter);

	/* ------------------------------- Input Path ------------------------------- */

	auto* filesystemInputPathParent = new QWidget{this};
	auto* filesystemInputPathLayout = new QHBoxLayout{filesystemInputPathParent};
	filesystemInputPathLayout->setSpacing(4);
	filesystemInputPathLayout->setContentsMargins(0, 0, 0, 0);

	this->filesystemInputPath = new QLineEdit{this};
	this->filesystemInputPath->setText(QMareCLIWrapper::joinPaths(inputPaths));
	this->filesystemInputPath->setMinimumWidth(200);
	this->filesystemInputPath->setReadOnly(true);
	filesystemInputPathLayout->addWidget(this->filesystemInputPath);

	this->filesystemInputPathSearch = new QPushButton{this};
	this->filesystemInputPathSearch->setIcon(this->style()->standardIcon(QStyle::SP_DirOpenIcon));
	filesystemInputPathLayout->addWidget(this->filesystemInputPathSearch);

	layout->addRow(tr("Input Path"), filesystemInputPathParent);

	/* ------------------------------  Output Path ------------------------------ */

	auto* filesystemOutputPathParent = new QWidget{this};
	auto* filesystemOutputPathLayout = new QHBoxLayout{filesystemOutputPathParent};
	filesystemOutputPathLayout->setSpacing(4);
	filesystemOutputPathLayout->setContentsMargins(0, 0, 0, 0);

	this->filesystemOutputPath = new QLineEdit{this};
	this->filesystemOutputPath->setPlaceholderText(tr("Leave empty for default"));
	this->filesystemOutputPath->setMinimumWidth(200);
	this->filesystemOutputPath->setReadOnly(true);
	filesystemOutputPathLayout->addWidget(this->filesystemOutputPath);

	this->filesystemOutputPathSearch = new QPushButton{this};
	this->filesystemOutputPathSearch->setIcon(this->style()->standardIcon(QStyle::SP_DirOpenIcon));
	filesystemOutputPathLayout->addWidget(this->filesystemOutputPathSearch);

	layout->addRow(tr("Output Path"), filesystemOutputPathParent);

	/* ------------------------------- Overwrite -------------------------------- */

	auto* overwriteGroup = new QGroupBox{this};
	auto* overwriteLayout = new QHBoxLayout{overwriteGroup};
	overwriteLayout->setSpacing(24);
	overwriteLayout->setAlignment(Qt::AlignHCenter);

	auto* overwriteRadioGroup = new QButtonGroup{overwriteGroup};
	overwriteRadioGroup->setExclusive(true);

	this->overwriteRadioYes = new QRadioButton{tr("Yes"), overwriteGroup};
	overwriteRadioGroup->addButton(this->overwriteRadioYes);
	overwriteLayout->addWidget(this->overwriteRadioYes);

	this->overwriteRadioAsk = new QRadioButton{tr("Ask"), overwriteGroup};
	overwriteRadioGroup->addButton(this->overwriteRadioAsk);
	overwriteLayout->addWidget(this->overwriteRadioAsk);

	this->overwriteRadioNo = new QRadioButton{tr("No"), overwriteGroup};
	overwriteRadioGroup->addButton(this->overwriteRadioNo);
	overwriteLayout->addWidget(this->overwriteRadioNo);

	this->overwriteRadioAsk->setChecked(true);

	layout->addRow(tr("Overwrite"), overwriteGroup);

	/* ----------------------------- Recurse/Watch ------------------------------ */

	if (createFromDir) {
		this->recurseIntoSubdirsCheck = new QCheckBox{this};
		this->recurseIntoSubdirsCheck->setChecked(true);
		layout->addRow(tr("Enter Subfolders"), this->recurseIntoSubdirsCheck);
	} else {
		this->recurseIntoSubdirsCheck = nullptr;
	}

	if (allowsWatching) {
		this->watchFilesCheck = new QCheckBox{this};
		layout->addRow(tr("Watch For Changes"), this->watchFilesCheck);
	} else {
		this->watchFilesCheck = nullptr;
	}
}

void QMareFilesystemBox::addArguments(QMareCLIWrapper& cli) const {
	if (
		const auto filesystemInputPathSplit = QMareCLIWrapper::splitPaths(this->filesystemInputPath->text());
		filesystemInputPathSplit.size() == 1
	) {
		cli.addArg(filesystemInputPathSplit[0]);
	} else {
		cli.addArg("--input");
		for (const auto& path : filesystemInputPathSplit) {
			cli.addArg(path);
		}
	}

	if (!this->filesystemOutputPath->text().isEmpty()) {
		cli.addArgPair("--output", this->filesystemOutputPath->text());
	}

	cli.addFlag(this->overwriteRadioYes, "--yes");

	cli.addFlag(this->overwriteRadioNo, "--no");

	if (this->recurseIntoSubdirsCheck) {
		cli.addFlag(this->recurseIntoSubdirsCheck, "--no-recurse", true);
	}

	if (this->watchFilesCheck) {
		cli.addFlag(this->watchFilesCheck, "--watch");
	}
}
