#pragma once

#include <QDialog>

class QListWidget;

class QMareCreateTextureDialog : public QDialog {
	Q_OBJECT;

protected:
	explicit QMareCreateTextureDialog(const QStringList& inputPaths, bool createFromDir, QWidget* parent = nullptr);

public:
	[[nodiscard]] static QMareCreateTextureDialog* fromImages(QWidget* parent = nullptr);

	[[nodiscard]] static QMareCreateTextureDialog* fromDir(QWidget* parent = nullptr);

signals:
	void createdTextures(const QStringList& paths);
};
