#pragma once

#include <QDialog>

class QListWidget;

class QMareExtractFromTextureDialog : public QDialog {
	Q_OBJECT;

protected:
	explicit QMareExtractFromTextureDialog(const QStringList& inputPaths, bool createFromDir, QWidget* parent = nullptr);

public:
	[[nodiscard]] static QMareExtractFromTextureDialog* fromTextures(QWidget* parent = nullptr, QStringList texturePaths = {});

	[[nodiscard]] static QMareExtractFromTextureDialog* fromDir(QWidget* parent = nullptr, QString dirPath = {});
};
