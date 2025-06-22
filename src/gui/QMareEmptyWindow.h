#pragma once

#include <QMainWindow>

class QPaintEvent;
class QPushButton;
class QResizeEvent;

class QMareEmptyWindow : public QMainWindow {
	Q_OBJECT;

public:
	QMareEmptyWindow();

protected:
	void paintEvent(QPaintEvent*) override;

	void resizeEvent(QResizeEvent*) override;

	QPushButton* newTexture;
	QPushButton* newTextures;
	QPushButton* loadTexture;
};
