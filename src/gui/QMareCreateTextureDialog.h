#pragma once

#include <QDialog>
#include <vtfpp/VTF.h>

class QListWidget;

class QMareCreateTextureDialog : public QDialog {
	Q_OBJECT;

public:
	explicit QMareCreateTextureDialog(bool createFromDir, QWidget* parent = nullptr);

signals:
	void createdTexture(const QString& path);
};
