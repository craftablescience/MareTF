#pragma once

#include <QDialog>
#include <vtfpp/VTF.h>

class QListWidget;

class QMareCreateTexture : public QDialog {
	Q_OBJECT;

public:
	explicit QMareCreateTexture(bool createFromDir, QWidget* parent = nullptr);

signals:
	void createdTexture(const QString& path);
};
