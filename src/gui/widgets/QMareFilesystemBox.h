#pragma once

#include <QGroupBox>

class QCheckBox;
class QLineEdit;
class QPushButton;
class QRadioButton;

class QMareCLIWrapper;

class QMareFilesystemBox : public QGroupBox {
	Q_OBJECT;

public:
	QMareFilesystemBox(const QStringList& inputPaths, bool createFromDir, bool allowsWatching, QWidget* parent = nullptr);

	void addArguments(QMareCLIWrapper& cli) const;

	QLineEdit* filesystemInputPath;
	QPushButton* filesystemInputPathSearch;
	QLineEdit* filesystemOutputPath;
	QPushButton* filesystemOutputPathSearch;
	QRadioButton* overwriteRadioYes;
	QRadioButton* overwriteRadioAsk;
	QRadioButton* overwriteRadioNo;
	QCheckBox* recurseIntoSubdirsCheck;
	QCheckBox* watchFilesCheck;
};
