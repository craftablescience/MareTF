#pragma once

#include <QMainWindow>

class QDragEnterEvent;
class QDropEvent;
class QPaintEvent;
class QToolBar;

class QMareEmptyWindow : public QMainWindow {
	Q_OBJECT;

public:
	QMareEmptyWindow();

protected:
	void paintEvent(QPaintEvent*) override;

	void dragEnterEvent(QDragEnterEvent* event) override;

	void dropEvent(QDropEvent* event) override;

	QToolBar* toolbar;
};
