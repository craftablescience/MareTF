#pragma once

#include <QDialog>

#include "widgets/QMareTextureWidget.h"

class QMareSpritesheetDialog : public QDialog {
	Q_OBJECT;

public:
	explicit QMareSpritesheetDialog(QWidget* parent = nullptr);

	static void showSpritesheetEditor(QWidget* parent = nullptr, QMareTextureWidget* currentTexture = nullptr);
};
