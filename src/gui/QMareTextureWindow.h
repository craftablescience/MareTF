#pragma once

#include <QMainWindow>

class QListWidget;
class QTabWidget;

class QMareTextureWindow : public QMainWindow {
	Q_OBJECT;

public:
	QMareTextureWindow();

	void loadTexture(const QString& path) const;

	void regenerateDetails();

protected:
	QTabWidget* textureTabs;
	QListWidget* flagsChecks;
};
