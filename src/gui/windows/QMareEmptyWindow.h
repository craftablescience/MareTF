#pragma once

#include <QMainWindow>

class QDragEnterEvent;
class QDropEvent;
class QPaintEvent;
class QToolBar;

class QMareTextureWindow;

class QMareEmptyWindow : public QMainWindow {
	Q_OBJECT;

public:
	explicit QMareEmptyWindow(QMareTextureWindow*& textureWindow_, QWidget* parent = nullptr);

signals:
	void showManeWindow(const QStringList& files);

protected:
	void paintEvent(QPaintEvent*) override;

	void dragEnterEvent(QDragEnterEvent* event) override;

	void dropEvent(QDropEvent* event) override;

	QToolBar* toolbar;
	QMareTextureWindow*& textureWindow;
};
